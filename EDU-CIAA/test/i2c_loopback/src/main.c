#include "sapi.h"
#include "chip.h"
#include <stdio.h>
#include <string.h>

#define TARGET_ADDRESS 0x42u
#define FRAME_SIZE 4
#define RX_CAPACITY 8
#define TIMEOUT_MS 100u

static I2C_XFER_T slaveTransfer;
static I2C_XFER_T masterTransfer;
static uint8_t slaveRx[RX_CAPACITY];
static uint8_t slaveTx[FRAME_SIZE];
static volatile uint8_t receivedFrame[FRAME_SIZE];
static volatile unsigned int receivedCount;
static volatile bool_t timedOut;

/* LPCOpen avanza los punteros durante las transferencias: rearmarlos al terminar. */
static void slaveEvent(I2C_ID_T id, I2C_EVENT_T event)
{
    (void) id;
    if (event == I2C_EVENT_DONE) {
        unsigned int count = RX_CAPACITY - slaveTransfer.rxSz;
        if (count != 0u) {
            receivedCount = count;
            for (unsigned int i = 0; i < FRAME_SIZE; ++i) {
                receivedFrame[i] = slaveRx[i];
                slaveTx[i] = slaveRx[i] ^ 0xFFu;
            }
        }
        slaveTransfer.rxBuff = slaveRx;
        slaveTransfer.rxSz = RX_CAPACITY;
        slaveTransfer.txBuff = slaveTx;
        slaveTransfer.txSz = FRAME_SIZE;
    }
}

void I2C1_IRQHandler(void)
{
    if (Chip_I2C_IsStateChanged(I2C1)) {
        Chip_I2C_SlaveStateHandler(I2C1);
    }
}

/* El maestro avanza por polling; el esclavo responde por interrupcion. */
static void masterEvent(I2C_ID_T id, I2C_EVENT_T event)
{
    if (event != I2C_EVENT_WAIT) {
        return;
    }
    tick_t started = tickRead();
    while (masterTransfer.status == I2C_STATUS_BUSY ||
           (LPC_I2C0->CONSET & I2C_I2CONSET_STO) != 0u) {
        if (masterTransfer.status == I2C_STATUS_BUSY &&
            Chip_I2C_IsStateChanged(id)) {
            Chip_I2C_MasterStateHandler(id);
        }
        if ((tick_t) (tickRead() - started) >= TIMEOUT_MS) {
            timedOut = TRUE;
            NVIC_DisableIRQ(I2C1_IRQn);
            /* Reset elimina tambien un STOP pendiente: evita la espera interna sin limite. */
            Chip_RGU_TriggerReset(RGU_I2C0_RST);
            Chip_RGU_TriggerReset(RGU_I2C1_RST);
            masterTransfer.status = I2C_STATUS_BUSERR;
            return;
        }
    }
}

static void setupBus(void)
{
    const uint16_t mode = SCU_MODE_INACT | SCU_MODE_INBUFF_EN |
                          SCU_MODE_ZIF_DIS | SCU_MODE_FUNC1;
    Chip_SCU_I2C0PinConfig(I2C0_STANDARD_FAST_MODE);
    Chip_SCU_PinMuxSet(2, 3, mode);
    Chip_SCU_PinMuxSet(2, 4, mode);
    Chip_I2C_Init(I2C0);
    Chip_I2C_Init(I2C1);
    Chip_I2C_SetClockRate(I2C0, 100000);
    Chip_I2C_SetMasterEventHandler(I2C0, masterEvent);

    /* Esta API de esclavo espera la direccion en bits 7:1, a diferencia del maestro. */
    slaveTransfer.slaveAddr = TARGET_ADDRESS << 1;
    slaveTransfer.rxBuff = slaveRx;
    slaveTransfer.rxSz = RX_CAPACITY;
    slaveTransfer.txBuff = slaveTx;
    slaveTransfer.txSz = FRAME_SIZE;
    Chip_I2C_SlaveSetup(I2C1, I2C_SLAVE_0, &slaveTransfer, slaveEvent, 0);
    NVIC_ClearPendingIRQ(I2C1_IRQn);
    NVIC_EnableIRQ(I2C1_IRQn);
}

static int transfer(const uint8_t *tx, int txSize, uint8_t *rx, int rxSize)
{
    memset(&masterTransfer, 0, sizeof(masterTransfer));
    masterTransfer.slaveAddr = TARGET_ADDRESS;
    masterTransfer.txBuff = tx;
    masterTransfer.txSz = txSize;
    masterTransfer.rxBuff = rx;
    masterTransfer.rxSz = rxSize;
    return Chip_I2C_MasterTransfer(I2C0, &masterTransfer);
}

static const char *statusName(int status)
{
    switch (status) {
    case I2C_STATUS_DONE: return "DONE";
    case I2C_STATUS_NAK: return "NAK de datos";
    case I2C_STATUS_SLAVENAK: return "NAK de direccion";
    case I2C_STATUS_ARBLOST: return "arbitraje perdido";
    case I2C_STATUS_BUSERR: return "error de bus";
    default: return "estado inesperado";
    }
}

int main(void)
{
    boardInit();
    uartConfig(UART_USB, 115200);
    setupBus();
    uint8_t sequence = 0;
    unsigned long passed = 0;
    unsigned long failed = 0;

    while (TRUE) {
        uint8_t tx[FRAME_SIZE] = {0x12, 0x34, sequence++, 0xA5};
        uint8_t rx[FRAME_SIZE] = {0};
        receivedCount = 0;
        int writeStatus = transfer(tx, FRAME_SIZE, NULL, 0);
        delay(1); /* Permite atender el STOP del esclavo antes de verificar o leer. */
        bool_t writeOk = writeStatus == I2C_STATUS_DONE && receivedCount == FRAME_SIZE;
        for (unsigned int i = 0; i < FRAME_SIZE; ++i) {
            writeOk = writeOk && receivedFrame[i] == tx[i];
        }
        int readStatus = -1;
        bool_t readOk = FALSE;
        if (writeOk && !timedOut) {
            readStatus = transfer(NULL, 0, rx, FRAME_SIZE);
            readOk = readStatus == I2C_STATUS_DONE;
            for (unsigned int i = 0; i < FRAME_SIZE; ++i) {
                readOk = readOk && rx[i] == (uint8_t) (tx[i] ^ 0xFFu);
            }
        }
        bool_t ok = writeOk && readOk;
        if (ok) { ++passed; } else { ++failed; }
        printf("\r\nI2C0 maestro -> I2C1 esclavo 0x42, 100 kHz\r\n");
        printf("TX: %02X %02X %02X %02X; escritura: %s; recibidos: %u\r\n",
               tx[0], tx[1], tx[2], tx[3], statusName(writeStatus), receivedCount);
        printf("RX: %02X %02X %02X %02X; lectura: %s\r\n",
               rx[0], rx[1], rx[2], rx[3], readStatus < 0 ? "omitida" : statusName(readStatus));
        printf("Resultado: %s; correctas=%lu; fallidas=%lu\r\n", ok ? "PASS" : "FAIL", passed, failed);
        gpioWrite(LEDG, ok);
        gpioWrite(LEDR, !ok);
        gpioToggle(LEDB);
        if (timedOut) {
            printf("TIMEOUT de 100 ms. Apagar, revisar conexion y volver a encender.\r\n");
            while (TRUE) { delay(1000); }
        }
        delay(2000);
    }
}
