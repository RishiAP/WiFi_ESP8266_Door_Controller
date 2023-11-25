#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>
Servo myservo;
long previousMillis=0;
long interval=5000;
String wifi_ip_str="";
WiFiClient wifi_cli;
HTTPClient client;
int responseCode;
/*Put WiFi SSID & Password*/
const char* ssid = "RishiRealme";   // Enter SSID here
const char* wifi_password = "Deb@5578"; // Enter Password here
ESP8266WebServer server(80);
wl_status_t last_status;
LiquidCrystal_I2C lcd(0x27,20,4);

StaticJsonDocument<512> doc;
String updateWebpage(String s,bool t);
void handle_home();
void handle_login();
void handle_NotFound();
void handle_door_close();
void check_door();
void lcd_init();
boolean open_door=false;
void lcd_init(){
  // Serial.println("WiFi connected..!");
  // Serial.print("Got IP: ");  
  // Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Wifi Connected..!");
  lcd.setCursor(0,1);
  lcd.print("IP: ");
  lcd.setCursor(4,1);
  lcd.print(WiFi.localIP());
}
void setup() {
  Serial.begin(9600);
  delay(100);
  myservo.attach(D3, 500, 2400);
  myservo.write(0);
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  // lcd.setContrast(128);
  pinMode(D4, OUTPUT);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  lcd.setCursor(0,0);
  lcd.print("Connecting .....");
  //connect to your local wi-fi network
  WiFi.begin(ssid, wifi_password);
  // //check NodeMCU is connected to Wi-fi network
  // while (WiFi.status() != WL_CONNECTED) {
  // delay(1000);
  // Serial.print(".");
  // }
  // lcd_init();

  server.on("/", handle_home);
  server.on("/login",HTTP_POST ,handle_login);
  server.on("/door_close" ,handle_door_close);
  server.on("/check_door" ,check_door);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP Server Started");
}
void loop() {
  server.handleClient();
  long currentMillis = millis();
  if (currentMillis - previousMillis >=interval){
    switch (WiFi.status()){
      case WL_NO_SSID_AVAIL:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi: ");
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,1);
        lcd.print("Not Found!");
        break;
      case WL_CONNECTED:
      if(last_status!=WL_CONNECTED){
        lcd_init();
      }
        if(open_door){
          lcd.setCursor(0,2);
          lcd.print("Door Opened!");
        }
        else{
          lcd.setCursor(0,2);
          lcd.print("Door Closed!");
        }
        break;
      case WL_CONNECT_FAILED:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi: ");
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,1);
        lcd.print("Connection Failed!");
        break;
      case WL_IDLE_STATUS:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi: ");
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,1);
        lcd.print("WiFi Idle!");
        break;
      case WL_SCAN_COMPLETED:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi: ");
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,1);
        lcd.print("WiFi Scan Completed!");
        break;
      case WL_CONNECTION_LOST:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi: ");
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,1);
        lcd.print("Connection lost!");
        break;
      case WL_WRONG_PASSWORD:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi: ");
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,1);
        lcd.print("Wrong WiFi Password!");
        break;
      case WL_DISCONNECTED:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi: ");
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,1);
        lcd.print("WiFi disconnected!");
        break;
      case WL_NO_SHIELD:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi: ");
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,1);
        lcd.print("No Shield!");
        break;
    }
    // Serial.printf("Connection status: %d\n", WiFi.status());
    // Serial.print("RRSI: ");
    // Serial.println(WiFi.RSSI());
    last_status=WiFi.status();
    previousMillis = currentMillis;
  }
  if(open_door){
    myservo.write(180);
  }
  else{
    myservo.write(0);
  }
    delay(1000);
}

void check_door(){
  server.send(200,"text/plain","{\"open_door\":"+(String)open_door+"}");
}

void handle_door_close(){
  open_door=false;
  lcd_init();
  lcd.setCursor(0,2);
  lcd.print("Door Closed!");
  server.send(200,"text/plain","{\"success\":true}");
}

void handle_home() {
  server.send(200, "text/html", updateWebpage("/",false)); 
}

void handle_login() {
lcd_init();
lcd.setCursor(0,2);
lcd.print("Verifying...");
  client.setTimeout(10000);
  wifi_ip_str=WiFi.localIP().toString();
  String link="http://"+wifi_ip_str.substring(0,wifi_ip_str.lastIndexOf("."))+".181/esp_wifi_door_control/";
  client.begin(wifi_cli,link);
  responseCode=client.POST("{\"username\":\""+server.arg("username")+"\",\"password\":\""+server.arg("password")+"\"}");
  Serial.println(client.getString());
  if(responseCode==200){
  DeserializationError error = deserializeJson(doc, client.getString());
  client.end();
  if(error){
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  else{
    if((boolean)doc["validated"]){
  server.send(200, "text/plain", "{\"success\":true}"); 
  open_door=true;
  lcd_init();
  lcd.setCursor(0,2);
  lcd.print("Door opened!");
  lcd.setCursor(0,3);
  lcd.print("By: "+server.arg("username"));
    }
    else{
  server.send(200, "text/plain", "{\"success\":false}");  
  lcd_init();
  lcd.setCursor(0,2);
  lcd.print("Wrong Credentials!");
  delay(5000);
  lcd_init();
    }
  }
  }else{
    Serial.println("responseCode="+responseCode);
  server.send(200, "text/plain", "{\"success\":false,\"message\":\"Something went wrong!\"}");  
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("response code = "+responseCode);
  }
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String updateWebpage(String s,bool t){
  String ptr = "<!DOCTYPE html> <html data-bs-theme=\"dark\">\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN\" crossorigin=\"anonymous\">";
  ptr +="<title>Door Control</title>\n";
  ptr +="</head>\n";
  ptr +="<body class=\"container d-flex flex-column align-items-center\">\n";
  ptr +="<div class=\"alert\" style=\"display:none;\" id=\"main_alert\"></div>";
  ptr +="<h1>Enter Password</h1>\n";
  ptr +="<form><fieldset class=\"d-flex flex-column align-items-center w-50\" style=\"min-width:22rem\">";
  ptr +="<input type=\"text\" class=\"form-control mb-2\" name=\"username\" id=\"username\" placeholder=\"username\" required>";
  ptr +="<input type=\"password\" class=\"form-control mb-2\" id=\"password\" name=\"password\" placeholder=\"password\" required>";
  ptr +="<button type=\"submit\" id=\"submit_btn\" class=\"btn btn-primary\">Open Door</button>";
  ptr +="<button type=\"button\" id=\"close_door_btn\" class=\"mt-4 btn btn-success\" style=\"display:";
  if(!open_door)
  ptr +="none";
  ptr +=";\">Close Door</button>";
  ptr +="</fieldset></form>";
  ptr +="<script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-C6RzsynM9kWDrMNeT87bh95OGNyZPhcTNXj1NW7RuBCsyN/o0jlpcV8Qyq46cDfL\" crossorigin=\"anonymous\"></script>";
  ptr +="<script>function show_alert(type,message){alert=document.getElementById('main_alert'); alert.classList='alert alert-'+type; alert.innerText=message; alert.style.display=''; setTimeout(()=>{document.getElementById('main_alert').style.display='none'},5000)}";
  ptr +="function check_door(){const xhr=new XMLHttpRequest;xhr.open(\"GET\",\"/check_door\",true);xhr.setRequestHeader(\"Content-Type\",\"application/x-www-form-urlencoded\");xhr.onload=function(){const res=JSON.parse(this.responseText); if(res.open_door){document.getElementById('close_door_btn').style.display='';}else{document.getElementById('close_door_btn').style.display='none';} check_door();};xhr.send();} check_door();";
  ptr +="document.getElementById('close_door_btn').addEventListener('click',function (){document.getElementById('close_door_btn').innerHTML='<span class=\"spinner-border spinner-border-sm\" aria-hidden=\"true\"></span><span class=\"visually-hidden\" role=\"status\">Closing...</span>';document.querySelector('fieldset').setAttribute('disabled',true);const xhr=new XMLHttpRequest;xhr.open(\"POST\",\"/door_close\",true);xhr.setRequestHeader(\"Content-Type\",\"application/x-www-form-urlencoded\");xhr.onload=function (){document.getElementById('close_door_btn').innerHTML='Close Door';document.querySelector('fieldset').removeAttribute('disabled');const res=JSON.parse(this.responseText); if(res.success){document.getElementById('close_door_btn').style.display=\"none\"; show_alert('success','Door closed!')}};xhr.send();});document.querySelector('form').addEventListener('submit',function (e){e.preventDefault(); document.getElementById('submit_btn').innerHTML='<span class=\"spinner-border spinner-border-sm\" aria-hidden=\"true\"></span><span class=\"visually-hidden\" role=\"status\">Verifying...</span>';;document.querySelector('fieldset').setAttribute('disabled',true);const xhr=new XMLHttpRequest;xhr.open(\"POST\",\"/login\",true);xhr.setRequestHeader(\"Content-Type\",\"application/x-www-form-urlencoded\");xhr.onload=function (){document.getElementById('submit_btn').innerHTML='Open Door';document.querySelector('fieldset').removeAttribute('disabled');const res=JSON.parse(this.responseText); if(res.success){document.getElementById('close_door_btn').style.display=\"\"; show_alert('success','Door opened!')}else{if(res.message!=null){show_alert('danger',res.message)}else{show_alert('danger','Wrong credentials!')}}};xhr.send(\"username=\"+document.getElementById('username').value+\"&password=\"+document.getElementById('password').value);})</script>";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}