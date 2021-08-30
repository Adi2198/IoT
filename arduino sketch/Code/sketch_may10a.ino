//DHT11 pin analog A0
//Soil Moisture Sensor pin analog A1
// IR sensor pin A2
// Digital pin 3 Soil Temperature D3
// pressure sensor pin D8
// digital pin 10 Rx D10
//digital pin 11 tx  D11

String myAPIkey = "AFMY42KSS1TXN837";  

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <DHT.h>;
#include <Wire.h>
#include "SPI.h" 
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP280.h"

//Setup connection of the sensor
Adafruit_BMP280 bmp;
SoftwareSerial ESP8266(10,11); // Rx,  Tx



#define ONE_WIRE_BUS 3 // Digital pin 3 Soil Temperature
#define DHTTYPE DHT11
#define DHTPIN  A0  //DHT11 pin analog 0
int sensorPin = A1;  //Soil Sensor pin analog 1
int sensorValue;  
int limit = 300;
int ir = 0 ;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHT dht(DHTPIN, DHTTYPE,8); // pressure sensor pin 8
//Variables
float Soil_t =  0;
float humidity, temp_f;  
long writingTimer = 17; 
long startTime = 0;
long waitTime = 0;
int Soil_m =0;

float pressure;   //To store the barometric pressure (Pa)
float temperature;  //To store the temperature (oC)
int altimeter;    //To store the altimeter (m) (you can also use it as a float variable)



boolean relay1_st = false; 
boolean relay2_st = false; 
unsigned char check_connection=0;
unsigned char times_check=0;
boolean error;

void setup()
{ 
  sensors.begin();
  bmp.begin();
  Serial.begin(9600); 
  ESP8266.begin(9600); 
  dht.begin();
  startTime = millis(); 
  ESP8266.println("AT+RST");
  delay(2000);
  Serial.println("Connecting to Wifi");
   while(check_connection==0)
  {
    Serial.print(".");
  ESP8266.print("AT+CWJAP=\"meri ma\",\"4142shyam\"\r\n");
  ESP8266.setTimeout(5000);
 if(ESP8266.find("WIFI CONNECTED\r\n")==1)
 {
 Serial.println("WIFI CONNECTED");
 break;
 }
 times_check++;
 if(times_check>3) 
 {
  times_check=0;
   Serial.println("Trying to Reconnect..");
  }
  }
}

void loop()
{
  Serial.print("waitTime = ");
  Serial.println(waitTime);
  
  Serial.print("startTime = ");
  Serial.println(startTime);
  waitTime = millis()-startTime;   
  if (waitTime > (writingTimer*1000)) 
  {
    readSensors();
    writeThingSpeak();
    startTime = millis();   
  }
  
  
}

void day_night(void){
 int sValue = analogRead(A2);
 Serial.print("IR reading = ");
 Serial.println(sValue);
  if(sValue > 90){
    ir = 1 ;
    }
  else {
    ir = 0;
    }
  }

  
void readSensors(void)
{
  day_night();
  sensors.requestTemperatures();
  Soil_t = sensors.getTempCByIndex(0);
  Soil_m = analogRead(sensorPin); //Soil Moisture Sensor
  float temp1 = dht.readTemperature();// DHT temperature
  humidity = dht.readHumidity();  // Humidity
  pressure = bmp.readPressure();  // pressure
  float temp2 = bmp.readTemperature();  // temperature
  if (temp2 == 0 or temp1 == 0){
     temperature = temp1 ;
    }
    else {
  temperature = (temp1+temp2)/2; 
      }//
  //altimeter = bmp.readAltitude (1010); // Altimeter
  Serial.print("Soil temperature = ");
  Serial.println(Soil_t);
  Serial.print("Soil Moisture = ");
  Serial.println(Soil_m);
  Serial.print("Temperature = ");
  Serial.println(temperature);
  Serial.print("Humidity = ");
  Serial.println(humidity);
  Serial.print("Pressure = ");
  Serial.println(pressure);
  Serial.print("IR = ");
  Serial.println(ir);
}


void writeThingSpeak(void)
{
  startThingSpeak();
  // preparacao da string GET
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr +="&field1=";
  getStr += String(temperature);
  getStr +="&field2=";
  getStr += String(humidity);
  getStr +="&field3=";
  getStr += String(pressure);
  getStr +="&field4=";
  getStr += String(Soil_t);
  getStr +="&field5=";
  getStr += String(Soil_m);
  getStr +="&field6=";
  getStr += String(ir);
  getStr += "\r\n\r\n";
  GetThingspeakcmd(getStr); 
}

void startThingSpeak(void)
{
  ESP8266.flush();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com IP address
  cmd += "\",80";
  ESP8266.println(cmd);
  Serial.print("Start Commands: ");
  Serial.println(cmd);

  if(ESP8266.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}

String GetThingspeakcmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ESP8266.println(cmd);
  Serial.println(cmd);

  if(ESP8266.find(">"))
  {
    ESP8266.print(getStr);
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (ESP8266.available()) 
    {
      String line = ESP8266.readStringUntil('\n');
      if (line.length() == 1) 
      { 
        messageBody = ESP8266.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    ESP8266.println("AT+CIPCLOSE");     
    Serial.println("AT+CIPCLOSE"); 
  } 
}
