//library code
#include <LiquidCrystal.h>
#include <Wire.h>

#define LCD_LIGHT_PIN A4
#define LCD_LIGHT_ON_TIME 32000

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int volt1;
float volt;
int analogVolt = A0;
//second sensor
int volt2 = 0;
float voltTwo;
int analogVolttwo = A1;
float Vin;
float R1 = 7500;
float R2 = 30000;

int relayone = 8;
int relaytwo = 7;
//toggle LCD backlight on off
const int lcdPin = 6;
int buttonState = 0;
boolean isLcdLightOn;
unsigned int currentLcdLightOnTime = 0;
// For calculating the lcd light on time.
unsigned long lcdLightOn_StartMillis;

unsigned long startMillis;
unsigned long currentMillis;
//delay 1 hours (36000000) or whatevere your choice to update the value/or charge time before update or check
const unsigned long period = 1800000;//half hour

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //Pin for relay module set as output
  pinMode(relayone, OUTPUT);
  digitalWrite(relayone, HIGH);
  pinMode(relaytwo, OUTPUT);
  digitalWrite(relaytwo, HIGH);
  //triggers detectVolt function on rising mode to turn relay on

  lcd.begin(16, 2);
  pinMode(lcdPin, INPUT);
  pinMode(LCD_LIGHT_PIN, OUTPUT);
  digitalWrite(LCD_LIGHT_PIN, LOW);
  isLcdLightOn = false;
  delay(1000);
  Serial.println("blacktoy");
  lcd.print("blacktoy");
  delay(500);
  lcd.clear();
  delay(500);
  startMillis = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = millis();
  delay(1000);
  volt1 = analogRead(analogVolt);
  //margin of error
  //its possible that the voltage divider resistor is not fixed and adc resolution is too small
  //power factor also 
  delay(500);
  volt = ((volt1 * 0.00489) * 5);//method 1

  volt2 = analogRead(analogVolttwo);
  delay(500);
  voltTwo = ((volt2 * 5) / 1024.0);//method 2
  Vin = voltTwo / (R1 / (R2 + R1));
  Serial.print(volt);//0.22 margin err.
  Serial.println(" V");
  Serial.print(Vin);
  Serial.println(" V");
  lcd.setCursor(0, 0);
  lcd.print("Volt:");
  lcd.print(volt);
  lcd.setCursor(15, 0);
  lcd.print("V");
  lcd.setCursor(0, 1);
  lcd.print("Solar:");
  lcd.print(Vin);
  lcd.setCursor(15, 1);
  lcd.print("V");
  //delay(1500);
  if (currentMillis - startMillis >= period) {
    relayCharge(volt, Vin);
    startMillis = currentMillis;
  }
  buttonState = digitalRead(lcdPin);

  if (buttonState == HIGH) {
    // Button pressed.
    Serial.println("Button pressed - HIGH");

    lcdLightOn_StartMillis = millis();
    currentLcdLightOnTime = 0;
    isLcdLightOn = true;
    digitalWrite(LCD_LIGHT_PIN, HIGH);
  } else {
    // Button not pressed.
    //Serial.println("Button not pressed - LOW");
    if (isLcdLightOn) {
      currentLcdLightOnTime = millis() - lcdLightOn_StartMillis;
      if (currentLcdLightOnTime > LCD_LIGHT_ON_TIME) {
         //if battery < 15 volt
        isLcdLightOn = false;
        digitalWrite(LCD_LIGHT_PIN, LOW);
      }
    }
  }
}

void relayCharge(float batv, float solarv) {
  if (solarv <= 14) {
    digitalWrite(relaytwo, LOW);//relay turning on adapter
    delay (2000);
    if (batv <= 14.95) { //margin of error volt measured by DMM
      Serial.println("Volt low");
      lcd.clear();
      //default relay connecting adaptor with the battery
      digitalWrite(relayone, LOW);
      delay(1000);
      Serial.println("relay on");
      lcd.setCursor(0, 0);
      lcd.print("charging..");
      delay(2000);
      lcd.clear();
    } else {
      digitalWrite(relayone, HIGH);//turning off relay, disconnect adaptor
      delay(2000);
    }
  } else {

    digitalWrite(relayone, HIGH);//turning off relay, disconnect adaptor
    delay(2000);
    digitalWrite(relaytwo, HIGH);
    delay(1500);
  }
}
