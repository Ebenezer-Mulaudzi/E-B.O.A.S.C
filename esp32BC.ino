#include <WiFi.h>
#include <ESPAsyncWebSrv.h>
#include <NewPing.h>
#include <SharpIR.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "Galaxy A012314";
const char* password = "gszj9046";

const int trigPin = 12;  // Trig pin for the ultrasonic sensor
const int echoPin = 14;  // Echo pin for the ultrasonic sensor
const int sharpIRPin = 13;  // Analog pin for the Sharp IR sensor

NewPing sonar(trigPin, echoPin, 200);  // NewPing object for ultrasonic sensor
//SharpIR sharpIR(SharpIR::GP2Y0A41SK0F, sharpIRPin, 25, 15);  // Sharp IR sensor configuration
//const int sharpIRPin = 13;

const int in1 = 2;
const int in2 = 4;
const int in3 = 18;
const int in4 = 19;
bool ObstacleDct = false;

AsyncWebServer server(80);

int speed = 0;
int distanceUltrasonic = 0;
int distanceIR = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change the address if needed

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(sharpIRPin, INPUT);
  
  
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  

  lcd.init();
  lcd.backlight();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<body>";
    html += "<h1>Smart Car Control</h1>";
    html += "<p>Distance (Ultrasonic): " + String(distanceUltrasonic) + " cm</p>";
    html += "<p>Distance (Sharp IR): " + String(distanceIR) + " cm</p>";
    html += "<form action='/speed' method='get'>";
    html += "<input type='range' name='speed' min='0' max='255' value='" + String(speed) + "'>";
    html += "<input type='submit' value='Set Speed'>";
    html += "</form>";
    html += "<form action='/control' method='get'>";
    html += "<button name='direction' value='forward'>Forward</button>";
    html += "<button name='direction' value='backward'>Backward</button>";
    html += "<button name='direction' value='left'>Left</button>";
    html += "<button name='direction' value='right'>Right</button>";
    html += "<button name='direction' value='stop'>Stop</button>";
    html += "</form>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/speed", HTTP_GET, [](AsyncWebServerRequest *request){
    String speedValue = request->arg("speed");
    speed = speedValue.toInt();
    request->send(200, "text/plain", "Speed set to: " + speedValue);
  });

  /*server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    String direction = request->arg("direction");
    if (direction == "forward") {
      if(distanceUltrasonic <=20)
      {
        stopCar();
        delay(300);
        moveBackward();
        delay(400);
        stopCar();
        delay(300);
        turnLeft();
        delay(500);
        moveForward();
      }
      moveForward();

    } else if (direction == "backward") {
      moveBackward();
    } else if (direction == "left") {
      turnLeft();
    } else if (direction == "right") {
      turnRight();
    } else if (direction == "stop") {
      stopCar();
    }
    request->send(200, "text/plain", "Direction: " + direction);
  });*/

  server.begin();
}

void loop() {
  distanceUltrasonic = sonar.ping_cm();
  //distanceIR = sharpIR.distance();
  int sensIR = analogRead(sharpIRPin);
  float distanceIR = 27.86 / (sensIR - 0.16);

  lcd.setCursor(0, 0);
  lcd.print("Ultrasonic: " + String(distanceUltrasonic) + " cm ");
 // lcd.setCursor(0, 1);
  //lcd.print("Sharp IR: " + String(distanceIR) + " cm  ");



  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    String direction = request->arg("direction");
    if (direction == "forward") 
    {  
      avoidObs();
    }
     else if (direction == "backward") {
      moveBackward();
    } else if (direction == "left") {
      turnLeft();
    } else if (direction == "right") {
      turnRight();
    } else if (direction == "stop") {
      stopCar();
    }
    request->send(200, "text/plain", "Direction: " + direction);
  });

  delay(500);
}

void moveForward() {  
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void moveBackward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  
}

void turnLeft() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  
}

void turnRight() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  
  
}

void stopCar() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  
}

void avoidObs(){
  distanceUltrasonic = sonar.ping_cm();
  if(distanceUltrasonic < 20)
  {
    ObstacleDct = true;
    stopCar();
    delay(300);
    moveBackward();
    delay(400);
    stopCar();
    delay(300);
    turnLeft();
    delay(500);
    stopCar();
  }
  else{
    moveForward();
  }
}
