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
#define SSID "XXXXXXXX"
#define SSID_PASSWORD "XXXXXXX"
#define TBTOKEN "Device token from ThingsBoard"

Adafruit_BME280 bme;
unsigned long delayTime;
char thingsboardServer[] = "nomre de dominio o ip";
WiFiClient wifiClient;
ThingsBoard tb(wifiClient);
int status = WL_IDLE_STATUS;
unsigned long lastSend;

void getAndSendTemperatureAndHumidityData();
void InitWiFi();
void reconnect();

void setup() {
  Serial.println(F("BME280 -> ESP32s -> ThingsBoard"));

  unsigned status;

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
  if ( !tb.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > delayTime ) { // Update and send only after [delayTime] seconds
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }

  tb.loop();

}

void getAndSendTemperatureAndHumidityData()
{
  Serial.println("Collecting temperature data.");

  // Reading temperature or humidity takes about 250 milliseconds!
  float humidity = bme.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = bme.readTemperature();
  // Read pressure as hPa
  float pressure = bme.readPressure() / 100.0F;

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature) || isnan(pressure)) {
    Serial.println("Failed to read from BME 280 sensor!");
    return;
  }

  Serial.println("Sending data to ThingsBoard:");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C ");
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa ");


  tb.sendTelemetryFloat("temperature", temperature);
  tb.sendTelemetryFloat("humidity", humidity);
  tb.sendTelemetryFloat("pressure", pressure);
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