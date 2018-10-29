#include <Ticker.h>
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
void _readSensor(MPU9250* IMU);
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

//タイマー
Ticker tickerSensor;
Ticker tickerWriteData;

//バッファ
sensorData *sdBuff = NULL;
int saveIndex = 0;
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
    M5.Lcd.drawCentreString("Loading...", LCDcenterX, LCDcenterY, 2);
    static MPU9250 IMU;
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    IMU.initMPU9250();
  
    MyMenu.drawAppMenu(F("Current Acc and Gyro"),F("START"),F("EXIT"),F(""));
    MyMenu.windowClr();
    M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY, 2);
    sdBuff = (struct sensorData*)malloc( sizeof(struct sensorData)*4096); //長さ4096を確保 ＊最後にfree(sdBuff)
    while(!M5.BtnB.wasPressed()){
      if(M5.BtnA.wasPressed()) {
        MyMenu.drawAppMenu(F("keisoku chu desu"),F(""),F("EXIT"),F("STOP"));

        // 16ミリ秒ごと(62.5Hz)にセンサーリード
        tickerSensor.attach_ms<MPU9250*>(16,_readSensor, &IMU);
        // 60秒ごとにフラグ（buffSaveFlg）を立てる
        tickerWriteData.attach_ms(60000, _buffSave); 

        while(!M5.BtnC.wasPressed()) {
          M5.update();
          if(buffSaveFlg) {
            //タイマーを止める
            tickerSensor.detach();
            tickerWriteData.detach();
            
            //ファイル作成
            char fileName[16];
            sprintf(fileName, "/walk/%d.csv", saveIndex++);
            File file = SD.open(fileName, FILE_WRITE);
            
            //Serial.println(fileName);
            //ファイルが開けないとき
            if(!file) {
                //M5.Lcd.drawCentreString("SD not found?Plz Insert SD and reboot", LCDcenterX, LCDcenterY, 2);
                Serial.println("SD not found?Plz Insert SD and reboot");
                tickerSensor.detach();
                tickerWriteData.detach();
                break;
            }

            //バッファを全て書き込み
            for(int i = 0; i < buffPointer; i++) {
              char buf[64];
              sprintf(buf, "%d, %d, %d, %d, %d, %d", sdBuff[i].accX, sdBuff[i].accY,sdBuff[i].accZ, sdBuff[i].gyroX, sdBuff[i].gyroY, sdBuff[i].gyroZ);
              file.println(buf);
            }
            
            file.close();
            
            //バッファ初期化
            buffPointer = 0;
            buffSaveFlg = false;
            
            // 計測開始
            tickerSensor.attach_ms<MPU9250*>(16,_readSensor, &IMU);
            tickerWriteData.attach_ms(60000, _buffSave); 
          }
          delay(100);
        }
        
        //タイマーを止める
        tickerSensor.detach();
        tickerWriteData.detach();
        //バッファ初期化
        buffPointer = 0;
        buffSaveFlg = false;

        // 加速度・ジャイロ表示ループ終了
        MyMenu.drawAppMenu(F("Current Acc and Gyro"),F("START"),F("EXIT"),F(""));
        MyMenu.windowClr();
        M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY, 2);
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

//ハンドラ－１（センサーを読んでバッファリング）
void _readSensor(MPU9250* IMU) {
  sensorData s;
  getAcc(*IMU, &s);
  sdBuff[buffPointer++] = s;
}

//ハンドラ－２（SD保存のフラグを管理）
void _buffSave() {
  buffSaveFlg = true;
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

String getWiFiMac() {
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
}


void appAbout(){
  MyMenu.drawAppMenu(F("ABOUT"),F(""),F("ESC"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }
  MyMenu.windowClr();
  M5.Lcd.drawCentreString(F("File count"),LCDcenterX,(LCDcenterY)-30,2);
  M5.Lcd.drawCentreString(String(fileCount(SD, "/jpg", 0))+"  CSVs",LCDcenterX,(LCDcenterY)-10,2);   
  while(!M5.BtnB.wasPressed()){
    M5.update();
  }

  MyMenu.show();
}


void appListDemo(){
  MyMenu.drawAppMenu(F("LIST DEMO"),F("SELECT"),F("ESC"),F("LIST"));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  MyMenu.clearList();
  MyMenu.setListCaption("DEMO LIST");
  MyMenu.addList("AHOOJ");  
  MyMenu.showList();  

  while(!M5.BtnB.wasPressed()){
    if(M5.BtnC.wasPressed()){
      MyMenu.nextList();
    }
    if(M5.BtnA.wasPressed()){
      MyMenu.windowClr();
      M5.Lcd.drawCentreString("ID: "+String(MyMenu.getListID()),LCDcenterX,(LCDcenterY)-10,2);
      M5.Lcd.drawCentreString("TEXT: "+MyMenu.getListString(),LCDcenterX,(LCDcenterY)+10,2);
    }
    M5.update();
  }
  
  MyMenu.show();
}


void appFacesGetString(){
  String tmpStr = "";
  MyMenu.drawAppMenu(F("FACES GET STRING"),F(""),F("ESC"),F(""));
  MyMenu.windowClr();
  M5.Lcd.drawCentreString("WRITE TEXT AND PRESS ENTER (OK)",LCDcenterX,LCDcenterY,2);
  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  tmpStr = MyMenu.keyboardGetString();
  MyMenu.windowClr();
  M5.Lcd.drawCentreString(tmpStr,LCDcenterX,LCDcenterY,2);

  while(!M5.BtnB.wasPressed()){
    M5.update();
  }
  
  MyMenu.show();
}

void appSysInfo(){
  MyMenu.drawAppMenu(F("M5 SYSTEM INFO"),F(""),F("ESC"),F(""));

  uint8_t chipRev = ESP.getChipRevision();
  uint8_t cpuSpeed = ESP.getCpuFreqMHz();
  uint32_t flashSize = ESP.getFlashChipSize();
  uint32_t flashSpeed = ESP.getFlashChipSpeed();
  const char * sdkVer = ESP.getSdkVersion();
  String WiFiMAC = getWiFiMac();
  uint32_t ramFree = system_get_free_heap_size();

  while(M5.BtnB.wasPressed()){
    M5.update();
  }
  
  M5.Lcd.drawString(F("CPU_FREQ:"),10,40,2);
  M5.Lcd.drawNumber(cpuSpeed, 120, 40, 2);
  
  M5.Lcd.drawString(F("CHIP_REV:"),10,60,2);
  M5.Lcd.drawNumber(chipRev, 120, 60, 2);
    
  M5.Lcd.drawString(F("FLASH_SIZE:"),10,80,2);
  M5.Lcd.drawNumber(flashSize, 120, 80, 2);

  M5.Lcd.drawString(F("FLASH_SPEED:"),10,100,2);
  M5.Lcd.drawNumber(flashSpeed, 120, 100, 2);

  M5.Lcd.drawString(F("SDK_VERSION:"),10,120,2);
  M5.Lcd.drawString(sdkVer,120,120,2);

  M5.Lcd.drawString(F("WIFI_STA_MAC:"),10,140,2);
  M5.Lcd.drawString(WiFiMAC,120,140,2);

  M5.Lcd.drawString(F("FREE_RAM:"),10,160,2);
  M5.Lcd.drawNumber(ramFree,120,160,2);
    
  while(!M5.BtnB.wasPressed()){
    M5.update();
  }

  MyMenu.show();
}

void appIICScanner(){
  byte error, address;
  int nDevices;
  byte ridx = 0;
  byte lidx = 0;
  boolean scanrun = HIGH;
  
  MyMenu.drawAppMenu(F("I2C SCANNER"),F("SCAN"),F("ESC"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }
    
  while(!M5.BtnB.wasPressed()){
    if(scanrun==HIGH){
      scanrun = LOW;
      nDevices = 0;
      for(address = 1; address < 127; address++ ){
        ridx++;
        if(ridx==17){
          ridx = 1;
          lidx++;
        }
        Wire.beginTransmission(address);
        error = Wire.endTransmission();    
        if (error == 0){
          M5.Lcd.drawString(String(address,HEX),0+(ridx*18),45+(lidx*20),2);
          nDevices++;
        }else if (error==4){
          M5.Lcd.drawString(F("ER"),0+(ridx*18),45+(lidx*20),2);
        }else{
          M5.Lcd.drawString(F("--"),0+(ridx*18),45+(lidx*20),2);
        }
      }
      M5.update();
    }else{
      if(M5.BtnA.wasPressed()){
        MyMenu.windowClr();
        ridx = 0;
        lidx = 0;
        scanrun = HIGH;
      }
      M5.update();
    }
  }  
  MyMenu.show();  
}

void appWiFiScanner(){
  uint16_t wifi_count = 0;
  boolean wifi_showlock = LOW;
  byte list_lines = 5;
  uint16_t list_page = 0;
  uint16_t list_pages = 0;
  uint16_t list_lastpagelines = 0;

  MyMenu.drawAppMenu(F("WiFi SCANNER"),F("SCAN"),F("ESC"),F("PAGE"));

  M5.Lcd.drawCentreString(F("SCANNING....."),LCDcenterX,LCDcenterY,2);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  while(M5.BtnB.wasPressed()){
    M5.update();
  }  

  wifi_count = WiFi.scanNetworks();

  if(wifi_count > 0){    
    if(wifi_count > list_lines){
      list_lastpagelines = wifi_count % list_lines;
      if(list_lastpagelines>0){
        list_pages = (wifi_count - list_lastpagelines) / list_lines;
        list_pages++;
      }else{
        list_pages = wifi_count / list_lines;
      }
    }else{
      list_pages = 1;
    }
  }

  while(!M5.BtnB.wasPressed()){
    if(wifi_count==0){
      if(!wifi_showlock){
        MyMenu.windowClr();
        M5.Lcd.drawCentreString(F("NO NETWORKS FOUND"),LCDcenterX,LCDcenterY,2);
        wifi_showlock = HIGH;        
      }
    }else{
      if(!wifi_showlock){
        MyMenu.windowClr();
        M5.Lcd.drawCentreString("FOUND "+String(wifi_count)+" NETWORKS, PAGE: "+String(list_page+1)+"/"+String(list_pages),LCDcenterX,40,2);
        if((list_page + 1) == list_pages){
          if(list_lastpagelines == 0 and wifi_count >= list_lines){
            for(byte i = 0;i<list_lines;i++){
              M5.Lcd.drawString(WiFi.SSID(i+(list_page*list_lines)),5,80+(i*20),2);
              M5.Lcd.drawString(String(WiFi.RSSI(i+(list_page*list_lines))) + " dB",250,80+(i*20),2);
              M5.Lcd.drawString((WiFi.encryptionType(i+(list_page*list_lines)) == WIFI_AUTH_OPEN)?" ":"*",310,80+(i*20),2);
            }
          }else{
            if(list_pages>1){
              for(byte i = 0;i<list_lastpagelines;i++){
                M5.Lcd.drawString(WiFi.SSID(i+(list_page*list_lines)),5,80+(i*20),2);
                M5.Lcd.drawString(String(WiFi.RSSI(i+(list_page*list_lines))) + " dB",250,80+(i*20),2);
                M5.Lcd.drawString((WiFi.encryptionType(i+(list_page*list_lines)) == WIFI_AUTH_OPEN)?" ":"*",310,80+(i*20),2);
              }            
            }else{
              for(byte i = 0;i<wifi_count;i++){
                M5.Lcd.drawString(WiFi.SSID(i+(list_page*list_lines)),5,80+(i*20),2);
                M5.Lcd.drawString(String(WiFi.RSSI(i+(list_page*list_lines))) + " dB",250,80+(i*20),2);
                M5.Lcd.drawString((WiFi.encryptionType(i+(list_page*list_lines)) == WIFI_AUTH_OPEN)?" ":"*",310,80+(i*20),2);
              }                          
            }
          }
        }else{
            for(byte i = 0;i<list_lines;i++){
              M5.Lcd.drawString(WiFi.SSID(i+(list_page*list_lines)),5,80+(i*20),2);
              M5.Lcd.drawString(String(WiFi.RSSI(i+(list_page*list_lines))) + " dB",250,80+(i*20),2);
              M5.Lcd.drawString((WiFi.encryptionType(i+(list_page*list_lines)) == WIFI_AUTH_OPEN)?" ":"*",310,80+(i*20),2);
            }          
        }
        wifi_showlock = HIGH;        
      }       
    }
    if(M5.BtnA.wasPressed()){
        list_page = 0;
        list_pages = 0;
        MyMenu.windowClr();
        M5.Lcd.drawCentreString(F("SCANNING....."),LCDcenterX,LCDcenterY,2);
        wifi_count = WiFi.scanNetworks();
        wifi_showlock = LOW;
        if(wifi_count > 0){    
          if(wifi_count > list_lines){
            list_lastpagelines = wifi_count % list_lines;
            if(list_lastpagelines>0){
              list_pages = (wifi_count - list_lastpagelines) / list_lines;
              list_pages++;
            }else{
              list_pages = wifi_count / list_lines;
            }
          }else{
            list_pages = 1;
          }
        }
    }
    if(M5.BtnC.wasPressed()){
      if(wifi_count > 0 and list_pages > 1){
        list_page++;
        if(list_page == list_pages){
          list_page = 0;
        }
        wifi_showlock = LOW;
      }
    }
    M5.update(); 
  }
  MyMenu.show();
}

void appSleep(){
  M5.setWakeupButton(BUTTON_B_PIN);
  M5.powerOFF();
}

void appCfgBrigthness(){
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

void appStopWatch(){
  boolean tmp_run = false;
  float tmp_sec = 0;
  unsigned int tmp_min = 0;
  unsigned long tmp_tmr = 0;
  
  MyMenu.drawAppMenu(F("STOPWATCH"),F("S/S"),F("ESC"),F("RES"));

  M5.Lcd.drawString(F("MIN"),40,120,2);
  M5.Lcd.drawString(F("SEC"),170,120,2);
  M5.Lcd.drawFloat(tmp_sec, 1, 210, 100, 6);
  M5.Lcd.drawNumber(tmp_min, 80, 100, 6);

  while(M5.BtnB.wasPressed()){
    M5.update();
  }  

  while(!M5.BtnB.wasPressed()){
    M5.update();
    if(M5.BtnC.wasPressed()){
      tmp_sec = 0;
      tmp_min = 0;      
      MyMenu.windowClr();
      M5.Lcd.drawString(F("MIN"),40,120,2);
      M5.Lcd.drawString(F("SEC"),170,120,2);
      M5.Lcd.drawFloat(tmp_sec, 1, 210, 100, 6);
      M5.Lcd.drawNumber(tmp_min, 80, 100, 6);
    }
    if(tmp_run){
      if(M5.BtnA.wasPressed()){
        tmp_run = LOW;
      }
      if(millis()-tmp_tmr > 100){
        tmp_tmr = millis();
        tmp_sec = tmp_sec + 0.1;
        if(tmp_sec > 59.9){
          tmp_sec = 0;
          tmp_min++;
          MyMenu.windowClr();
          M5.Lcd.drawString(F("MIN"),40,120,2);
          M5.Lcd.drawString(F("SEC"),170,120,2);
          M5.Lcd.drawFloat(tmp_sec, 1, 210, 100, 6);
          M5.Lcd.drawNumber(tmp_min, 80, 100, 6);
        }
        M5.Lcd.drawFloat(tmp_sec, 1, 210, 100, 6);
        M5.Lcd.drawNumber(tmp_min, 80, 100, 6);
      }      
    }else{
      if(M5.BtnA.wasPressed()){
        //tmp_sec = 0;
        //tmp_min = 0;
        tmp_run = HIGH;
      }
    }
  }
  MyMenu.show();  
}
