// Agam 21/10/19
// beta
// include the library code:
#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTPIN 8
#define DHTTYPE DHT22

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
DHT dht(DHTPIN, DHTTYPE);
int relay = 7; //relay pin signal connected to pin D7, Normally closed

unsigned long startMillis;
unsigned long currentMillis;
//delay 15 minutes to update the value, avoiding constant on-off on-off relay state
const unsigned long period = 900000; //milliseconds gap to updating relay value function. Because 15 min x 60 sec=900. So its 900000 miliseconds
float TD;
float dwpoints;

void setup() {
  Serial.begin(9600);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH); // on optical isolated relay high side 5 volt tied together to arduino board set pin HIGH to make relay off
  // set up the LCD's number of columns and rows:
  delay(1000);
  lcd.begin(16, 2);
  // Print a message to the LCD.
  dht.begin();
  startMillis = millis(); //initial start time
}

void loop() {
  currentMillis = millis(); //get current time since the program start
  delay(2000);

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    lcd.print("Fail to read..!");
    lcd.setCursor(0, 0);
    delay(2000);
    return;
  }
  lcd.setCursor(0, 0); // Sets the location at which subsequent text written to the LCD will be displayed
  lcd.print("Temp:"); // Prints string "Temp" on the LCD
  lcd.print(t); // Prints the temperature value from the sensor
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity:");
  lcd.print(h);
  lcd.print(" %");

  //Calculate dew point in celcius
  //

  float humi  = h;
  float temp = t;//in degrees C
  TD =  (temp - (14.55 + 0.114 * temp) * (1 - (0.01 * humi)) - pow(((2.5 + 0.007 * temp) * (1 - (0.01 * humi))), 3) - (15.9 + 0.117 * temp) * pow((1 - (0.01 * humi)), 14));

  //Serial.println (TD);

  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DewPoint:");
  lcd.print(TD);
  lcd.print(" C");
  delay(1500);
  lcd.clear();
  /*back to read temp. and humidity input display
    if current room temperature > dew point temp then deactivate relay, connecting live wire
    default relay normally closed
    if current room temp <= (less than or equal to) dew point temp then relay activated, disconnect live wire
    only work in tropic climate and default fridge controller in cooling mode */
  
  if(startMillis >= 5000 || currentMillis <= 15000){
    dwpoints = TD; //store the initial dew temperature so it does not change in the next 15 min
    return dwpoints;
  }

  if (currentMillis - startMillis >= period) { //test wether period has elapsed
    Serial.println("running the function!");
    myRelayDewpoint(temp, dwpoints); //if so run the function and input the current value

    startMillis = currentMillis; //save the current time as the currentMillis increment
  }
  //Serial.println(dwpoints);

  //you can add second temperature sensor such as DS18b20 to recalculate average temperature in the room and use that instead
  //not tested yet
}

void myRelayDewpoint(float x, float y) {
  if (x <= y + 1) { //added tolerance value 1 to avoid compressor working hard and give the evaporator plate time to defrost if theres any
    digitalWrite(relay, LOW);
    delay(500);
    Serial.println("Relay on, disconnecting mains live wire");
    if(currentMillis - startMillis >= 5000 || currentMillis - startMillis <= 15000){
      dwpoints = TD;//update the current dewpoint in the next 15 minute
      Serial.println(dwpoints);
    }
  } else {
    digitalWrite(relay, HIGH);
    Serial.println("The dew point not reached yet, wait 15 min to update");
    
  }
}
