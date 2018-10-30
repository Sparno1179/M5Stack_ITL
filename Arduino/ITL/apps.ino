#include <time.h>
#include "utility/MPU9250.h"
#include "utility/quaternionFilters.h"

// LCDの中央座標を示す。何かと使うので定義。
#define LCDcenterX 160
#define LCDcenterY 120

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
void getAcc(MPU9250 IMU, sensorData *pSensorData);
int writeFile(fs::FS &fs, const char * path, const char * message);

// VScodeでエラーを出さないための記述。Arduinoでコンパイル時はコメントアウトすること。
// #include <M5StackSAM.h>
// M5SAM MyMenu;


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
  M5.Lcd.drawCentreString(F("File count"),LCDcenterX,(LCDcenterY)-30,2);
  M5.Lcd.drawCentreString(String(fileCount(SD, "/acc", 0))+"  CSVs in SD card",LCDcenterX,(LCDcenterY)-10,2);   
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
 * @attention どうやらセンサ初期化時の加速度・ジャイロが基準になるため
 *            動いてるときにセンサの初期化が始まるとでたらめな計測結果になります。
 */
void appDrawAccGyro() {
  MyMenu.drawAppMenu(F("current Acc and Gyro"),F(""),F("EXIT"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  // 加速度・ジャイロセンサの宣言・初期化
  M5.Lcd.drawCentreString("Loading...", LCDcenterX, LCDcenterY, 2);
  MPU9250 IMU;
  IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
  IMU.initMPU9250();
  struct sensorData sensorData = {0, 0, 0, 0, 0, 0};

  MyMenu.drawAppMenu(F("Current Acc and Gyro"),F("START"),F("EXIT"),F(""));
  MyMenu.windowClr();
  M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY, 2);
    
    while(!M5.BtnB.wasPressed()){
    
      if(M5.BtnA.wasPressed()) {
        MyMenu.drawAppMenu(F("Current Acc and Gyro"),F(""),F("EXIT"),F("STOP"));

        // 加速度・ジャイロ表示ループ
        while(!M5.BtnC.wasPressed()) {
          getAcc(IMU, &sensorData);
          MyMenu.windowClr();
          // 加速度を表示
          M5.Lcd.drawCentreString("Acceleration", LCDcenterX, LCDcenterY-35, 2);
          M5.Lcd.drawCentreString(String(sensorData.accX)+"  "+
                                  String(sensorData.accY)+"  "+
                                  String(sensorData.accZ),
                                  LCDcenterX, LCDcenterY-20, 2);
          // ジャイロを表示
          M5.Lcd.drawCentreString("Gyro", LCDcenterX, LCDcenterY, 2);
          M5.Lcd.drawCentreString(String(sensorData.gyroX)+"  "+
                                  String(sensorData.gyroY)+"  "+
                                  String(sensorData.gyroZ),
                                  LCDcenterX, LCDcenterY+15, 2);
          delay(15.625);
          M5.update();
        }

        // 加速度・ジャイロ表示ループ終了
        MyMenu.drawAppMenu(F("Current Acc and Gyro"),F("START"),F("EXIT"),F(""));
        MyMenu.windowClr();
        M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY, 2);
      }
    M5.update();
    }
    MyMenu.show();
}

// TODO
// 加速度を計測し、SDカードに継続的に保存。
/**
 * @brief 加速度・ジャイロを計測し、SDカードに保存
 * 
 * 主要機能。
 * ファイル名を時刻にしようかと思ったがきつそう(nowTimeメソッド参照)なので
 * どのようにファイル名を付けるか模索中。書き込みタイミングや形式などいろいろ決めなければいけない。
 * まだ未完成。
 * センサデータの取得についてはほぼ問題なし。SDカードへの書き込みもテスト済み。
 * 
 */
void appSaveAcc() {
  MyMenu.drawAppMenu(F("Save Acceleration"),F("OK"),F("EXIT"),F("NEXT"));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  // 加速度センサを初期化
  MPU9250 IMU;
  IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
  IMU.initMPU9250();  

  // センサデータ構造体を初期化
  struct sensorData sensorData = {0, 0, 0, 0, 0, 0};

  // メインループ
  while(!M5.BtnB.wasPressed()){
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

  M5.Lcd.drawCentreString(nowTime(), LCDcenterX, LCDcenterY-30, 2);
  
  while(!M5.BtnB.wasPressed()){
    if(M5.BtnA.wasPressed()) {
      M5.Lcd.drawCentreString(nowTime(), LCDcenterX, LCDcenterY-30, 2);
    }
    M5.update();
  }

  MyMenu.show();
}


// TODO
/**
 * @brief SDカードへファイルを保存するテスト
 * 
 * @note テストなので実際に使うことはない。そのうち削除。
 */
void appSaveToSD() {
  MyMenu.drawAppMenu(F("Write Data To SD"),F(""),F("EXIT"),F("WRITE"));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  M5.Lcd.drawCentreString("Write test CSV file to SD/acc", LCDcenterX, LCDcenterY-30, 2);
  writeFile(SD, "/acc/test.csv", "1,5,6");
  
  while(!M5.BtnB.wasPressed()){
    if(M5.BtnC.wasPressed()) {  
      switch(writeFile(SD, "/acc/test.csv", "1,5,6")) {
        case 0: M5.Lcd.drawCentreString("Write data successfully", LCDcenterX, LCDcenterY, 2); break;
        case 1: M5.Lcd.drawCentreString("Failed to open file", LCDcenterX, LCDcenterY, 2); break;
        case 2: M5.Lcd.drawCentreString("Failed to write data", LCDcenterX, LCDcenterY, 2); break;
      }

    }
    M5.update();
  }

  MyMenu.show();
}

/**
 * @brief SD/jpg/Tetris.jpgを表示する
 * 
 * 上記の写真を表示するだけです。
 * 画像表示テストに使っただけなのでそのうち消します。
 */
void appDrawJpeg(){
  MyMenu.drawAppMenu(F("BEST"),F(""),F("EXIT"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }
  M5.Lcd.drawJpgFile(SD, "/jpg/Tetris.jpg", 3, 34, 150, 150);
  while(!M5.BtnB.wasPressed()){
    M5.update();
  }
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
    if(!root || !root.isDirectory()){
        return -1; // フォルダを開けなかった
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
 *
 * @param[in] fs 書き出し先ファイルシステム。たいていはSD
 * @param[in] path 出力先ファイルパス。なければ自動で作成される。
 * @retval 0 ファイル読み込みエラー。同名ファイルが書き込み禁止になっているなど
 * @retval 1 書き出し正常終了
 * @retval 2 ファイル書き込みエラー
 */
int writeFile(fs::FS &fs, const char * path, const char * message){
  
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        return 1;
    }
    if(file.println(message)){
        return 0;
    } else {
        return 2;
    }
}

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


/**
 * @fn
 * getAcc
 * @brief センサから加速度とジャイロを取得する
 * @param[in] MPU 9軸センサのインスタンス
 * @param[in,out] *pSensorData センサデータ構造体のポインタ
 * @return なし。与えられたポインタ先のメンバに直接書き込む
 */
void getAcc(MPU9250 IMU, sensorData *pSensorData) {
  // センサから各種情報を読み取り
  IMU.readAccelData(IMU.accelCount);
  IMU.getAres();
  IMU.readGyroData(IMU.gyroCount);
  IMU.getGres();

  // 取得した加速度を何かしらの計算にかける。
  IMU.ax = (float)IMU.accelCount[0]*IMU.aRes;
  IMU.ay = (float)IMU.accelCount[1]*IMU.aRes;
  IMU.az = (float)IMU.accelCount[2]*IMU.aRes;

  // 取得したジャイロを何かしらの計算にかける
  IMU.gx = (float)IMU.gyroCount[0]*IMU.gRes;
  IMU.gy = (float)IMU.gyroCount[1]*IMU.gRes;
  IMU.gz = (float)IMU.gyroCount[2]*IMU.gRes;

  // 四元数を更新する際に必ず呼び出し
  IMU.updateTime();

  // 加速度・ジャイロを与えられた構造体に代入
  pSensorData -> accX = (int)(1000*IMU.ax);
  pSensorData -> accY = (int)(1000*IMU.ay);
  pSensorData -> accZ = (int)(1000*IMU.ax);
  pSensorData -> gyroX = (int)(IMU.gx);
  pSensorData -> gyroY = (int)(IMU.gy);
  pSensorData -> gyroZ = (int)(IMU.gz);
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