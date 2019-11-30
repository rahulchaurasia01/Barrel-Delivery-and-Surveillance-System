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

int duration;
float distance;
int hasObstacle1 = LOW;      //There is no Obstacle "0 means LOW and 1 means High"
int hasObstacle2 = LOW;      //There is no Obstacle "0 means LOW and 1 means High"

void setup() {
  // put your setup code here, to run once:
  Serial.begin (9600);
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  pinMode(in3,OUTPUT);
  pinMode(in4,OUTPUT);
  pinMode(Ir1,INPUT);
  pinMode(Ir2,INPUT);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Relay, OUTPUT);
  mq2.begin();
  dht.begin();
}

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

void Forward()
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  delay(500);
}

void Stop()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void Right()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  delay(500);
}

void Left()
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  delay(500);
}

void loop() {
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
  if(distance < 35)
  {
    Stop();
    tone(Buzzer, 3000);
    Serial.println("Stop \t Distance is less < 20; \t Light On");
  }
  else if(hasObstacle1 == LOW && hasObstacle2 == LOW)
  {
    Stop();
    noTone(Buzzer);
    Serial.println("Stop \\ Reached Destination");
  }
  else if(lpg < 400 && co < 5000 && smoke < 1000 && t < 40 && h < 75 && ((hasObstacle1 == HIGH && hasObstacle2 == HIGH) || (hasObstacle1 == LOW && hasObstacle2 == HIGH) || (hasObstacle1 == HIGH && hasObstacle2 == LOW)))
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
  delay(1000);
}
