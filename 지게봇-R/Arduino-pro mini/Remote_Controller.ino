/*
 WiFiEsp test: ClientTest
http://www.kccistc.net/
작성일 : 2019.12.17 
작성자 : IoT 임베디드 KSH
*/
//#define DEBUG
//#define DEBUG_WIFI
#define AP_SSID "iot1"
#define AP_PASS "iot10000"
#define SERVER_NAME "10.10.141.35"
#define SERVER_PORT 5000  
#define LOGID "ARD_ROS"
#define PASSWD "PASSWD"


#define JOYSTIC_X A0
#define JOYSTIC_Y A1
#define JOYSTIC_SW 2
#define WIFITX 7  //7:TX -->ESP8266 RX
#define WIFIRX 6 //6:RX-->ESP8266 TX
#define LED_TEST_PIN 12
#define LED_BUILTIN_PIN 13

#define CMD_SIZE 50
#define ARR_CNT 5           

#include <ezButton.h>
#include "WiFiEsp.h"
#include "SoftwareSerial.h"
#include <TimerOne.h>
#include <U8glib.h>

// I2C OLED 디스플레이 초기화
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI

char sendBuf[CMD_SIZE];

bool timerIsrFlag = false;
unsigned int secCount;
int sensorTime;
int joxOld;
int joyOld;
int z = 0;  //button 값 변수

const int BUTTON_NUM = 5;
const int BUTTON_1_PIN = 4;
const int BUTTON_2_PIN = 8;
const int BUTTON_3_PIN = 10;
const int BUTTON_4_PIN = 5;
const int BUTTON_5_PIN = 9;

ezButton buttonArray[] = {
  ezButton(BUTTON_1_PIN),
  ezButton(BUTTON_2_PIN),
  ezButton(BUTTON_3_PIN),
  ezButton(BUTTON_4_PIN),
  ezButton(BUTTON_5_PIN)
};

bool updateTimeFlag = false;
typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
} DATETIME;
DATETIME dateTime = {0, 0, 0, 12, 0, 0};
SoftwareSerial wifiSerial(WIFIRX, WIFITX); 
WiFiEspClient client;

//char oledLine1[17]="Home IoT By KSH";
char oledLine2[17]="WiFi Connecting";
char oledLine3[17]="";
char oledLine4[17]="";

void setup() {
  // put your setup code here, to run once:
    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);
    pinMode(BUTTON_3_PIN, INPUT_PULLUP);
    pinMode(BUTTON_4_PIN, INPUT_PULLUP);
    pinMode(BUTTON_5_PIN, INPUT_PULLUP);

    for (byte i = 0; i < BUTTON_NUM; i++) {
      buttonArray[i].setDebounceTime(50); // set debounce time to 50 milliseconds
    }

    pinMode(JOYSTIC_SW,INPUT_PULLUP);
  //u8g.setRot180(); 
    u8g.setColorIndex(1);
    u8g.setFont(u8g_font_unifont);
    displayDrawStr();

    pinMode(LED_TEST_PIN, OUTPUT);    //D5
    pinMode(LED_BUILTIN_PIN, OUTPUT); //D13
    Serial.begin(115200); //DEBUG
    wifi_Setup();

    Timer1.initialize(200000);    //500ms>250ms로 변경
    Timer1.attachInterrupt(timerIsr); // timerIsr to run every 1 seconds
}

void loop() {
  // put your main code here, to run repeatedly:
  if(client.available()) {
    socketEvent();
  }

  for (byte i = 0; i < BUTTON_NUM; i++){
    buttonArray[i].loop(); // MUST call the loop() function first
  }

  // 버튼 1이 눌렸을 때 (+값 전송)
  if (buttonArray[2].isReleased()) {
    if (z < 9) {
      z++;
    }
    sprintf(sendBuf, "[%s]CMDLIFT@%d\n", "QT_ROS", z);
    // Serial.print("Sending: ");
    // Serial.println(sendBuf);  // 디버깅용
    client.write(sendBuf, strlen(sendBuf));
    sprintf(oledLine3,"Lift:%d",-z);
    displayDrawStr();
  }

  // 버튼 2가 눌렸을 때 (0값 전송)
  if (buttonArray[1].isReleased()) {
    z = 0;
    sprintf(sendBuf, "[%s]CMDLIFT@%d\n", "QT_ROS", z);
    // Serial.print("Sending: ");
    // Serial.println(sendBuf);  // 디버깅용
    client.write(sendBuf, strlen(sendBuf));
    sprintf(oledLine3,"Lift:%d",z);
    displayDrawStr();
  }

  // 버튼 3이 눌렸을 때 (-값 전송)
  if (buttonArray[0].isReleased()) {
    if (z > -9) {
      z--;
    }
    sprintf(sendBuf, "[%s]CMDLIFT@%d\n", "QT_ROS", z);
    // Serial.print("Sending: ");
    // Serial.println(sendBuf);  // 디버깅용
    client.write(sendBuf, strlen(sendBuf));
    sprintf(oledLine3,"Lift:%d",-z);
    displayDrawStr();
  }

  if (timerIsrFlag)   //500ms 마다 실행
  {
    timerIsrFlag = false; 
    if (updateTimeFlag)
    {
      client.print("[GETTIME]\n");
      updateTimeFlag = false;
    } 
    int x = analogRead(JOYSTIC_X)+18; //off:522, 중심값 537  0범위 : 512~562 :
    int y = analogRead(JOYSTIC_Y)+24; //off:514, 중삼겂 537  0범위 : 512~562 
    int jox = -map(x, 0, 1023, -10, 10);
    int joy = map(y, 0, 1023, -10, 10);
    
    if(jox != joxOld || joy != joyOld)
    {
  /*  
      Serial.print(" Joy X : ");
      Serial.print(jox);
      Serial.print(" adc0 ");
      Serial.print(x);
      
      Serial.print("    Joy y : ");
      Serial.print(joy);
      Serial.print(" adc1 ");
      Serial.print(y);
      Serial.print(" sw ");
      Serial.println(!digitalRead(JOYSTIC_SW)); 
      joxOld = jox;
      joyOld = joy;  
  */    
  
      sprintf(sendBuf,"[%s]CMDVEL@%d@%d\n","QT_ROS",jox,joy);
      client.write(sendBuf,strlen(sendBuf));
  
      sprintf(oledLine3,"Vel:%d, Ang:%d",jox,joy);
      displayDrawStr();

      joxOld = jox;
      joyOld = joy;
    }
    if(!digitalRead(JOYSTIC_SW))
    {
      sprintf(sendBuf,"[%s]CMDVEL@%d@%d\n","QT_ROS",0,0);
      client.write(sendBuf,strlen(sendBuf));
      sprintf(oledLine3,"Vel:%d, Ang:%d",jox,joy);
      displayDrawStr();
    }
    
    if(!(secCount%10))
    {
      if (!client.connected()) { 
        sprintf(oledLine4,"Server Down");
        displayDrawStr();
        server_Connect();
      }
    } 
    if(!(secCount%10))
    {
      sprintf(oledLine2, "%02d.%02d  %02d:%02d:%02d", dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec );
      displayDrawStr();
    }

  }

}
void socketEvent()
{
  int i=0;
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0}; 
  int len;

  len =client.readBytesUntil('\n',recvBuf,CMD_SIZE); 
  client.flush();
#ifdef DEBUG
  Serial.print("recv : ");
  Serial.println(recvBuf);
#endif
  pToken = strtok(recvBuf,"[@]");
  while(pToken != NULL)
  {
    pArray[i] =  pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }

  if(!strncmp(pArray[1]," New",4))  // New Connected
  {
    sprintf(oledLine2, "Server Connected");
    displayDrawStr();
    updateTimeFlag = true;
    return ;
  }
  else if(!strncmp(pArray[1]," Alr",4)) //Already logged
  {
    client.stop();
    server_Connect();
    return ;
  }   
  if((pArray[1] != NULL) && (strlen(pArray[1])< 17))
  {
    sprintf(oledLine4,"%s",pArray[1]);
    if((pArray[2] != NULL) && (strlen(pArray[1]) + strlen(pArray[2])) < 16)
    {
      strcat(oledLine4," ");
      strcat(oledLine4,pArray[2]);
    }
    displayDrawStr();
  }  
  if(!strcmp(pArray[1],"LED")) {
    if(!strcmp(pArray[2],"ON")) {
      digitalWrite(LED_BUILTIN_PIN,HIGH);
    }
    else if(!strcmp(pArray[2],"OFF")) {
      digitalWrite(LED_BUILTIN_PIN,LOW);
    }
    sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  } else if(!strcmp(pArray[1],"LAMP")) {
    if(!strcmp(pArray[2],"ON")) {
      digitalWrite(LED_TEST_PIN,HIGH);
    }
    else if(!strcmp(pArray[2],"OFF"))
    {
      digitalWrite(LED_TEST_PIN,LOW);
    }
    sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  }else if(!strcmp(pArray[1],"GETSTATE")) {
    if(!strcmp(pArray[2],"DEV")) {
      sprintf(sendBuf,"[%s]DEV@%s@%s\n",pArray[0],digitalRead(LED_BUILTIN_PIN)?"ON":"OFF",digitalRead(LED_TEST_PIN)?"ON":"OFF");
    }
  }
  else if(!strcmp(pArray[0],"GETTIME")) {  //GETTIME
    dateTime.year = (pArray[1][0]-0x30) * 10 + pArray[1][1]-0x30 ;
    dateTime.month =  (pArray[1][3]-0x30) * 10 + pArray[1][4]-0x30 ;
    dateTime.day =  (pArray[1][6]-0x30) * 10 + pArray[1][7]-0x30 ;
    dateTime.hour = (pArray[1][9]-0x30) * 10 + pArray[1][10]-0x30 ;
    dateTime.min =  (pArray[1][12]-0x30) * 10 + pArray[1][13]-0x30 ;
    dateTime.sec =  (pArray[1][15]-0x30) * 10 + pArray[1][16]-0x30 ;
#ifdef DEBUG
//    sprintf(sendBuf,"\nTime %02d.%02d.%02d %02d:%02d:%02d\n\r",dateTime.year,dateTime.month,dateTime.day,dateTime.hour,dateTime.min,dateTime.sec );
//    Serial.println(sendBuf);
#endif
    return;
  } 
  client.write(sendBuf,strlen(sendBuf));
  client.flush();


#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendBuf);
#endif
}

void timerIsr()
{
//  digitalWrite(LED_BUILTIN_PIN,!digitalRead(LED_BUILTIN_PIN));
  timerIsrFlag = true;
  secCount++;
  if(secCount%2)
  {
    clock_calc(&dateTime);
  }
}
void clock_calc(DATETIME *dateTime)
{
  int ret = 0;
  dateTime->sec++;          // increment second

  if(dateTime->sec >= 60)                              // if second = 60, second = 0
  { 
      dateTime->sec = 0;
      dateTime->min++; 
             
      if(dateTime->min >= 60)                          // if minute = 60, minute = 0
      { 
          dateTime->min = 0;
          dateTime->hour++;                               // increment hour
          if(dateTime->hour == 24) 
          {
            dateTime->hour = 0;
            updateTimeFlag = true;
          }
       }
    }
}

void wifi_Setup() {
  wifiSerial.begin(38400);  //wifiSerial.begin(19200);
  wifi_Init();
  server_Connect();
}
void wifi_Init()
{
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI    
      Serial.println("WiFi shield not present");
#endif 
    }
    else
      break;   
  }while(1);

#ifdef DEBUG_WIFI    
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif     
  while(WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {   
#ifdef DEBUG_WIFI  
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);   
#endif   
  }
  sprintf(oledLine2,"AP:%s",AP_SSID);  
  sprintf(oledLine3,"IP:%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  sprintf(oledLine4,"RSSI:%ld dBm",WiFi.RSSI());
  displayDrawStr();
#ifdef DEBUG_WIFI      
  Serial.println("You're connected to the network");    
  printWifiStatus();
#endif 
}
int server_Connect()
{
#ifdef DEBUG_WIFI     
  Serial.println("Starting connection to server...");
#endif  

  if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI     
    Serial.println("Connected to server");
#endif  
    client.print("["LOGID":"PASSWD"]"); 
  }
  else
  {
#ifdef DEBUG_WIFI      
     Serial.println("server connection failure");
#endif    
  } 
}
void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
void displayDrawStr()
{
  u8g.firstPage();
  do {
//    u8g.drawStr( 0, 15, oledLine1);
    u8g.drawStr( 0, 15, "Intel AIOT & ROS");
    u8g.drawStr( 0, 31, oledLine2);
    u8g.drawStr( 0, 47, oledLine3);
    u8g.drawStr( 0, 63, oledLine4);
  } while( u8g.nextPage() );
}
