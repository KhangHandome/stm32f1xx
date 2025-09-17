#ifndef _HAL_TIMER_H_
#define _HAL_TIMER_H_
#include "main.h"

/*
 * @brief : Defines for timer channels
 */
#define MAX_CHANNEL_TIMER 4 

/*
 * @brief : Defines a callback function pointer for interrupt 	handling
 */
typedef void (*CallbackFunction_t)(void);

/*
 * @brief : Enumerates counter modes including up counting, down counting, and center-aligned
 */
typedef enum {
    COUNTER_UP_COUNTING = 0x00,   /* Chế độ đếm lên */
    COUNTER_DOWN_COUNTING = 0x01, /* Chế độ đếm xuống */
    COUNTER_CENTER_ALIGNED = 0x02       /* Hiếm khi dùng */
} HAL_Timer_CounterMode_t;

typedef enum {
	CHANNEL_CONFIG_OUTPUT_ACTIVE_HIGH  = 0x00,
	CHANNEL_CONFIG_OUTPUT_ACTIVE_LOW   = 0x01
} HAL_Timer_Channel_Config_Output_t;

typedef enum {
	CHANNEL_CONFIG_INPUT_RISING_EDGE = 0x00,  /* Capture by rising */
	CHANNEL_CONFIG_INPUT_FAILING_EDGE = 0x01 /* Capture by faling  */
} HAL_Timer_Channel_Config_Input_Edge_t;

typedef union
{
	HAL_Timer_Channel_Config_Output_t HAL_Timer_Channel_Config_Output;
	HAL_Timer_Channel_Config_Input_Edge_t HAL_Timer_Channel_Config_Input_Edge;
} HAL_Channel_Config_Pin_t;

typedef enum
{
	TIM_IRQ_ENABLE = 0x01,
	TIM_IRQ_DISABLE = 0x00
} HAL_Timer_IRQ;

typedef enum
{
	TIM_AUTO_RELOAD_ENALBE = 0x01,
	TIM_AUTO_RELOAD_DISABLE = 0x00
} TIM_AutoReload_t;

typedef enum
{
	HAL_TIM_EDGE_ALIGNED_MODE,  /* Depending when up couter or downcounter */
	/* It work when config at output compare */
	HAL_CENTER_ALIGNED_MODE_1,  /* Generate a interupt about CCIF When counting down */
	HAL_CENTER_ALIGNED_MODE_2,  /* Generate a interupt about CCIF When counting up */
	HAL_CENTER_ALIGNED_MODE_3   /* Generate a interupt about CCIF When counting down and counting up  */
} HAL_Timer_Center_Aligned_Mode_t;
typedef enum
{
	HAL_TIM_COUNTER_UP = 0x0,
	HAL_TIM_COUNTER_DONW =0x1
} HAL_Timer_Counter_Dir_t;
typedef enum
{
	HAL_TIM_ONE_PULSE_MODE = 0x01,
	HAL_TIM_CONTINUOUS_MODE = 0x00
} HAL_Timer_Mode_t;

// Bit 2 URS: Update Request Source
typedef enum {
    HAL_TIMER_UPDATE_REQ_SRC_ALL = 0x0, // Counter overflow/underflow + UG bit + Slave mode
    HAL_TIMER_UPDATE_REQ_SRC_OVF_ONLY = 0x1 // Only counter overflow/underflow
} HAL_Timer_UpdateReqSrc_t;

// Bit 1 UDIS: Update Disable
typedef enum {
    HAL_TIMER_UEV_ENABLED  = 0x0, // UEV enabled: ARR/PSC/CCR buffered registers updated on UEV
    HAL_TIMER_UEV_DISABLED = 0x1  // UEV disabled: No UEV generated, shadow regs not updated
} HAL_Timer_UpdateEventState_t;

// CCxS: Capture/Compare selection (cho cả input/output)
typedef enum {
    HAL_TIM_COMPARE_OUTPUT     = 0x0,    // 00: channel configured as output
    HAL_TIM_CAPTURE_INPUT_TI1  = 0x1, // 01: input mapped on TIx
    HAL_TIM_CAPTURE_INPUT_TI2  = 0x2, // 10: input mapped on opposite TIx
    HAL_TIM_CAPTURE_INPUT_TRC  = 0x3  // 11: input mapped on TRC
} HAL_Timer_Capture_Compare_Select_t;

// OCxM: Output Compare Mode
typedef enum {
    HAL_TIM_OCM_FROZEN            = 0x0,
    HAL_TIM_OCM_ACTIVE_ON_MATCH   = 0x1,
    HAL_TIM_OCM_INACTIVE_ON_MATCH = 0x2,
    HAL_TIM_OCM_TOGGLE            = 0x3,
    HAL_TIM_OCM_FORCE_INACTIVE    = 0x4,
    HAL_TIM_OCM_FORCE_ACTIVE      = 0x5,
    HAL_TIM_OCM_PWM1              = 0x6,
    HAL_TIM_OCM_PWM2              = 0x7
} HAL_Timer_Output_Compare_Mode_t;

// Input capture prescaler (ICxPSC)
typedef enum {
    HAL_TIM_ICPSC_DIV1 = 0x0, // Capture every event
    HAL_TIM_ICPSC_DIV2 = 0x1, // Capture once every 2 events
    HAL_TIM_ICPSC_DIV4 = 0x2, // Capture once every 4 events
    HAL_TIM_ICPSC_DIV8 = 0x3  // Capture once every 8 events
} HAL_Timer_Input_Capture_Prescaler_t;

/*
 * @brief : Interrupt source typedef
 */
typedef enum {
    TIMER_INT_NONE    = 0,
    TIMER_INT_UPDATE  = 1,
    TIMER_INT_CC1     = 2,
    TIMER_INT_CC2     = 3,
    TIMER_INT_CC3     = 4,
    TIMER_INT_CC4     = 5,
    TIMER_INT_TRIGGER = 6,
    TIMER_INT_BREAK   = 7
} HAL_Timer_InterruptSource_t;
/**
 * @brief: Input capture filter values
 * @details: Defines digital filtering for input capture to reduce noise.
 *          The digital filter requires N consecutive events to validate a transition.
 *          fDTS = Digital filter sampling frequency, fCK_INT = Internal clock frequency
 */
typedef enum {
    HAL_TIM_ICF_NO_FILTER = 0x0,        /**< No filter, sampling at fDTS */
    HAL_TIM_ICF_CK_INT_N2 = 0x1,        /**< fSAMPLING=fCK_INT, N=2 */
    HAL_TIM_ICF_CK_INT_N4 = 0x2,        /**< fSAMPLING=fCK_INT, N=4 */
    HAL_TIM_ICF_CK_INT_N8 = 0x3,        /**< fSAMPLING=fCK_INT, N=8 */
    HAL_TIM_ICF_DTS_2_N6 = 0x4,         /**< fSAMPLING=fDTS/2, N=6 */
    HAL_TIM_ICF_DTS_2_N8 = 0x5,         /**< fSAMPLING=fDTS/2, N=8 */
    HAL_TIM_ICF_DTS_4_N6 = 0x6,         /**< fSAMPLING=fDTS/4, N=6 */
    HAL_TIM_ICF_DTS_4_N8 = 0x7,         /**< fSAMPLING=fDTS/4, N=8 */
    HAL_TIM_ICF_DTS_8_N6 = 0x8,         /**< fSAMPLING=fDTS/8, N=6 */
    HAL_TIM_ICF_DTS_8_N8 = 0x9,         /**< fSAMPLING=fDTS/8, N=8 */
    HAL_TIM_ICF_DTS_16_N5 = 0xA,        /**< fSAMPLING=fDTS/16, N=5 */
    HAL_TIM_ICF_DTS_16_N6 = 0xB,        /**< fSAMPLING=fDTS/16, N=6 */
    HAL_TIM_ICF_DTS_16_N8 = 0xC,        /**< fSAMPLING=fDTS/16, N=8 */
    HAL_TIM_ICF_DTS_32_N5 = 0xD,        /**< fSAMPLING=fDTS/32, N=5 */
    HAL_TIM_ICF_DTS_32_N6 = 0xE,        /**< fSAMPLING=fDTS/32, N=6 */
    HAL_TIM_ICF_DTS_32_N8 = 0xF         /**< fSAMPLING=fDTS/32, N=8 */
} HAL_TIM_Inputr_Capture_Filter_t;

typedef enum {
	HAL_TIM_DISABLE = 0,
	HAL_TIM_ENABLE = 1
} HAL_TIM_BitState_t;

// Bitfield cho chế độ OUTPUT (OC)
typedef struct {
    HAL_TIM_BitState_t Output_Compare_Fast_Enable ; // bit 2
    HAL_TIM_BitState_t Output_Compare_Preload; // bit 3
    HAL_Timer_Output_Compare_Mode_t HAL_Timer_Output_Compare_Mode ; // bit 6:4
    HAL_TIM_BitState_t Output_Compare_Clear_Enable ; // bit 7
} HAL_TIM_Output_Compare_Mode_t;

// Bitfield cho chế độ INPUT (IC)
typedef struct {
	HAL_Timer_Input_Capture_Prescaler_t HAL_Timer_Input_Capture_Prescaler_t ; // bit 3:2
	HAL_TIM_Inputr_Capture_Filter_t HAL_TIM_Inputr_Capture_Filter ;   // bit 7:4
} HAL_TIM_Input_Capture_Mode_t;

typedef union {
	HAL_TIM_Output_Compare_Mode_t Compare_Mode;
	HAL_TIM_Input_Capture_Mode_t Capture_Mode;
} HAL_TIM_Capture_Compare_Mode_t;
typedef enum
{
	HAL_TIM_Channel_Disable = 0x00 ,
	HAL_TIM_Channel_Enable  = 0x01
} HAL_TIM_Channel_Enable_t;
/*
 * @brief : Structure to hold timer configuration including timer instance, prescaler, auto-reload value, timer mode, counter mode, and capture/compare registers
 */
typedef struct{
	HAL_TIM_Channel_Enable_t HAL_TIM_Channel_Enable; /*Enable or disable */
	HAL_Timer_Capture_Compare_Select_t HAL_Timer_Capture_Compare_Select;  /* Mode output compare or input capture */
	uint16_t HAL_TIM_Capture_Compare_Register; /*Timer capture compare register */
	HAL_Channel_Config_Pin_t HAL_Channel_Config_Pin;
	HAL_TIM_Capture_Compare_Mode_t HAL_TIM_Capture_Compare_Mode;
	HAL_Timer_IRQ HAL_TIM_Capture_Compare_IRQ; /* Enable or disable Interrupt */
} HAL_Timer_Channel_Config_t;

typedef struct {
    TIM_TypeDef* Timer;             /* Timer instance */
    uint16_t Prescale_Value;                   /* Setup prescaler */
    uint16_t Auto_Reload_Value;                   /* Setup auto-reload value */
    TIM_AutoReload_t TIM_AutoReloadEnable;/* Arpe disabe or enable */
    HAL_Timer_Center_Aligned_Mode_t HAL_Timer_Center_Aligned_Mode ;
    HAL_Timer_Counter_Dir_t HAL_Timer_Counter_Dir ;   /*Direction of CNT, counting up or counting down */
    HAL_Timer_Mode_t HAL_Timer_Mode;  /* Select mode for timer, once times, for continuous */
    HAL_Timer_UpdateReqSrc_t HAL_Timer_UpdateReqSrc; /* Select update request */
    HAL_Timer_UpdateEventState_t HAL_Timer_UpdateEventState;
    HAL_Timer_IRQ Timer_Irq;
    HAL_Timer_Channel_Config_t HAL_Timer_Channel[MAX_CHANNEL_TIMER]; /* Config for each channel */
    CallbackFunction_t CallbackFunction; /*Callback function */
} HAL_TimerInit_t;


/*
 * @brief : Initializes the timer peripheral with the specified configuration
 * @param : TimerInit - Timer configuration structure
 */
extern void HAL_Timer_Init(HAL_TimerInit_t* TimerInit);
/*
 * @brief : Initializes and configures the timer with the provided setup parameters
 * @param : TimerInit - Timer configuration structure
 */
extern void HAL_Timer_Setup(HAL_TimerInit_t* TimerInit);

extern void HAL_Timer_Stop(HAL_TimerInit_t * TimerInit);

extern void HAL_Timer_ChangePSC(HAL_TimerInit_t* TimerInit,uint16_t PSC);

extern void HAL_Timer_ChangeARR(HAL_TimerInit_t* TimerInit,uint16_t ARR);

extern void HAL_Timer_Set_CCR(HAL_TimerInit_t* TimerInit,uint8_t Channel, uint16_t CCR);

extern void HAL_Timer_Set_Input_Edge(HAL_TimerInit_t* TimerInit,uint8_t Channel, HAL_Timer_Channel_Config_Input_Edge_t Config_Input_Edge);

/*
 * @brief : Starts the timer with the specified configuration
 * @param : TimerInit - Timer configuration structure
 */
extern void HAL_Timer_Start(HAL_TimerInit_t* TimerInit);

/*
 * @brief : Sets the callback function for timer interrupts
 * @param : TimerInit - Timer configuration structure
 */
extern void HAL_Timer_Set_CallbackFunction(HAL_TimerInit_t* TimerInit);

/*
 * @brief : Clears the timer's interrupt flag
 * @param : TimerInit - Timer configuration structure
 */
extern void HAL_Timer_ClearInterruptFlag(HAL_TimerInit_t* TimerInit);

/*
 * @brief : Returns the timer's interrupt flag status
 * @param : TimerInit - Timer configuration structure
 */
extern uint16_t HAL_Timer_GetInterruptFlag(HAL_TimerInit_t* TimerInit);

extern HAL_Timer_InterruptSource_t HAL_Timer_GetInterruptSource(HAL_TimerInit_t *TimerInit);

/*
 * @brief : Returns the value of the capture/compare register
 * @param : TimerInit - Timer configuration structure
 */
extern uint16_t HAL_Timer_GetCapture_CompareRegister(HAL_TimerInit_t* TimerInit,uint8_t channel);

#endif
