#include <stdio.h>
#include <stdlib.h>
#include <pigpiod_if2.h>
// コンパイルと実行方法
// gcc -Wall -pthread -o output_filename source_file.c -lpigpiod_if2 -lrt
// mcp3008（A/D）が接続されている SPI のチャンネル
// 0 か 1 のどちらか（SPICS0 か SPICS1 のどちらを使用するか）
#define SPICH 0
// SPI の通信速度
// mcp3008 では 10kHz（最低速度）-1.35MHz（VDD=2.7V の最高速度）
// 最低速度はサンプリングキャパシタからの漏れ電流による誤差のため
// 実験では 1MHz（=1Mbps）に設定
#define SPISPEED 1000000
// プロトタイプ宣言
int ad_read(int pd, int fd, int ch);

int main(){
	int fd,ch,i;
	int pd;
	// 最初にデーモンに接続する（NULL:localhost, NULL:default port）
	pd = pigpio_start(NULL,NULL);
	if (pd < 0){
		printf("pigpiod の接続に失敗しました。\n");
		printf("pigpiod が起動しているか確認してください。\n");
		exit(EXIT_FAILURE);
	}
	// SPI の初期化
	// 最後の引数はフラグ、デフォルト動作は０でよい
	fd = spi_open(pd, SPICH, SPISPEED, 0);
	if (fd < 0){
		printf("SPI 初期化エラー！\n");
		exit(EXIT_FAILURE);
	}
	ch = 0;
	while (ch >= 0) {
		printf("mcp3008 からA/D 変換値を読み込みます。\n");
		printf(" チャンネル（0-7, それ以上は全て, 負数は終了）を入力：");
		scanf("\n%d",&ch);
		if (ch > 7){
			for (i=0; i<8; i++){
				printf(" ch%d = %d\n",i,ad_read(pd, fd, i));
			}
		} else if (ch >= 0)	printf(" ch%d = %d\n",ch,ad_read(pd, fd, ch));
	}
	// 終了時には使用した SPI をクローズする
	spi_close(pd, fd);
	// プログラムの終了時には必ず切断する
	pigpio_stop(pd);
	return 0;
}

int ad_read(int pd, int fd, int ch) {
	// mcp3008 からA/D 変換値を読み出す関数
	// バッファを用意して、そこに制御ビットを入れて書き込むと値がバッファに返る
	// 以下、MCP3008 の仕様
	// 送受信に必要なバッファサイズは 3 バイト、バッファの内容が書き換わるので注意
	// 送信時：0 バイト目 0000 0001：ビット0 はスタートビットで'1'、その他は'0'
	// 1 バイト目 SCCC XXXX：S はシングルエンド指定'1'、CCC は入力チャンネル指定
	// 2 バイト目 XXXX XXXX：なんでもよい
	// 受診時：0 バイト目 ???? ????：不定、A/D 値とは無関係
	// 1 バイト目 ???? ?0BB：?は不定、BB は上位2 ビット（B9,B8）
	// 2 バイト目 BBBB BBBB：下位8 ビット（B7-B0）

	unsigned char wdata[3], rdata[3]; // SPI でやりとりするためのバッファ
	int val;
	wdata[0] = 0b00000001; // スタートビットになる
	wdata[1] = 0b10000000 + (ch & 0x0007) << 4;// シングルエンド、チャンネルセット
	spi_xfer(pd, fd, wdata, rdata, 3); // SPI データの送受信
	val = (rdata[1] & 0x03) << 8; // 上位2 ビット分を取り込んでシフト
	val = val + (int)rdata[2];// 下位8 ビット分を取り込んで加算
	return val;
}
