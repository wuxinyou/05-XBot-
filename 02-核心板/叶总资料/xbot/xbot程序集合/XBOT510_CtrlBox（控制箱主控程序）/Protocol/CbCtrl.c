


/*******************************************************Copyright*********************************************************
**                                            北京博创兴盛机器人技术有限公司
**                                                       研发部
**                                               http://www.uptech-eod.com
**
**-------------------------------------------------------文件信息---------------------------------------------------------
** 文件名称:			CbIoCtrl.c
** 最后修订日期:		2010-04-09
** 最后版本:			1.0
** 描述:				用于有线与无线控制的协议解析。
**
**------------------------------------------------------------------------------------------------------------------------
** 创建人:			吴琳
** 创建日期:			2010-04-09
** 版本:				1.0
** 描述:				解析来自于控制箱的协议，同时反馈相当的控制信息。
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

#include "Protocol/CbCtrl.h"


//A5 00 00 00 00 00 00 00 0F 00 00 87 3B
const uint8_t CB_PREPARE_DATA[CB_TXD_LEN]= {
CB_FIRE_START,	0X00,	0X00,	0X00,	0X00,
0X00,	0X00,	0X00,	0X0F,	0X00,	0X00,	0X87
};

//A5 50 AA BB CC DD EE FF 11 22 33 44 XX
const uint8_t CB_FIRE_DATA[CB_TXD_LEN]= {
CB_FIRE_START,	0X50,	0XAA,	0XBB,	0XCC,
			0XDD,	0XEE,	0XFF,	0X11,	0X22, 0X33,0X44
};

static void CbCtrlInitDevicesPre(void)
{
	cli();

	InitClkSys();

	/* 用于LCD屏幕控制 CB_LCD */
	InitUsartC1(19200);

	/* 用于无线控制，CB_RC 	*/
	InitUsartC0(19200);

	/* 用于有线控制, CB_RC1 	*/
	InitUsartD0(19200);


	/* 用于串行数据接收控制 */
	InitTCD0();

	/* 用于串行数据发送控制 */
	InitTCD1();


	/* Enable all interrupt levels. */
	PMIC_SetVectorLocationToApplication();
	PMIC_EnableLowLevel();
	PMIC_EnableMediumLevel();
	PMIC_EnableHighLevel();

	/**********IO输入口设置操作**************/
	//	PC0  	IN_Aim   		瞄准控制
	//	PC1		IN_Aim_Lock		开火钥匙
	//	PC4		IN_Fire			开火控制
	PORTC.DIRCLR  = PIN0_bm|PIN1_bm|PIN4_bm|PIN5_bm;
	PORTC.OUTSET  = PIN0_bm|PIN1_bm|PIN4_bm|PIN5_bm;

	PORTD.DIRCLR  = PIN5_bm|PIN6_bm|PIN7_bm;
	PORTD.OUTSET  = PIN5_bm|PIN6_bm|PIN7_bm;

	PORTE.DIRCLR  = PIN3_bm|PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm;
	PORTE.OUTSET  = PIN3_bm|PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm;

	PORTF.DIRCLR  = 0XFF;
	PORTF.OUTSET  = 0XFF;


	/**********IO输出口设置操作**************/

	PORTD.DIRSET  = PIN0_bm;
	PORTD.OUTCLR  = PIN0_bm;

	PORTE.DIRSET  = PIN0_bm|PIN1_bm|PIN2_bm;
	PORTE.OUTSET  = PIN0_bm|PIN1_bm|PIN2_bm;


	/* 设备总线的RS485发送端控制，设置输出，并使能无效 */
	PORTD.DIRSET  = PIN1_bm;
	PORTD.OUTCLR  = PIN1_bm;


	/* Enable global interrupts. */
	sei();
}

static void CbCtrlInit(void)
{
	CbCtrlInitDevicesPre();

	InitAdcA();
	InitAdcB();

	_delay_ms(1000);

	TC1_ConfigClockSource( &TCD1, TC_CLKSEL_DIV64_gc );
	TC0_ConfigClockSource( &TCD0, TC_CLKSEL_DIV64_gc );

}


#define HEAD_MID		0X80
#define HEAD_MID1		0X60
#define HEAD_MID2		0XA0

#define HEAD_MAX_SPEED	0X0007


static uint8_t GetHeadSpeed(uint8 InputAdc)
{

	if(InputAdc<HEAD_MID)
		InputAdc = HEAD_MID1 - InputAdc;
	else
		InputAdc = InputAdc - HEAD_MID2;

	InputAdc >>= 0x03;
	InputAdc += 0x01;

	if(InputAdc > 0x07)
		InputAdc = 0x07;
	else if(InputAdc < 0x02)
		InputAdc = 0x02;

	return InputAdc;
}

static void CbIoToArmCtlrData(void)
{
//	PE5		IN_HANDA		手爪抓持		Arm6Ctrl
//	PE6		IN_HANDB		手爪松开


//	PE7		IN_HAND_AUTO	手爪自动	*******************************此功能取消
//
//	PF0		IN_JOINT1_A		底座左转		Arm1Ctrl
//	PF1		IN_JOINT1_B		底座右转
//	PF2		IN_JOINT2_A		大臂下俯		Arm2Ctrl
//	PF3		IN_JOINT2_B		大臂上仰
//	PF4		IN_JOINT3_A		小臂上仰		Arm3Ctrl
//	PF5		IN_JOINT3_B		小臂下俯
//	PF6		IN_JOINT4_A		手爪右转		Arm5Ctrl
//	PF7		IN_JOINT4_B		手爪左转



	uint8_t TempStateJoint1,TempStateJoint2,TempStateJoint3,TempStateJoint4,TempStateHand;
	uint16_t TempArmSpeed,TempHandForce,TempMoveZ;

	uint8_t TempHeadX,TempHeadY,TempHeadFocus;
	uint8_t TempChar;

	unsigned char TempPINF,TempPINE;

	TempPINE = PORT_GetPortValue( &PORTE );
	TempPINF = PORT_GetPortValue( &PORTF );

	TempStateJoint1 = TempPINF&0x03;
	TempStateJoint2 = (TempPINF>>2)&0x03;
	TempStateJoint3 = (TempPINF>>4)&0x03;
	TempStateJoint4 = (TempPINF>>6)&0x03;


	CbCtrlData.Arm12356Speed 	= 0x00;
	CbCtrlData.Arm12Ctrl 		= 0x00;
	CbCtrlData.Arm34Ctrl 		= 0x00;
	CbCtrlData.Arm50Ctrl 		= 0x00;
	CbCtrlData.Arm78Ctrl 		= 0x00;
	CbCtrlData.Arm6Ctrl 		= 0x00;

	/********************设置机械臂速度******************/
	TempArmSpeed = adc_a_control.ad0;
	TempArmSpeed >>= 0x09;
	TempArmSpeed += 1;
	if(TempArmSpeed>0x0007)
		TempArmSpeed =0x0007;
	else if(TempArmSpeed<0x0002)
		TempArmSpeed =0x0002;

	//TempArmSpeed = 0x00 为自由状态
	//TempArmSpeed = 0x01 为速度为0状态
	CbCtrlData.Arm12356Speed = TempArmSpeed;


	/******************* ARM1 ****************************/
	//	PF0		IN_JOINT1_A		底座左转
	//	PF1		IN_JOINT1_B		底座右转
	if( TempStateJoint1 == 0x02)			//底盘左转
		CbCtrlData.Arm12Ctrl |= (0x80|(CbCtrlData.Arm12356Speed<<4));
	else if( TempStateJoint1 == 0x01)		//底盘右转
		CbCtrlData.Arm12Ctrl |= (0x00|(CbCtrlData.Arm12356Speed<<4));
	else
		CbCtrlData.Arm12Ctrl |= 0x00;

	/******************** ARM2 ***************************/
	//	PF2		IN_JOINT2_A		大臂下俯
	//	PF3		IN_JOINT2_B		大臂上仰
	if( TempStateJoint2 == 0x02)			//大臂下俯
		CbCtrlData.Arm12Ctrl |= (0x00|(CbCtrlData.Arm12356Speed<<0));
	else if( TempStateJoint2 == 0x01)		//大臂上仰
		CbCtrlData.Arm12Ctrl |= (0x08|(CbCtrlData.Arm12356Speed<<0));
	else
		CbCtrlData.Arm12Ctrl |= 0x00;

	/********************* ARM3 ****************************/
	//	PF4		IN_JOINT3_A		小臂上仰
	//	PF5		IN_JOINT3_B		小臂下俯
	if( TempStateJoint3 == 0x02)			//小臂上仰
		CbCtrlData.Arm34Ctrl |= (0x00|(CbCtrlData.Arm12356Speed<<4));
	else if( TempStateJoint3 == 0x01)		//小臂下俯
		CbCtrlData.Arm34Ctrl |= (0x80|(CbCtrlData.Arm12356Speed<<4));
	else
		CbCtrlData.Arm34Ctrl |= 0x00;

	/*****************************************************/
	/**************云台摇杆旋转控制机器人腕关节 ARM4 *******/
	TempHeadFocus = adc_a_control.ad6>>4;
	TempChar = GetHeadSpeed(TempHeadFocus);

	if(TempHeadFocus<HEAD_MID1)
		CbCtrlData.Arm34Ctrl |= (0x08|(TempChar<<0));			//下俯，逆时针
	else if(TempHeadFocus<HEAD_MID2)
		CbCtrlData.Arm34Ctrl |= 0X00;			//静止
	else
		CbCtrlData.Arm34Ctrl |= (0x00|(TempChar<<0));			//上仰，顺时针

	/*****************************************************/
	/**************云台旋转控制 ARM7 *******/
	TempHeadX 	  = adc_a_control.ad5>>4;
	TempChar = GetHeadSpeed(TempHeadX);

	if(TempHeadX<HEAD_MID1)
		CbCtrlData.Arm78Ctrl |= (0x00|(TempChar<<4));			//右旋
	else if(TempHeadX<HEAD_MID2)
		CbCtrlData.Arm78Ctrl |= 0X00;							//静止
	else
		CbCtrlData.Arm78Ctrl |= (0x80|(TempChar<<4));			//左旋

	/*****************************************************/
	/**************云台俯仰控制 ARM8 *******/
	TempHeadY 	  = adc_a_control.ad4>>4;
	TempChar = GetHeadSpeed(TempHeadY);

	if(TempHeadY<HEAD_MID1)
		CbCtrlData.Arm78Ctrl |= (0x08|(TempChar<<0));			//下俯
	else if(TempHeadY<HEAD_MID2)
		CbCtrlData.Arm78Ctrl |= 0X00;							//静止
	else
		CbCtrlData.Arm78Ctrl |= (0x00|(TempChar<<0));			//上仰


	/******************** ARM5 ****************************/
	//	PF6		IN_JOINT4_A		手爪右转
	//	PF7		IN_JOINT4_B		手爪左转
	if( TempStateJoint4 == 0x02)			//手爪右转
		CbCtrlData.Arm50Ctrl |= (0x00|(CbCtrlData.Arm12356Speed<<4));
	else if( TempStateJoint4 == 0x01)		//手爪左转
		CbCtrlData.Arm50Ctrl |= (0x80|(CbCtrlData.Arm12356Speed<<4));
	else
		CbCtrlData.Arm50Ctrl |= 0x00;

	/*****************************************************/
	/**************前支臂 ARM0 *******/
	TempMoveZ 	  = adc_a_control.adb1>>4;
	TempChar = GetHeadSpeed(TempMoveZ);

	if(TempMoveZ<HEAD_MID1)
		CbCtrlData.Arm50Ctrl |= (0x00|(TempChar<<0));			//下俯，逆时针
	else if(TempMoveZ<HEAD_MID2)
		CbCtrlData.Arm50Ctrl |= 0X00;							//静止
	else
		CbCtrlData.Arm50Ctrl |= (0x08|(TempChar<<0));			//上仰，顺时针


	/*********************手爪抓持***************************/
	//	PE5		IN_HANDA		手爪抓持
	//	PE6		IN_HANDB		手爪松开
	TempStateHand   = (TempPINE>>5)&0x03;

	if( TempStateHand == 0x02)			//手爪抓持
		CbCtrlData.Arm6Ctrl |= (0x80|(CbCtrlData.Arm12356Speed<<4));
	else if( TempStateHand == 0x01)		//手爪松开
		CbCtrlData.Arm6Ctrl |= (0x00|(CbCtrlData.Arm12356Speed<<4));
	else
		CbCtrlData.Arm6Ctrl |= 0x00;

	//设置抓持力量
	TempHandForce = 0x0F;
	CbCtrlData.Arm6Ctrl |= (TempHandForce&0x0F);
}

#define MOVE_X_MID		0X0200
#define MOVE_X_MID1		(MOVE_X_MID-0x40)
#define MOVE_X_MID2		(MOVE_X_MID+0x40)

#define MOVE_Y_MID		0X0200
#define MOVE_Y_MID1		(MOVE_Y_MID-0x40)
#define MOVE_Y_MID2		(MOVE_Y_MID+0x40)

#define MOVE_Z_MID		0X0200
#define MOVE_Z_MID1		(MOVE_Z_MID-0xC0)
#define MOVE_Z_MID2		(MOVE_Z_MID+0xC0)

#define MOVE_MAX_SPEED	160		//210


#define SPEEDK_DIV		430		//380		//
#define TURN_DIV		430		//330		//

#define ACC_SPEED		200		//30
#define DEC_SPEED		200		//30


static int16_t  GetMoveSpeed(int16_t InputAdc_Y,int16_t InputAdc_X,
							int16_t Input_Adc_Z,int16_t SpeedK)
{
	int32_t TempLong;

	TempLong = Input_Adc_Z;

	TempLong = InputAdc_Y+InputAdc_X;
	TempLong  *= SpeedK;
	TempLong  /= SPEEDK_DIV;

	if(TempLong > MOVE_MAX_SPEED)
		TempLong = MOVE_MAX_SPEED;
	else if(TempLong < -MOVE_MAX_SPEED)
		TempLong = -MOVE_MAX_SPEED;

	return TempLong;
}

static void SetAccSpeed(int16_t TempSpeed,int16_t* p_SetSpeed)
{
	if( TempSpeed==0)
	{
		if((*p_SetSpeed)<0)
		{
			if((*p_SetSpeed)<(-DEC_SPEED))
				(*p_SetSpeed) += DEC_SPEED;
			else
				(*p_SetSpeed) = 0;
		}
		else
		{
			if((*p_SetSpeed)>DEC_SPEED)
				(*p_SetSpeed) -= DEC_SPEED;
			else
				(*p_SetSpeed) = 0;
		}
	}
	else if( TempSpeed>0 && (*p_SetSpeed)>=0)
	{
		if(TempSpeed>(*p_SetSpeed))
		{
			if(TempSpeed-(*p_SetSpeed)>ACC_SPEED)
				(*p_SetSpeed) += ACC_SPEED;
			else
				(*p_SetSpeed) = TempSpeed;
		}
		else
		{
			if((*p_SetSpeed)-TempSpeed>DEC_SPEED)
				(*p_SetSpeed) -= DEC_SPEED;
			else
				(*p_SetSpeed) = TempSpeed;
		}
	}
	else if( TempSpeed<0 && (*p_SetSpeed)<0)
	{
		if(TempSpeed<(*p_SetSpeed))
		{
			if((*p_SetSpeed)-TempSpeed>ACC_SPEED)
				(*p_SetSpeed) -= ACC_SPEED;
			else
				(*p_SetSpeed) = TempSpeed;
		}
		else
		{
			if(TempSpeed-(*p_SetSpeed)>DEC_SPEED)
				(*p_SetSpeed) += DEC_SPEED;
			else
				(*p_SetSpeed) = TempSpeed;
		}
	}
	else
	{
		if(TempSpeed-(*p_SetSpeed)<ACC_SPEED && TempSpeed-(*p_SetSpeed)> -ACC_SPEED)
		{
			(*p_SetSpeed) = TempSpeed;
		}
		else
		{
			if(TempSpeed>(*p_SetSpeed))
				(*p_SetSpeed) += ACC_SPEED;
			else
				(*p_SetSpeed) -= ACC_SPEED;
		}

	}
}

static void CbIoToMotionCtlrData(void)
{
//	B-ADC0	MOVE_X			运动左右控制
//	A-ADC7	MOVE_Y			运动前后控制
//	B-ADC1	MOVE_Z			运动旋转控制
//	B-ADC2	MOVE_SPEED		运动速度控制

	int16_t TempMoveX,TempMoveY,TempMoveK,TempMoveZ;
	int16_t TempSpeedL = 0,TempSpeedR = 0;
	static int16_t SetSpeedL = 0,SetSpeedR = 0;

	TempMoveX 	  = adc_a_control.adb0>>2;
	TempMoveY 	  = adc_a_control.ad7>>2;
	TempMoveZ 	  = adc_a_control.adb1>>2;
	TempMoveK 	  = adc_a_control.adb2>>4;



	/**************修正X轴AD值*******************/
	if(TempMoveX<MOVE_X_MID1)
		TempMoveX = (TempMoveX-MOVE_X_MID1);
	else if(TempMoveX<MOVE_X_MID2)
		TempMoveX = 0x00;
	else
		TempMoveX = (TempMoveX-MOVE_X_MID2);

	/**************修正Y轴AD值*******************/
	if(TempMoveY<MOVE_Y_MID1)
		TempMoveY = (TempMoveY-MOVE_Y_MID1);
	else if(TempMoveY<MOVE_Y_MID2)
		TempMoveY = 0x00;
	else
		TempMoveY = (TempMoveY-MOVE_Y_MID2);


	/**************修正Z轴AD值*******************/
	if(TempMoveZ<MOVE_Z_MID1)
		TempMoveZ = (TempMoveZ-MOVE_Z_MID1);
	else if(TempMoveZ<MOVE_Z_MID2)
		TempMoveZ = 0x00;
	else
		TempMoveZ = TempMoveZ-MOVE_Z_MID2;

	TempSpeedL = GetMoveSpeed( TempMoveY ,0-TempMoveX/4 ,  TempMoveZ,		TempMoveK);
	TempSpeedR = GetMoveSpeed( TempMoveY ,TempMoveX/4, 0-TempMoveZ,	TempMoveK);

	SetAccSpeed(TempSpeedL,&SetSpeedL);
	SetAccSpeed(TempSpeedR,&SetSpeedR);

//	if(SetSpeedL>160)
//		SetSpeedL=160;
//	else if(SetSpeedL<-160)
//		SetSpeedL=-160;
//
//	if(SetSpeedR>160)
//		SetSpeedR=160;
//	else if(SetSpeedR<-160)
//		SetSpeedR=-160;

	TempSpeedL = SetSpeedL;
	TempSpeedR = SetSpeedR;

//	TempSpeedL = adc_a_control.adb0>>4;
//	TempSpeedR = TempMoveY>>2;


	/**************设置左电机速*******************/
	if(TempSpeedL==0x00)
	{
		CbCtrlData.LeftMoveCtrl 	= 0x00;
		CbCtrlData.LeftMoveSpeed	= 0x00;
	}
	else if(TempSpeedL>0x00)
	{
		CbCtrlData.LeftMoveCtrl 	= 0x01;
		CbCtrlData.LeftMoveSpeed	= TempSpeedL;
	}
	else
	{
		CbCtrlData.LeftMoveCtrl 	= 0x02;
		CbCtrlData.LeftMoveSpeed	= 0-TempSpeedL;
	}
	/**************设置右电机速*******************/
	if(TempSpeedR==0x00)
	{
		CbCtrlData.RightMoveCtrl 	= 0x00;
		CbCtrlData.RightMoveSpeed	= 0x00;
	}
	else if(TempSpeedR>0x00)
	{
		CbCtrlData.RightMoveCtrl 	= 0x01;
		CbCtrlData.RightMoveSpeed	= TempSpeedR;
	}
	else
	{
		CbCtrlData.RightMoveCtrl 	= 0x02;
		CbCtrlData.RightMoveSpeed	= 0-TempSpeedR;
	}

}

static void CbIoToCameraCtlrData(void)
{
//	PD5		IN_CAM0			图像通道编码输入0
//	PD6		IN_CAM1			图像通道编码输入1
//	PD7		IN_CAM2			图像通道编码输入2
//
//	PE0		OUT_CAM0		图像通道LED编码控制0
//	PE1		OUT_CAM1		图像通道LED编码控制1
//	PE2		OUT_CAM2		图像通道LED编码控制2

//	A0 A1 A2 A3 A4
//	M1 M4 M3 M5 M2

//	A0 A1 A2 A3 A4
//	L1 L2 L5 L4 L3

	static uint8_t TempCamIn,TempCamOut=0X02;
	static int16_t TempZommVal;
	static uint8_t PreCamIn = 0x04;

	CbCtrlData.CamZoomCtrl = 0x00;

	TempCamIn = PORT_GetPortValue( &PORTD );
	TempCamIn >>= 0X05;

	switch(TempCamIn)
	{
	case 0x06:			//云台图像 1
		PreCamIn = 0x00;
		TempCamOut = 0X00;
		break;
	case 0x02:			//前视图像 2
		PreCamIn = 0x01;
		TempCamOut = 0X01;
		break;
	case 0x04:			//手爪图像 3
		PreCamIn = 0x02;
		TempCamOut = 0X04;
		break;
	case 0x05:			//后视图像 4
		PreCamIn = 0x03;
		TempCamOut = 0X03;
		break;
	case 0x03:			//四画面图像 5
		PreCamIn = 0x04;
		TempCamOut = 0X02;
		break;

	}

	PORTE.OUTCLR  = 0X07;
	PORTE.OUTSET  = TempCamOut;

	CbCtrlData.CamZoomCtrl |= (PreCamIn<<5);

	/********************设置云台焦距*******************/
	TempZommVal = adc_a_control.ad1;
	if(TempZommVal>544&&TempZommVal<3584)
	{
		TempZommVal-=544;
	}
	else if(TempZommVal<=544)
	{
		TempZommVal=0;
	}
	else if(TempZommVal>=3584)
	{
		TempZommVal=3039;
	}
	//去抖动
	if(TempZommVal%95<20)//在死区外
	{
		TempZommVal /=95;
	}
	else
	{
		if((int16_t)(TempZommVal/95)!=(int16_t)((int16_t)(CbCtrlData.CamZoomCtrl|0x1f)-1))
		{
			TempZommVal /=95;
		}
	}

//	if(TempZommVal>31)
//		TempZommVal = 31;
//	TempZommVal = ((TempZommVal-0x02)*0x1F)/0X1D;//什么意思？？？？

	if(TempZommVal<0)
		TempZommVal = 0;
	else if(TempZommVal>0x1F)
		TempZommVal = 0X1F;
	CbCtrlData.CamZoomCtrl |= TempZommVal;
}

static void CbIoToLightCtlrData(void)
{
//	PC0  	IN_Aim   		瞄准控制

//	PE3		IN_HD_LIGHT		云台灯光控制
//	PE4		IN_MV_LIGHT		行使灯光控制

	static uint8_t TempAim,TempCtrl,TempStateHand;

	TempAim = PORT_GetPortValue( &PORTC );
	TempAim &= 0X01;

	TempCtrl = PORT_GetPortValue( &PORTE );
	TempCtrl >>= 0X03;
	TempCtrl  &= 0X03;

	TempStateHand   = (PORT_GetPortValue( &PORTE )>>5)&0x07;

	CbCtrlData.ExCtrl = 0x00;
	CbCtrlData.SwCtrl &= 0x1f;

	if(TempAim == 0X01)							//瞄准控制
		CbCtrlData.SwCtrl &= ~(1<<6);
	else
		CbCtrlData.SwCtrl |= (1<<6);

	if( (TempStateHand&0x04) != 0x04)			//云台、手抓、前向灯光照明
		CbCtrlData.SwCtrl |= (0x80|0x20);
	else
		CbCtrlData.SwCtrl &= ~(0x80|0x20);

	if( TempCtrl == 0x01)			//行车摄像头俯
		CbCtrlData.ExCtrl |= 0x80;
	else if( TempCtrl == 0x02)		//行车摄像头仰
		CbCtrlData.ExCtrl |= 0x40;
	else
		CbCtrlData.ExCtrl |= 0x00;
}

#define FireDelayTime 24

static void CbIoToFireCtlrData(void)
{
//	PC1		IN_Aim_Lock		开火钥匙
//	PC4		IN_Fire			开火控制
	static uint8_t TempAimLock,TempFire;
	static bool fireOk=false;
	static uint16_t TempFireTick = 0;

	TempAimLock = (PORT_GetPortValue( &PORTC )>>1)&0x01;//钥匙按钮
	TempFire    = (PORT_GetPortValue( &PORTC )>>4)&0x01;//开火按钮

	if(TempAimLock == 0x01) //钥匙按钮未打开
	{
		CbCtrlData.FireState = FIRE_LOCK;
		TempFireTick = 0;
		fireOk=false;
	}
	else //钥匙按钮打开
	{
		if(TempFire == 0x01)//开火按钮未按下
		{
			CbCtrlData.FireState = FIRE_PREPARE; //0x01 准备
			TempFireTick = 0;
		}
		else if(!fireOk)
		{
			if(TempFireTick>FireDelayTime)//开火
			{
				CbCtrlData.FireState = FIRE_START;//0x02
				fireOk=true;
			}
			else//开火倒计时
			{
				TempFireTick++;
				CbCtrlData.FireState = TempFireTick+0x04;
			}
		}
	}
}

static void CbIoToComSwCtrl(void)
{
//	PC5		IN_COM_SW		有线无线切换检测
//
//	PD0		OUT_COM_CTRL	有线无线切换控制
	static uint8_t TempComState;

	TempComState   = PORT_GetPortValue( &PORTC );
	TempComState >>= 0x05;
	TempComState  &= 0x01;

	if(TempComState == 0x01)	//为无线控制
	{
		PORTD.OUTCLR = PIN0_bm;
		CbCtrlData.ComSwState = COM_WIRELESS_EN;
	}
	else						//为有线控制
	{
		PORTD.OUTSET  = PIN0_bm;
		CbCtrlData.ComSwState = COM_WIRE_EN;
	}
}

static void CbStatrOneFrameTxd(void)
{
	uint8_t count = 0;

	if(CbCtrlData.FireState == FIRE_LOCK)
	{
		/********************  帧头 *********************/
		CbCtrlData.TxdDataBuf[0] = CB_TXD_START;

		/*************  反馈设备号与左右电机控制   **************/
//		if(CbCtrlData.DevID > CB_DEVID_MAX)
//			CbCtrlData.DevID = 1;
//		else
//			CbCtrlData.DevID++;

		CbCtrlData.DevID = 0x00;

		CbCtrlData.TxdDataBuf[1] = CbCtrlData.DevID<<4;

		/****************  相关控制指令  ****************/

		CbCtrlData.TxdDataBuf[1] |= (CbCtrlData.LeftMoveCtrl<<2)|CbCtrlData.RightMoveCtrl;
		CbCtrlData.TxdDataBuf[2] = CbCtrlData.LeftMoveSpeed;
		CbCtrlData.TxdDataBuf[3] = CbCtrlData.RightMoveSpeed;

		CbCtrlData.TxdDataBuf[9] = 0x00;
		if(CbCtrlData.Arm18WorkMode)										//机械臂控制模式
		{
			CbCtrlData.TxdDataBuf[9] |= (1<<2);
			CbCtrlData.TxdDataBuf[4] &= 0XF0;
			CbCtrlData.TxdDataBuf[4] |= (CbCtrlData.Arm18WorkMode&0x07);	//机械臂0关节控制

			CbCtrlData.TxdDataBuf[5] = CbCtrlData.Arm34Ctrl;				//机械臂34关节控制
			CbCtrlData.TxdDataBuf[6] &= 0X0F;
			CbCtrlData.TxdDataBuf[6] |= (CbCtrlData.Arm50Ctrl&0XF0);		//机械臂5关节控制
			CbCtrlData.TxdDataBuf[7] = CbCtrlData.Arm78Ctrl;				//机械臂78关节控制
		}
		else
		{
			CbCtrlData.TxdDataBuf[9] &= ~(1<<2);
			CbCtrlData.TxdDataBuf[4] = CbCtrlData.Arm12Ctrl;				//机械臂12关节控制
			CbCtrlData.TxdDataBuf[5] = CbCtrlData.Arm34Ctrl;				//机械臂34关节控制
			CbCtrlData.TxdDataBuf[6] &= 0X0F;
			CbCtrlData.TxdDataBuf[6] |= (CbCtrlData.Arm50Ctrl&0XF0);		//机械臂5关节控制
			CbCtrlData.TxdDataBuf[7] = CbCtrlData.Arm78Ctrl;				//机械臂78关节控制
			if(CbCtrlData.YtWorkMode)										//收放线模式
			{
				CbCtrlData.TxdDataBuf[9] = CbCtrlData.YtWorkMode;
//				CbCtrlData.TxdDataBuf[9] |= (1<<1);
//				CbCtrlData.TxdDataBuf[7] = CbCtrlData.YtWorkMode;
			}
//			else
//			{
//				CbCtrlData.TxdDataBuf[9] &= ~(1<<1);
//				CbCtrlData.TxdDataBuf[7] = CbCtrlData.Arm78Ctrl;				//机械臂78关节控制
//			}
		}

		if(CbCtrlData.Arm0WorkMode)
		{
			CbCtrlData.TxdDataBuf[9] |= (1<<3);
			CbCtrlData.TxdDataBuf[6] &= 0XF0;
			CbCtrlData.TxdDataBuf[6] |= (CbCtrlData.Arm0WorkMode&0x07);			//机械臂0关节控制
		}
		else
		{
			CbCtrlData.TxdDataBuf[9] &= ~(1<<3);
			CbCtrlData.TxdDataBuf[6] &= 0XF0;
			CbCtrlData.TxdDataBuf[6] |= (CbCtrlData.Arm50Ctrl&0x0F);			//机械臂0关节控制
		}

		CbCtrlData.SwCtrl&= 0xE0;
		CbCtrlData.SwCtrl|=(CbCtrlData.KeyVal&0x1F);					//加上小液晶屏的按键量
		CbCtrlData.TxdDataBuf[8] = CbCtrlData.Arm6Ctrl;					//机械臂6关节控制
		CbCtrlData.TxdDataBuf[9] &= 0X0F;
		CbCtrlData.TxdDataBuf[9] |= (CbCtrlData.ExCtrl&0xF0);			//其它控制信号
		CbCtrlData.TxdDataBuf[10] = CbCtrlData.SwCtrl;					//开关量控制信号
		CbCtrlData.TxdDataBuf[11] = CbCtrlData.CamZoomCtrl;				//图像及焦距设置

		if(CbCtrlData.TestMode == TEST_NO_LIMIT)
		{
			CbCtrlData.TxdDataBuf[1] = 0x30;
			CbCtrlData.TxdDataBuf[2] = 0x11;
			CbCtrlData.TxdDataBuf[3] = 0x22;
			CbCtrlData.TxdDataBuf[4] = 0x33;
			CbCtrlData.TxdDataBuf[5] = 0x44;
			CbCtrlData.TestMode = TEST_NC;

		}
		else if(CbCtrlData.TestMode == TEST_SET_ZERO)
		{
			CbCtrlData.TxdDataBuf[1] = 0x30;
			CbCtrlData.TxdDataBuf[2] = 0x55;
			CbCtrlData.TxdDataBuf[3] = 0x66;
			CbCtrlData.TxdDataBuf[4] = 0x77;
			CbCtrlData.TxdDataBuf[5] = 0x88;
			CbCtrlData.TestMode = TEST_NC;
		}


	}
	else if(CbCtrlData.FireState == FIRE_PREPARE)
	{
		for(count=0;count<CB_TXD_LEN-1;count++)
			CbCtrlData.TxdDataBuf[count] = CB_PREPARE_DATA[count];
			CbCtrlData.TxdDataBuf[10] = CbCtrlData.SwCtrl;					//开关量控制信号
			CbCtrlData.TxdDataBuf[11] = CbCtrlData.CamZoomCtrl;				//图像及焦距设置
	}
	else if(CbCtrlData.FireState == FIRE_START)
	{
		for(count=0;count<CB_TXD_LEN-1;count++)
			CbCtrlData.TxdDataBuf[count] = CB_FIRE_DATA[count];
	}


	/******************  求检验和   ******************/
	CbCtrlData.TxdDataBuf[CB_TXD_LEN-1] = 0x00;
	for(count=0;count<CB_TXD_LEN-1;count++)
		CbCtrlData.TxdDataBuf[CB_TXD_LEN-1] += CbCtrlData.TxdDataBuf[count];
}

#define POWER_AD_K0		0
#define POWER_AD_K1		(0X05E3)
#define POWER_AD_K2		(0X0750)
#define POWER_AD_K3		(0X08C5)
#define POWER_AD_K4		(0X0A32)

#define POWER_Val_K0	0
#define POWER_Val_K1	80
#define POWER_Val_K2	100
#define POWER_Val_K3	120
#define POWER_Val_K4	140

#define POWER_AD_K10	(POWER_AD_K1-POWER_AD_K0)
#define POWER_AD_K21	(POWER_AD_K2-POWER_AD_K1)
#define POWER_AD_K32	(POWER_AD_K3-POWER_AD_K2)
#define POWER_AD_K43	(POWER_AD_K4-POWER_AD_K3)

#define POWER_Val_K10	(POWER_Val_K1-POWER_Val_K0)
#define POWER_Val_K21	(POWER_Val_K2-POWER_Val_K1)
#define POWER_Val_K32	(POWER_Val_K3-POWER_Val_K2)
#define POWER_Val_K43	(POWER_Val_K4-POWER_Val_K3)




inline static uint16_t CbCtrlAdValToPowerVal(uint16_t AdVal)
{
	uint16_t PowerVal;
	if(AdVal<POWER_AD_K1)
	{
		PowerVal= 0;
	}
	else if(AdVal<POWER_AD_K2)
	{
		PowerVal= (AdVal-POWER_AD_K1)*POWER_Val_K21/POWER_AD_K21+POWER_Val_K1;
	}
	else if(AdVal<POWER_AD_K3)
	{
		PowerVal= (AdVal-POWER_AD_K2)*POWER_Val_K32/POWER_AD_K32+POWER_Val_K2;
	}
	else
	{
		PowerVal= (AdVal-POWER_AD_K3)*POWER_Val_K43/POWER_AD_K43+POWER_Val_K3;
	}
	return PowerVal;
}

#define SUM_TICK	10

void CbSendDataToLcd(void)
{
	static uint32_t SysPower=0,SysPowerSum=0;

	static uint8_t TimeTick = SUM_TICK;

	if(TimeTick>SUM_TICK)
	{
		SysPower = SysPowerSum/(SUM_TICK+1);
		SysPowerSum = 0;
		TimeTick = 0;

		CbCtrlData.CtrlBoxPowerVal = CbCtrlAdValToPowerVal(SysPower);

	}
	else
	{
		TimeTick++;
		SysPowerSum += adc_a_control.ad3;

	}
	CbDisCtrl.pSendDisCtrl(&USART_data_C1,&CbCtrlData);
	CbCtrlData.SendToDisCounter++;
	if(CbCtrlData.SendToDisCounter>10)
		CbCtrlData.SendToDisCounter=0;

//	ADC3	Voltage			系统工作电压
}

//#define ADC_M	26
//#define ADC_C	10

#define ADC_M	185
#define ADC_C	100

void AdjustAdc(void)
{
	uint32_t TempLong;

	TempLong = adc_a_control.ad0;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.ad0 = TempLong;

	TempLong = adc_a_control.ad1;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.ad1 = TempLong;

	TempLong = adc_a_control.ad2;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.ad2 = TempLong;

	TempLong = adc_a_control.ad3;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.ad3 = TempLong;

	TempLong = adc_a_control.ad4;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.ad4 = TempLong;

	TempLong = adc_a_control.ad5;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.ad5 = TempLong;

	TempLong = adc_a_control.ad6;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.ad6 = TempLong;

	TempLong = adc_a_control.ad7;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.ad7 = TempLong;


	TempLong = adc_a_control.adb0;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.adb0 = TempLong;

	TempLong = adc_a_control.adb1;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.adb1 = TempLong;

	TempLong = adc_a_control.adb2;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.adb2 = TempLong;

	TempLong = adc_a_control.adb3;
	TempLong = (TempLong*ADC_M)/ADC_C;
	adc_a_control.adb3 = TempLong;
}

/*******  工作在5ms的定时器内  *******/
#define TXD_ONE_BYTE_TIME	2
#define TXD_ONE_FRAME_TIME	25
static void CbDealCtrl(void)
{
	static uint16 TimeTick = 0;
	static uint16 FrameTimeTick = 0;

//	PORTC.DIRCLR  = PIN0_bm|PIN1_bm|PIN4_bm|PIN5_bm;
//	PORTC.OUTSET  = PIN0_bm|PIN1_bm|PIN4_bm|PIN5_bm;

//	if(FrameTimeTick < CB_TXD_LEN)
//	{
////		if(TimeTick%2 == 0)
////		{
////			if(CbCtrlData.ComSwState == COM_WIRELESS_EN)
////			{
////				while (!USART_TXBuffer_PutByte(&USART_data_C0,CbCtrlData.TxdDataBuf[FrameTimeTick]));
////			}
////			else if(CbCtrlData.ComSwState == COM_WIRE_EN)
////			{
////				CBIOCTRL_TXD_EN;
////				while (!USART_TXBuffer_PutByte(&USART_data_D0,CbCtrlData.TxdDataBuf[FrameTimeTick]));
////
////			}
////
//			FrameTimeTick++;
////		}
//	}
	if(TimeTick <TXD_ONE_FRAME_TIME-1);
	else if(TimeTick == TXD_ONE_FRAME_TIME-1)
		adc_a_control.pStart();
	else if (TimeTick >= TXD_ONE_FRAME_TIME)
	{
		AdjustAdc();

		CbIoToComSwCtrl();				//	OK

		CbIoToArmCtlrData();			//	OK

		CbIoToMotionCtlrData();			//	OK

		CbIoToCameraCtlrData();			//	OK

		CbIoToLightCtlrData();			//	OK

		CbIoToFireCtlrData();			//	OK

		CbStatrOneFrameTxd();

		CbSendDataToLcd();				//发送数据到小液晶屏板子上

		FrameTimeTick = 0;
		TimeTick = 0;

		if(CbCtrlData.ComSwState == COM_WIRELESS_EN)
		{
			uint8_t Index=0x00;
			for(Index=0;Index<CB_TXD_LEN;Index++)
				while (!USART_TXBuffer_PutByte(&USART_data_C0,CbCtrlData.TxdDataBuf[Index]));
		}
		else if(CbCtrlData.ComSwState == COM_WIRE_EN)
		{
			uint8_t Index=0x00;
			for(Index=0;Index<CB_TXD_LEN;Index++)
				while (!USART_TXBuffer_PutByte(&USART_data_D0,CbCtrlData.TxdDataBuf[Index]));

		}
		CbCtrlData.SendToXbotCounter++;
		if(CbCtrlData.SendToXbotCounter>7)
			CbCtrlData.SendToXbotCounter=0;


		//PORTC.OUTTGL  = PIN1_bm| PIN4_bm;	//////////////////////////////////

		//USART_TXBuffer_PutByte(&USART_data_C0,adc_a_control.adb2 >> 4);
	}

	TimeTick++;
}

inline static void CbCtrlRcSetState(CB_CTRL_DATA_STRUCT* pDa)
{
	uint8_t FeedIndex;

	if((pDa->RxdDataBuf[1]&0x80)!=0x80)
		return;

	FeedIndex = pDa->RxdDataBuf[1]&0x03;
	pDa->LeftSpeed=pDa->RxdDataBuf[2];
	pDa->RightSpeed=pDa->RxdDataBuf[3];

	if(FeedIndex == 0x00)
	{
		pDa->Arm1Angle = pDa->RxdDataBuf[4];
		pDa->Arm5Angle = pDa->RxdDataBuf[5];
		pDa->HandWidth = pDa->RxdDataBuf[6];
		pDa->Battery_1 = pDa->RxdDataBuf[7];
		pDa->Error_1   = pDa->RxdDataBuf[8];
		pDa->Error_2   = pDa->RxdDataBuf[9];
	}
	else if(FeedIndex == 0x01)
	{
		pDa->Arm2Angle = pDa->RxdDataBuf[4];
		pDa->Arm6Angle = pDa->RxdDataBuf[5];
		pDa->Arm0Angle = pDa->RxdDataBuf[6];
		pDa->Battery_2 = pDa->RxdDataBuf[7];
		pDa->Error_3   = pDa->RxdDataBuf[8];
		pDa->Error_4   = pDa->RxdDataBuf[9];
	}
	else if(FeedIndex == 0x02)
	{
		pDa->Arm3Angle = pDa->RxdDataBuf[4];
		pDa->Arm7Angle = pDa->RxdDataBuf[5];
		pDa->Wirelength= pDa->RxdDataBuf[6];
		pDa->Battery_3 = pDa->RxdDataBuf[7];
		pDa->Error_5   = pDa->RxdDataBuf[8];
		pDa->Error_6   = pDa->RxdDataBuf[9];
	}
	else
	{
		pDa->Arm4Angle 		= pDa->RxdDataBuf[4];
		pDa->CamPtAngle 	= pDa->RxdDataBuf[5];
		pDa->RobotSelfPowerVal = pDa->RxdDataBuf[6];
		pDa->Battery_4 = pDa->RxdDataBuf[7];
		pDa->Error_7   = pDa->RxdDataBuf[8];
		pDa->Error_8   = pDa->RxdDataBuf[9];
	}
	pDa->ReceiveFormXbotCounter++;
}

/*************************************************************************************************************************
** 函数名称:			CbCtrlDealRcData
**
** 函数描述:
**
**
**
** 输入变量:
** 返回值:			uint8_t;
**
** 使用宏或常量:		None;
** 使用全局变量:		None;
**
** 调用函数:			None;
**
** 创建人:			吴琳
** 创建日期:			2009-03-14
**------------------------------------------------------------------------------------------------------------------------
** 修订人:
** 修订日期:
**------------------------------------------------------------------------------------------------------------------------
*************************************************************************************************************************/
static void CbCtrlDealRcData(USART_data_t* p_usart_data, CB_CTRL_DATA_STRUCT* p_Xbot_receive)
{
	uint8_t TempData,TempCount;

	static uint8_t DataIndex = 0;

	while (USART_RXBufferData_Available(p_usart_data))
	{
		TempData = USART_RXBuffer_GetByte(p_usart_data);

		if (DataIndex == 0X00)
		{
			if(CBDIS_RXD_START != TempData)
			{
				DataIndex = 0;
			}
			else
			{
				p_Xbot_receive->RxdDataBuf[DataIndex] = TempData;
				DataIndex++;
			}
		}
		else if (DataIndex < CB_RXD_LEN-1)
		{
			p_Xbot_receive->RxdDataBuf[DataIndex] = TempData;
			DataIndex++;
		}
		else if (DataIndex == CB_RXD_LEN-1)
		{
			p_Xbot_receive->RxdDataBuf[DataIndex] = TempData;
			DataIndex++;

			/*******    求校验和       *******/
			p_Xbot_receive->RxdCheckSum = 0;

			for(TempCount=0;TempCount<CB_RXD_LEN-1;TempCount++)
			{
				p_Xbot_receive->RxdCheckSum += p_Xbot_receive->RxdDataBuf[TempCount];
			}

			/******* 如果校验和对，解析指令 *******/
			if(p_Xbot_receive->RxdDataBuf[CB_RXD_LEN-1] == p_Xbot_receive->RxdCheckSum )
			{

				/******* 成功解析新的一帧数据 *******/
				CbCtrlRcSetState(p_Xbot_receive);

			}

			/******* 复位，重新开始新的一帧探测 *******/
			DataIndex = 0;
		}
		else
		{
			DataIndex = 0;
		}
	}

}












CB_CTRL_STRUCT CbCtrl =
{

	.CtrlState = CTRL_DIS,

	.p_InitDev = CbCtrlInit,
	.p_DealCtrl = CbDealCtrl,
	.pDearRcData = CbCtrlDealRcData,

//	.pRxdPacket =
//	.pTxdToRpCtrl =
//	.pTxdToRpFire =
};








