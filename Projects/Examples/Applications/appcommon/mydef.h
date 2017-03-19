#ifndef _MY_DEF_H_
#define _MY_DEF_H_



/**
 * RF packet type
 */
typedef enum {
    RF_CMD_SYNC_REQ = 0,
    RF_CMD_SYNC_REP,
    RF_CMD_DATA_TMP,

    /* always keep it last */
    RF_CMD_END
} rf_cmd_t;

/**
 * Serial packet type
 */
typedef enum {
    UART_CMD_ECHO = 0,
    UART_CMD_SYNC_TIME,
//    UART_CMD_SYNC_INTV,
//    UART_CMD_UPLOAD_DATA,

    /* always keep it last */
    UART_CMD_END
} uart_cmd_t;

#pragma pack (1)

/* RF packet format: pkt header */
typedef struct {
    uint16_t nodeid;
    uint8_t cmd;
    uint8_t rssi;
} pkt_app_header_t;

/* RF packet format: pkt header + pkt payload */
typedef struct {
    pkt_app_header_t hdr;
    uint8_t data[MAX_APP_PAYLOAD - sizeof(pkt_app_header_t)];
} pkt_app_t;

typedef struct {
    uint32_t fTimeOffset;
    uint32_t fTimeWkup;
} app_msg_sync_req_t;

typedef struct {
    uint8_t _cmd;
    union {
        uint8_t _str[4]; /* echo string */
        uint32_t _time; /* sync time */
        uint16_t _wkupIntv; /* sync wakeup interval */
    };
} uart_pkt_t;

#pragma pack ()









#endif //_MY_DEF_H_
// eof...
