#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define LED         27
#define AN_Sensor1  34
#define AN_Sensor2  35
#define ONEWIRE     25

// https://www.esp32.com/viewtopic.php?t=13053

uint8_t temprature_sens_read();
void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void readAnalogValues();

const char* clientName = CLIENT_NAME;
const char* topicInput = TOPIC_INPUT;
const char* topicOutput = TOPIC_OUTPUT;
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER;
const char* mqtt_user = MQTT_USERNAME;
const char* mqtt_passwd = MQTT_PASSWORD;
const int adcAt4mA = ADC_AT4MA;
const int adcAt20mA = ADC_AT20MA;
const int sensorMin = ADC_MIN;                      // 0 bar
const int sensorMax = ADC_MAX;                   // 1000 bar
const uint8_t numReadings = 16;
const uint8_t numSensors = 4;
uint32_t readings[numSensors][numReadings];   // multi-dimensional readings from analog input
uint32_t readIndex = 0;                       // the index of the current reading
uint32_t total[numSensors] = {0};             // the running total
uint32_t average = 0;                         // the average

// interval to stabilize analog input smoothing readings
// ADC1 Channel read time ~9.5µs per sample --> 64 readings x 9.5 = 608 µs = .608 ms
uint8_t numReadingsInterval = 2;
uint32_t numReadingsStart = 0;

WiFiClient espClient;
PubSubClient client(espClient);

OneWire oneWire(ONEWIRE);
DallasTemperature sensors(&oneWire);
int numberOfDevices;
DeviceAddress tempDeviceAddress; 

long lastMsg = 0;
char msg[50];
int value = 0;
int sensor1value = 0;
int sensor2value = 0;
bool ledOn = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  analogReadResolution(12);
  analogSetWidth(12);
  analogSetAttenuation(ADC_11db);
  adcAttachPin(AN_Sensor1);
  adcAttachPin(AN_Sensor2);

  // initialize all the readings to 0
  for (uint8_t thisReading = 0; thisReading < numReadings; thisReading++) {
    for ( uint8_t j = 0; j < numSensors; j++)  readings[j][thisReading] = 0;
  }

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  numberOfDevices = sensors.getDeviceCount();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if (ledOn) {
      digitalWrite (LED, LOW);
      ledOn = false;
    } else {
      digitalWrite (LED, HIGH);
      ledOn = true;
    }
  }

  digitalWrite (LED, HIGH);
  ledOn = true;

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    digitalWrite (LED, HIGH);
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName, mqtt_user, mqtt_passwd)) {
    //if (client.connect(clientName)) {
      Serial.println("connected");
      digitalWrite (LED, LOW);
      // Subscribe
      client.subscribe(topicInput);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void readAnalogValues() {
  // Read the sensor
  uint16_t sensor1 = analogRead(AN_Sensor1);
  uint16_t sensor2 = analogRead(AN_Sensor2);
  uint16_t inputPin[numSensors] = { sensor1, sensor2 };

  for (uint8_t ai = 0; ai < numSensors ; ai++) {
    total[ai] = total[ai] - readings[ai][readIndex];  // remove previous value from total
    readings[ai][readIndex] = inputPin[ai];           // store current value
    total[ai] = total[ai] + readings[ai][readIndex];  // add current to total
  }

  // advance index or loop around
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  // calculate average from total
  sensor1value = total[0] / numReadings;
  sensor2value = total[1] / numReadings;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  int hall = hallRead();
  // temperature sensor is not present
  //float temp = (temperatureRead() - 32 ) / 1.8;
  
  if (millis() - numReadingsStart >= numReadingsInterval * numReadings) {
    numReadingsStart = millis();
    readAnalogValues();
  }

  long now = millis();
  if (now - lastMsg >= 1000) {
    // led is on every other second
    ledOn = !ledOn;

    if (ledOn) {
      digitalWrite (LED, HIGH);
    } else {
      digitalWrite (LED, LOW);
    }

    //Serial.println("Temp: " + String(temp) + "C, hall: " + String(hall) + "C, S1: " + String(sensor1value) + "V, S2: " + String(sensor2value) + "V");
    //Serial.println("Hall: " + String(hall) + "C, S1: " + String(sensor1value) + ", " + String(sensor1Pressure) + " bar, S2: " + String(sensor2value) + ", " + String(sensor1Pressure) + " bar, [" + String(adcAt4mA) + ", " + String(adcAt20mA) + "]");

    lastMsg = now;
    //char tempString[8];
    //dtostrf(temp, 1, 2, tempString);

    char sensor1rawString[8];
    dtostrf(sensor1value, 1, 2, sensor1rawString);

    char sensor2rawString[8];
    dtostrf(sensor2value, 1, 2, sensor2rawString);

    char hallString[8];
    dtostrf(hall, 1, 2, hallString);

    DynamicJsonDocument json(300);
    
    json["hall"] = hallString;
    json["s1raw"] = sensor1rawString;
    json["s2raw"] = sensor2rawString;
 
    sensors.requestTemperatures();
    for(int i=0; i < numberOfDevices; i++) {
      if (sensors.getAddress(tempDeviceAddress, i)) {
        //float tempC = sensors.getTempC(tempDeviceAddress);
        json["temp" + i] = sensors.getTempC(tempDeviceAddress);
      }
    }

    char buffer[100];
    size_t n = serializeJson(json, buffer);
    
    if (client.publish(topicOutput, buffer, n) != true) {
      Serial.println("Error sending message");
    }
  }

  // no need to run too fast
  delay(50);
}
