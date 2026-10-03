#include "config.h"
#include "hal_imu.h"
#include "sapi.h"

namespace {
  MPU60X0_address_t imu_addr = static_cast<MPU60X0_address_t>(IMU_I2C_ADDR);
  bool imu_ok = false;
}

bool hal_imu_init(void) {
  printf("# HAL: iniciando MPU en I2C0, direccion 0x%02X\r\n", (unsigned)imu_addr);
  int8_t status = mpu60X0Init(imu_addr);
  printf("# HAL: mpu60X0Init devolvio %d\r\n", status);
  if (status < 0) {
    printf("# IMU MPU6050 no inicializado, revisar conexiones.\r\n");
    imu_ok = false;
    return false;
  }
  imu_ok = true;
  return true;
}

float hal_imu_get_gyro_z_rads(void) {
  if (!imu_ok) {
    return 0.0f;
  }
  mpu60X0Read();
  return mpu60X0GetGyroZ_rads();
}
