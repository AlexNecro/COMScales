//it has inverted logic!!! : 1==off, 0==on!
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#include "Display595.h"
#include "License.h"

#define PROGRAM_STRING 1
#define PROGRAM_TIME 2
#define PROGRAM_TIMESECONDS 4
#define PROGRAM_DATE 8
#define PROGRAM_DATETIME 16

//number of character places:
#define OPT_DIGITS 6
//indicator type (7 == 7-segment)
#define OPT_ITYPE 7
#define BUFFER_SIZE 30
String versionInfo = "USB Indicator v.1.0";
Display595 display;
License license;
char timeBuffer[] = "20160125141814";
char dataBuffer[BUFFER_SIZE];

unsigned long starttick = 0;
unsigned long lasttick = 0;
unsigned long lastprogramtick = 0;
unsigned long tick = 0;
unsigned long showTime = 1000;
int pos = 5;
String demoString = "DAVSoft-Ecspert 0123456789abcdefghijklmnopqrstuvwxyz\'.!-_^";
byte program = PROGRAM_TIME;
char buffer[50];
String inData = "";

//A4 = SDA, A5= SCL

void SetTime(String strTime) {//"20160125141814"
  tmElements_t tm;
  tm.Year = (strTime[0]-'0')*1000+(strTime[1]-'0')*100+(strTime[2]-'0')*10+(strTime[3]-'0') - 1970;
  tm.Month = (strTime[4]-'0')*10+(strTime[5]-'0');
  tm.Day = (strTime[6]-'0')*10+(strTime[7]-'0');
  tm.Hour = (strTime[8]-'0')*10+(strTime[9]-'0');
  tm.Minute = (strTime[10]-'0')*10+(strTime[11]-'0');
  tm.Second = (strTime[12]-'0')*10+(strTime[13]-'0');
  RTC.write(tm);  
}

void GetTime() {
  tmElements_t tm;
  if (RTC.read(tm)) {
    timeBuffer[0] = '0'+(tm.Year+1970)/1000;
    timeBuffer[1] = '0'+(tm.Year+1970)/100 - (tm.Year+1970)/1000*10;
    timeBuffer[2] = '0'+(tm.Year+1970)/10 - (tm.Year+1970)/100*10;
    timeBuffer[3] = '0'+(tm.Year+1970)%10;

    timeBuffer[4] = '0'+(tm.Month/10);
    timeBuffer[5] = '0'+(tm.Month%10);

    timeBuffer[6] = '0'+(tm.Day/10);
    timeBuffer[7] = '0'+(tm.Day%10);

    timeBuffer[8] = '0'+(tm.Hour/10);
    timeBuffer[9] = '0'+(tm.Hour%10);

    timeBuffer[10] = '0'+(tm.Minute/10);
    timeBuffer[11] = '0'+(tm.Minute%10);

    timeBuffer[12] = '0'+(tm.Second/10);
    timeBuffer[13] = '0'+(tm.Second%10);    

    license.Encode(timeBuffer, 14);
    timeBuffer[14] = 0;
    Serial.println(timeBuffer);
  } else {
    Serial.println("error");
  }
}

void GetConf() {
  dataBuffer[0] = 't';
  dataBuffer[1] = ' ';
  dataBuffer[2] = '0'+OPT_ITYPE;  
  dataBuffer[3] = 'd';  
  dataBuffer[4] = ' ';
  dataBuffer[5] = '0'+OPT_DIGITS;
  dataBuffer[6] = '.';  
  license.Encode(dataBuffer, 7);
  dataBuffer[7] = 0;
  Serial.println(dataBuffer);
}

void ShowString() {  
  display.ShowString(pos,demoString.c_str());  
  if (tick-lasttick > 200)  {
    lasttick = tick;
    pos--;      
  }
  if (pos < -(int)demoString.length()) {      
    pos = 5;
  }  
}

void ShowTime(int format = 0) {  
  if (tick-lasttick > 300)  {
    lasttick = tick;
    tmElements_t tm;
    timeBuffer[0] = 0;
    if (RTC.read(tm)) {
      if (format == 0) {
        if (tm.Second%2==0) {
          timeBuffer[0] = '0'+(tm.Hour/10);
          timeBuffer[1] = '0'+(tm.Hour%10);          
          timeBuffer[2] = '0'+(tm.Minute/10);
          timeBuffer[3] = '0'+(tm.Minute%10);
          timeBuffer[4] = 0;
        } else {
          timeBuffer[0] = '0'+(tm.Hour/10);
          timeBuffer[1] = '0'+(tm.Hour%10);
          timeBuffer[2] = '.';
          timeBuffer[3] = '0'+(tm.Minute/10);
          timeBuffer[4] = '0'+(tm.Minute%10);
          timeBuffer[5] = 0;
        }
      }
      display.ShowString(1, timeBuffer);
    } else {
      display.ShowString(0, "------");
    }
  }  
}

void ProcessMessage(String msg) {
  lastprogramtick = millis();
  if (msg.indexOf("p ")==0) {//print
    demoString = inData.substring(2);
    lastprogramtick = millis();
    program = PROGRAM_STRING;
  } else if (msg.indexOf("pt")==0) {//print time
    program = PROGRAM_TIME;
  } else if (msg.indexOf("gt")==0) {//get time
    GetTime();
  } else if (msg.indexOf("st")==0) {//set time
    SetTime(inData.substring(3));
  } else if (msg.indexOf("c")==0) {//get configuration
    GetConf();
  } else if (msg.indexOf("?")==0) {//some info    
     Serial.println(versionInfo);
  }
}

void setup() {
  display.Init(10, 9, 8, OPT_DIGITS);
  //license.Init(0xdeafbeef,0);
  license.Init(0,0);
  license.Encode((char*)versionInfo.c_str(), versionInfo.length());
  Serial.begin(9600);
  starttick = millis();
}

void loop() {  
  tick = millis();
  if (tick - starttick < 5000) { //at start show copyright demo
    ShowString();
    return;
  }
  switch (program) {    
    case PROGRAM_STRING: 
      if (showTime && tick - lastprogramtick > showTime) {//text expired
        program = PROGRAM_TIME;
        lastprogramtick = tick;
      }
      ShowString();
      break;
    default: ShowTime();
  }  
}

void serialEvent() {    
  while (Serial.available() > 0)
  {
    char recieved = Serial.read();    
    inData += recieved;     
    // Process message when new line character is recieved
    if (recieved == '\n')
    {        
        inData.trim();
        license.Decode((char*)inData.c_str(), inData.length());
        Serial.print("Arduino Received: ");
        Serial.println(inData);
        ProcessMessage(inData);        
        inData = ""; // Clear recieved buffer        
    }
  }
}
