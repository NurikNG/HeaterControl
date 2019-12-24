#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <Ticker.h> 

#ifndef STASSID
#define STASSID "****"
#define STAPSK  "****"
#endif

#define MAGIC_COMMAND 0x55
#define MAGIC_COMMAND2 0x65

int addr = 15;

byte data[32];

String HTMLpage1 = "";
String HTMLpage2 = "";
String HTMLpageValue1 = "";
String HTMLpageValue2 = "";
String HTMLpageValue3 = "";
String HTMLpageValue4 = "";
String HTMLpageValue5 = "";
String HTMLpageValue6 = "";
String HTMLpageValue7= "";
String HTMLpageValue8= "";
String HTMLpageValue9= "";
String HTMLpageValue10 = "";
String LOGPAGE = "";
String Script = "";

byte temp = 0;

unsigned int firstTemp;
unsigned int secondTemp;
unsigned int thirdTemp;

byte firstHour = 3;
byte secondHour = 7;
byte thirdHour = 19;

byte oldType = 0;

Ticker updateTimer;

#define LOG_SIZE 150
String logTime[LOG_SIZE];
byte currentLog = 0;

bool manual = false;


/*IPAddress ip(192,168,1,17);  //статический IP
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);*/

const char* ssid = STASSID;
const char* password = STAPSK;

WiFiUDP ntpUDP;
  
ESP8266WebServer server(80);

NTPClient timeClient(ntpUDP, "pool.ntp.org", 6*3600 , 12*3600*1000);

byte needUpdateTime = 0;
int currentDay = 0;

byte needToReset = 0;

unsigned int currentMinutes = 0;
void updateHandler(){
  needUpdateTime = 1;
}

void setup(void) {
  LOGPAGE +="<head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3pro.css\"><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">";
  HTMLpage1 += "<head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3pro.css\"><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"> <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style></head><title>Печка</title></head><div class=\"w3-container w3-card\"><h3>Температура <span id=\"temp\">";
  HTMLpage2 += "</span>°C</p></h3><span id=\"time\"></span><p><button id=\"bp5\" onclick=\"sendData(5)\" style=\"font-size : 50px; height:100px;width:100px\">+5</button>&nbsp;<button id=\"bm5\" onclick=\"sendData(-5)\" style=\"font-size : 50px; height:100px;width:100px\">-5</button></p><p><button id=\"bp1\" onclick=\"sendData(1)\" style=\"font-size : 50px; height:100px;width:100px\">+1</button>&nbsp;<button id=\"bm1\" onclick=\"sendData(-1)\" style=\"font-size : 50px; height:100px;width:100px\">-1</button></p><a href=\"/set\">Настройки</a>   <a href=\"/log\">Log</a>";
  Script = "<script>function senButtons(flag) {document.getElementById(\"bp5\").disabled = flag; document.getElementById(\"bm5\").disabled = flag;document.getElementById(\"bp1\").disabled = flag; document.getElementById(\"bm1\").disabled = flag; if (flag){setTimeout(function(){senButtons(false) }, 2000);}} function sendData(temp) { var xhttp = new XMLHttpRequest(); senButtons(true); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById(\"temp\").innerHTML = this.responseText; } }; xhttp.open(\"GET\", \"setTemp?Temp=\"+temp, true); xhttp.send();} setInterval(function() { getData();}, 1000); function getData() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) {document.getElementById(\"temp\").innerHTML = this.responseText;  }  };  xhttp.open(\"GET\", \"readTemp\", true);  xhttp.send();} setInterval(function() { getTime();}, 1000); function getTime() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) {document.getElementById(\"time\").innerHTML = this.responseText;  }  };  xhttp.open(\"GET\", \"readTime\", true);  xhttp.send();}</script>";
  HTMLpageValue1 += "<head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3pro.css\"><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"> <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style></head><title>Печка</title></head><div class=\"w3-container w3-card\"><h3>Настройки</h3><form action=\"/setTemp\" method=\"post\"><b>Температура на печке</b><input name=\"temp\" id=\"t\" value=\"";
  HTMLpageValue2 += "\" size=\"2\"><br><b>В 03:00-07:00</b><input name=\"temp1\" id=\"t\" value=\"";
  HTMLpageValue3 += "\" size=\"2\"><br><b>В 07:00-19:00</b><input name=\"temp2\" id=\"t\" value=\"";
  HTMLpageValue4 += "\" size=\"2\"><br><b>В 19:00-03:00</b><input name=\"temp3\" id=\"t\" value=\"";
  HTMLpageValue5 += "\" size=\"2\"><div><button>Сохранить</button></div></div><br><a href=\"/reset\">Перезагрузить</a>";
  HTMLpageValue6 += "<br><a href=\"/setManual\">Включить ручное управление</a>";
  HTMLpageValue7 += "<br><a href=\"/resetManual\">Включить работу по расписанию</a>";
  HTMLpageValue8 += "<br><a href=\"/setManualTemp\">Задать конкретную температуру</a>";
  HTMLpageValue9 += "<head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3pro.css\"><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"> <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style></head><title>Печка</title></head><div class=\"w3-container w3-card\"><h3>Настройки</h3><form action=\"/setManualFormTemp\" method=\"post\"><b>Температура</b><input name=\"temp\" id=\"t\" value=\"";\
  HTMLpageValue10 += "\" size=\"2\"><div><button>Установить</button></div>";
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

   EEPROM.begin(512);
   temp = EEPROM.read(addr);
   firstTemp = EEPROM.read(addr+1);
   secondTemp = EEPROM.read(addr+2);
   thirdTemp = EEPROM.read(addr+3);
   manual = EEPROM.read(addr+4);
   if (temp == 0xff)
      temp = 0;
   if (firstTemp == 0xff)
      firstTemp = 55;
   if (secondTemp == 0xff)
      secondTemp = 45;
   if (thirdTemp == 0xff)
      thirdTemp = 50;
  if (MDNS.begin("esp8266")) { }
  
  server.on("/", [](){
    if (temp == 0) {
      server.sendHeader("Location", "/set",true); //Redirect to our html web page 
      server.send(302, "text/plane",""); 
    } else {
      server.send(200, "text/html", HTMLpage1+String(temp)+HTMLpage2+HTMLpageValue8+Script);
    }
  });
 /* server.on("/plus", [](){
    temp++;
    server.sendHeader("Location", "/",true); //Redirect to our html web page 
    server.send(302, "text/plane",""); 
    Serial.write(0x55);
    Serial.write(0x03);
    delay(1000);
  });
  server.on("/minus", [](){
    temp--;
    server.sendHeader("Location", "/",true); //Redirect to our html web page 
    server.send(302, "text/plane",""); 
    Serial.write(0x55);
    Serial.write(0x04);
    delay(1000); 
  });*/

  server.on("/setTemp", HTTP_POST, handleSetTemp);
  server.on("/set", set);
  server.on("/setTemp", handleTemp);
  server.on("/readTemp", handleReadTemp);
  server.on("/readTime", handleReadTime);
  server.on("/reset", handleReset);
  server.on("/log", getLog);
  server.on("/setManual", setManual);
  server.on("/resetManual", resetManual);
  server.on("/setManualTemp", setManualTemp);
  server.on("/setManualFormTemp", setManualFormTemp);
  
  
 
  server.begin();
  needUpdateTime = 1;
  updateTimer.attach(30, updateHandler); 
  timeClient.begin();
  oldType = 0;
  currentDay = -1;
  currentLog = 0;
}


void getLog(){
  String output = LOGPAGE;
  for (byte i = 0; i < currentLog; i++){
    output+=logTime[i]+"<br>";
  }
  
  server.send(200, "text/html", output);
}

void addLogRecord(String rec){
  if (currentLog == LOG_SIZE){
    for (byte i = 0 ; i < LOG_SIZE-1; i++)
      logTime[i] = logTime[i+1];
    currentLog = LOG_SIZE-1;
  }
  logTime[currentLog] = String(timeClient.getDay())+" "+timeClient.getFormattedTime()+" - "+rec+" \\"+String(temp); //String(timeClient.getEpochTime())+" "+
  currentLog++;
}

void setNewTemp(byte t, byte type){
  if (type != 99)
    oldType = type;
  int value = abs(t-temp);
  if (value == 0) {
    addLogRecord("Температура без изменений "+String(t));
    return;
  }
  if (temp > t){
    Serial.write(MAGIC_COMMAND2);
    Serial.write(0x04);
    Serial.write(value);
  } else {
    Serial.write(MAGIC_COMMAND2);
    Serial.write(0x03);
    Serial.write(value);
  }
  temp = t;
  EEPROM.write(addr, temp);
  EEPROM.commit();
  if (type != 99)
    addLogRecord("Поставили "+String(t));
  else
    addLogRecord("Поставили вручную "+String(t));
}


void loop(void) {
  server.handleClient();
  MDNS.update();

  
  if (needUpdateTime){
     if (needToReset){
      needToReset = 0;
      ESP.reset();
    }
    timeClient.update();

    if (timeClient.getEpochTime() < 1573295213){
      timeClient.forceUpdate();
      //addLogRecord("Ошибка времени");
      return;
    }

    if (currentDay == -1) {
      currentDay = timeClient.getDay();
      addLogRecord("Запуск");
    }

  if ((currentDay != timeClient.getDay()) && (timeClient.getHours() == 0)){
      currentDay = timeClient.getDay();
      currentMinutes = 0;
    }
      
    if (currentMinutes == 0)
      currentMinutes = timeClient.getHours()*60+timeClient.getMinutes();
     needUpdateTime = 0;
           
    currentMinutes = timeClient.getHours()*60+timeClient.getMinutes();

   if (manual == 0){   
      if (timeClient.getHours() >= thirdHour) {
        if ((oldType != 3)){ //(temp != thirdTemp) && 
          setNewTemp(thirdTemp, 3);
        }
        return;
      }
      if ((timeClient.getHours() >= 0) && (timeClient.getHours() < firstHour)) {
        if ((oldType != 3)){  //(temp != thirdTemp) && 
          setNewTemp(thirdTemp, 3);
        }
        return;
      }
      if (timeClient.getHours() >= secondHour) {
        if ((oldType != 2)){ //(temp != secondTemp)  && 
          setNewTemp(secondTemp, 2);
        }
        return;
      }
      if (timeClient.getHours() >= firstHour) {
        if ((oldType != 1)){  //(temp != firstTemp) && 
          setNewTemp(firstTemp, 1);
        }
        return;
      }
    }
  } 

 

 /* byte numBytes = Serial.available();
  if (numBytes >= 2) {
    byte index;
    for (index = 0; index < numBytes; index++) {
        data[index] = Serial.read();
    }
    if (index == 2) {
      if (data[0] == MAGIC_COMMAND) {
        if (data[1] == 0x3)
          temp++;
         if (data[1] == 0x4)
          temp--;
         EEPROM.write(addr, temp);
         EEPROM.commit();
      }
    }
  }*/
}

void handleReadTemp() {
 server.send(200, "text/plane", String(temp)); 
}

String getNextTimeAndTemp(){
  if (timeClient.getHours() >= thirdHour){
    return String(firstHour)+":00"+" будет "+String(firstTemp);
  }
  if ((timeClient.getHours() >= 0) && (timeClient.getHours() <firstHour)){
   return String(firstHour)+":00"+" будет "+String(firstTemp);
  }
  if (timeClient.getHours() >= secondHour){
    return String(thirdHour)+":00"+" будет "+String(thirdTemp);
  }
  if (timeClient.getHours() >= firstHour){
    return String(secondHour)+":00"+" будет "+String(secondTemp);
  }
  return "Error";
}

void handleReadTime() {
  String hours = String(timeClient.getHours());
  if (timeClient.getHours() < 10)
    hours = "0"+hours;
   String minutes = String(timeClient.getMinutes());
   if (timeClient.getMinutes() < 10)
    minutes = "0"+minutes;

  String output;
  if (needToReset){
    output = "Ожидайте перезугрузки";
  } else {
    if (manual == 0)
      output = "Время "+hours+":"+minutes+"("+String(currentDay)+"/"+String(currentMinutes)+"), в "+getNextTimeAndTemp()+"°C";
    else
       output = "Ручное управление";
  }
 server.send(200, "text/plane", output); 
}

void set(){
  server.send(200, "text/html", HTMLpageValue1+String(temp)+HTMLpageValue2+String(firstTemp)+HTMLpageValue3+String(secondTemp)+HTMLpageValue4+String(thirdTemp)+HTMLpageValue5 + (manual == 0 ? HTMLpageValue6 : HTMLpageValue7));
}

void handleReset(){
  server.sendHeader("Location", "/",true);
  server.send(302, "text/plane",""); 
  needToReset = 1;
}

void setManual(){
  server.sendHeader("Location", "/",true);
  server.send(302, "text/plane",""); 
  manual = 1;
  EEPROM.write(addr+4, manual);
  EEPROM.commit();
  addLogRecord("Включили ручное управление");
}

void resetManual(){
  server.sendHeader("Location", "/",true);
  server.send(302, "text/plane",""); 
  manual = 0;
  EEPROM.write(addr+4, manual);
  EEPROM.commit();
  addLogRecord("Выключили ручное управление");
}

void setManualTemp(){
  server.send(200, "text/html",HTMLpageValue9+String(temp)+HTMLpageValue10);
}
 
void handleTemp() {
 String ledState = "OFF";
 String t_state = server.arg("Temp"); //Refer  xhttp.open("GET", "setTemp?Temp="+led, true);
 if(t_state == "1") {
    temp++;
    Serial.write(MAGIC_COMMAND);
    Serial.write(0x03);
    addLogRecord("Вручную +1");
 } else if (t_state == "-1") {
    temp--;
    if (temp <= 30)
      temp = 30;
    Serial.write(MAGIC_COMMAND);
    Serial.write(0x04);
    addLogRecord("Вручную -1");
 } else if (t_state == "5"){
    temp+=5;
    if (temp >= 80)
      temp = 80;
    Serial.write(MAGIC_COMMAND2);
    Serial.write(0x03);
    Serial.write(0x05);
    addLogRecord("Вручную +5");
 } else if (t_state == "-5"){
    temp-=5;
    if (temp <= 30)
      temp = 30;
    Serial.write(MAGIC_COMMAND2);
    Serial.write(0x04);
    Serial.write(0x05);
    addLogRecord("Вручную -5");
 }
 EEPROM.write(addr, temp);
 EEPROM.commit();
 server.send(200, "text/plane", String(temp)); //Send web page
}

void setManualFormTemp(){
  if (server.hasArg("temp")){
    byte newtemp = server.arg("temp").toInt();
    if (newtemp >80)
      newtemp = 80;
    if (newtemp < 30)
      newtemp = 30;
     setNewTemp(newtemp, 99);
  }
  server.sendHeader("Location", "/",true); //Redirect to our html web page 
  server.send(302, "text/plane",""); 
}

void handleSetTemp(){
  if (server.hasArg("temp")){
    temp = server.arg("temp").toInt();
  }
  if (server.hasArg("temp1")){
    firstTemp = server.arg("temp1").toInt();
    if (firstTemp < 30)
      firstTemp= 30;
    if (firstTemp > 80)
      firstTemp= 80;
  }
  if (server.hasArg("temp2")){
    secondTemp = server.arg("temp2").toInt();
    if (secondTemp < 30)
      secondTemp = 30;
    if (secondTemp > 80)
      secondTemp = 80;
  }
  if (server.hasArg("temp3")){
    thirdTemp = server.arg("temp3").toInt();
    if (thirdTemp < 30)
      thirdTemp = 30;
     if (thirdTemp > 80)
      thirdTemp = 80;
  }
  EEPROM.write(addr, temp);
  EEPROM.write(addr+1, firstTemp);
  EEPROM.write(addr+2, secondTemp);
  EEPROM.write(addr+3, thirdTemp);
  EEPROM.commit();
  addLogRecord("Настройки "+String(temp)+" "+String(firstTemp)+" "+String(secondTemp)+" "+String(thirdTemp));
  server.sendHeader("Location", "/",true); //Redirect to our html web page 
  server.send(302, "text/plane",""); 
}
