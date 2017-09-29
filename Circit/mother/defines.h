/*	defines.h
 *	2017年度高専ロボコン 釧路高専Bチーム 定義領域
 */
#ifndef _predefine_
#define _predefine_

#include <16F886.h>

#fuses INTRC_IO,NOWDT,NOPROTECT,PUT,NOMCLR,NOLVP,BROWNOUT
#use delay(CLOCK=8000000)
#use fast_io(a)//先に宣言する。
#use fast_io(b)
#use fast_io(c)
#use fast_io(e)
/*	RS232(USART)モジュール使用
 *	速度			9600Hz
 *	受信ピン		C7
 *	送信ピン		C6
 *	エラー記録	有効
 */	#use RS232(BAUD=9600,RCV=PIN_C7,XMIT=PIN_C6,ERRORS,FORCE_SW)
#byte port_a = 5
#byte port_b = 6
#byte port_c = 7
#byte port_e = 9
#byte INTCON = 0x0B

/*	Program
 *	コンパイル制御による制御プログラムの選択
 */	#define Program	0

/* 共通設定項目 */
	/*	F_TIME
	 *	LED試験点灯のLED点灯時間
	 */	#define F_TIME	200
	/*	LED_PULLUP
	 *	LED1のプルアップ使用フラグ
	 */	#define LED_PULLUP			   0
	/*	LED_OPR
	 *	オペレーションLEDのピン設定
	 */	#define LED_OPR				PIN_C5
	/*	LED_F1
	 *	LED1のピン設定
	 */	#define LED_F1				PIN_C0
	/*	LED_F1
	 *	LED1のピン設定
	 */	#define LED_F2				PIN_C1
	/*	LED_F1
	 *	LED1のピン設定
	 */	#define LED_F3				PIN_C2
	/*	EMITRULE_LED_F1
	 *	LED1の点灯条件
	 */	#define EMITRULE_LED_F1		0
	/*	EMITRULE_LED_F1
	 *	LED1の点灯条件
	 */	#define EMITRULE_LED_F2		(rcv && !PS2_PUSH_R1)
	/*	EMITRULE_LED_F1
	 *	LED1の点灯条件
	 */	#define EMITRULE_LED_F3		(motor_isemit(pwr))
#if Program == 1
/* ロボット1"サヴァニン" 設定項目 */
	/*	MOTOR_NUM
	 *	使用モーター数
	 */	#define MOTOR_NUM	4
	/*	MOTOR_MOVER
	 *	タイヤ右
	 */	#define MOTOR_MOVER 'R'
	/*	MOTOR_MOVEL
	 *	タイヤ左
	 */	#define MOTOR_MOVEL 'L'
	/*	MOTOR_ARM
	 *	竿
	 */	#define MOTOR_ARM	'A'
	/*	MOTOR_AIR
	 *	エアー
	 */	#define MOTOR_AIR	'B'
	/*	PWR_MOV_TABLE
	 *	移動に用いるパワー
	 */	#define PWR_MOV_TABLE	{-100, -30, 0, 30, 100}
	/*	PWR_AIR
	 *	エアシリンダ駆動に用いるパワー
	 */	#define PWR_AIR	100
	/*	PWR_ARM
	 *	アーム駆動に用いるパワー
	 */	#define PWR_ARM	100
	/*	LOOP_DELAY
	 *	プログラムループの待ち時間
	 */	#define LOOP_DELAY	0
	/*	LOOP_DELAYUNIT_US
	 *	プログラムループの待ち時間の単位をusにするかどうか
	 */	#define LOOP_DELAYUNIT_US	1
	/*	TIME_MOTOR_MARGIN
	 *	モータードライバに信号を送信した直後の待ち時間
	 */	#define TIME_MOTOR_MARGIN	0
	/*	DATA_BUFFER_SIZE
	 *	受信データのバッファサイズ
	 */	#define PS2_DATA_BUFFER_SIZE		  32
	/*	RCV_THRESHOLD
	 *	PS2の信号が途絶えた際に通信断絶状態と判定するための閾値
	 */	#define RCV_THRESHOLD		   3
	#define ANALOG_ENABLE	0
	#define ANALOG_THRESHOLD	80
#elif Program == 2
/* ロボット2"アレニン" 設定項目 */
	/*	MOTOR_NUM
	 *	使用モーター数
	 */	#define MOTOR_NUM	3
	/*	MOTOR_MOVER
	 *	タイヤ右
	 */	#define MOTOR_MOVER 'R'
	/*	MOTOR_MOVEL
	 *	タイヤ左
	 */	#define MOTOR_MOVEL 'L'
	/*	MOTOR_ARM
	 *	竿
	 */	#define MOTOR_ARM	'A'
	/*	PWR_MOV_TABLE
	 *	移動に用いるパワー
	 */	#define PWR_MOV_TABLE	{-100, -30, 0, 30, 100}
	/*	PWR_ARM
	 *	アーム駆動に用いるパワー
	 */	#define PWR_ARM	100
	/*	LOOP_DELAY
	 *	プログラムループの待ち時間
	 */	#define LOOP_DELAY	0
	/*	LOOP_DELAYUNIT_US
	 *	プログラムループの待ち時間の単位をusにするかどうか
	 */	#define LOOP_DELAYUNIT_US	1
	/*	TIME_MOTOR_MARGIN
	 *	モータードライバに信号を送信した直後の待ち時間
	 */	#define TIME_MOTOR_MARGIN	0
	/*	DATA_BUFFER_SIZE
	 *	受信データのバッファサイズ
	 */	#define PS2_DATA_BUFFER_SIZE		  32
	/*	RCV_THRESHOLD
	 *	PS2の信号が途絶えた際に通信断絶状態と判定するための閾値
	 */	#define RCV_THRESHOLD		   3
	#define ANALOG_ENABLE	0
	#define ANALOG_THRESHOLD	80
#else
/* デフォルト値 */
	/*	MOTOR_NUM
	 *	使用モーター数
	 */	#define MOTOR_NUM	0
	#define PWR_MOV_TABLE	{0}
	/*	LOOP_DELAY
	 *	プログラムループの待ち時間
	 */	#define LOOP_DELAY	0
	/*	LOOP_DELAYUNIT_US
	 *	プログラムループの待ち時間の単位をusにするかどうか
	 */	#define LOOP_DELAYUNIT_US	1
	/*	TIME_MOTOR_MARGIN
	 *	モータードライバに信号を送信した直後の待ち時間
	 */	#define TIME_MOTOR_MARGIN	0
	/*	DATA_BUFFER_SIZE
	 *	受信データのバッファサイズ
	 */	#define PS2_DATA_BUFFER_SIZE		  32
	/*	RCV_THRESHOLD
	 *	PS2の信号が途絶えた際に通信断絶状態と判定するための閾値
	 */	#define RCV_THRESHOLD		   3
	#define ANALOG_ENABLE	0
	#define ANALOG_THRESHOLD	80

#endif

#endif
/* defines.h EOF */