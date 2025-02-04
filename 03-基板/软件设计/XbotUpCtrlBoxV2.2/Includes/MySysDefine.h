

#ifndef MY_SYS_DEFINE_H
#define MY_SYS_DEFINE_H



//typedef enum _BOOL
//{
//    FALSE 	= 0,
//	TRUE 	= 1
//} BOOL;
//
//typedef unsigned char bool;
//
//#define false 0
//#define true  (!false)

typedef uint8_t bool;
#define true 	1
#define false 	0
typedef enum {FALSE = 0, TRUE = !FALSE} BOOL;

#define USART_OMAP_CTRL			    USART_data_1
#define USRT_XBOT_WIRE_CTRL		  USART_data_3
#define USRT_XBOT_WIRELESS_CTRL	USART_data_2

#define CB_MOTOR_WORK_START	
#define CB_MOTOR_WORK_STOP
#define CB_GLED1_ON
#define CB_GLED1_OFF
#define CB_GLED2_ON
#define CB_GLED2_OFF

#endif

