# RetroVGen-for-CH32V203
Retro RGB Video Signal Generator for CH32V203K8T6/CH32V20C8T6  

    2023-11-25 copyright (c) antarcticlion  

    https://github.com/antarcticlion/RetroVGen-for-CH32V203/  

	Origin with  
		GORRY - RetroVGen  
		Nick Gammon - VGA video generation  
  
----  
<img src="images/disp.jpg" alt="動作時の画面表示例" title="動作時の画面表示例" width="512" height="288">
<img src="images/test_board.jpg" alt="テストボード" title="テストボード" width="512" height="288">
----  
  
GORRYさんのRetroVGenをCH32V203に移植したものです。  
ビルドにはarduino ideにWCHのボードをインストールしたものが必要です。  
  
R.01の時点ではオリジナルとは以下の点が異なっています。  
　・スクリーンモードを5つ追加。  
　・144MHzで動作するCH32V203ではペリフェラルクロックが9MHzの倍数になるため各種タイミングは若干の差異があります。  
  
まだ作業中のため、NTSC/PALは動作不可です。  
ファーストリリースはオリジナルを知っている人向けの説明しかしませんが、詳しい解説は後日書きたいと思います。  
  
現時点でオリジナルRetroVGenとほぼ同等の動作をします。  
詳しい説明はGORRYさんのオリジナルの方を見てください。  
https://gorry.haun.org/pw/?RetroVGen  
  
----  
  
note:  
Arduino IDEのメニュー→ツールから最適化設定を”無し” -O0 に設定してください。

サンプルで付けている基板 RetroVGen_CASE_K8_11.zip は、スピーカーが付いているモニターだと雑音が出ることがあるようです。  
そうなる場合、対策としては裏面のVGA端子9番ピンに繋がる+5Vの線をパターンカットしてください。  

8MHzのクリスタルがPD0/PD1につながっていてHSEで動作することが必要です。  
  
  
　システムのコードを HSE 144MHz 設定に、HSE timeout値を 0x1000 から 0x4000に変更する必要があります。  
  具体的に変更する2カ所を示します。  
  windowsでの例です。OSやインストール先に合わせて読み替えてください。  
    
① ユーザー名\AppData\Local\Arduino15\packages\WCH\hardware\ch32v\1.0.3\system\CH32V20x\USER\system_ch32v20x.c の  
  
  #define SYSCLK_FREQ_144MHz_HSE  144000000  
  
　を有効にする（他のクロック設定を無効にする）。  
   
② ユーザー名\AppData\Local\Arduino15\packages\WCH\hardware\ch32v\1.0.3\system\CH32V20x\SRC\Peripheral\inc\ch32v20x.h の  
  
  #define HSE_STARTUP_TIMEOUT    ((uint16_t)0x1000) /* Time out for HSE start up */  
  
  を  
  
  #define HSE_STARTUP_TIMEOUT    ((uint16_t)0x4000) /* Time out for HSE start up */  
  
  に書き換えてください。  
  
----  
オリジナルは5V、CH32V203は3.3Vですが、外部回路はオリジナルと同じで大丈夫です。

ピンアウト
            PA7 : VIDEO out  
            PA15: VSYNC LED  
            PB0 : HSYNC  
            PB1 : VSYNC  
            PB3 : SW1  
            PB4 : SW2  
            PB5 : SW3  
            PB6 : SW4  

----

スクリーンモード  
          mode 0 VGA 31KHz 60Hz  
          mode 1 NTSC 作業中につき動作不可  
          mode 2 X1 15KHz  
          mode 3 X68000 15KHz  
          mode 4 PC9801 15KHz  
          mode 5 X1turbo 24kHz 設定値の出典が見つからず要調査  
          mode 6 PC-9801 24kHz  
          mode 7 X68000 31kHz  
          mode 8 SVGA 800x600 60Hz  
          mode 9 PAL 作業中につき動作不可  
          mode A XGA 1024x768 60Hz   
          mode B SXGA 1280x1024 60Hz    
          mode C MZ-3500 640x400 21KHz  
          mode D MZ-3500 640x200 15KHz   
          mode E PC-9801 24kHz  
          mode F X68000 31kHz  
	
----  

  
ライセンス / LICENSE  
  
このプロジェクトはフルスクラッチで書かれたファイルとGORRY氏作のRetroVGenのファイルを改変したものが混在しています。  
プロダクト全体のライセンスを RetroVGen および派生元のNick Gammon氏による「VGA video generation」と揃えます。  
  
 本プロダクトは、自由かつ無償で使用・コピー・配布・変更・流用を行うことが  
 できます。また許可なく再配布・出版・販売を行うことができます。  
   
 本プロダクトは、無保証です。使用した、あるいはしなかったことによる一切の  
 責任は使用者にあるものとします。  
  
----  
have fun :)  
