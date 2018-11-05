#include <Ticker.h>
#include <time.h>
#include <string.h>
#include "utility/MPU9250.h"
#include "utility/quaternionFilters.h"

// ↓いつの間にこれなしでもVScodeがエラー出さなくなりました
//
// VScodeでエラーを出さないための記述。Arduinoでコンパイル時はコメントアウトすること。
// #include <M5StackSAM.h>
// M5SAM MyMenu;

// LCDの中央座標を示す。Y座標は3列分用意。
#define LCDcenterX 160
#define LCDcenterY1 100
#define LCDcenterY2 120
#define LCDcenterY3 140

// 加速度・ジャイロを格納する構造体
struct sensorData {
  int accX;
  int accY;
  int accZ;
  int gyroX;
  int gyroY;
  int gyroZ;
};

// プロトタイプ宣言
int fileCount(fs::FS &fs, const char * dirname, uint8_t levels);
void getAcc(MPU9250* IMU, sensorData* pSensorData);
int writeFile(fs::FS &fs, const char * path, const char * message);
void _readSensor(MPU9250* IMU);

/**
 * @brief SD/accフォルダ内のファイルの数を数えて表示
 * 
 */
void appCsvFileCount(){
  MyMenu.drawAppMenu(F("CSV File Count"),F(""),F("ESC"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }
  MyMenu.windowClr();
  M5.Lcd.drawCentreString(F("File count"),LCDcenterX,LCDcenterY1,2);
  M5.Lcd.drawCentreString(String(fileCount(SD, "/walk", 0))+"  CSVs in SD card",LCDcenterX,LCDcenterY2,2);   
  while(!M5.BtnB.wasPressed()){
    M5.update();
  }
  MyMenu.show();
}

/**
 * @brief 加速度・ジャイロをリアルタイムで計測・表示
 * 
 * スタート、ストップ機能付き。ループ間隔は15.625ms(64Hz)
 * 結果表示ごとに画面をクリアしているため、画面がちらつきます。
 * 
 * @attention 加速度の初期値をそろえるため、既定の初期値になるまで初期化を続けます。
 */
void appDrawAccGyro() {
  MyMenu.drawAppMenu(F("current Acc and Gyro"),F(""),F("EXIT"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  // 加速度・ジャイロセンサの宣言・初期化
  M5.Lcd.drawCentreString("Loading...", LCDcenterX, LCDcenterY2, 2);
  delay(1000);
  MPU9250 IMU;
  struct sensorData sensorData = {0, 0, 0, 0, 0, 0};

  while(true) {
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    IMU.initMPU9250();
    getAcc(&IMU, &sensorData);

    if(abs(abs(sensorData.accX) - abs(sensorData.accY)) < 30) {
      break;
    }
  }
  
  MyMenu.drawAppMenu(F("Current Acc and Gyro"),F("START"),F("EXIT"),F(""));
  MyMenu.windowClr();
  M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY2, 2);
    
    while(!M5.BtnB.wasPressed()){
    
      if(M5.BtnA.wasPressed()) {
        MyMenu.drawAppMenu(F("Current Acc and Gyro"),F(""),F("EXIT"),F("STOP"));

        // 加速度・ジャイロ表示ループ
        while(!M5.BtnC.wasPressed()) {
          getAcc(&IMU, &sensorData);
          MyMenu.windowClr();
          // 加速度を表示
          M5.Lcd.drawCentreString("Acceleration", LCDcenterX, LCDcenterY1, 2);
          M5.Lcd.drawCentreString(String(sensorData.accX)+"  "+
                                  String(sensorData.accY)+"  "+
                                  String(sensorData.accZ),
                                  LCDcenterX, LCDcenterY2, 2);
          // ジャイロを表示
          M5.Lcd.drawCentreString("Gyro", LCDcenterX, LCDcenterY3, 2);
          M5.Lcd.drawCentreString(String(sensorData.gyroX)+"  "+
                                  String(sensorData.gyroY)+"  "+
                                  String(sensorData.gyroZ),
                                  LCDcenterX, LCDcenterY3+15, 2);
          delay(15.625);
          M5.update();
        }

        // 加速度・ジャイロ表示ループ終了
        MyMenu.drawAppMenu(F("Current Acc and Gyro"),F("START"),F("EXIT"),F(""));
        MyMenu.windowClr();
        M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY2, 2);
      }
    M5.update();
    }
    MyMenu.show();
}


/**
 * @brief 現在時刻を表示する。
 * 
 */
void appShowNowTime() {
  MyMenu.drawAppMenu(F("Now Time"),F("REFRESH"),F("EXIT"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  M5.Lcd.drawCentreString(nowTime(), LCDcenterX, LCDcenterY2, 2);
  
  while(!M5.BtnB.wasPressed()){
    if(M5.BtnA.wasPressed()) {
      M5.Lcd.drawCentreString(nowTime(), LCDcenterX, LCDcenterY2, 2);
    }
    M5.update();
  }

  MyMenu.show();
}


//タイマー
Ticker tickerSensor; // センサの値を読む
Ticker tickerShowTime; // 計測経過時間表示
Ticker tickerWriteData; // バッファにためた加速度データをCSVに書き込み

//バッファ
sensorData *sdBuff = NULL;
int saveIndex = 0;
int elapsedTime = 0;
//バッファのインデックス
volatile int buffPointer = 0;
volatile bool buffSaveFlg = false;

/**
 * @brief 加速度をタイマー割り込みでとるパターン
 * 
 * タイマーがミリ秒単位しかないため64Hzに最も近い16ミリ秒間隔（62.5Hz）を指定
 * 60秒計測ー＞新しいファイルで保存ー＞60秒計測.......
 */
void appAccTimer() {
    MyMenu.drawAppMenu(F("Save Acc by Timer"),F(""),F("EXIT"),F(""));
  
    while(M5.BtnB.wasPressed()){
      M5.update();
    }
  
    // 加速度・ジャイロセンサの宣言・初期化
    M5.Lcd.drawCentreString("Loading...", LCDcenterX, LCDcenterY2, 2);
    delay(1000);
    static MPU9250 IMU;
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    IMU.initMPU9250();
  
    MyMenu.drawAppMenu(F("Save acc and gyro"),F("START"),F("EXIT"),F(""));
    MyMenu.windowClr();
    M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY2, 2);

    sdBuff = (struct sensorData*)malloc( sizeof(struct sensorData)*4096); //長さ4096を確保 ＊最後にfree(sdBuff)
    while(!M5.BtnB.wasPressed()){
      if(M5.BtnA.wasPressed()) {
        MyMenu.drawAppMenu(F("keisoku chu desu"),F(""),F("EXIT"),F("STOP"));

        // 16ミリ秒ごと(62.5Hz)にセンサーリード
        tickerSensor.attach_ms<MPU9250*>(16, _readSensor, &IMU);
        // 1秒ごとに経過時間を表示
        tickerShowTime.attach_ms(1000, _showElapsedTime);
        // 60秒ごとにフラグ（buffSaveFlg）を立てる
        tickerWriteData.attach_ms(10000, _buffSave); 

        while(!M5.BtnC.wasPressed()) {
          M5.update();
          if(buffSaveFlg) {
            Serial.println("buffSaveFlg setted!");
            //タイマーを止める
            tickerSensor.detach();
            tickerShowTime.detach();
            tickerWriteData.detach();
            Serial.println("All ticker detached!");

            //ファイル作成
            Serial.println("making csv file...");
            saveIndex = fileCount(SD, "/walk", 0);
            Serial.print("saveIndex = "); Serial.println(saveIndex);
            char fileName[16] = {};
            sprintf(fileName, "/walk/%d.csv", saveIndex);
            Serial.print("fileName = "); Serial.println(fileName);
            File file = SD.open(fileName, FILE_WRITE);
            Serial.print("Opened file = "); Serial.println(file);
            
            
            //Serial.println(fileName);
            //ファイルが開けないとき
            if(!file) {
                MyMenu.windowClr();
                M5.Lcd.drawCentreString("SD not found?Plz Insert SD and reboot", LCDcenterX, LCDcenterY2, 2);
                Serial.println("SD not found?Plz Insert SD and reboot");
                tickerSensor.detach();
                tickerShowTime.detach();
                tickerWriteData.detach();
                break;
            }
            Serial.println("opened csv file successfully!");
            //バッファを全て書き込み
            clearY1();
            Serial.println("CSV writing...");
            M5.Lcd.drawCentreString("CSV Writing...", LCDcenterX, LCDcenterY1, 2);
            for(int i = 0; i < buffPointer; i++) {
              char buf[64];
              sprintf(buf, "%d, %d, %d, %d, %d, %d", sdBuff[i].accX, sdBuff[i].accY,sdBuff[i].accZ, sdBuff[i].gyroX, sdBuff[i].gyroY, sdBuff[i].gyroZ);
              file.println(buf);
            }
            
            file.close();
            Serial.println("File closed!");

            clearY1();
            M5.Lcd.drawCentreString("Write Complete!", LCDcenterX, LCDcenterY3, 2);
            
            //バッファ初期化
            buffPointer = 0;
            buffSaveFlg = false;
            
            // 計測開始
            tickerSensor.attach_ms<MPU9250*>(16,_readSensor, &IMU);
            tickerShowTime.attach_ms(1000, _showElapsedTime);
            tickerWriteData.attach_ms(10000, _buffSave); 
            M5.Lcd.drawCentreString("Measurement Start!", LCDcenterX, LCDcenterY1, 2);
            Serial.println("All ticker attached!");
          }
          delay(100);
        }
        
        //タイマーを止める
        tickerSensor.detach();
        tickerShowTime.detach();
        tickerWriteData.detach();
        //バッファ初期化
        buffPointer = 0;
        elapsedTime = 0;
        buffSaveFlg = false;

        MyMenu.windowClr();

        // 加速度・ジャイロ表示ループ終了
        MyMenu.drawAppMenu(F("Save acc and gyro"),F("START"),F("EXIT"),F(""));
        MyMenu.windowClr();
        M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY2, 2);
      }
      M5.update();
    }
    free(sdBuff);
    MyMenu.show();
}

// TODO
// SD/accフォルダ内のCSVファイルの名前をリスト形式で羅列する。
void csvList() {
  int csv_count = fileCount(SD, "/acc", 0);
  byte list_lines = 7;
  uint16_t list_page = 0;
  uint16_t list_pages = 0;
  uint16_t list_lastpagelines = 0;

  MyMenu.drawAppMenu(F("CSV file list"),F("BACK"),F("ESC"),F("NEXT"));
}


/**
 * @brief 指定されたフォルダのファイル数をカウントする。
 * 
 * @param[in] fs 検索対象ファイルシステム。たいていはSD
 * @param[in] dirname フォルダパス名
 * @param[in] levels 探査階層。0の場合は子フォルダを無視する
 * @return ファイル数。フォルダはカウントされない
 * @retval -1 エラー。フォルダが開けなかったか、フォルダを指定しなかった。
 */
int fileCount(fs::FS &fs, const char * dirname, uint8_t levels) {

    int filecount = 0;

    File root = fs.open(dirname);

    if(!root) {
      Serial.println("no such directory");
      return -1; // フォルダを開けなかった
    }
    if(!root.isDirectory()) {
      Serial.println("not directory");
      return -1;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            if(levels) {
              filecount += fileCount(fs, file.name(), levels -1);
            }
         } else {
              filecount++;
            }
        file = root.openNextFile();
    }
    return filecount;
}


/**
 * @brief 指定されたファイルに文字を出力
 *
 * 引数に書き込み先と内容を与え、それをファイルに出力。
 * 呼び出すごとに「オープン→書き込み→クローズ」をするので
 * 速度的に難あり。
 * 
 * @param[in] fs 書き出し先ファイルシステム。たいていはSD
 * @param[in] path 出力先ファイルパス。なければ自動で作成される。
 * @retval 0 書き出し正常終了
 * @retval 1 ファイル読み込みエラー。同名ファイルが書き込み禁止になっているなど
 * @retval 2 ファイル書き込みエラー
 */
int writeFile(fs::FS &fs, const char * path, const char * message){
  
    File file = fs.open(path, FILE_APPEND);
    if(!file){
      file.close();
      return 1;
    }
    if(file.println(message)){
      file.close();
      return 0;
    } else {
      file.close();
      return 2;
    }    
}




/**
 * @fn
 * getAcc
 * @brief センサから加速度とジャイロを取得する
 * @param[in] MPU 9軸センサのインスタンス
 * @param[in,out] *pSensorData センサデータ構造体のポインタ
 * @return なし。与えられたポインタ先のメンバに直接書き込む
 */
void getAcc(MPU9250* IMU, sensorData* pSensorData) {
  // センサから各種情報を読み取り
  IMU->readAccelData(IMU->accelCount);
  IMU->getAres();
  IMU->readGyroData(IMU->gyroCount);
  IMU->getGres();

  // 取得した加速度に解像度をかけて、バイアス値を引く
  IMU->ay = (float)IMU->accelCount[1]*IMU->aRes - IMU->accelBias[1];
  IMU->az = (float)IMU->accelCount[2]*IMU->aRes - IMU->accelBias[2];
  IMU->ax = (float)IMU->accelCount[0]*IMU->aRes - IMU->accelBias[0];

  // 取得したジャイロに解像度をかける
  IMU->gx = (float)IMU->gyroCount[0]*IMU->gRes;
  IMU->gy = (float)IMU->gyroCount[1]*IMU->gRes;
  IMU->gz = (float)IMU->gyroCount[2]*IMU->gRes;

  // 四元数を更新する際に必ず呼び出し
  IMU->updateTime();

  // 加速度・ジャイロを与えられた構造体に代入
  pSensorData -> accX = (int)(1000*IMU->ax);
  pSensorData -> accY = (int)(1000*IMU->ay);
  pSensorData -> accZ = (int)(1000*IMU->az);
  pSensorData -> gyroX = (int)(IMU->gx);
  pSensorData -> gyroY = (int)(IMU->gy);
  pSensorData -> gyroZ = (int)(IMU->gz);

}

//ハンドラ－１（センサーを読んでバッファリング）
void _readSensor(MPU9250* IMU) {
  sensorData s;
  getAcc(IMU, &s);
  sdBuff[buffPointer++] = s;
}

//ハンドラ－２（SD保存のフラグを管理）
void _buffSave() {
  buffSaveFlg = true;
}

// ハンドラ3 (経過秒数の表示)
void _showElapsedTime() {
  elapsedTime++;
  clearY2;
  M5.Lcd.drawCentreString((String(elapsedTime)+"seconds"), LCDcenterX, LCDcenterY2, 2);
}

// Y1, Y2, Y3それぞれの行のみをクリアするメソッド
void clearY1() { M5.Lcd.fillRect(0, LCDcenterY1, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearY2() { M5.Lcd.fillRect(0, LCDcenterY2, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearY3() { M5.Lcd.fillRect(0, LCDcenterY3, 320, 20, MyMenu.getrgb(128, 128, 128)); }

// いろいろと試してみたいことをテストするメソッド。本機能には絡みません。
void appSandBox(){
  MyMenu.drawAppMenu(F("SANDBOX"),F("A"),F("B"),F("C"));
  int a=0, b=0, c=0;
  char stringA[10] = "";
  char stringB[10] = "";
  char stringC[10] = "";

  M5.Lcd.drawCentreString("Press A to draw here", LCDcenterX, LCDcenterY1, 2);
  M5.Lcd.drawCentreString("Press B to draw here", LCDcenterX, LCDcenterY2, 2);
  M5.Lcd.drawCentreString("Press C to draw here", LCDcenterX, LCDcenterY3, 2);
  
  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  while(!M5.BtnB.wasPressed()){

    if(M5.BtnA.wasPressed()) {
      a++;
      sprintf(stringA, "a = %d", a);
      M5.Lcd.fillRect(0, LCDcenterY1, 320, 20, MyMenu.getrgb(128, 128, 128));
      M5.Lcd.drawCentreString(stringA, LCDcenterX, LCDcenterY1, 2);
    }

    if(M5.BtnB.wasPressed()) {
      b++;
      sprintf(stringB, "b = %d", b);
      M5.Lcd.fillRect(0, LCDcenterY2, 320, 20, MyMenu.getrgb(128,128,128));
      M5.Lcd.drawCentreString(stringB, LCDcenterX, LCDcenterY2, 2);
    }

    if(M5.BtnC.wasPressed()) {
      c++;
      sprintf(stringC, "c = %d", c);
      M5.Lcd.fillRect(0, LCDcenterY3, 320, 20, MyMenu.getrgb(128,128,128));
      M5.Lcd.drawCentreString(stringC, LCDcenterX, LCDcenterY3, 2);
    }

    M5.update();
  }

  MyMenu.show();
}


// --------------------- ここより下は一応残してある未使用メソッドたち ---------------------


/**
 * @fn nowTime
 * @brief 現在時刻を取得し、返す
 * 
 * @return    "YYYY-MM-DD_HH-MM-SS"の形の時刻情報
 * @attention 時刻情報を返すのはいいが、M5Stackは起動時に時刻が1970/1/1でスタートするので
 *            ほとんどこのメソッドの意味はないかもしれない。
 * @note      return文はもっときれいな書き方があるはず。
 */
String nowTime() {
  struct tm tm;
  time_t t = time(NULL);
  localtime_r(&t, &tm);
  
  return String(tm.tm_year + 1900)+"-"+
         String(tm.tm_mon + 1)+"-"+
         String(tm.tm_mday)+"_"+
         String(tm.tm_hour)+"-"+
         String(tm.tm_min)+"-"+
         String(tm.tm_sec);
}



// --------------------- ここより下はもともと用意されていたメソッドたち ---------------------

//void appTEMPLATE(){
//  MyMenu.drawAppMenu(F("APP_TITTLE"),F("BTN_A_TITTLE"),F("BTN_B_TITTLE"),F("BTN_C_TITTLE"));
//
//  while(M5.BtnB.wasPressed()){
//    M5.update();
//  }
//    
//  while(!M5.BtnB.wasPressed()){
//    M5.update();
//  }
//
//  MyMenu.show();
//}