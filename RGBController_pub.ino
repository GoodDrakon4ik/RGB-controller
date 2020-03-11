/*
WiFi RGB controller
By Drakon4ik
Version 0.0.1_b10
Date last update 06.01.2020
*/

#include <ESP8266WiFi.h>
#include <EEPROM.h>

/*---  ---*/

/*--- Конфигурация отладки/Config DEBUG mode ---*/
bool debugActiv = false;
/*---  ---*/

/*--- Конфигурация WiFi/Config WiFi ---*/
const char* ssid = "";
const char* password = "";
/*---  ---*/

/*--- Конфигурация веб-сервера/Config Web Server ---*/
WiFiServer server(80);
/*---  ---*/

/*--- Конфигурация HTTP запросов/Config HTTP requests ---*/
String redString = "0";
String greenString = "0";
String blueString = "0";
int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int pos4 = 0;

String header;
/*---  ---*/

/*--- GPIO ---*/
const int RPin = 4;
const int GPin = 2;
const int BPin = 5;
/*---  ---*/

/*--- Статус каналов/Status channel ---*/
bool ROn = false;
bool GOn = false;
bool BOn = false;
/*---  ---*/

/*--- Статус кнопок/Status buttom ---*/
int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;
int buttonState4 = 0;
int buttonState5 = 0;
/*---  ---*/

/*--- Конфигурация кнопок/Config buttom ---*/
uint8_t SW1 = 9;
uint8_t SW2 = 14;
uint8_t SW3 = 12;
uint8_t SW4 = 13;
uint8_t SW5 = 10;
/*---  ---*/

/*--- Конфигурация EPROM памяти/Config EPROM memory ---*/
const int eprom_r = 0;
const int eprom_g = 1;
const int eprom_b = 2;
/*---  ---*/

/*---Конфигурация анимации/Config animation ---*/
int Rtmp = 255;
int Gtmp = 255;
int Btmp = 255;
int RPos = 255;
int GPos = 255;
int BPos = 255;

int statusAnim = 1;
bool enterAnim = false;
/*---  ---*/

/*--- Прочая конфигурация/Config other ---*/
#define BUFFER_SIZE 100

int tm = 300;
float temp = 0;
int c_r = 0;
int c_g = 0;
int c_b = 0;

const int resolution = 256;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

WiFiClient wclient;
//PubSubClient client(wclient); //testing
/*---  ---*/

/*--- Инициализация прошивки/Initial ---*/
void setup() {
  Serial.begin(115200);
  if (debugActiv) {
    Serial.println("Start system...");
  }

  /*--- Инициальизация силовых выходов/Initial output ---*/
  if (debugActiv) {
    Serial.print("Start pin modes...");
  }
  pinMode(RPin,OUTPUT);
  pinMode(GPin, OUTPUT);
  pinMode(BPin, OUTPUT);
  digitalWrite(RPin, ROn);
  digitalWrite(GPin, GOn);
  digitalWrite(BPin, BOn);
  if (debugActiv) {
    Serial.println("OK");
  }
  /*---  ---*/

  /*--- Инициализация кнопок/Initial button ---*/
  if (debugActiv) {
    Serial.print("Start button...");
  }
  pinMode(SW1 , INPUT_PULLUP);
  pinMode(SW2 , INPUT_PULLUP);
  pinMode(SW3 , INPUT_PULLUP);
  pinMode(SW4 , INPUT_PULLUP);
  pinMode(SW5 , INPUT_PULLUP);
  if (debugActiv) {
    Serial.println("OK");
  }
  /*---  ---*/

  /*--- Включение света/Light enable ---*/
  if (debugActiv) {
    Serial.print("Start light...");
  }
  analogWriteRange(resolution);
  analogWrite(RPin, HIGH);
  analogWrite(GPin, HIGH);
  analogWrite(BPin, HIGH);
  analogWrite(RPin, RPos);
  analogWrite(GPin, GPos);
  analogWrite(BPin, BPos);
  if (debugActiv) {
    Serial.println("OK");
  }
  /*---  ---*/

  /*--- Подключение к WiFi/Connect to WiFi ---*/
  if (debugActiv) {
    Serial.print("Conect to WiFi...");
  }
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    if (debugActiv) {
      Serial.println("OK");
    }
  }else {
    if (debugActiv) {
      Serial.println("ERR...");
    }
  }
  /*---  ---*/

  /*--- Инициализация сервера/Initial server ---*/
  if (debugActiv) {
    Serial.print("Start server...");
  }
  server.begin();
  if (debugActiv) {
    Serial.println("OK");
  }
  /*---  ---*/

  /*--- Инициализация памяти/Initial memory ---*/
  if (debugActiv) {
    Serial.print("Start EEPROM...");
  }
  EEPROM.begin(4);
  Rtmp = EEPROM.read(eprom_r);
  Gtmp = EEPROM.read(eprom_g);
  Btmp = EEPROM.read(eprom_b);
  EEPROM.commit();
  if (debugActiv) {
    Serial.println("OK");
  }
  /*---  ---*/

  /*--- Установка цвета из памяти/Install color from memory ---*/
  if (debugActiv) {
    Serial.print("Color switch from EEPROM...");
  }
  colorSwitch();
  if (debugActiv) {
    Serial.println("OK");
    Serial.println("Start system...OK");
  }
  /*---  ---*/
}
/*---  ---*/

void loop() {
  //Веб интерфейс/Web interface
  WiFiClient client = server.available();
  if (!enterAnim) {
    if (client) {
    currentTime = millis();
    previousTime = currentTime;
    if (debugActiv) {
      Serial.println("***");
      Serial.println("New client on web interface");
    }
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">");
            client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js\"></script>");
            client.println("<title>LED color</title>");
            client.println("</head><body><div class=\"container\"><div class=\"row\"><h1>Color changer</h1></div>");
            client.println("<a class=\"btn btn-primary btn-lg\" href=\"#\" id=\"change_color\" role=\"button\">Change Color</a> ");
            client.println("<input class=\"jscolor {onFineChange:'update(this)'}\" id=\"rgb\"></div>");
            client.println("<script>function update(picker) {document.getElementById('rgb').innerHTML = Math.round(picker.rgb[0]) + ', ' +  Math.round(picker.rgb[1]) + ', ' + Math.round(picker.rgb[2]);");
            client.println("document.getElementById(\"change_color\").href=\"?r\" + Math.round(picker.rgb[0]) + \"g\" +  Math.round(picker.rgb[1]) + \"b\" + Math.round(picker.rgb[2]) + \"&\";}</script></body></html>");
            client.println();
            if(header.indexOf("GET /?r") >= 0) {
              pos1 = header.indexOf('r');
              pos2 = header.indexOf('g');
              pos3 = header.indexOf('b');
              pos4 = header.indexOf('&');
              redString = header.substring(pos1+1, pos2);
              greenString = header.substring(pos2+1, pos3);
              blueString = header.substring(pos3+1, pos4);
              if (debugActiv) {
                Serial.println(redString.toInt());
                Serial.println(greenString.toInt());
                Serial.println(blueString.toInt());
              }
              if (RPos != redString.toInt()) {
                Rtmp = redString.toInt();
              }
              if (GPos != greenString.toInt()) {
                Gtmp = greenString.toInt();
              }
              if (BPos != blueString.toInt()) {
                Btmp = blueString.toInt();
              }
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
      }
      }
    }
    header = "";
    client.stop();
    if (debugActiv) {
      Serial.println("Client on web interface disconnected");
      Serial.println("***");
    }
    }
  }
  
  //Считываеться значение кнопок/Read button value
  buttonState1 = digitalRead(SW1);
  buttonState2 = digitalRead(SW2);
  buttonState3 = digitalRead(SW3);
  buttonState4 = digitalRead(SW4);
  buttonState5 = digitalRead(SW5);

  //Обработка кнопки SW1/Function SW1
  if (buttonState1 == LOW) {
    if (enterAnim) {
      statusAnim = 1;
      EEPROM.begin(4);
      Rtmp = EEPROM.read(eprom_r);
      Gtmp = EEPROM.read(eprom_g);
      Btmp = EEPROM.read(eprom_b);
      EEPROM.commit();
      colorSwitch();
      enterAnim = false;
    }else {
      enterAnim = true;   
      colorAnimation0();   
    }
    if (debugActiv) {
      Serial.println("Color animation button");
    }
    if (statusAnim != 1) {
      colorAnimation0();
    }
    delay(500);
  }

  //Обработка кнопки SW2/Function SW2
  if (buttonState2 ==LOW) {
    /*Serial.println("Green button");
    GOnOFF();
    delay(500);*/
  }

  //Обработка кнопки SW3/Function SW3
  if (buttonState3 ==LOW) {
    /*Serial.println("Blue button"); 
    BOnOFF();
    delay(500);*/
  }

  //Обработка кнопки SW4/Function SW4
  if (buttonState4 ==LOW) { 
    /*Serial.println("RGB on button"); 
    RGBOn();
    delay(500);*/
  }
  
  //Обработка кнопки SW5/Function SW5
  /*if (buttonState5 ==LOW) {
    Serial.println("RGB off button");
    RGBOFF();
    delay(500);
  }*/
  
  //Смена цвета/Switch color
  if (!enterAnim){
    colorSwitch();
  }  

  if (enterAnim) {
    colorAnimation0();
  }

  delay(25);
}

/*--- Анимация #1/Animation #1 ---*/
void colorAnimation0() {
  if (statusAnim == 1) {
    analogWrite(RPin, 0);
    analogWrite(GPin, 0);
    analogWrite(BPin, 0);
    Rtmp = 255;
    Gtmp = 18;
    Btmp = 5;
    RPos = 0;
    GPos = 0;
    BPos = 0;
    statusAnim = 2;
  }
  if (Rtmp > RPos) {
    RPos = RPos + 1;
    analogWrite(RPin, RPos);
  }
  if (Rtmp < RPos) {
    RPos = RPos - 1;
    analogWrite(RPin, RPos);
  }
  if (Gtmp > GPos) {
    GPos = GPos + 1;
    analogWrite(GPin, GPos);
  }
  if (Gtmp < GPos) {
    GPos = GPos - 1;
    analogWrite(GPin, GPos);
  }
  if (Btmp > BPos) {
    BPos = BPos + 1;
    analogWrite(BPin, BPos);
  }
  if (Btmp < BPos) {
    BPos = BPos - 1;
    analogWrite(BPin, BPos);
  }
  if (Rtmp == RPos && Gtmp == GPos && Btmp == BPos) {
    if (statusAnim == 9) {
      Rtmp = 255;
      Gtmp = 18;
      Btmp = 5;
      if (Rtmp == RPos && Gtmp == GPos && Btmp == BPos) {
        statusAnim = 2;
      }
    }
    if (statusAnim == 8) {
      Rtmp = 171;
      Gtmp = 13;
      Btmp = 255;
      statusAnim = 9;
    }
    if (statusAnim == 7) {
      Rtmp = 97;
      Gtmp = 255;
      Btmp = 221;
      statusAnim = 8;
    }
    if (statusAnim == 6) {
      Rtmp = 181;
      Gtmp = 18;
      Btmp = 255;
      statusAnim = 7;
    }
    if (statusAnim == 5) {
      Rtmp = 10;
      Gtmp = 255;
      Btmp = 56;
      statusAnim = 6;
    }
    if (statusAnim == 4) {
      Rtmp = 255;
      Gtmp = 13;
      Btmp = 48;
      statusAnim = 5;
    }
    if (statusAnim == 3) {
      Rtmp = 250;
      Gtmp = 255;
      Btmp = 3;
      statusAnim = 4;
    }
    if (statusAnim == 2) {
      Rtmp = 22;
      Gtmp = 18;
      Btmp = 255;
      statusAnim = 3;
    }
  }
  delay(20);
}
/*---  ---*/

/*--- Плавная смена цветов/Slow switch color ---*/
void colorSwitch() {
  if (Rtmp > RPos) {
    RPos = RPos + 1;
    analogWrite(RPin, RPos);
  }
  if (Rtmp < RPos) {
    RPos = RPos - 1;
    analogWrite(RPin, RPos);
  }
  if (Gtmp > GPos) {
    GPos = GPos + 1;
    analogWrite(GPin, GPos);
  }
  if (Gtmp < GPos) {
    GPos = GPos - 1;
    analogWrite(GPin, GPos);
  }
  if (Btmp > BPos) {
    BPos = BPos + 1;
    analogWrite(BPin, BPos);
  }
  if (Btmp < BPos) {
    BPos = BPos - 1;
    analogWrite(BPin, BPos);
  }
  if (Rtmp == RPos && Gtmp == GPos && Btmp == BPos) {
    if (debugActiv) {
      Serial.println("Color switch...OK");
      Serial.print("Write EEPROM...");
    }
    EEPROM.begin(4);
    EEPROM.write(eprom_r, RPos);
    EEPROM.write(eprom_g, GPos);
    EEPROM.write(eprom_b, BPos);
    EEPROM.commit();
    if (debugActiv) {
      Serial.println("OK");
    }
    
  }
  delay(5);
}
/*---  ---*/

//Обработка сигналов кнопки R
/*void ROnOFF() {
  if( ROn == HIGH ){
    ROn = LOW;
  }
  else {
    ROn = HIGH;
  }
  digitalWrite(RPin , ROn);
}*/

//Обработка сигналов кнопки G
/*void GOnOFF() {
  if( GOn == HIGH ){
    GOn = LOW;
  }
  else {
    GOn = HIGH;
  }
  digitalWrite(GPin , GOn);
}

void BOnOFF() {
  if( BOn == HIGH ){
    BOn = LOW;
  }
  else {
    BOn = HIGH;
  }
  digitalWrite(BPin , BOn);
}*/

//Обработка вкл\выкл всех
/*void RGBOn() {
  ROn = HIGH;
  GOn = HIGH;
  BOn = HIGH;
  digitalWrite(RPin , ROn);
  digitalWrite(GPin , GOn);
  digitalWrite(BPin , BOn);
}

void RGBOFF() {
  ROn = LOW;
  GOn = LOW;
  BOn = LOW;
  digitalWrite(RPin , ROn);
  digitalWrite(GPin , GOn);
  digitalWrite(BPin , BOn);
}*/
