#include "../config.h"

#ifdef BOARD_EDUCIAA

#include "hal_sensors.h"
#include "sapi.h"
#include <math.h>

// Port del driver de Adafruit/Pololu para VL53L0X (single-shot ranging,
// sin modo continuo). Traducido registro por registro desde el driver
// de referencia (github.com/pololu/vl53l0x-arduino, tal como lo porta
// Adafruit para CircuitPython) usando i2cWrite/i2cWriteRead de sAPI en
// vez de Wire.h.

namespace {
  MPU60X0_address_t imu_addr = MPU60X0_ADDRESS_0; // AD0 a GND

  const uint8_t VL53L0X_ADDR = 0x29;

  // ---- Registros usados ----
  const uint8_t REG_SYSRANGE_START = 0x00;
  const uint8_t REG_SYSTEM_SEQUENCE_CONFIG = 0x01;
  const uint8_t REG_SYSTEM_INTERRUPT_CONFIG_GPIO = 0x0A;
  const uint8_t REG_GPIO_HV_MUX_ACTIVE_HIGH = 0x84;
  const uint8_t REG_SYSTEM_INTERRUPT_CLEAR = 0x0B;
  const uint8_t REG_RESULT_INTERRUPT_STATUS = 0x13;
  const uint8_t REG_RESULT_RANGE_STATUS = 0x14;
  const uint8_t REG_MSRC_CONFIG_CONTROL = 0x60;
  const uint8_t REG_PRE_RANGE_CONFIG_VCSEL_PERIOD = 0x50;
  const uint8_t REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI = 0x51;
  const uint8_t REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD = 0x70;
  const uint8_t REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI = 0x71;
  const uint8_t REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44;
  const uint8_t REG_MSRC_CONFIG_TIMEOUT_MACROP = 0x46;
  const uint8_t REG_IDENTIFICATION_MODEL_ID = 0xC0;
  const uint8_t REG_IDENTIFICATION_REVISION_ID = 0xC2;
  const uint8_t REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD = 0x4E;
  const uint8_t REG_DYNAMIC_SPAD_REF_EN_START_OFFSET = 0x4F;
  const uint8_t REG_GLOBAL_CONFIG_REF_EN_START_SELECT = 0xB6;
  const uint8_t REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0 = 0xB0;

  const int VCSEL_PERIOD_PRE_RANGE = 0;
  const int VCSEL_PERIOD_FINAL_RANGE = 1;

  const int TIMEOUT_TRIES = 1000; // ~1s con delay(1) adentro del loop

  uint8_t stop_variable = 0;
  uint32_t measurement_timing_budget_us = 0;
  bool vl53l0x_ok = false;

  // ---- Acceso a registros de bajo nivel ----

  uint8_t read_u8(uint8_t reg) {
    uint8_t val = 0;
    i2cWriteRead(I2C0, VL53L0X_ADDR, &reg, 1, TRUE, &val, 1, TRUE);
    return val;
  }

  void write_u8(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2cWrite(I2C0, VL53L0X_ADDR, buf, 2, TRUE);
  }

  uint16_t read_u16(uint8_t reg) {
    uint8_t buf[2] = {0, 0};
    i2cWriteRead(I2C0, VL53L0X_ADDR, &reg, 1, TRUE, buf, 2, TRUE);
    return ((uint16_t)buf[0] << 8) | buf[1];
  }

  void write_u16(uint8_t reg, uint16_t val) {
    uint8_t buf[3] = {reg, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF)};
    i2cWrite(I2C0, VL53L0X_ADDR, buf, 3, TRUE);
  }

  // ---- Conversión de timeouts (formato propio del chip) ----

  float decode_timeout(uint16_t val) {
    return (float)(val & 0xFF) * powf(2.0f, (float)((val & 0xFF00) >> 8)) + 1.0f;
  }

  uint16_t encode_timeout(uint32_t timeout_mclks) {
    uint32_t ls_byte = 0;
    uint16_t ms_byte = 0;
    if (timeout_mclks > 0) {
      ls_byte = timeout_mclks - 1;
      while (ls_byte > 255) {
        ls_byte >>= 1;
        ms_byte++;
      }
      return (uint16_t)((ms_byte << 8) | (ls_byte & 0xFF));
    }
    return 0;
  }

  uint32_t timeout_mclks_to_us(uint32_t timeout_period_mclks, uint8_t vcsel_period_pclks) {
    uint32_t macro_period_ns = ((2304UL * vcsel_period_pclks * 1655UL) + 500UL) / 1000UL;
    return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000UL;
  }

  uint32_t timeout_us_to_mclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks) {
    uint32_t macro_period_ns = ((2304UL * vcsel_period_pclks * 1655UL) + 500UL) / 1000UL;
    return ((timeout_period_us * 1000UL) + (macro_period_ns / 2)) / macro_period_ns;
  }

  uint8_t get_vcsel_pulse_period(int type) {
    uint8_t val = (type == VCSEL_PERIOD_PRE_RANGE)
                      ? read_u8(REG_PRE_RANGE_CONFIG_VCSEL_PERIOD)
                      : read_u8(REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD);
    return ((val + 1) & 0xFF) << 1;
  }

  void get_sequence_step_enables(bool* tcc, bool* dss, bool* msrc,
                                  bool* pre_range, bool* final_range) {
    uint8_t seq = read_u8(REG_SYSTEM_SEQUENCE_CONFIG);
    *tcc = (seq >> 4) & 0x1;
    *dss = (seq >> 3) & 0x1;
    *msrc = (seq >> 2) & 0x1;
    *pre_range = (seq >> 6) & 0x1;
    *final_range = (seq >> 7) & 0x1;
  }

  void get_sequence_step_timeouts(bool pre_range, uint32_t* msrc_dss_tcc_us,
                                   uint32_t* pre_range_us, uint32_t* final_range_us,
                                   uint8_t* final_range_vcsel_pclks,
                                   uint32_t* pre_range_mclks) {
    uint8_t pre_range_vcsel_pclks = get_vcsel_pulse_period(VCSEL_PERIOD_PRE_RANGE);
    uint8_t msrc_dss_tcc_mclks = (read_u8(REG_MSRC_CONFIG_TIMEOUT_MACROP) + 1) & 0xFF;
    *msrc_dss_tcc_us = timeout_mclks_to_us(msrc_dss_tcc_mclks, pre_range_vcsel_pclks);

    uint32_t pre_mclks = (uint32_t)decode_timeout(read_u16(REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI));
    *pre_range_mclks = pre_mclks;
    *pre_range_us = timeout_mclks_to_us(pre_mclks, pre_range_vcsel_pclks);

    *final_range_vcsel_pclks = get_vcsel_pulse_period(VCSEL_PERIOD_FINAL_RANGE);
    uint32_t final_mclks = (uint32_t)decode_timeout(read_u16(REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI));
    if (pre_range) final_mclks -= pre_mclks;
    *final_range_us = timeout_mclks_to_us(final_mclks, *final_range_vcsel_pclks);
  }

  uint32_t get_measurement_timing_budget() {
    uint32_t budget = 1910 + 960;
    bool tcc, dss, msrc, pre_range, final_range;
    get_sequence_step_enables(&tcc, &dss, &msrc, &pre_range, &final_range);

    uint32_t msrc_dss_tcc_us, pre_range_us, final_range_us, pre_range_mclks;
    uint8_t final_vcsel;
    get_sequence_step_timeouts(pre_range, &msrc_dss_tcc_us, &pre_range_us,
                                &final_range_us, &final_vcsel, &pre_range_mclks);

    if (tcc) budget += msrc_dss_tcc_us + 590;
    if (dss) budget += 2 * (msrc_dss_tcc_us + 690);
    else if (msrc) budget += msrc_dss_tcc_us + 660;
    if (pre_range) budget += pre_range_us + 660;
    if (final_range) budget += final_range_us + 550;

    return budget;
  }

  bool set_measurement_timing_budget(uint32_t budget_us) {
    if (budget_us < 20000) return false;

    uint32_t used = 1320 + 960;
    bool tcc, dss, msrc, pre_range, final_range;
    get_sequence_step_enables(&tcc, &dss, &msrc, &pre_range, &final_range);

    uint32_t msrc_dss_tcc_us, pre_range_us, final_range_us, pre_range_mclks;
    uint8_t final_vcsel;
    get_sequence_step_timeouts(pre_range, &msrc_dss_tcc_us, &pre_range_us,
                                &final_range_us, &final_vcsel, &pre_range_mclks);

    if (tcc) used += msrc_dss_tcc_us + 590;
    if (dss) used += 2 * (msrc_dss_tcc_us + 690);
    else if (msrc) used += msrc_dss_tcc_us + 660;
    if (pre_range) used += pre_range_us + 660;

    if (final_range) {
      used += 550;
      if (used > budget_us) return false; // no queda tiempo para final range
      uint32_t final_timeout_us = budget_us - used;
      uint32_t final_timeout_mclks = timeout_us_to_mclks(final_timeout_us, final_vcsel);
      if (pre_range) final_timeout_mclks += pre_range_mclks;
      write_u16(REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, encode_timeout(final_timeout_mclks));
    }
    return true;
  }

  bool get_spad_info(uint8_t* count, bool* is_aperture) {
    write_u8(0x80, 0x01);
    write_u8(0xFF, 0x01);
    write_u8(0x00, 0x00);
    write_u8(0xFF, 0x06);
    write_u8(0x83, read_u8(0x83) | 0x04);
    write_u8(0xFF, 0x07);
    write_u8(0x81, 0x01);
    write_u8(0x80, 0x01);
    write_u8(0x94, 0x6B);
    write_u8(0x83, 0x00);

    int tries = 0;
    while (read_u8(0x83) == 0x00) {
      delay(1);
      if (++tries > TIMEOUT_TRIES) return false;
    }

    write_u8(0x83, 0x01);
    uint8_t tmp = read_u8(0x92);
    *count = tmp & 0x7F;
    *is_aperture = ((tmp >> 7) & 0x01) == 1;

    write_u8(0x81, 0x00);
    write_u8(0xFF, 0x06);
    write_u8(0x83, read_u8(0x83) & ~0x04);
    write_u8(0xFF, 0x01);
    write_u8(0x00, 0x01);
    write_u8(0xFF, 0x00);
    write_u8(0x80, 0x00);
    return true;
  }

  bool perform_single_ref_calibration(uint8_t vhv_init_byte) {
    write_u8(REG_SYSRANGE_START, 0x01 | (vhv_init_byte & 0xFF));
    int tries = 0;
    while ((read_u8(REG_RESULT_INTERRUPT_STATUS) & 0x07) == 0) {
      delay(1);
      if (++tries > TIMEOUT_TRIES) return false;
    }
    write_u8(REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    write_u8(REG_SYSRANGE_START, 0x00);
    return true;
  }

  bool vl53l0x_init() {
    write_u8(0x88, 0x00);
    write_u8(0x80, 0x01);
    write_u8(0xFF, 0x01);
    write_u8(0x00, 0x00);
    stop_variable = read_u8(0x91);
    write_u8(0x00, 0x01);
    write_u8(0xFF, 0x00);
    write_u8(0x80, 0x00);

    // Deshabilita los límites de SIGNAL_RATE_MSRC y SIGNAL_RATE_PRE_RANGE
    uint8_t config_control = read_u8(REG_MSRC_CONFIG_CONTROL) | 0x12;
    write_u8(REG_MSRC_CONFIG_CONTROL, config_control);

    // Signal rate limit = 0.25 MCPS, en fixed point 9.7 -> 0.25 * 128 = 32
    write_u16(REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 32);

    write_u8(REG_SYSTEM_SEQUENCE_CONFIG, 0xFF);

    uint8_t spad_count;
    bool spad_is_aperture;
    if (!get_spad_info(&spad_count, &spad_is_aperture)) return false;

    uint8_t ref_spad_map[6];
    {
      uint8_t reg = REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0;
      i2cWriteRead(I2C0, VL53L0X_ADDR, &reg, 1, TRUE, ref_spad_map, 6, TRUE);
    }

    write_u8(0xFF, 0x01);
    write_u8(REG_DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
    write_u8(REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
    write_u8(0xFF, 0x00);
    write_u8(REG_GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

    uint8_t first_spad_to_enable = spad_is_aperture ? 12 : 0;
    uint8_t spads_enabled = 0;
    for (int i = 0; i < 48; i++) {
      if (i < first_spad_to_enable || spads_enabled == spad_count) {
        ref_spad_map[i / 8] &= ~(1 << (i % 8));
      } else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1) {
        spads_enabled++;
      }
    }

    {
      uint8_t buf[7];
      buf[0] = REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0;
      for (int i = 0; i < 6; i++) buf[1 + i] = ref_spad_map[i];
      i2cWrite(I2C0, VL53L0X_ADDR, buf, 7, TRUE);
    }

    // Secuencia larga de tuning "de fábrica" (valores tal cual el driver
    // de referencia, no son inventables ni deducibles del datasheet público).
    static const uint8_t tuning[][2] = {
      {0xFF,0x01},{0x00,0x00},{0xFF,0x00},{0x09,0x00},{0x10,0x00},{0x11,0x00},
      {0x24,0x01},{0x25,0xFF},{0x75,0x00},{0xFF,0x01},{0x4E,0x2C},{0x48,0x00},
      {0x30,0x20},{0xFF,0x00},{0x30,0x09},{0x54,0x00},{0x31,0x04},{0x32,0x03},
      {0x40,0x83},{0x46,0x25},{0x60,0x00},{0x27,0x00},{0x50,0x06},{0x51,0x00},
      {0x52,0x96},{0x56,0x08},{0x57,0x30},{0x61,0x00},{0x62,0x00},{0x64,0x00},
      {0x65,0x00},{0x66,0xA0},{0xFF,0x01},{0x22,0x32},{0x47,0x14},{0x49,0xFF},
      {0x4A,0x00},{0xFF,0x00},{0x7A,0x0A},{0x7B,0x00},{0x78,0x21},{0xFF,0x01},
      {0x23,0x34},{0x42,0x00},{0x44,0xFF},{0x45,0x26},{0x46,0x05},{0x40,0x40},
      {0x0E,0x06},{0x20,0x1A},{0x43,0x40},{0xFF,0x00},{0x34,0x03},{0x35,0x44},
      {0xFF,0x01},{0x31,0x04},{0x4B,0x09},{0x4C,0x05},{0x4D,0x04},{0xFF,0x00},
      {0x44,0x00},{0x45,0x20},{0x47,0x08},{0x48,0x28},{0x67,0x00},{0x70,0x04},
      {0x71,0x01},{0x72,0xFE},{0x76,0x00},{0x77,0x00},{0xFF,0x01},{0x0D,0x01},
      {0xFF,0x00},{0x80,0x01},{0x01,0xF8},{0xFF,0x01},{0x8E,0x01},{0x00,0x01},
      {0xFF,0x00},{0x80,0x00}
    };
    for (unsigned i = 0; i < sizeof(tuning) / sizeof(tuning[0]); i++) {
      write_u8(tuning[i][0], tuning[i][1]);
    }

    write_u8(REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    uint8_t gpio_hv = read_u8(REG_GPIO_HV_MUX_ACTIVE_HIGH);
    write_u8(REG_GPIO_HV_MUX_ACTIVE_HIGH, gpio_hv & ~0x10); // activo en bajo
    write_u8(REG_SYSTEM_INTERRUPT_CLEAR, 0x01);

    measurement_timing_budget_us = get_measurement_timing_budget();

    write_u8(REG_SYSTEM_SEQUENCE_CONFIG, 0xE8);
    set_measurement_timing_budget(measurement_timing_budget_us);

    write_u8(REG_SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (!perform_single_ref_calibration(0x40)) return false;
    write_u8(REG_SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (!perform_single_ref_calibration(0x00)) return false;
    write_u8(REG_SYSTEM_SEQUENCE_CONFIG, 0xE8); // restaura la config normal

    return true;
  }

  bool vl53l0x_do_range_measurement() {
    write_u8(0x80, 0x01);
    write_u8(0xFF, 0x01);
    write_u8(0x00, 0x00);
    write_u8(0x91, stop_variable);
    write_u8(0x00, 0x01);
    write_u8(0xFF, 0x00);
    write_u8(0x80, 0x00);
    write_u8(REG_SYSRANGE_START, 0x01);

    int tries = 0;
    while (read_u8(REG_SYSRANGE_START) & 0x01) {
      delay(1);
      if (++tries > TIMEOUT_TRIES) return false;
    }
    return true;
  }

  uint16_t vl53l0x_read_range_mm() {
    uint16_t range = read_u16(REG_RESULT_RANGE_STATUS + 10);
    write_u8(REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    return range;
  }
}

void hal_sensors_init() {
  // boardConfig() ya se llama una vez en main() antes de esto.
  int8_t status = mpu60X0Init(imu_addr); // esto ya hace i2cInit(I2C0, ...) adentro
  if (status < 0) {
    printf("IMU MPU6050 no inicializado, revisar conexiones.\r\n");
    while (1);
  }

  // Chequeo de identidad antes de meterse con el init completo del VL53L0X.
  uint8_t model_id = read_u8(REG_IDENTIFICATION_MODEL_ID);
  uint8_t revision_id = read_u8(REG_IDENTIFICATION_REVISION_ID);
  if (model_id != 0xEE || revision_id != 0x10) {
    printf("VL53L0X no responde como se espera (model=0x%02X, rev=0x%02X). "
           "Revisar conexiones.\r\n", model_id, revision_id);
    vl53l0x_ok = false;
    return;
  }

  vl53l0x_ok = vl53l0x_init();
  if (!vl53l0x_ok) {
    printf("VL53L0X: fallo la inicializacion (timeout en calibracion).\r\n");
  } else {
    printf("VL53L0X inicializado correctamente.\r\n");
  }
}

float hal_get_distance_cm() {
  if (!vl53l0x_ok) return -1;
  if (!vl53l0x_do_range_measurement()) return -1;
  uint16_t mm = vl53l0x_read_range_mm();
  return mm / 10.0f;
}

float hal_get_gyro_z_rads() {
  mpu60X0Read();
  return mpu60X0GetGyroZ_rads();
}

#endif // BOARD_EDUCIAA
