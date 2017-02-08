#ifndef _APP_COMMON_H_
#define _APP_COMMON_H_


typedef enum {
	LOG_DEBUG = 0,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
} logLevel_t;



uint8_t log(logLevel_t level, char *fmt, ...);
uint8_t my_itoa(int32_t aInNum, char *aOutStr, uint8_t aInBase);
uint8_t my_utoa(uint32_t aInNum, char *aOutStr, uint8_t aInBase);



#endif // APP_COMMON_H_
// eof...
