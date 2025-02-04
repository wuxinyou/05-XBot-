
/*******************************************************Copyright*********************************************************
**                                            北京博创兴盛机器人技术有限公司
**                                                       研发部
**                                               http://www.uptech-eod.com
**
**-------------------------------------------------------文件信息---------------------------------------------------------
** 文件名称:			CbCtrlData.h
** 最后修订日期:		2010-05-01
** 最后版本:			1.0
** 描述:				应用于排爆机器人
**
**------------------------------------------------------------------------------------------------------------------------
** 创建人:			吴琳
** 创建日期:			2009-04-01
** 版本:				1.0
** 描述:				应用于排爆机器人；
**
**------------------------------------------------------------------------------------------------------------------------
** 修订人:
** 修订日期:
** 版本:
** 描述:
**
**------------------------------------------------------------------------------------------------------------------------
** 修订人:
** 修订日期:
** 版本:
** 描述:
**
*************************************************************************************************************************/


#ifndef CBCTRLDATA_H_
#define CBCTRLDATA_H_

#include "avr_compiler.h"
#include "ConfigTypes.h"

#include "Apps/UsartInit.h"



#include "Apps/SystemInit.h"

#include "Drivers/port_driver.h"
#include "Apps/AdcControl.h"


#define		CBCTRLDATA_GLOBALS

#ifndef   CBCTRLDATA_GLOBALS
     #define CBCTRLDATA_EXT
#else
     #define CBCTRLDATA_EXT  extern
#endif


/*************************************************************************************************************************
**                                                    协议相关控制字
*************************************************************************************************************************/


#define CB_TXD_START 			0xA5
#define CB_RXD_START 			0xA8

#define CB_FIRE_START 			0xA5



#define CB_TXD_LEN				0x0D			// 通讯数据发送长度
#define CB_RXD_LEN				0x0D			// 通讯数据接收长度

#define CB_DEVID_MAX			0X05

#define COM_WIRELESS_EN					0X01
#define COM_WIRE_EN						0X02

#define FIRE_LOCK						0X00
#define FIRE_PREPARE					0X01
#define FIRE_START						0X02
#define FIRE_STOP						0X03

#define TEST_NC							0X00
#define TEST_NO_LIMIT					0X01
#define TEST_SET_ZERO					0X02

/*************************************************************************************************************************
**                                                     结构体定义
*************************************************************************************************************************/
typedef struct S_CbCtrlData
{
	uint8_t TxdDataBuf[CB_TXD_LEN];
	uint8_t RxdDataBuf[CB_RXD_LEN];

	uint8_t TxdCheckSum;
	uint8_t RxdCheckSum;

	uint8_t DevID;					//设备端口号

	uint8_t LeftMoveCtrl;			//左电机控制
	uint8_t RightMoveCtrl;			//右电机控制
	uint8_t LeftMoveSpeed;			//左电机速度
	uint8_t RightMoveSpeed;			//右电机速度
	uint8_t LeftSpeed;				//真实的左电机速度
	uint8_t RightSpeed;				//真实的右电机速度
	uint8_t Arm12Ctrl;				//机械臂12关节控制
	uint8_t Arm34Ctrl;				//机械臂34关节控制
	uint8_t Arm50Ctrl;				//机械臂50关节控制
	uint8_t Arm78Ctrl;				//机械臂78关节控制
	uint8_t Arm6Ctrl;				//机械臂6关节控制

	uint8_t ExCtrl;					//其它控制信号
	uint8_t SwCtrl;					//开关量控制信号
	uint8_t CamZoomCtrl;			//图像及焦距设置


	uint8_t Arm12356Speed;			//机械臂12356速度控制


	uint8_t ComSwState;				//有线无线控制

	uint8_t FireState;				//武器状态

	uint8_t Arm0Angle;				//摆腿旋转
	uint8_t Arm1Angle;				//底盘旋转
	uint8_t Arm2Angle;				//大臂俯仰
	uint8_t Arm3Angle;				//小臂俯仰
	uint8_t Arm4Angle;				//腕关节俯仰
	uint8_t Arm5Angle;				//手爪旋转
	uint8_t Arm6Angle;				//云台俯仰
	uint8_t Arm7Angle;				//云台旋转

	uint8_t HandWidth;				//手爪抓持
	uint8_t CamPtAngle;				//前视摄像头俯仰

	uint8_t Arm0ModeTime;			//摆腿运动时间
	uint8_t Arm18ModeTime;			//机械臂运动时间

	uint8_t CtrlBoxPowerVal;		//控制箱电压
	uint8_t RobotSelfPowerVal;		//机器本体电压

	uint8_t Battery_1;				//本体电池电压1
	uint8_t Battery_2;				//本体电池电压2
	uint8_t Battery_3;				//本体电池电压3
	uint8_t Battery_4;				//本体电池电压4
	uint8_t Wirelength;				//有线的线长
	uint8_t Error_1;				//错误信息1
	uint8_t Error_2;				//错误信息2
	uint8_t Error_3;				//错误信息3
	uint8_t Error_4;				//错误信息4
	uint8_t Error_5;				//错误信息5
	uint8_t Error_6;				//错误信息6
	uint8_t Error_7;				//错误信息7
	uint8_t Error_8;				//错误信息8

	uint8_t Arm0WorkMode;			//摆腿工作模式
	uint8_t Arm18WorkMode;			//机械臂工作模式
	uint8_t YtWorkMode;				//云台工作模式

	uint8_t TestMode;				//测试模式
	uint8_t KeyVal;					//小液晶屏的按键状态
	uint8_t SendToXbotCounter;		//发送到机器人数据帧计数器
	uint8_t ReceiveFormXbotCounter;	//接收自机器人数据帧计数器
	uint8_t SendToDisCounter;		//发送到液晶屏数据帧计数器
	uint8_t ReceiveFormDisCounter;	//接收自液晶屏数据帧计数器
	uint8_t signal;					//和机器人通信的信号强度

}CB_CTRL_DATA_STRUCT;							// Raptor从控制箱接收到的控制信息


CBCTRLDATA_EXT CB_CTRL_DATA_STRUCT CbCtrlData;

#endif /* CBIOCTRL_H_ */



/******************************所有控制接口定义**************************/

/*************** IO 输入接口**************/
/****************************************
PC0  	IN_Aim   		瞄准控制
PC1		IN_Aim_Lock		开火钥匙
PC4		IN_Fire			开火控制
PC5		IN_COM_SW		有线无线切换检测

PD0		OUT_COM_CTRL	有线无线切换控制
PD5		IN_CAM0			图像通道编码输入0
PD6		IN_CAM1			图像通道编码输入1
PD7		IN_CAM2			图像通道编码输入2

PE0		OUT_CAM0		图像通道LED编码控制0
PE1		OUT_CAM1		图像通道LED编码控制1
PE2		OUT_CAM2		图像通道LED编码控制2
PE3		IN_HD_LIGHT		云台灯光控制
PE4		IN_MV_LIGHT		行使灯光控制
PE5		IN_HANDA		手爪抓持
PE6		IN_HANDB		手爪松开
PE7		IN_HAND_AUTO	手爪自动

PF0		IN_JOINT1_A		底座左转
PF1		IN_JOINT1_B		底座右转
PF2		IN_JOINT2_A		大臂下俯
PF3		IN_JOINT2_B		大臂上仰
PF4		IN_JOINT3_A		小臂上仰
PF5		IN_JOINT3_B		小臂下俯
PF6		IN_JOINT4_A		手爪右转
PF7		IN_JOINT4_B		手爪左转

**********	ADC-A	通道 	**********
ADC0	ARM_SPEED		机械臂运动速度
ADC1	HAND_FORCE		手爪抓持力度
ADC2	REVERVATION		保留
ADC3	Voltage			系统工作电压
ADC4	HEADX			云台旋转控制
ADC5	HEADY			云台俯仰控制
ADC6	HEAD_FOCUS		云台变焦控制
ADC7	MOVE_X			运动左右控制


**********	ADC-B	通道 	**********
ADC0	MOVE_Y			运动前后控制
ADC1	MOVE_Z			保留
ADC2	MOVE_SPEED		运动速度控制


********************************************/






