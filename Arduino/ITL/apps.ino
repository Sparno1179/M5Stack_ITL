#include <Ticker.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "utility/MPU9250.h"
#include "utility/quaternionFilters.h"
#include <M5StackSAM.h>


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
  M5.Lcd.drawString(String(fileCount(SD, "/acc/walk", 0))+"  CSVs in WALK",LCDcenterX-50,LCDcenterY1-50,2);
  M5.Lcd.drawString(String(fileCount(SD, "/acc/stand", 0))+"  CSVs in STAND",LCDcenterX-50,LCDcenterY1-30,2);
  M5.Lcd.drawString(String(fileCount(SD, "/acc/sit", 0))+"  CSVs in SIT",LCDcenterX-50,LCDcenterY1-10,2);
  M5.Lcd.drawString(String(fileCount(SD, "/acc/stdown", 0))+"  CSVs in STAIR_DOWN",LCDcenterX-50,LCDcenterY2-10,2);
  M5.Lcd.drawString(String(fileCount(SD, "/acc/stup", 0))+"  CSVs in STAIR_UP",LCDcenterX-50,LCDcenterY3-10,2);
  M5.Lcd.drawString(String(fileCount(SD, "/acc/lying", 0))+"  CSVs in LYING",LCDcenterX-50,LCDcenterY3+10,2);
  M5.Lcd.drawString(String(fileCount(SD, "/acc/vehicle", 0))+"  CSVs in VEHICLE",LCDcenterX-50,LCDcenterY3+30,2);
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

    if(abs(sensorData.accX - sensorData.accY) < 70) {
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
int measure_time = 0;
int fileIndex = 0;
char randStr[10];
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
  MPU9250 IMU;
  struct sensorData sensorData = {0, 0, 0, 0, 0, 0};

  while(true) {
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    IMU.initMPU9250();
    getAcc(&IMU, &sensorData);

    if(abs(sensorData.accX - sensorData.accY) < 70) {
      break;
    }
    delay(50);
  }

  // ファイル名用ランダム文字列の生成
  rand_text(randStr);

  MyMenu.drawAppMenu(F("Save acc and gyro"),F("OK"),F("EXIT"),F("NEXT"));
  MyMenu.windowClr();

  char movementList[7][20] = {"walk", "stand", "sit", "stdown", "stup", "lying", "vehicle"};
  int movementID = -1;
  MyMenu.clearList();
  MyMenu.setListCaption("Movement Type");
  MyMenu.addList("WALK");
  MyMenu.addList("STAND");
  MyMenu.addList("SIT");
  MyMenu.addList("STAIR_DOWN");
  MyMenu.addList("STAIR_UP");
  MyMenu.addList("LYING");
  MyMenu.addList("VEHICLE");
  MyMenu.showList();

  sdBuff = (struct sensorData*)malloc( sizeof(struct sensorData)*4096); //長さ4096を確保 ＊最後にfree(sdBuff)


  // メインループ
  while(!M5.BtnB.wasPressed()){

    // Cボタンでリスト送り。ついでにファイル名用乱数生成
    if(M5.BtnC.wasPressed()) {
      MyMenu.drawAppMenu(F("save acc and gyro"),F("OK"),F("EXIT"),F("NEXT"));
      MyMenu.nextList();
      // ファイル名用ランダム文字列の生成
      fileIndex = 0;
      rand_text(randStr);
    }

    // Aボタンを押すと計測スタート
    if(M5.BtnA.wasPressed()) {

      // 行動IDを取得
      for(int i=-1; i < MyMenu.getListID(); i++) {
        movementID = i + 1;
      }
      // 行動IDによってタイトルやTickerの時間を調整
      MyMenu.drawAppMenu(("SAVE "+String(movementList[movementID])),F(""),F("EXIT"),F("STOP"));
      if(movementID == 3 || movementID == 4) {
        measure_time = 5000; // 階段なら５秒計測
      } else {
        measure_time = 30000; // 階段以外なら30秒計測
      }

      // 計測開始
      M5.Lcd.drawCentreString("Waiting...", LCDcenterX, LCDcenterY2, 2);
      delay(5000);
      MyMenu.windowClr();
      // 16ミリ秒ごと(62.5Hz)にセンサーリード
      tickerSensor.attach_ms<MPU9250*>(16, _readSensor, &IMU);
      // 1秒ごとに経過時間を表示
      tickerShowTime.attach_ms(1000, _showElapsedTime);
      // 30秒ごとにフラグ（buffSaveFlg）を立てる
      tickerWriteData.attach_ms(measure_time, _buffSave);

      while(!M5.BtnC.wasPressed()) {
        M5.update();
        if(buffSaveFlg) {
          Serial.println("buffSaveFlg setted!");
          //タイマーを止める
          tickerSensor.detach();
          tickerShowTime.detach();
          tickerWriteData.detach();
          Serial.println("All ticker detached!");

          // ファイル名のフォーマットは "MACアドレス_乱数_通し番号.csv"
          char fileName[50] = {};
          // movementIDより、保存先パスの設定
          sprintf(fileName, "/acc/%s", movementList[movementID]);
          //ファイル作成
          Serial.println("making csv file...");
          sprintf(fileName, "/acc/%s/%s_%s_%d.csv", movementList[movementID], getMacAddr().c_str(), randStr, fileIndex);
          Serial.println("filename created completely.");
          fileIndex++;
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
          clearLCDY1();
          Serial.println("CSV writing...");
          M5.Lcd.drawCentreString("CSV Writing...", LCDcenterX, LCDcenterY1, 2);
          for(int i = 0; i < buffPointer; i++) {
            char buf[64];
            sprintf(buf, "%d, %d, %d, %d, %d, %d", sdBuff[i].accX, sdBuff[i].accY,sdBuff[i].accZ, sdBuff[i].gyroX, sdBuff[i].gyroY, sdBuff[i].gyroZ);
            file.println(buf);
          }

          file.close();
          Serial.println("File closed!");

          clearLCDY1();
          M5.Lcd.drawCentreString("Write Complete!", LCDcenterX, LCDcenterY3, 2);

          //バッファ初期化
          buffPointer = 0;
          buffSaveFlg = false;

          // 計測開始
          MyMenu.windowClr();
          tickerSensor.attach_ms<MPU9250*>(16,_readSensor, &IMU);
          tickerShowTime.attach_ms(1000, _showElapsedTime);
          tickerWriteData.attach_ms(measure_time, _buffSave);
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

      // 加速度・ジャイロ保存終了
      MyMenu.drawAppMenu(F("Save acc and gyro"),F("OK"),F("EXIT"),F("NEXT"));
      MyMenu.showList();
    }
    M5.update();
  }
  free(sdBuff);
  MyMenu.show();
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
  clearLCDY2;
  M5.Lcd.drawCentreString((String(elapsedTime)+"seconds"), LCDcenterX, LCDcenterY2, 2);
}

// Y1, Y2, Y3それぞれの行のみをクリアするメソッド
void clearLCDY1() { M5.Lcd.fillRect(0, LCDcenterY1, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY2() { M5.Lcd.fillRect(0, LCDcenterY2, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY3() { M5.Lcd.fillRect(0, LCDcenterY3, 320, 20, MyMenu.getrgb(128, 128, 128)); }

// いろいろと試してみたいことをテストするメソッド。本機能には絡みません。
void appSandBox(){
  MyMenu.drawAppMenu(F("SANDBOX"),F("A"),F("B"),F("C"));
  int a=0, b=0, c=0;
  char stringA[10] = "";
  char stringB[11];
  char stringC[128] = "";

  MPU9250 sensor;


  M5.Lcd.drawCentreString("Press A to draw macAddr", LCDcenterX, LCDcenterY1, 2);
  M5.Lcd.drawCentreString("Press B to EXIT", LCDcenterX, LCDcenterY2, 2);
  M5.Lcd.drawCentreString("Press C to show randStr", LCDcenterX, LCDcenterY3, 2);

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  while(!M5.BtnB.wasPressed()){

    if(M5.BtnA.wasPressed()) {
      a++;
      M5.Lcd.fillRect(0, LCDcenterY1, 320, 20, MyMenu.getrgb(128, 128, 128));
      M5.Lcd.drawCentreString(getMacAddr(), LCDcenterX, LCDcenterY1, 2);
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
      rand_text(stringA);
      M5.Lcd.drawCentreString(stringA, LCDcenterX, LCDcenterY3, 2);
    }

    M5.update();
  }

  MyMenu.show();
}


// 画面の輝度を設定
void appSetBrightness(){
  byte tmp_brigth = byte(EEPROM.read(0));
  byte tmp_lbrigth = 0;

  MyMenu.drawAppMenu(F("DISPLAY BRIGHTNESS"),F("-"),F("OK"),F("+"));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  while(!M5.BtnB.wasPressed()){
    if(M5.BtnA.wasPressed() and tmp_brigth >= 16){
      tmp_brigth = tmp_brigth - 16;
    }
    if(M5.BtnC.wasPressed() and tmp_brigth <= 239){
      tmp_brigth = tmp_brigth + 16;
    }
    if(tmp_lbrigth != tmp_brigth){
      tmp_lbrigth = tmp_brigth;
      EEPROM.write(0,tmp_lbrigth);
      EEPROM.commit();
      M5.lcd.setBrightness(byte(EEPROM.read(0)));
      MyMenu.windowClr();
      M5.Lcd.drawNumber(byte(EEPROM.read(0)), 120 , 90, 6);
    }
    M5.update();
  }
  MyMenu.show();
}


// スリープ（充電時などに使用）
void appSleep(){
  M5.setWakeupButton(BUTTON_B_PIN);
  M5.powerOFF();
}

// 乱数ジェネレータ(10文字)
void rand_text(char *randStr) {
  int i;
  char char_set[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  // 起動からの経過時間（マイクロ秒）をシード値に。
  srand(getMicroSec());

  for (i = 0; i < 10; i++) {
    randStr[i] = char_set[rand() % 61 + 1];
  }
  randStr[10] = 0;
}


// 現在時刻のマイクロ秒部分を取得し、返す。rand()のシード用
unsigned long getMicroSec() {
  struct timeval tv = {0};
  gettimeofday(&tv, NULL);

  return tv.tv_usec;
}

// MACアドレスを取得。間のダブルコロンは省きます
String getMacAddr() {
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[16] = {0};
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
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