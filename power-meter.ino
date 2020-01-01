// include the library code:
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

RTC_DS3231 rtc;
File myFile;
const int pinCS = 10;
//Measuring Current Using ACS712

const int analogchannel = A0; //Connect current sensor with A0 of Arduino
int sensitivity = 66; // use 100 for 20A Module and 66 for 30A,185 for 5a module
int adcvalue = 0;
int offsetvoltage = 2500;
double Voltage = 0; //voltage measuring
double ecurrent = 0;// Current measuring
int volt1;
float volt;
const int analogVolt = A1;

void setup() {
  //baud rate
  Serial.begin(9600);//baud rate at which arduino communicates with Laptop/PC
  Wire.begin();
  pinMode(pinCS, OUTPUT);
  if (!rtc.begin()) {
    Serial.println("couldn't find RTC");
    while (1);
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, lets set the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // following line sets the RTC to the date & time this sketch was compiled
      // Set the current date, and time in the following format:
      // for example to set January 27 2017 at 12:56 you would call:
      // rtc.adjust(DateTime(2017, 1, 27, 12, 56, 0));
    }
  }
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  delay(1000);//time delay for 3 sec
  if (!SD.begin(pinCS)) {
    Serial.println("Failed read microsd card module!");
    return;
  }
  Serial.println("Success read microsd card");
  delay(1000);
}

void loop() //method to run the source code repeatedly
{
  adcvalue = analogRead(analogchannel);//reading the value from the analog pin
  Voltage = (adcvalue / 1024.0) * 5000; // Gets you mV
  ecurrent = ((Voltage - offsetvoltage) / sensitivity);

  //volt measuring
  volt1 = analogRead(analogVolt);
  volt = ((volt1 * 0.00489) * 5);
  float volttwo = volt -0.22; //minus margin of error 0.22 volt
  //can easily fine tune with trim pot for voltage divider resistance

  DateTime now = rtc.now();

  char buf2[] = "DD/MM/YY,hh:mm:ss";//"YYMMDD-hh:mm:ss";
  Serial.println(now.toString(buf2));
  String date = buf2;

  /*Serial.print("volt:");
  Serial.print(volt - 0.22);
  Serial.print(", ");
  Serial.print("current:");
  Serial.println(ecurrent);*/

  String dataString = "";
  float watt = volttwo * ecurrent;
  
  dataString += date;
  dataString += ",";
  dataString += String(ecurrent);
  dataString += ",";
  dataString += String(volttwo);
  dataString += ",";
  dataString += String(watt);
  myFile = SD.open("datalog.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(dataString);

    myFile.close();
    Serial.println(dataString);
  } else {
    Serial.println("Failed open datalog.txt");
  }
  delay(10000);

}
