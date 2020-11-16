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
const char * myAPIKey = SECRET_READ_APIKEY_COUNTER;
unsigned int player1FieldNumber = SECRET_PLAYER_ONE_FIELD_NUMBER; 
unsigned int player2FieldNumber = SECRET_PLAYER_TWO_FIELD_NUMBER; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

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
  if(checkTurn()) {
    //switch goes here
  
    calculateTime();
  }
}

//checks to see if there has been a change in the opposite channel, if so then it is this arduino's turn
bool checkTurn() {
  bool myTurn = false;

  //read the channel other persons channel
  long firstRead = ThingSpeak.readLongField(channelNumber, player2FieldNumber, myAPIKey);
  while(!myTurn){
    long nextRead = ThingSpeak.readLongField(channelNumber, player2FieldNumber, myAPIKey);
    if(firstRead != nextRead) {
     myTurn = true;
    }
    delay(1000);
  }
  startTime = millis();
  return myTurn;
}

void calculateTime() {
  endTime = millis();
  timeTook = (endTime - startTime)/1000;
  uploadTime();
}

void uploadTime() {
  int x = ThingSpeak.writeField(channelNumber, player1FieldNumber, timeTook, myAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}

void potentionmeterRGB() {
  
}

void buttonBuzzer() {
  
}

void distanceUltrasonic() {
  
}

void switchLED() {
  
}
