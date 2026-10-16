#include "sapi.h"
#include "chip.h"
#include "i2cm_18xx_43xx.h"

#define I2C_SCAN_FIRST_ADDRESS 0x08
#define I2C_SCAN_LAST_ADDRESS  0x77
#define MPU6050_WHO_AM_I       0x75

static bool_t i2cTransfer(uint8_t address,
                          const uint8_t *txBuffer,
                          uint16_t txSize,
                          uint8_t *rxBuffer,
                          uint16_t rxSize,
                          uint16_t *status)
{
   I2CM_XFER_T transfer;
   transfer.slaveAddr = address;
   transfer.options = 0;
   transfer.status = I2CM_STATUS_BUSY;
   transfer.txSz = txSize;
   transfer.rxSz = rxSize;
   transfer.txBuff = txBuffer;
   transfer.rxBuff = rxBuffer;

   uint32_t completed = Chip_I2CM_XferBlocking(LPC_I2C0, &transfer);
   if (status != 0) {
      *status = transfer.status;
   }

   return completed != 0 && transfer.status == I2CM_STATUS_OK;
}

static void reportMpuIdentity(uint8_t address)
{
   uint8_t registerAddress = MPU6050_WHO_AM_I;
   uint8_t identity = 0;
   uint16_t status = I2CM_STATUS_BUSY;

   if (i2cTransfer(address, &registerAddress, 1, &identity, 1, &status)) {
      printf("  WHO_AM_I en 0x%02X = 0x%02X\r\n", address, identity);
   } else {
      printf("  No se pudo leer WHO_AM_I en 0x%02X (estado I2C 0x%02X)\r\n",
             address, status);
   }
}

static void scanBus(void)
{
   uint8_t devicesFound = 0;

   printf("\r\nEscaneo I2C0, 100 kHz\r\n");
   for (uint8_t address = I2C_SCAN_FIRST_ADDRESS;
        address <= I2C_SCAN_LAST_ADDRESS;
        address++) {
      uint8_t readByte = 0;
      uint16_t status = I2CM_STATUS_BUSY;

      if (i2cTransfer(address, 0, 0, &readByte, 1, &status)) {
         printf("  ACK en 0x%02X\r\n", address);
         devicesFound++;

         if (address == 0x68 || address == 0x69) {
            reportMpuIdentity(address);
         }
      }
   }

   if (devicesFound == 0) {
      printf("No respondio ningun dispositivo. Revisar SDA, SCL, GND, VCC y pull-ups.\r\n");
   } else {
      printf("Dispositivos que respondieron: %u\r\n", devicesFound);
      if (devicesFound > 1) {
         printf("El bus comparte dispositivos; confirmar que cada direccion sea esperada.\r\n");
      }
   }
}

int main(void)
{
   boardConfig();
   printf("Diagnostico de bus I2C de EDU-CIAA\r\n");

   if (!i2cInit(I2C0, 100000)) {
      printf("No se pudo inicializar I2C0.\r\n");
      while (TRUE) {
         delay(1000);
      }
   }

   while (TRUE) {
      scanBus();
      delay(3000);
   }
}