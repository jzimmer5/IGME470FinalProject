
#include "ThingSpeak.h"
#include <WiFiNINA.h>
#include "secrets.h"

//time it takes to complete task variables
unsigned long startTime;
unsigned long endTime;
long timeTook = 0;

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
WiFiClient  client;

// channel details

unsigned long channelNumber = SECRET_CH_ID_COUNTER;
const char * myReadAPIKey = SECRET_READ_APIKEY_COUNTER;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY_COUNTER;
unsigned int player1FieldNumber = SECRET_PLAYER_ONE_FIELD_NUMBER;
unsigned int player2FieldNumber = SECRET_PLAYER_TWO_FIELD_NUMBER;


int turnLight = 1;

//the pins for the RGB LED task
int rgbLEDPinR=3;
int rgbLEDPinG=5;
int rgbLEDPinB=6;
int rgbAnalog=A0;

//the pins for the buzzer task
int buttonPin = 11;
int buzzerPin = 12;

//the pins for the distance sensor task
int triggerPin = 8;
int echoPin = 10;

//the pins for the switch task
int switchLEDPin = 7;
int switchPin = 9;

bool taskAssigned = false;
int currentTask;
int taskParameter;
int previousSwitchState;
bool taskComplete = false;
bool start = false;
int blinkVal;

unsigned long millisCounter=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //set up all the pins we'll be using
  pinMode(rgbLEDPinR,OUTPUT);
  pinMode(rgbLEDPinG,OUTPUT);
  pinMode(rgbLEDPinB,OUTPUT);
  pinMode(rgbAnalog,INPUT);
  pinMode(buttonPin,INPUT);
  pinMode(buzzerPin,OUTPUT);
  pinMode(triggerPin,OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(switchLED,OUTPUT);
  pinMode(switchPin,INPUT_PULLUP);
  pinMode(turnLight,OUTPUT);

  previousSwitchState = digitalRead(switchPin);
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  ThingSpeak.begin(client);//Initialize ThingSpeak
}

void loop() {
  // put your main code here, to run repeatedly:
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected");
  }
  
  //all task method calls should be called within this if statement
  if(start) {
    //designate a task for this turn
    startTime = millis();
    if(!taskAssigned){
      currentTask=getNewTask();
      taskAssigned = true;
      Serial.println(currentTask);
    }
    
    while(!taskComplete){
      //turn on the light that tells you it's your turn
      digitalWrite(turnLight, HIGH);
      
      //run the designated task method for the assigned task
      switch(currentTask)
      {
        case 0:
          potentiometerRGB(taskParameter);
          break;
        case 1:
          buttonBuzzer();
          break;
        case 2:
          distanceUltrasonic(taskParameter);
          break;
        case 3:
          switchLED();
          break;
        
      }
    }
    calculateTime();
    digitalWrite(turnLight, LOW);
  }
   start = checkTurn();
}

int getNewTask()
{
  taskComplete=false;
  taskParameter = random(0,1024);
  return random(0,4);
}

void completeTask()
{
  taskComplete=true;
  taskAssigned = false;
  millisCounter = 0;
}

//checks to see if there has been a change in the opposite channel, if so then it is this arduino's turn
bool checkTurn() {
  bool myTurn = false;

  //read the channel other persons channel
  long firstRead = ThingSpeak.readLongField(channelNumber, player2FieldNumber, myReadAPIKey);
  Serial.println("checkTurn called...");
  Serial.println(firstRead);
  while(!myTurn){
    Serial.println(". ");
    long nextRead = ThingSpeak.readLongField(channelNumber, player2FieldNumber, myReadAPIKey);
    Serial.println(nextRead);
    if(firstRead != nextRead) {
     myTurn = true;
     delay(15000);
    }
    delay(1000);
  }
  return myTurn;
}

void calculateTime() {
  endTime = millis();
  timeTook = (endTime - startTime)/1000;
  uploadTime();
}

void uploadTime() {
  int x = ThingSpeak.writeField(channelNumber, player1FieldNumber, timeTook, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}

void potentiometerRGB(int target) {
  
  int input = analogRead(rgbAnalog);
  Serial.print(input);
  Serial.print("\t");
  Serial.println(target);
  //check if the potentiometer is set to a value close enough to the target
  if(abs(input-target)<10)
  {
      millisCounter++;
      if(millisCounter%50==0)
      {
        if(blinkVal==0)
        {
          blinkVal=255;
        }
        else
        {
          blinkVal=0;
        }
      }
      if(millisCounter>=500)
      {
        completeTask();
      }
      analogWrite(rgbLEDPinR,blinkVal);
      analogWrite(rgbLEDPinG,blinkVal);
      analogWrite(rgbLEDPinB,blinkVal);
  }
  else{
    millisCounter = 0;
    
    double  r;
    double g;
    double b;
    int circ = map(input,0,1023,0,359);
    int deg = (int)floor(circ/60.0);
    switch(deg)
    {
      case 0:
        r=1;
        g=(circ-(60*deg))/60.0;
        b=0;
        break;
      case 1:
        r=1-(circ-60*deg)/60.0;
        g=1;
        b=0;
        break;
      case 2:
        r=0;
        g=1;
        b=(circ-(60*deg))/60.0;
        break;
      case 3:
        r=0;
        g=1-(circ-60*deg)/60.0;
        b=1;
        break;
      case 4:
        r=(circ-(60*deg))/60.0;
        g=0;
        b=1;
        break;
      case 5:
        r=1;
        g=0;
        b=1-(circ-60*deg)/60.0;
        break;  
    }
    
    analogWrite(rgbLEDPinR,(1.0-r)*255);
    analogWrite(rgbLEDPinG,(1.0-g)*255);
    analogWrite(rgbLEDPinB,(1.0-b)*255);
  
    delay(10);
  }
}

void buttonBuzzer()
{
  tone(buzzerPin,1000);
  if( digitalRead(buttonPin) == HIGH)
  {
    noTone(buzzerPin);
    completeTask();
  }
}

void distanceUltrasonic(int target)
{
  float mappedTarget = map(target, 0,1023, 5,10);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(5);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  float timeDiff = pulseIn(echoPin, HIGH);
  float distance = (timeDiff/2)/73.94;
  if(abs(mappedTarget-distance)<1)
  {
    millisCounter+=1;
    if(millisCounter>=2)
    {
      completeTask();
    }
  }
  delay(250);
  
}

void switchLED()
{
   int slideState = digitalRead(switchPin);
   digitalWrite(switchLEDPin, slideState);
   if (digitalRead(switchPin) != previousSwitchState)
   {
      completeTask();
   }
   previousSwitchState=slideState;
}
