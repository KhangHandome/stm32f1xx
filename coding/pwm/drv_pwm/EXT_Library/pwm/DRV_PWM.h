#ifndef _DRV_PWM_H_
#define _DRV_PWM_H_
#ifndef _STD_TYPE_H_
#include "stdint.h"

#ifndef STD_TYPES_H
#define STD_TYPES_H

/*
 * Return type used for standard AUTOSAR APIs.
 */
typedef enum
{
    /* Service executed successfully */
    STD_E_OK = 0x00u,
    /* Service execution failed */
    STD_E_NOT_OK = 0x01u
} Std_ReturnType;
#endif

/*
 * Type to define the PWM output state.
 */
typedef enum
{
    /* PWM output is in low state */
    PWM_LOW = 0x00u,
    /* PWM output is in high state */
    PWM_HIGH = 0x01u
} Pwm_OutputStateType;

/*
 * Type to define the PWM channel class.
 */
typedef enum
{
    /* PWM channel with fixed period */
    PWM_FIXED_PERIOD = 0x00u,
    /* PWM channel with fixed period but shifted phase */
    PWM_FIXED_PERIOD_SHIFTED = 0x01u,
    /* PWM channel with variable period */
    PWM_VARIABLE_PERIOD = 0x02u
} Pwm_ChannelClassType;

/*
 * Type to define the result of a power state request.
 */
typedef enum
{
    /* Power state change request executed successfully */
    PWM_SERVICE_ACCEPTED = 0x00u,
    /* PWM module has not been initialized */
    PWM_NOT_INIT = 0x01u,
    /* API service called in wrong sequence */
    PWM_SEQUENCE_ERROR = 0x02u,
    /* Hardware failure prevents power state transition */
    PWM_HW_FAILURE = 0x03u,
    /* Requested power state is not supported */
    PWM_POWER_STATE_NOT_SUPP = 0x04u,
    /* Transition to the requested power state is not possible */
    PWM_TRANS_NOT_POSSIBLE = 0x05u
} Pwm_PowerStateRequestResultType;

/*
 * Configuration structure of a single PWM channel.
 */
typedef struct
{
    /* Unique identifier of the PWM channel */
    uint8_t             ChannelId[4];
    /* PWM signal period */
    uint32_t            Period;
    /* PWM duty cycle value */
    uint16_t            DutyCycle[4];
    /* Idle state of the PWM output */
    Pwm_OutputStateType IdleState;
    /* Class type of the PWM channel */
    Pwm_ChannelClassType ChannelClass;
    /* Timer for PWM*/
    TIM_HandleTypeDef*   Timer;
} Pwm_ChannelConfigType;
typedef uint16_t Pwm_Period_Type;
typedef uint16_t Pwm_DutyCycle_Type;

/*
 * Prototype function
 * */
/*
 * @brief : Init module pwm to run
 */
void DRV_PwmInit(Pwm_ConfigType* config);
/*
 * @brief : Deinit module pwm
 */
void DRV_PwmDeinit(Pwm_ConfigType* config);
/*
 *
 */
void DRV_PwmSetDutyCycle(Pwm_ChannelConfigType* config, Pwm_DutyCycle_Type dutyCycle );
void DRV_PwmSetPeriodAndDuty(Pwm_ChannelConfigType* config, Pwm_DutyCycle_Type dutyCycle, Pwm_Period_Type period);
void DRV_PwmSetOutputToIdle(Pwm_ChannelConfigType* config);
Pwm_OutputStateType DRV_PwmGetOutputState(Pwm_ConfigType* config);
Pwm_PowerStateRequestResultType DRV_PwmGetPowerState(Pwm_ConfigType* config);


#endif  /* _DRV_PWM_H*/
