#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
Servo myservo;
long previousMillis=0;
long interval=5000;
/*Put WiFi SSID & Password*/
const char* ssid = "RishiRealme";   // Enter SSID here
const char* wifi_password = "Deb@5578"; // Enter Password here
const String door_password="mummykamagarmach";
ESP8266WebServer server(80);
wl_status_t last_status;
LiquidCrystal_I2C lcd(0x27,20,4);

String updateWebpage(String s,bool t);
void handle_home();
void handle_login();
void handle_NotFound();
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
  server.on("/login", handle_login);
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
    delay(1000);
  }
  else{
    myservo.write(0);
    delay(1000);
  }
}

void handle_home() {
  server.send(200, "text/html", updateWebpage("/",false)); 
}

void handle_login() {
  
  server.send(200, "text/html", updateWebpage("/login",server.arg("password").equals(door_password))); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String updateWebpage(String s,bool t){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Door Control</title>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  if(s.equals("/login") && t){
    open_door=true;
    lcd_init();
    lcd.setCursor(0,2);
    lcd.print("Door Opened!");
  ptr +="<h3>Opened door successfully</h3>";
  ptr +="<a href=\"/\">Close Door</a>";
  }
  else if(s.equals("/login")){
    lcd_init();
    lcd.setCursor(0,2);
    lcd.print("Wrong password");
  ptr="<h3>Wrong password</h3>";
  }
  if(s.equals("/")){
lcd_init();
    lcd.setCursor(0,2);
    lcd.print("Door Closed!");
  open_door=false;
  }
  ptr +="<h1>Enter Password</h1>\n";
  ptr +="<form method=\"post\" action=\"/login\">";
  ptr +="<input type=\"text\" name=\"password\">";
  ptr +="<button type=\"submit\">Submit</button.";
  ptr +="</form>";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}