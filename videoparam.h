
/***********************************************************/
//
//  RetroVGen for CH32V203K8T6/CH32V20CK8T6
//
/***********************************************************/
// videoparam.h
// (c) Hiroaki GOTO as GORRY
//  R.01 2023-11-25 copyright (c) antarcticlion 
/***********************************************************/
// ライセンス / LICENSE
//
// このファイルはGORRY氏作のRetroVGenのソースコードを元にしています。
// プロダクト全体のライセンスはRetroVGenおよび派生元のNick Gammon氏による
// 「VGA video generation」と揃えます。
//
// 本プロダクトは、自由かつ無償で使用・コピー・配布・変更・流用を行うことが
// できます。また許可なく再配布・出版・販売を行うことができます。
// 
// 本プロダクトは、無保証です。使用した、あるいはしなかったことによる一切の
// 責任は使用者にあるものとします。
//
/***********************************************************/
//
//  note:
//    R.01
//      1. CH32Vに合わせて、項目と設定値を変更。
//      2. スクリーンモードを変更
//        mode 8 SVGA 800x600 60Hz
//        mode A XGA 1024x768 60Hz 
//        mode B SXGA 1280x1024 60Hz  
//        mode C MZ-3500 640x400 21KHz
//        mode D MZ-3500 640x200 15KHz 
//      3. 作業中のモード、および設定が不確定なモード
//        mode 1 NTSC 作業中につき動作不可
//        mode 5 X1turbo 24KHz 設定値の出典が見つからず要調査
//        mode 9 PAL 作業中につき動作不可
//
/***********************************************************/

/* original copyright

◇ RetroVGen : Retro RGB Video Signal Generator
  Hiroaki GOTO as GORRY / http://GORRY.hauN.org/
  2022/01/05 Version 20220105a

7. 著作権表記

本プロダクトは、Nick Gammon氏による「VGA video generation」からの派生で
制作されています。当方は本プロダクトの著作権を保有しますが、Nick Gammon
氏の著作権表記を引き継ぎます。詳細はoriginal.hをご覧ください。

本プロダクトは、自由かつ無償で使用・コピー・配布・変更・流用を行うことが
できます。また許可なく再配布・出版・販売を行うことができます。

本プロダクトは、無保証です。使用した、あるいはしなかったことによる一切の
責任は使用者にあるものとします。

本プロダクトは、以下URLを配布先とします。利便性などのためにこれ以外の
URLで配布することがありますが、以下が最も正式なものであり、完全な最新の
パッケージを得ることができます。

http://GORRY.hauN.org/pw/?RetroVGen
*/

// "RetroVGen" : Retro RGB Video Signal Generator

#include "ch32v20x.h"

typedef struct tagScreenParam
{
	uint8_t mHorizontalChars;
	uint8_t mVerticalChars;
	uint8_t mLineDoubler;
	uint8_t mHprescaler;      //Hプリスケーラ設定値
	uint8_t mHsync;           //Hsync インターバル値
	uint8_t mHbp;             //Hバックポーチ インターバル値
	uint16_t mHtimer;         //Htotal タイマー値
	uint16_t mVerticalTotalLines;
	uint16_t mVerticalFrontPorch;
	uint16_t mVerticalSyncLines; 
	uint16_t mVerticalBackPorch;
	char mMsg1[16];
	char mMsg2[16];
} ScreenParam;

static const ScreenParam sScreenParam[16] = {
	/***********************************************************/
  //VGA 31KHz 60Hz  31.778us   DISP 25.422us   HFP 0.636us   HS 3.813us   HBP 1.907us
  //出典:サポート対象の解像度タイミング・チャート https://www.ibm.com/docs/ja/power8?topic=display-supported-resolution-timing-charts
	{
		 29,	// Horizontal Chars //55
		 30,	// Vertical Chars
		  1,	// Line Doubler

      SPI_BaudRatePrescaler_16, // H Prescaler /16
      41,                       // H sync         544(549)  3.813us
      16,                       // H back porch   275(274)  1.907us
      4558,                     // H timer        4558 計算値4576、実測から少し引く

		525,	// V Total
		 10,	// V Front Porch
		  2,	// V Sync Line
		 33,	// V Back Porch
		"0: VGA 31kHz    ",
		"31.50kHz/60.00Hz",
  },

/***********************************************************/
//***要修正
//NTSC 15.70KHz 
//出典:NTSC信号の扱いかたとフレーム・バッファの設計法 https://www.cqpub.co.jp/dwm/contents/0083/dwm008300490.pdf
/* NTSC ITU-R BT.601  
  13.5MHz  1px     74.07407407407407
  HTOTAL 858px  63555.55555555556
  HDISP  720px  53333.33333333333
  HFP     16px   1185.185185185185
  HS      60px   4444.444444444444
  HBP     62px   4592.592592592593
*/
/*NTSC Square Pixel  
  12.2727MHz 1px      81.48166255184271
  HTOTAL   780px   63555.69679043731
  HDISP    640px   52148.26403317933
  HFP       28px    2281.486551451596
  HS        60px    4888.899753110562
  HBP       50px    4074.083127592135
*/
/*NTSC 4Fsc          
  14.313818MHz 1px      69.86256217593377
  HTOTAL     910px   63574.93158009973
  HDISP      768px   53654.44775111714
  HFP          8px     558.9004974074702
  HS          60px    4191.753730556026
  HBP         74px    5169.829601019099
*/
/* 1frame = 262.5line / 2frame=525line /interrace */

	{
		 50,	// Horizontal Chars
		 28,	// Vertical Chars
		  0,	// Line Doubler

      SPI_BaudRatePrescaler_8,
      41, //4.6us 60px
      16,
      4558,

		262,	// V Total
		 10,	// V Front Porch
		  2,	// V Sync Time
		 10,	// V Back Porch
		"1: NTSC 15kHz  *",
		"15.70kHz/59.94Hz",
	},

/***********************************************************/
//X1 15KHz
//  出典: 試験に出る X1 / 34p / ISBN4-930795-88-5
//  62.57822277847309us = 62578.22277847309ns
//  HTOTAL 55*8=440px (1px=142.2232335874388ns)
//  HDISP40*8=320px   (45511.43474798042ns)
//  HFP 5*8=40px      (5688.929343497552ns)
//  HS 4*8=32px       (4551.143474798042ns) 
//  HBP 48px(逆算)    (6826.715212197076ns)
	{
		 50,	// Horizontal Chars
		 25,	// Vertical Chars
		  0,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
      53,          //649(655)  HS 4*8=32px       (4551.143474798042ns) 
      76,          //981(983)  HBP 48px(逆算)    (6826.715212197076ns)
		  9011,       //9011 計算値9011

		258,	// V Total
		 24,	// V Front Porch
		  3,	// V Sync Time
		 31,	// V Back Porch
		"2: X1 15kHz     ",
		"15.98kHz/61.94Hz",
	},

/***********************************************************/
//X68000 15KHz   62.58us  DISP 52.69us HFP 1.65us HS 3.3us HBP 4.94us
//出典:Inside X68000 / 231p / ISBN4-89052-304-9
	{
		 59,	// Horizontal Chars
		 30,	// Vertical Chars
		  0,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  44,         //541(549)   3.3us
		  53,         //705(711)   4.94us
		  9011,       //9011 計算値9011

		260,	// V Total
		  3,	// V Front Porch
		  3,	// V Sync Time
		 14,	// V Back Porch
		"3: X68000 15kHz ",
		"15.98kHz/61.46Hz",
	},

/***********************************************************/
//PC9801 15KHz  62.58us  DISP 44.70us HFP 4.47us HS 4.47us BP 8.94us 
//出典:PC-9800シリーズ テクニカルデータブック HARDWARE 編 1993年 / 351p / ISBN4-7561-0456-8
	{
		 50,	// Horizontal Chars
		 25,	// Vertical Chars
		  0,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  44,         // 541(549)   4.47us
		  102,       // 1293(1287)  8.94us
		  9011, //9011 計算値9011

		261,	// V Total
		 15,	// V Front Porch
		  8,	// V Sync Time
		 38,	// V Back Porch
		"4: PC-9801 15kHz",
		"15.98kHz/61.23Hz",
	},

/***********************************************************/
//***要出典
	{
		 35,	// Horizontal Chars
		 25,	// Vertical Chars
		  1,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  32,   //427(437) 2.98us
		  39,    //574(574) 3.8us
		  5792,

		448,	// V Total
		 16,	// V Front Porch
		  8,	// V Sync Time
		 24,	// V Back Porch
		"5: X1turbo 24kHz",
		"24.86kHz/55.49Hz",
	},

/***********************************************************/
//出典:PC-9800シリーズ テクニカルデータブック HARDWARE 編 1993年 / 351p / ISBN4-7561-0456-8
	{
		 35,	// Horizontal Chars
		 25,	// Vertical Chars
		  1,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  32,  //427(437) 3.04us
		  39, //574(574) 3.8us
		  5790,

		440,	// V Total
		  7,	// V Front Porch
		  8,	// V Sync Time
		 25,	// V Back Porch
		"6: PC-9801 24kHz",
		"24.83kHz/56.42Hz",
	},

/***********************************************************/
//出典:Inside X68000 / 231p / ISBN4-89052-304-9
	{
		 25,	// Horizontal Chars
		 32,	// Vertical Chars
		  1,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  37,     //492(496)
		  40,     //587(596) 
		  4554,   //4554 計算値4,572

		568,	// V Total
		 15,	// V Front Porch
		  6,	// V Sync Line
		 35,	// V Back Porch
		"7: X68000 31kHz ",
		"31.50kHz/55.46Hz",
	},

	// ========

/***********************************************************/
//SVGA 800x600 60Hz
//出典:サポート対象の解像度タイミング・チャート https://www.ibm.com/docs/ja/power8?topic=display-supported-resolution-timing-charts
	{
		 47,	// Horizontal Chars
		 36,	// Vertical Chars
		  1,	// Line Doubler

		  SPI_BaudRatePrescaler_8,
		  34, //453(460) 3.2us
		  19, //314(316) 2.2us
		  3790,  //3790  計算値3801

		628,	// V Total
		  1,	// V Front Porch
		  4,	// V Sync Line
		 23,	// V Back Porch
		"8: SVGA 60Hz    ",
		"*****kHz/60.00Hz",
	},

/***********************************************************/
//PAL 15KHz 
//出典:NTSC信号の扱いかたとフレーム・バッファの設計法 https://www.cqpub.co.jp/dwm/contents/0083/dwm008300490.pdf
//PAL  ITU-R BT.601  13.5MHz        HTOTAL 864px   HDISP 720px   HFP 12px   HS 60px   HBP 72px
//PAL  Square Pixel  14.75MHz       HTOTAL 944px   HDISP 768px   HFP 34px   HS 60px   HBP 82px
	{
		 50,	// Horizontal Chars
		 30,	// Vertical Chars
		  0,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  0,
		  0,
		  0,

		312,	// V Total
		 10,	// V Front Porch
		  4,	// V Sync Time
		 10,	// V Back Porch
		"9: PAL 15kHz   *",
		"15.63kHz/50.08Hz",
	},

/***********************************************************/
//XGA 1024x768 60Hz 
//出典:サポート対象の解像度タイミング・チャート https://www.ibm.com/docs/ja/power8?topic=display-supported-resolution-timing-charts
	{
		 36,	// Horizontal Chars
		 24,	// Vertical Chars
		  2,	// Line Doubler = quad

		  SPI_BaudRatePrescaler_8,
		  22,  //297(301) 2.092us
		  22, //353(354) 2.462us
		  2977,  //2960  計算値2977

		806,	// V Total
		  3,	// V Front Porch
		  6,	// V Sync Time
		 29,	// V Back Porch
		"A: XGA 60Hz     ",
		"*****kHz/60.00Hz",
	},

/***********************************************************/
//SXGA 1280x1024 60Hz  
//出典:サポート対象の解像度タイミング・チャート https://www.ibm.com/docs/ja/power8?topic=display-supported-resolution-timing-charts
	{
		 28,	// Horizontal Chars
		 32,	// Vertical Chars
		  2,	// Line Doubler = quad

		  SPI_BaudRatePrescaler_8,
		  10,  //141(149) 1.037us
		  20, //327(330) 2.296us
		  2250,  //2250  計算値2250

		1066,	// V Total
		  1,	// V Front Porch
		  3,	// V Sync Time
		 38,	// V Back Porch
		"B: SXGA 60Hz    ",
		"*****kHz/60.00Hz",
	},

/***********************************************************/
//MZ3500 640x400     disp 32.55us   HFP 4.88us   HS 4us   HBP 6.5us   total 47.93us 
//出典:MZ-3500 SERVICE MANUAL / 33-34p 
	{
		 36,	// Horizontal Chars
		 25,	// Vertical Chars
		  1,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  43,  //570(576)
		  67, //938(936)
		  6890,  //6890  計算値6901

		441,	// V Total // 400+11+5+25  19.16ms
		 11,	// V Front Porch  //0.527ms
		  5,	// V Sync Time //0.24ms
		 25,	// V Back Porch //1.198ms
		"C: MZ-3500 21kHz",
		"20.92kHz/47.30Hz",
	},

/***********************************************************/
/* MZ-3500 640x200
      1px=62.5ns 15.87KHz
      HTOTAL  63us  1008px
      HDISP   40us   640px
      HFP      7us   112px
      HS       6us    96px
      HBP     10us   160px

      1line=62.5us
      VTOTAL 261line 16.6ms 60.24Hz
      VDISP 200line 12.6ms 200line 12600us
      VBLANK                61line 3843us
      VFP 1.2ms             18line 1134us
      VS 1ms                15line  945us
      VBP 1.8ms             28line 1764us
      
*/
//出典:MZ-3500 SERVICE MANUAL / 33-34p 
	{
		 40,	// Horizontal Chars
		 25,	// Vertical Chars
		  0,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  71,    //865(864) 6us
		  78,    //1005(1008) 7us
		  2390,  //2390　計算値2390

		261,	// V Total
		 18,	// V Front Porch
		 15,	// V Sync Time
		 28,	// V Back Porch
		"D: MZ-3500 15kHz",
		"15.87kHz/60.24Hz",
	},

/***********************************************************/
	{
		 35,	// Horizontal Chars
		 25,	// Vertical Chars
		  1,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  32,  //427(437) 3.04us
		  39, //574(574) 3.8us
		  5790,

		440,	// V Total
		  7,	// V Front Porch
		  8,	// V Sync Time
		 25,	// V Back Porch
		"E: 24kHz PC-9801",
		"24.83kHz/56.42Hz",
	},

/***********************************************************/
	{
		 25,	// Horizontal Chars
		 32,	// Vertical Chars
		  1,	// Line Doubler

		  SPI_BaudRatePrescaler_16,
		  37,     //492(496)
		  40,     //587(596) 
		  4554,   //4554 計算値4,572

		568,	// V Total
		 15,	// V Front Porch
		  6,	// V Sync Line
		 35,	// V Back Porch
		"F: X68000 31kHz ",
		"31.50kHz/55.46Hz",
	},


};