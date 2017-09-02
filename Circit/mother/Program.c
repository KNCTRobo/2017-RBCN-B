/*	Program.c
 *	2017年度高専ロボコン Bチームメインプログラム
 *	version	0.90
 */
#include <16F886.h>
#include "defines.h"

#fuses INTRC_IO,NOWDT,NOPROTECT,PUT,NOMCLR,NOLVP,BROWNOUT
#use delay(CLOCK=8000000)
#use fast_io(a)//先に宣言する。
#use fast_io(b)
#use fast_io(c)
#use fast_io(e)
#use RS232(BAUD=9600,RCV=PIN_C7,XMIT=PIN_C6,ERRORS,FORCE_SW)
	//XBeeからの受信->C7ピン  モータードライバへの送信->C6ピン
#byte port_a = 5
#byte port_b = 6
#byte port_c = 7
#byte port_e = 9
#byte INTCON = 0x0B

#define DIGITAL_MODE 0x41
#define ANALOG_MODE 0x73
	//アナログ・デジタルモード
#define BUFFER_SIZE 32

typedef struct
{
	unsigned int X, Y;
} Point;
/*	Analogs
 *		Analogs.sticks
 *		->	傾きの方向フラグ
 *			0x01 右スティック左
 *			0x02 右スティック右
 *			0x04 右スティック上
 *			0x08 右スティック下
 *			0x10 左スティック左
 *			0x20 左スティック右
 *			0x40 左スティック上
 *			0x80 左スティック下
 *		Analogs.depth_L
 *		->	左スティック傾き深度
 *		Analogs.depth_R
 *		->	右スティック傾き深度
 */	typedef struct
	{
		unsigned char sticks;
		Point depth_L;
		Point depth_R;
	} Analogs;

/* プロトタイプ宣言 */
/*	main()
 *	プログラムのメインエントリポイントです
 */	void main();
/*	setup()
 *	プログラムのセットアップセッションです
 */	void setup();
/*	Analogs = gen_Analog(uchar* source, int offset)
 *	コントローラのデータからアナログスティックのデータを抽出します
 *	@params	source	抽出元のデータ
 *	@params	offset	データのオフセット
 *	@return	抽出したデータの構造体が返ります
 */	Analogs gen_Analog(unsigned char source[], int offset);
/*	PS2_dataread(uchar* source)
 *	データを受信し、PS2からのデータとして処理します
 */	void PS2_dataread(unsigned char source[]);
/*	motor_emit(char channel, int power)
 *	モーターに信号を出力します
 *	@params	channel	出力先モータドライバのチャンネル
 *	@params power	モータへの出力
 */	void motor_emit(char channel, int power);
/*	motor_reset()
 *	モータにリセットのための信号を送信します
 */	void motor_reset();
/*	led_reset()
 *	LEDを全消灯します
 */	void led_reset();
/*	led_flash()
 *	LEDの試験点灯を行います
 */	void led_flash();
/* プロトタイプ宣言ここまで */

/* マクロ関数 */
#define PS2_MODE_DIGI	(Data[ofs+0] == 0x41)
#define PS2_MODE_ANLG	(Data[ofs+0] == 0x53)
#define PS2_PUSH_LE		((Data[ofs+2] & 0x80) != 0)
#define PS2_PUSH_DN		((Data[ofs+2] & 0x40) != 0)
#define PS2_PUSH_RI		((Data[ofs+2] & 0x20) != 0)
#define PS2_PUSH_UP		((Data[ofs+2] & 0x10) != 0)
#define PS2_PUSH_START	((Data[ofs+2] & 0x08) != 0)
#define PS2_PUSH_R3		((Data[ofs+2] & 0x04) != 0)
#define PS2_PUSH_L3		((Data[ofs+2] & 0x02) != 0)
#define PS2_PUSH_SELECT	((Data[ofs+2] & 0x01) != 0)
#define PS2_PUSH_SQU	((Data[ofs+3] & 0x80) != 0)
#define PS2_PUSH_CRO	((Data[ofs+3] & 0x40) != 0)
#define PS2_PUSH_CIR	((Data[ofs+3] & 0x20) != 0)
#define PS2_PUSH_TRI	((Data[ofs+3] & 0x10) != 0)
#define PS2_PUSH_R1		((Data[ofs+3] & 0x08) != 0)
#define PS2_PUSH_L1		((Data[ofs+3] & 0x04) != 0)
#define PS2_PUSH_R2		((Data[ofs+3] & 0x02) != 0)
#define PS2_PUSH_L2		((Data[ofs+3] & 0x01) != 0)
/* マクロ関数ここまで */

/* グローバル変数 */
/*	int pwt[]
 *	移動用モータのパワーテーブル
 */	int pwt[]={-PWR_MOV_H, -PWR_MOV_L, 0, PWR_MOV_L, PWR_MOV_H};
/*	char pad_mode
 *	PS2コントローラの状態
 *	あらかじめデジタルにセットしておく
 */	char pad_mode = DIGITAL_MODE;				//最初にパッドの状態をデジタルにセットしておく
/*	uchar Data[BUFFER_SIZE]
 *	PSコントローラからのデータを格納するバッファ
 *	本来受信するデータは8Byteだが、あらかじめバッファを大きく設定しておく
 *	ちなみにPS2から来るデータの最大サイズは32byte
 */	unsigned char Data[BUFFER_SIZE];
/*	int ofs
 *	PS2コントローラから受信したデータのオフセット
 */	int ofs = 0;
/*	long rcv_err
 *	PSコントローラからの通信が途絶えたときに通信断絶状態と判定するための変数
 *	閾値はRCV_THRESHOLDとする
 */	long int rcv_err = RCV_THRESHOLD;
/*	int rcv
 *	PSコントローラからの受信状態フラグ
 */	int rcv = 0;
/* グローバル変数ここまで */

void main()
{
	RESET:								// ソフトリセットのエントリポイント

	/* 初期化 */
	setup();
	int i;								// Index変数
	int f;								// Frame変数
	/*	int level[2]
	 *	移動用モータのレベル一時変数
	 */	int level[2];
	/*	int motor_level[4]
	 *	モータの出力レベル
	 */	int motor_level[4] = {0};
	#if (Program == 2)
	/*	int motor_stat
	 *	刀モータ駆動状態
	 */	int motor_stat = 0;
	#endif
	unsigned char
		motor_buf =0;

	/* モータードライバに0(出力0%)を送信(暴走を防ぐため) */
	motor_reset();

	/* LED試験点灯 */
	led_flash();
	led_reset();
	delay_ms(500);

	/* メインループ */
	for(;;)	//無限ループ
	{

		/* 各変数をリフレッシュ */
		for (i = 0; i < 2; i++)
			level[i] = 0;
		for (i = 0; i < 4; i++)
			motor_level[i] = 0;
		f++;
		if (f >= 2) f = 0;

		/* PSコントローラのデータ処理 */
		if (rcv)
		{
			if (PS2_PUSH_SELECT) goto RESET;	// SELECTキーが押されたらソフトリセット
			if (PS2_PUSH_R1)
			{
				/* ロボット1 */
				#if (Program == 1)
				level[0] = (PS2_PUSH_UP ? (PS2_PUSH_L1 ? 2 : 1) : 0) - (PS2_PUSH_DN ? (PS2_PUSH_L1 ? 2 : 1) : 0);
				level[1] = (PS2_PUSH_UP ? (PS2_PUSH_L1 ? 2 : 1) : 0) - (PS2_PUSH_DN ? (PS2_PUSH_L1 ? 2 : 1) : 0);
				if (PS2_PUSH_LE)
				{
					level[0] = 2;
					level[1] -= 2;
				}
				if (PS2_PUSH_RI)
				{
					level[0] -= 2;
					level[1] = 2;
				}
				motor_level[2] = (PS2_PUSH_TRI ? PWR_ROD : 0) - (PS2_PUSH_CRO ? PWR_ROD : 0);
				motor_level[3] = PS2_PUSH_CIR ? PWR_AIR : 0;
				#endif
				/* ロボット2 */
				#if (Program == 2)
				level[0] = (PS2_PUSH_UP ? (PS2_PUSH_L1 ? 2 : 1) : 0) - (PS2_PUSH_DN ? (PS2_PUSH_L1 ? 2 : 1) : 0);
				level[1] = (PS2_PUSH_UP ? (PS2_PUSH_L1 ? 2 : 1) : 0) - (PS2_PUSH_DN ? (PS2_PUSH_L1 ? 2 : 1) : 0);
				if (PS2_PUSH_LE)
				{
					level[0] = (PS2_PUSH_L1 ? 2 : 1);
					level[1] -= (PS2_PUSH_L1 ? 2 : 1);
				}
				if (PS2_PUSH_RI)
				{
					level[0] -= (PS2_PUSH_L1 ? 2 : 1);
					level[1] = (PS2_PUSH_L1 ? 2 : 1);
				}
				motor_stat = (PS2_PUSH_TRI ? 0 : (PS2_PUSH_CIR ? 1 : motor_stat));
				#endif
			}
		}

		/* 入力内容からデータを処理 */
		motor_level[0] = pwt[level[0]];
		motor_level[1] = pwt[level[1]];
		#if (Program == 2)
		motor_level[2] = motor_stat ? PWR_ROD : 0;
		#endif

		/* LED点灯制御 */
		if (f)
			output_high(LED_OPR);
		else
			output_low(LED_OPR);
		if (EMITRULE_LED_F1 ^ LED_PULLUP)
			output_high(LED_F1);
		else
			output_low(LED_F1);

		/* モータードライバの出力内容を決定、信号を出力する */
		if (rcv && PS2_PUSH_R1)
		{
			if (motor_level[0] || motor_level[1] || (motor_buf&0x01))
			{
				motor_emit(MOTOR_MOVEL, motor_level[0]);
				motor_emit(MOTOR_MOVER, motor_level[1]);
				motor_buf = motor_buf & (!0x01);
				motor_buf = motor_buf | ((motor_level[0] || motor_level[1]) ? 0x01 : 0x00);
			}
			#if (Program == 1)
			if (motor_level[2] || (motor_buf&0x02))
			{
				motor_emit(MOTOR_ROD, motor_level[2]);
				motor_buf = motor_buf & (!0x02);
				motor_buf = motor_buf | ((motor_level[2]) ? 0x02 : 0x00);
			}
			if (motor_level[3] || (motor_buf&0x04))
			{
				motor_emit(MOTOR_AIR, motor_level[3]);
				motor_buf = motor_buf & (!0x04);
				motor_buf = motor_buf | ((motor_level[3]) ? 0x04 : 0x00);
			}
			#endif
			#if (Program == 2)
			if (motor_level[2] || (motor_buf&0x02))
			{
				motor_emit(MOTOR_KATANA, motor_level[2]);
				motor_buf = motor_buf & (!0x02);
				motor_buf = motor_buf | ((motor_level[2]) ? 0x02 : 0x00);
			}
			#endif
		}
		else
		{
			motor_emit(MOTOR_MOVEL, 0);
			motor_emit(MOTOR_MOVER, 0);
			#if (Program == 1)
			motor_emit(MOTOR_ROD, 0);
			motor_emit(MOTOR_AIR, 0);
			#endif
			#if (Program == 2)
			motor_emit(MOTOR_KATANA, 0);
			#endif
		}
		delay_us(LOOP_DELAY);
	}	/* これ以降メインループ内到達できないコード */
}	/* メインルーチンここまで */

void setup(void)
{
	//入力ピンのセット
	set_tris_a(0xC0);
	set_tris_b(0xFF);
	//C7は受信に使用するため該当ビットを1にする
	set_tris_c(0x8D);//XMIT=6,RCV=7

	//オシレータの初期化
	setup_oscillator(OSC_8MHZ);
}//初期化ルーチンおわり-----------------

void led_reset(void)
{
	output_low(LED_OPR);
	output_low(LED_F1);
}

void led_flash(void)
{
	output_high(LED_OPR);
	int i;
	for(i= 5; i; i--)
	{
		output_high(LED_F1);
		delay_ms(F_TIME);
		output_low(LED_F1);
		delay_ms(F_TIME);
	}
}

#if 0
/*/Analogs gen_Analog(int source[], int offset)
	//アナログスティックの計算
	Analogs の中身
		Analogs.sticks
		->	傾きの方向フラグ
			0x01 右スティック左
			0x02 右スティック右
			0x04 右スティック上
			0x08 右スティック下
			0x10 左スティック左
			0x20 左スティック右
			0x40 左スティック上
			0x80 左スティック下
		Analogs.depth_L
		->	左スティック傾き深度
		Analogs.depth_R
		->	右スティック傾き深度
/*/
Analogs gen_Analog(unsigned char source[], int offset)
{
	Analogs data;
	int StickRX;
	int StickRY;
	int StickLX;
	int StickLY;

	StickRX= source[4+offset];
	StickRY= source[5+offset];
	StickLX= source[6+offset];
	StickLY= source[7+offset];

	data.sticks= 0;
	data.depth_R.X= 0;
	data.depth_R.Y= 0;
	data.depth_L.X= 0;
	data.depth_L.Y= 0;

	//--右X-

	if (StickRX < (0x80 - ANALOG_THRESHOLD))
	{
		//右Stickが左
		data.sticks += 0x01;
		data.depth_R.X = 0x7F - StickRX;
	}
	else if (StickRX > (0x80 + ANALOG_THRESHOLD))
	{
		//右Stickが右
		data.sticks += 0x02;
		data.depth_R.X = StickRX - 0x80;
	}
	//--右Y-
	if (StickRY < (0x80 - ANALOG_THRESHOLD))
	{
		//右Stickが上
		data.sticks += 0x04;
		data.depth_R.Y = 0x7F - StickRY;
	}
	else if (StickRY > (0x80 + ANALOG_THRESHOLD))
	{
		//右Stickが下
		data.sticks += 0x08;
		data.depth_R.Y = StickRY - 0x80;
	}

	//--左X-
	if (StickLX < (0x80 - ANALOG_THRESHOLD))
	{
		//左Stickが左
		data.sticks += 0x10;
		data.depth_L.X = 0x7F - StickLX;
	}
	else if (StickLX > (0x80 + ANALOG_THRESHOLD))
	{
		//左Stickが右
		data.sticks += 0x20;
		data.depth_L.X = StickLX - 0x80;
	}
	//--左Y-
	if (StickLY < (0x80 - ANALOG_THRESHOLD))
	{
		//左Stickが上
		data.sticks += 0x40;
		data.depth_L.Y = 0x7F - StickLY;
	}
	else if (StickLY > (0x80 + ANALOG_THRESHOLD))
	{
		//左Stickが下
		data.sticks += 0x80;
		data.depth_L.Y = StickLY - 0x80;
	}
	return data;
}
#endif

void PS2_dataread(unsigned char source[])
{
	int i;
	for (i = 0; i < BUFFER_SIZE; i++)
		Data[i] = 0;
	/* PSコントローラからデータ受信 */
	if (kbhit())
		gets(Data);
	/* PSコントローラの識別子0x5a('Z')を捜索、オフセットを決定 */
	for (i = 0; i < BUFFER_SIZE; i++)
	{
		if (Data[i] == 'Z')
		{
			/* 正常に受信できている場合 */
			ofs = i - 1;					// オフセット情報をセット
			rcv_err = 0;	rcv = 1;				// 再試行回数のカウントをリセット、通信状態のフラグを1にする
			break;
		}
	}
	if(Data[1 + ofs] != 'Z')
	{
		/* PSコントローラから信号を受信できなかった時の処理 */
		/* 再試行回数が閾値を超えた場合通信断絶と判断する */
		if(rcv_err >= RCV_THRESHOLD)
		{
			rcv = 0;
			pad_mode = DIGITAL_MODE;
		}
		/* 再試行回数のカウント */
		else rcv_err++;
	}
}

void motor_emit(char channel, int power)
{
	printf("%c%U*\r", channel, power);
	delay_us(TIME_MOTOR_MARGIN);
}

void motor_reset(void)
{
	printf("A%U*\r",0);   delay_us(TIME_MOTOR_MARGIN);
	printf("B%U*\r",0);   delay_us(TIME_MOTOR_MARGIN);
	printf("C%U*\r",0);   delay_us(TIME_MOTOR_MARGIN);
	printf("D%U*\r",0);   delay_us(TIME_MOTOR_MARGIN);
	printf("E%U*\r",0);   delay_us(TIME_MOTOR_MARGIN);
	printf("F%U*\r",0);   delay_us(TIME_MOTOR_MARGIN);
	printf("L%U*\r",0);   delay_us(TIME_MOTOR_MARGIN);
	printf("R%U*\r",0);   delay_us(TIME_MOTOR_MARGIN);
}

/* Program.c EOF */