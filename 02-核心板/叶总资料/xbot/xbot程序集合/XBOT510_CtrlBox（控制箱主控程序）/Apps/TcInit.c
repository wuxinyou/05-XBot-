/*******************************************************Copyright*********************************************************
**                                            北京博创兴盛机器人技术有限公司
**                                                       研发部
**                                               http://robot.up-tech.com
**
**-------------------------------------------------------文件信息---------------------------------------------------------
** 文件名称:			TcInit.c
** 最后修订日期:  	2010-04-12
** 最后版本:			1.0
** 描述:				TC初始化
**
**------------------------------------------------------------------------------------------------------------------------
** 创建人:			律晔
** 创建日期:			2010-04-12
** 版本:				1.0
** 描述:				TC初始化
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
#include "Apps/TcInit.h"





/*************************************************************************************************************************
** 函数名称:			InitTCD0
**
** 函数描述:			初始化InitTCD0,启动溢出中断，中断周期为1ms;
**
**
** 输入变量:			void;
** 返回值:			void;
**
** 使用宏或常量:		None;
** 使用全局变量:		None;
**
** 调用函数:			None;
**
** 创建人:			律晔
** 创建日期:			2010-04-12
**------------------------------------------------------------------------------------------------------------------------
** 修订人:
** 修订日期:
**------------------------------------------------------------------------------------------------------------------------
*************************************************************************************************************************/
void InitTCD0(void)
{
	/* Set period ( TOP value ). */
	TC_SetPeriod( &TCD0, (0x01F4 - 1) );

	/* Enable overflow interrupt at low level */
	TC0_SetOverflowIntLevel( &TCD0, TC_OVFINTLVL_LO_gc );

	/* Configure the TC for single slope mode. */
	//TC0_ConfigWGM( &TCD0, TC_WGMODE_SS_gc );

	/* Enable Compare channel A. */
//	TC0_EnableCCChannels( &TCD0, TC0_CCAEN_bm );

//	TC_SetCompareA( &TCD0, 0x00FF );

	/* Enable CCA interrupt. */
//	TC0_SetCCAIntLevel( &TCD0, TC_CCAINTLVL_LO_gc );

	/* Start Timer/Counter. */
//	TC0_ConfigClockSource( &TCD0, TC_CLKSEL_DIV64_gc );
}





/*************************************************************************************************************************
** 函数名称:			InitTCD1
**
** 函数描述:			初始化InitTCD0,启动溢出中断，中断周期为1ms;
**
**
** 输入变量:			void;
** 返回值:			void;
**
** 使用宏或常量:		None;
** 使用全局变量:		None;
**
** 调用函数:			None;
**
** 创建人:			律晔
** 创建日期:			2010-04-12
**------------------------------------------------------------------------------------------------------------------------
** 修订人:
** 修订日期:
**------------------------------------------------------------------------------------------------------------------------
*************************************************************************************************************************/
void InitTCD1(void)
{
	/* Set period ( TOP value ). */
	TC_SetPeriod( &TCD1, (0x07D0 - 1) );

	/* Enable overflow interrupt at low level */
	TC1_SetOverflowIntLevel( &TCD1, TC_OVFINTLVL_LO_gc );

	/* Start Timer/Counter. */
	//
}


/*************************************************************************************************************************
**														结构体声明
*************************************************************************************************************************/


/*************************************************************************************************************************
**                                                      文件结束
*************************************************************************************************************************/
