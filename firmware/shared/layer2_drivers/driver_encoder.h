#ifndef DRIVER_ENCODER_H
#define DRIVER_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { 
    ENC_IZQ = 0,
    ENC_DER = 1
} encoder_t;


typedef enum {
    ENCODER_OK = 0,
    ENCODER_ERROR_ARGUMENT,
    ENCODER_ERROR_NOT_INITIALIZED,
    ENCODER_ERROR_COMMUNICATION,
    ENCODER_ERROR_INTERNAL_API
} encoder_status_t;

encoder_status_t encoder_init(encoder_t encoder);
encoder_status_t encoder_get_angle_rad(encoder_t encoder, float *value);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_ENCODER_H
