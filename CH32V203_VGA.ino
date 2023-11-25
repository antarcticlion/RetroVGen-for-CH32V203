/***********************************************************/
//
//  RetroVGen for CH32V203K8T6/CH32V20CK8T6
//
/***********************************************************/
// CH32V203_VGA.ino
//  R.01 2023-11-25 copyright (c) antarcticlion 
/***********************************************************/
// ライセンス / LICENSE
//
// このファイルはフルスクラッチで書かれた物ですが、
// GORRY氏作のRetroVGenのソースコードを元にしたファイルを
// 外部から読み込んでいるため、プロダクト全体のライセンスを
// RetroVGenおよび派生元のNick Gammon氏による
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
//    8MHzのクリスタルがPD0/PD1につながっていてHSEで動作することが必要です。
//    システムのコードを HSE 144MHz 設定に、
//    HSE timeout値を 0x1000 から 0x4000に変更する必要があります。
//
//    144MHzで動作するCH32V203ではペリフェラルクロックが9MHzの倍数になるため
//    各種タイミングは若干の差異があります。
//
//      R.01
//        ファーストリリース
//        　一部のスクリーンモードが2つ動作しませんが、それ以外は概ね良好。
//          オリジナルの仕様を踏襲しつつ、Line doubler設定にx4(quad)モードを追加。
//        pinout
//            PA7 : VIDEO out
//            PA15: VSYNC LED
//            PB0 : HSYNC
//            PB1 : VSYNC
//            PB3 : SW1
//            PB4 : SW2
//            PB5 : SW3
//            PB6 : SW4
//        スクリーンモードを変更
//          mode 8 SVGA 800x600 60Hz
//          mode A XGA 1024x768 60Hz 
//          mode B SXGA 1280x1024 60Hz  
//          mode C MZ-3500 640x400 21KHz
//          mode D MZ-3500 640x200 15KHz 
//      　作業中のモード、および設定が不確定なモード
//          mode 1 NTSC 作業中につき動作不可
//          mode 5 X1turbo 24KHz 設定値の出典が見つからず要調査
//          mode 9 PAL 作業中につき動作不可
//
/***********************************************************/
#include "ch32v20x.h"
#include "CH32V203_VGA.h"
#include "videoparam.h"
#include "screenFont.h"

/***********************************************************/
// HSYNCタイミングデバッグ設定
//
//　desc: デバッグ用。HSYNCとHBPの処理クロック数をシリアル出力する場合はこの定義を有効にする。
//
// #define SYNC_DEBUG

/***********************************************************/
//　フォント切り替え
//
//　desc: フォントをRetroVGenオリジナルに変更するにはこの定義を有効にする。
//
#define FONT_RETROVGEN

/***********************************************************/
//ビデオパラメタ
//VGA 31KHz 60Hz  31.778us   DISP 25.422us   HFP 0.636us   HS 3.813us   HBP 1.907us
//出典:サポート対象の解像度タイミング・チャート https://www.ibm.com/docs/ja/power8?topic=display-supported-resolution-timing-charts
volatile static uint8_t param_H_chars = 29;
volatile static uint8_t param_H_Prescaler = SPI_BaudRatePrescaler_16;
volatile static uint8_t param_line_doubler = 1;
volatile static uint8_t param_H_sync = 41;        //544(549)  3.813us
volatile static uint8_t param_H_back_porch = 16;  // 275(274)   1.907us
volatile static uint8_t param_V_chars = 30;
volatile static uint16_t param_V_total = 525;
volatile static uint8_t param_V_sync_line = 2;
volatile static uint8_t param_V_back_porch = 33;
volatile static uint8_t param_V_front_porch = 10;
volatile static uint16_t param_timer = 4558;  //4558 計算値4576、実測から17引く
/***********************************************************/
volatile static uint16_t curr_line = 0;
volatile static uint16_t curr_view_line = 0;
volatile static uint16_t curr_mode = 0;
volatile static uint16_t curr_mode_remain = 0;
/***********************************************************/
volatile static uint8_t text_vram[64][128];  // 128 x 32 文字
/***********************************************************/
volatile static uint8_t data[2][256] = { 0 };  //ダブルバッファ
volatile static uint8_t page = 0;              //ページ

/***********************************************************/
// 割り込みハンドラ
//
// desc:  HSYNCタイマーの割り込み時に呼ばれ、アップデート割り込みフラグを消す。
//        将来のインターレース対応に備えてタイマー3のハンドラも用意しておく。
//
void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));  //割り込みハンドラの属性でプロトタイプ宣言
void TIM4_IRQHandler(void) {
  TIM4->INTFR &= ~TIM_UIF;  //アップデート割り込みフラグのクリア
  //  Serial.println("IRQ");
}

void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));  //割り込みハンドラの属性でプロトタイプ宣言
void TIM3_IRQHandler(void) {
  TIM3->INTFR &= ~TIM_UIF;  //アップデート割り込みフラグのクリア
  //  Serial.println("IRQ");
}

/***********************************************************/
//
/***********************************************************/
void setup() {
  {  //シリアルポートの初期化
    Serial.begin(115200);
    Serial.println("init");
    Serial.println("---");
  }

  uint8_t paramsel = 0;
  {  //ディップスイッチの設定を読む
    pinMode(PB3, INPUT_PULLUP);
    pinMode(PB4, INPUT_PULLUP);
    pinMode(PB5, INPUT_PULLUP);
    pinMode(PB6, INPUT_PULLUP);
    paramsel |= (digitalRead(PB3) ? 0 : 0x01);
    paramsel |= (digitalRead(PB4) ? 0 : 0x02);
    paramsel |= (digitalRead(PB5) ? 0 : 0x04);
    paramsel |= (digitalRead(PB6) ? 0 : 0x08);

    param_H_chars = sScreenParam[paramsel].mHorizontalChars;
    param_V_chars = sScreenParam[paramsel].mVerticalChars;
    param_line_doubler = sScreenParam[paramsel].mLineDoubler;
    param_H_Prescaler = sScreenParam[paramsel].mHprescaler;
    param_H_sync = sScreenParam[paramsel].mHsync;
    param_H_back_porch = sScreenParam[paramsel].mHbp;
    param_timer = sScreenParam[paramsel].mHtimer;
    param_V_total = sScreenParam[paramsel].mVerticalTotalLines;
    param_V_front_porch = sScreenParam[paramsel].mVerticalFrontPorch;
    param_V_sync_line = sScreenParam[paramsel].mVerticalSyncLines;
    param_V_back_porch = sScreenParam[paramsel].mVerticalBackPorch;

    Serial.println("PARAM");
    Serial.println(paramsel);
    Serial.println("PARAM SEL");
  }

  {  //テキストVRAMのクリア
    //全体を0クリア
    for (int y = 0; y < 40; y++) {
      for (int x = 0; x < 128; x++) {
        text_vram[y][x] = 0x00;
      }
    }
    //表示域を数字で埋める
    for (int y = 0; y < param_V_chars; y++) {
      for (int x = 0; x < (param_H_chars - 1); x++) {
        text_vram[y][x] = '0' + (x % 10);
      }
      text_vram[y][0] = '0' + (y / 10);
      text_vram[y][1] = '0' + (y % 10);
      text_vram[y][2] = ':';
    }
    //罫線
    for (int y = 0; y < 40; y++) {
      if (!(y % 10)) {
        for (int x = 3; x < (param_H_chars - 1); x++) {
          text_vram[y][x] = ((x % 10) ? '-' : '+');
        }
      }
    }
    //モード名
    for (int y = 2; y < 8; y++) {
      for (int x = 5; x < 23; x++) {
        text_vram[y][x] = 0x20;
      }
    }
    text_vram[6][9] = '/';
    text_vram[6][10] = '6';
    text_vram[6][11] = '0';
    for (int x = 5; x < 21; x++) {
      text_vram[3][x + 1] = sScreenParam[paramsel].mMsg1[x - 5];
    }
    for (int x = 5; x < 21; x++) {
      text_vram[4][x + 1] = sScreenParam[paramsel].mMsg2[x - 5];
    }
#if 0 
    //文字枠
    for (int y = 4; y < 22; y++) {
      for (int x = 5; x < 23; x++) {
        text_vram[y][x] = ' ';
      }
    }
    //キャラ表
    for (int y = 5; y < 21; y++) {
      for (int x = 6; x < 22; x++) {
        text_vram[y][x] = (x-6) * 16 + (y-3);
      }
    }
#endif
    Serial.println("VRAM");
  }

  {  //VSYNC LEDの初期化
    pinMode(PA15, OUTPUT);
    digitalWrite(PA15, HIGH);
    Serial.println("LED");
  }

  {                                 //System tick設定
    NVIC_DisableIRQ(SysTicK_IRQn);  //SysTickの割り込みを止める、これを止めないとループ末尾のWFIが効かなくなる。
    SysTick->SR = 0;
    SysTick->CTLR = 0;
    SysTick->CNT = 0;
    SysTick->CMP = SystemCoreClock - 1;
    SysTick->CTLR = 0x0D;
    Serial.println("SYSTICK");
  }


  {                                                              //タイマーの初期化
    SetVTFIRQ((uint32_t)TIM4_IRQHandler, TIM4_IRQn, 0, ENABLE);  //タイマー割込ベクター設定
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);         // TIM4クロックを有効化
    TIM_TimeBaseInitStructure.TIM_Period = param_timer;          //計算値4576、実測から少し引く
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;                 //プリスケーラは無効化設定
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;  //供給クロック 144MHz 1clk=6.944444444444444ns
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);      //TIM4 ON
    NVIC_EnableIRQ(TIM4_IRQn);  //TIM4の割り込みON
    Serial.println("TIM");
  }

  {  // DMAの初期化 SPI1はDMA1のCH3固定
    DMA_InitTypeDef DMA_InitStructure = { 0 };
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DATAR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)data[page];
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = param_H_chars;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    Serial.println("DMA");
  }

  {  // SPIの初期化
    SPI_InitTypeDef SPI_InitStructure = { 0 };
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

    //SCK
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //MISO
    //    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    //    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    //    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;  //ここはクロック18MHz or 9MHz に都度設定する
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = param_H_Prescaler;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);

    SPI_Cmd(SPI1, ENABLE);  //SPI有効化
    Serial.println("SPI");
  }

  {                        // SYNC PIN
    pinMode(PB0, OUTPUT);  //HSYNC
    digitalWrite(PB0, HIGH);
    pinMode(PB1, OUTPUT);  //VSYNC
    digitalWrite(PB1, HIGH);
    Serial.println("SIGNAL PIN");
  }

  Serial.println("---");
  Serial.println("Init done");
}

static uint8_t v_curr = 0;

static uint64_t prev_systick = 0;
static uint64_t curr_systick = 0;
static uint64_t systick_p1 = 0;
static uint64_t systick_p2 = 0;
static uint64_t systick_p3 = 0;
static uint64_t systick_p4 = 0;

/***********************************************************/
//
/***********************************************************/
void loop() {
  while (1) {
    //ループ関数を抜けない
    //  curr_systick = SysTick->CNT;
    {  //HSYNC生成
      //  asm volatile ("nop");
      uint8_t loopcnt = param_H_sync;
#ifdef SYNC_DEBUG
      systick_p1 = SysTick->CNT;
#endif
      GPIOB->BCR = GPIO_Pin_0;  //digitalWrite(PB0, LOW);
      while (--loopcnt) {}
#ifdef SYNC_DEBUG
      systick_p2 = SysTick->CNT;
#endif
      GPIOB->BSHR = GPIO_Pin_0;  //digitalWrite(PB0, HIGH);
    }

    {  //HSYNCバックポーチ
      uint8_t loopcnt = param_H_back_porch;
      while (--loopcnt) {}
#ifdef SYNC_DEBUG
      systick_p3 = SysTick->CNT;
#endif
    }

    {  //HSYNV後の処理
      uint16_t curr_char;
      uint8_t curr_char_line;
      switch (curr_mode) {
        case 0:
          {  //INIT
            curr_line = 0;
            curr_mode = 1;
            curr_mode_remain = param_V_sync_line;
            curr_view_line = 0;
            page = 0;

            switch (param_line_doubler) {
              case 1:  //double
                curr_char = (curr_view_line / 8) / 2;
                curr_char_line = ((curr_view_line % 16) >> 1);
                break;
              case 2:  //quad
                curr_char = (curr_view_line / 8) / 4;
                curr_char_line = ((curr_view_line % 32) >> 2);
                break;
              default:  //normal
                curr_char = (curr_view_line / 8);
                curr_char_line = (curr_view_line % 8);
            }
            for (uint8_t index = 0; index < param_H_chars; index++) {
#ifdef FONT_RETROVGEN
              data[page][index] = screen_font[curr_char_line][text_vram[curr_char][index]];
#else
              data[page][index] = font[text_vram[curr_char][index]][curr_char_line];
#endif
            }
          }
          //フォールスルーさせるのでbreak無し
        case 1:
          {  //VSYNC
            digitalWrite(PB1, LOW);
            if (!(--curr_mode_remain)) {
              v_curr++;
              curr_mode = 2;
              curr_mode_remain = param_V_back_porch;
            }
            break;
          }
        case 2:
          {
            //back portch
            digitalWrite(PB1, HIGH);
            if (!(--curr_mode_remain)) {
              if (v_curr > 60) {
                v_curr = 0;
              }
              text_vram[6][7] = ((v_curr + 1) / 10) + '0';
              text_vram[6][8] = ((v_curr + 1) % 10) + '0';
              if (v_curr > 4) {
                GPIOA->BCR = GPIO_Pin_15;  //digitalWrite(PA15, LOW);
                text_vram[6][6] = ' ';
                text_vram[6][12] = ' ';
              } else {
                GPIOA->BSHR = GPIO_Pin_15;  //digitalWrite(PA15, HIGH);
                text_vram[6][6] = 0xFF;
                text_vram[6][12] = 0xFF;
              }
              curr_mode = 3;
              curr_mode_remain = param_V_total - param_V_sync_line - param_V_back_porch - param_V_front_porch;
            }
            break;
          }
        case 3:
          {  // DRAW
            // DMAを使用してSPIにデータを送信
            //SPI転送開始
            DMA1_Channel3->CFGR &= (uint16_t)(~DMA_CFGR1_EN);  // DMAチャンネルを無効化
            DMA1_Channel3->MADDR = (uint32_t)data[page];       // データバッファのアドレスを設定
            DMA1_Channel3->CNTR = param_H_chars;               // 転送サイズを設定
            DMA1->INTFCR = (DMA1_FLAG_TC3 | DMA1_FLAG_HT3);    // 転送完了フラグクリア}
#ifdef SYNC_DEBUG
            systick_p3 = SysTick->CNT;
#endif
            DMA1_Channel3->CFGR |= DMA_CFGR1_EN;  // DMAチャンネルを有効化
            curr_view_line++;
            page++;
            page &= 1;
            switch (param_line_doubler) {
              case 1:  //double
                curr_char = (curr_view_line / 8) / 2;
                curr_char_line = ((curr_view_line % 16) >> 1);
                break;
              case 2:  //quad
                curr_char = (curr_view_line / 8) / 4;
                curr_char_line = ((curr_view_line % 32) >> 2);
                break;
              default:  //normal
                curr_char = (curr_view_line / 8);
                curr_char_line = (curr_view_line % 8);
            }
            for (uint8_t index = 0; index < param_H_chars; index++) {
#ifdef FONT_RETROVGEN
              data[page][index] = screen_font[curr_char_line][text_vram[curr_char][index]];
#else
              data[page][index] = font[text_vram[curr_char][index]][curr_char_line];
#endif
            }
            while (!(DMA1->INTFR & DMA1_FLAG_TC3)) {}  // 転送は投げっぱなしにしない。

            if (!(--curr_mode_remain)) {
              curr_mode = 4;
              curr_mode_remain = param_V_front_porch;
            }
            break;
          }
        case 4:
          {  //front portch

            if (!(--curr_mode_remain)) {
              curr_mode = 0;
              curr_mode_remain = 0;
            }
            break;
          }
        default:
          {  //Recover
            curr_mode = 0;
            curr_mode_remain = 0;
            break;
          }
      }
      curr_line++;
#ifdef SYNC_DEBUG
      Serial.println("p2 - p1");
      Serial.println(systick_p2 - systick_p1);
      Serial.println("p3 - p2");
      Serial.println(systick_p3 - systick_p2);
#endif
      //  Serial.println(curr_systick - prev_systick);
      //  prev_systick = curr_systick;
      //  Serial.println("Loop");
      __WFI();  //スリープモードに入る
      //  Serial.println("wake");
    }
  }
}
