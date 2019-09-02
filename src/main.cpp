#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ThingsBoard.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define SSID "HOME-8F16-2"
#define SSID_PASSWORD "45164309"
#define TBTOKEN "IEwjI1mYcMvHHEmTSDNx"

Adafruit_BME280 bme;
unsigned long delayTime;
char thingsboardServer[] = "iotwit.info";
WiFiClient wifiClient;
ThingsBoard tb(wifiClient);
int status = WL_IDLE_STATUS;
unsigned long lastSend;

void getAndSendTemperatureAndHumidityData();
void InitWiFi();
void reconnect();

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ; // time to get serial running

  
  Serial.println(F("BME280 test"));

  unsigned status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin();
  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x");
    Serial.println(bme.sensorID(), 16);
    Serial.print("   ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("   ID of 0x60 represents a BME 280.\n");
    Serial.print("   ID of 0x61 represents a BME 680.\n");
    while (1)
      ;
  }
  Serial.println("BME280 found it!"); 
  Serial.print("SensorID is: 0x");
  Serial.println(bme.sensorID(), 16);

  lastSend = 0;
  Serial.println("-- Default Test --");
  delayTime = 5000;
  Serial.println();
  delay(10);
  InitWiFi();
}

void loop() {
/*  printValues();
  delay(delayTime);*/
  if ( !tb.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }

  tb.loop();

}

/*void printValues()
{
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");

  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}*/

void getAndSendTemperatureAndHumidityData()
{
  Serial.println("Collecting temperature data.");

  // Reading temperature or humidity takes about 250 milliseconds!
  float humidity = bme.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = bme.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from BME 280 sensor!");
    return;
  }

  Serial.println("Sending data to ThingsBoard:");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C ");

  tb.sendTelemetryFloat("temperature", temperature);
  tb.sendTelemetryFloat("humidity", humidity);
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(SSID, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  while (!tb.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(SSID, SSID_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    if ( tb.connect(thingsboardServer, TBTOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}