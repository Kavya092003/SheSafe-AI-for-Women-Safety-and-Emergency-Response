#include <LiquidCrystal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define heart 13                              

const int rs = 13, en = 12, d4 =14 , d5 = 27, d6 = 26, d7 = 25;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000



#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 15  


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperature;


#include <WiFi.h>
#include "ThingSpeak.h"

const char* ssid = "Health123";   // your network SSID (name) 
const char* password = "123456789";   // your network password

WiFiClient  client;

unsigned long myChannelNumber = 2798296;
const char * myWriteAPIKey = "OAQPGPLB72MMXOBO";


int data=36;//heartbeat sensor


unsigned long temp=0;
byte customChar1[8] = {0b00000,0b00000,0b00011,0b00111,0b01111,0b01111,0b01111,0b01111};
byte customChar2[8] = {0b00000,0b11000,0b11100,0b11110,0b11111,0b11111,0b11111,0b11111};
byte customChar3[8] = {0b00000,0b00011,0b00111,0b01111,0b11111,0b11111,0b11111,0b11111};
byte customChar4[8] = {0b00000,0b10000,0b11000,0b11100,0b11110,0b11110,0b11110,0b11110};
byte customChar5[8] = {0b00111,0b00011,0b00001,0b00000,0b00000,0b00000,0b00000,0b00000};
byte customChar6[8] = {0b11111,0b11111,0b11111,0b11111,0b01111,0b00111,0b00011,0b00001};
byte customChar7[8] = {0b11111,0b11111,0b11111,0b11111,0b11110,0b11100,0b11000,0b10000};
byte customChar8[8] = {0b11100,0b11000,0b10000,0b00000,0b00000,0b00000,0b00000,0b00000};



// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

void Connect_wifi();
float BPM, SpO2,BP;
int count=0;

char mystr[20];

PulseOximeter pox;
uint32_t tsLastReport = 0;

char ch;

void Init_spo2();

void onBeatDetected()
{
  Serial.println("Beat Detected!");
}



void setup() 
{
   
    pinMode(data,INPUT);
         
  
    Serial.begin(9600);  //Initialize serial
    Serial2.begin(9600);
     Serial.begin(9600);  //Initialize serial
   lcd.begin(16, 2);
        lcd.createChar(1, customChar1);
    lcd.createChar(2, customChar2);
    lcd.createChar(3, customChar3);
    lcd.createChar(4, customChar4);
    lcd.createChar(5, customChar5);
    lcd.createChar(6, customChar6);
    lcd.createChar(7, customChar7);
    lcd.createChar(8, customChar8);
    lcd.clear();
    lcd.print(" Women Safety");
    lcd.setCursor(0,1);
    lcd.print("Monitoring S/m");
  
  WiFi.mode(WIFI_STA);   
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Connect_wifi();

   Serial.print("Initializing pulse oximeter..");

  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {  
    Serial.println("SUCCESS");

    pox.setOnBeatDetectedCallback(onBeatDetected);
  }

    
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    //Connect_wifi();
    
}

void Connect_wifi()
{



  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Init_spo2();
}
void Init_spo2()
{
   Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");

    pox.setOnBeatDetectedCallback(onBeatDetected);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);



}
void loop() 
{
  
 int i=0;
 Init_spo2();
    while(i<10000)
    {
        pox.update();
        BP = pox.getHeartRate();
        SpO2 = pox.getSpO2();
      
        if (millis() - tsLastReport > REPORTING_PERIOD_MS)
        {
//      
          Serial.print("BP: ");
          Serial.println(BP);
      
          Serial.print("SpO2: ");
          Serial.print(SpO2);
          Serial.println("%");
          lcd.clear();
          lcd.print("BP:");
          lcd.print(BP); 
          lcd.setCursor(0,1);
          lcd.print("Spo2:");
          lcd.print(SpO2);   // print the temperature in °C
          lcd.print("%");
      
          Serial.println("*");
          Serial.println();
      
          tsLastReport = millis();
        }
//        delay(500);
        i++;
    }
  Heartbeat_Check();
  Temp_check();


    // set the fields with the values
    ThingSpeak.setField(1, count);
    ThingSpeak.setField(2, temperature);
    ThingSpeak.setField(3, SpO2);
    ThingSpeak.setField(4, BP);
   
 
    
    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
      Connect_wifi();
//      Init_spo2();
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
//  }
} 

void Temp_check()
{
     sensors.requestTemperatures(); 
      temperature = sensors.getTempCByIndex(0);
      Serial.print("Temperature: ");
      Serial.println(temperature);
      lcd.clear();
      lcd.print("Temp: ");
      lcd.print(temperature);
      delay(1000);
      if(temperature>35)
      {
        Serial.println("More Temperature");
        lcd.clear();
        lcd.print("More Temperature");

        delay(1000);
      }
     
}


void Heartbeat_Check()
{

   count=0;
   lcd.setCursor(0, 0);
   lcd.print("Place The Finger");
   lcd.setCursor(0, 1);
   lcd.print("to check HB");
   Serial.println("Place The Finger to check HB");
   delay(2000);
 // while(digitalRead(start)>0);
  
   lcd.clear();
   temp=millis();
   
   while(millis()<(temp+5000))
   {
   
      if(analogRead(data)<100)
        {
         count=count+1;
         
         lcd.setCursor(6, 0);
         lcd.write(byte(1));
         lcd.setCursor(7, 0);
         lcd.write(byte(2));
         lcd.setCursor(8, 0);
         lcd.write(byte(3));
         lcd.setCursor(9, 0);
         lcd.write(byte(4));

         lcd.setCursor(6, 1);
         lcd.write(byte(5));
         lcd.setCursor(7, 1);
         lcd.write(byte(6));
         lcd.setCursor(8, 1);
         lcd.write(byte(7));
         lcd.setCursor(9, 1);
         lcd.write(byte(8));
         
         while(analogRead(data)<100);
//
         lcd.clear();
        }
   }
   
         lcd.clear();
         lcd.setCursor(0, 0);
         count=count*5;
         lcd.setCursor(2, 0);
         lcd.write(byte(1));
         lcd.setCursor(3, 0);
         lcd.write(byte(2));
         lcd.setCursor(4, 0);
         lcd.write(byte(3));
         lcd.setCursor(5, 0);
         lcd.write(byte(4));

         lcd.setCursor(2, 1);
         lcd.write(byte(5));
         lcd.setCursor(3, 1);
         lcd.write(byte(6));
         lcd.setCursor(4, 1);
         lcd.write(byte(7));
         lcd.setCursor(5, 1);
         lcd.write(byte(8));
         lcd.setCursor(7, 1);
         lcd.print(count);
         lcd.print(" BPM");
         temp=0;
//delay(1000);
       sprintf(mystr, "HB:%d", count);
      Serial.print(mystr);
      Serial.println("");


      delay(1000);
  Init_spo2(); 
}