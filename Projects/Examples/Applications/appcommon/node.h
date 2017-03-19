#ifndef _NODE_H_
#define _NODE_H_


/**************************************************
 * MACRO definitions
 *************************************************/
#define AWAKE_INTERVAL (5000)
#define AWAKE_PERIOD   (AWAKE_INTERVAL / 2)



/* green: LED1 */
#define node_turn_on_green_led()      BSP_TURN_ON_LED1()
#define node_turn_off_green_led()     BSP_TURN_OFF_LED1()
#define node_toggle_green_led()       BSP_TOGGLE_LED1()
#define node_is_green_led_on()        BSP_LED1_IS_ON()


/* red: LED2 */
#define node_turn_on_red_led()      BSP_TURN_ON_LED2()
#define node_turn_off_red_led()     BSP_TURN_OFF_LED2()
#define node_toggle_red_led()       BSP_TOGGLE_LED2()
#define node_is_red_led_on()        BSP_LED2_IS_ON()




/**************************************************
 * DATA STRUCTURES
 *************************************************/

typedef enum {
        NODEOFF = 0,
        STARTUP,
        RUNNING,
        SPDWNLD,
        RFDWNLD,
} phase_t;

typedef enum {
	STARTUP_STAGE_INIT = 0,
	STARTUP_STAGE_LINK,
	STARTUP_STAGE_SYNC
} startup_stage_t;

typedef enum {
    RUNNING_STAGE_SLEEP = 0,
    RUNNING_STAGE_RTC_WKUP,
    RUNNING_STAGE_UART_WKUP,
    RUNNING_STAGE_UART_READ
} running_stage_t;


#pragma pack (1)

/**
 * node configuration
 */
typedef struct {
	/* node basic info: id, type, phase */
	uint16_t _id;
	uint8_t _type;
	phase_t _phase;
	/** the above three fields must be kept as leading fields **/

//	uint32_t gSeqno; // last seqno before restart
//	uint8_t gPDR; // PDR (4-bit): 0x0 ~ 0xF
//	uint8_t	gRand;

	/* time info: time base, next wakeup time, wakeup period */
	uint32_t _timeBase; // how many seconds passed from 1970/1/1 since last sync
	uint32_t _nextWkup;//RTC_TimeTypeDef nextAlarm;//
	uint16_t _awakePeriod;	// wakeup interval

	/* route info: */
//	route_info_t gRouteInfo; // route info
} node_config_t;

#pragma pack ()


/**************************************************
 * GLOBAL VARIABLES
 *************************************************/
extern volatile int8_t gWkupTimerSlot;
extern volatile int8_t gUartWkupTimerSlot;
extern volatile uint32_t gNextWkup;
extern volatile node_config_t gConfig;


/**************************************************
 * PUBLIC FUNCTIONS
 *************************************************/
bool node_init(void);
//void node_sleepISR(uint16_t);
//void node_awakeISR(uint16_t);
void node_setPhase(phase_t);




#endif // _NODE_H_
// eof...
