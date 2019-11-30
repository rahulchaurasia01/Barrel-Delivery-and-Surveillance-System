#include <DHT.h>
#include <MQ2.h>

#define in1 4                     //IN1
#define in2 5                     //IN2
#define in3 6                     //IN3
#define in4 7                     //IN4
#define Ir1 22                    //Ir Sensor 1
#define Ir2 23                    //Ir Sensor 2
#define echo 24                   //Echo
#define trigger 25                //Trigger
#define Buzzer 26                 //Buzzer
#define DHTPIN 27                 // Connect the signal pin of DHT11 sensor to digital pin 27
#define Relay 28                  //Relay
#define DHTTYPE DHT11
#define Gas A0                    //Gas Sensor 

DHT dht(DHTPIN, DHTTYPE);
MQ2 mq2(Gas);

//---------------------------- USER SETTINGS ------------------------------------
String apiKey = "WUP01DQW7LJ0BGAA";     // replace with your channel's thingspeak WRITE API key

int duration;
float distance;
int hasObstacle1 = LOW;           //There is no Obstacle
int hasObstacle2 = LOW;           //There is no Obstacle
String ssid="Chaurasia";          // Wifi network SSID
String password ="holaadios";     // Wifi network password
//-----------------------------------------------------------------------------


boolean DEBUG=true;

//======================================================================== showResponce
void showResponse(int waitTime){
    long t=millis();
    char c;
    while (t+waitTime>millis()){
      if (Serial1.available()){
        c=Serial1.read();
        if (DEBUG) Serial.print(c);
      }
    }
                   
}

//========================================================================
boolean thingSpeakWrite(float value1, float value2, int value3){
  String cmd = "AT+CIPSTART=\"TCP\",\"";                  // TCP connection
  cmd += "184.106.153.149";                               // api.thingspeak.com
  cmd += "\",80";
  Serial1.println(cmd);
  if (DEBUG) Serial.println(cmd);
  if(Serial1.find("Error")){
    if (DEBUG) Serial.println("AT+CIPSTART error");
    return false;
  }
  
  
  String getStr = "GET /update?api_key=";   // prepare GET string
  getStr += apiKey;
  
  getStr +="&field1=";
  getStr += String(value1);
  getStr +="&field2=";
  getStr += String(value2);
  getStr +="&field3=";
  getStr += String(value3);
  //getStr +="&field4=";
  //getStr += String(value4);
  
  // ...
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  Serial1.println(cmd);
  if (DEBUG)  Serial.println(cmd);
  
  delay(100);
  if(Serial1.find(">")){
    Serial1.print(getStr);
    if (DEBUG)  Serial.print(getStr);
  }
  else{
    Serial1.println("AT+CIPCLOSE");
    // alert user
    if (DEBUG)   Serial.println("AT+CIPCLOSE");
    return false;
  }
  return true;
}

//================================================================================ Calculate forward obstacle
void inputs()
{
  digitalWrite(trigger, LOW);
  delayMicroseconds(2); 
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  duration = pulseIn(echo, HIGH);
  distance = (duration / 2) / 29.1;
  digitalWrite(trigger, LOW); 
}

//================================================================================ Move Forward
void Forward()
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  delay(500);
}

//================================================================================ Stop
void Stop()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

//================================================================================ Turn Right
void Right()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  delay(500);
}

//================================================================================ Turn Left
void Left()
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  delay(500);
}

//================================================================================ setup
void setup() {                
  DEBUG=true;           // enable debug serial
  Serial.begin(115200); 
  dht.begin();
  Serial1.begin(115200);   // Your esp8266 module's speed is probably at 115200. 

  //Serial1.println("AT+UART_CUR=9600,8,1,0,0");         // Enable this line only if you want to change the ESP8266 baud rate to 9600
 
  showResponse(1000);
  
  //Serial1.println("AT+RST");         // reset esp8266
  //showResponse(1000);

  Serial1.println("AT+CWMODE=1");   // set esp8266 as client
  showResponse(1000);

  Serial1.println("AT+CWJAP=\""+ssid+"\",\""+password+"\"");  // set your home router SSID and password
  showResponse(5000);

  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Ir1, INPUT);
  pinMode(Ir2, INPUT);
  pinMode(in1 ,OUTPUT);
  pinMode(in2,OUTPUT);
  pinMode(in3,OUTPUT);
  pinMode(in4,OUTPUT);
  pinMode(Relay, OUTPUT);
  mq2.begin();
  dht.begin();
   if (DEBUG)  Serial.println("Setup completed");
}


// ====================================================================== loop
void loop() {

  
  // thingspeak needs 15 sec delay between updates, 

      inputs();
  int lpg = mq2.readLPG();
  Serial.print("Lpg:- ");
  Serial.println(lpg);
  int co = mq2.readCO();
  Serial.print("CO:- ");
  Serial.println(co);
  int smoke = mq2.readSmoke();
  Serial.print("Smoke:- ");
  Serial.println(smoke);
  float t = dht.readTemperature();
  Serial.print("Temperature:- ");
  Serial.println(t);
  float h = dht.readHumidity();
  Serial.print("Humidity:- ");
  Serial.println(h);
  hasObstacle1 = digitalRead(Ir1);
  Serial.print("Ir1:- ");
  Serial.println(hasObstacle1);
  hasObstacle2 = digitalRead(Ir2);
  Serial.print("Ir2:- ");
  Serial.println(hasObstacle2);
  Serial.print("Distance:- ");
  Serial.println(distance);
  thingSpeakWrite(t,h,co);
  if(distance < 20)
  {
    Stop();
    tone(Buzzer, 3000);
    Serial.println("Stop \t Distance is less < 20; \t Light On");
  }
  else if(hasObstacle1 == HIGH && hasObstacle2 == HIGH)
  {
    Stop();
    noTone(Buzzer);
    Serial.println("Stop \\ Reached Destination");
  }
  else if(lpg < 400 && co < 5000 && smoke < 1000 && t < 35 && h < 75 && ((hasObstacle1 == LOW && hasObstacle2 == LOW) || (hasObstacle1 == LOW && hasObstacle2 == HIGH) || (hasObstacle1 == HIGH && hasObstacle2 == LOW)))
  { 
    Forward();
    digitalWrite(Relay,LOW);
    noTone(Buzzer);
    Serial.println("Forward \t\t Light On");
  }
  else
  {
    Stop();
    digitalWrite(Relay,HIGH);
    tone(Buzzer, 3000);
    Serial.println("Stop \t\t Light Off");
  }
  delay(16000);  
}
