#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif 

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <RTClib.h>

#define OLED_ADDR   0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1);

#include <PubSubClient.h>

#define GreenLed      15                           // ESP32 in-board led pin
#define YellowLed 2
#define RedLed 4

const int RELAY_PIN = 5;

#define DHT11_PIN 23 // Chân GPIO kết nối với cảm biến DHT11
#define DHT11_TYPE DHT11 // Loại cảm biến (DHT11 hoặc DHT22)
DHT dht(DHT11_PIN, DHT11_TYPE);
 
#define SOIL_MOISTURE_PIN 34 

#define LIGHT_SENSOR_PIN 32

const int RELAY_LED_PIN = 18;

RTC_DS3231 rtc; 

// Change this to your Wifi SSID
// const char* ssid = "VJU Student";       
const char* ssid = "VJU Student";        
// Change this to your Wifi Password
// const char* password = "studentVJU@2022"; 
const char* password = "studentVJU@2022";              
const char* mqtt_server = "test.mosquitto.org"; // Mosquitto Server URL

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi()
{ 
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED) 
    { 
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length) 
{ 
    char msg = 0;
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");

    for(int i = 0 ; i < length; i++){ msg = (char)payload[i]; }
    Serial.println(msg);
    
    if('1' == msg){ 
      digitalWrite(RELAY_PIN, HIGH); 
      }
    else if('2' == msg){ digitalWrite(RELAY_PIN, LOW); }
}

void reconnect() 
{ 
  while(!client.connected()) 
  {
      Serial.println("Attempting MQTT connection...");

      if(client.connect("ESPClient")) 
      {
          Serial.println("Connected");
          client.subscribe("/LedControl");
      } 
      else 
      {
          Serial.print("Failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
          delay(5000);
      }
    }
}

void setup()
{    
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    Serial.begin(115200);
    pinMode(GreenLed, OUTPUT);
    pinMode(YellowLed, OUTPUT);
    pinMode(RedLed, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(RELAY_LED_PIN, OUTPUT);
    setup_wifi(); 
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    dht.begin();
    Wire.begin();

    if (!rtc.begin()) {
      Serial.println("Không thể tìm thấy RTC");
      while (1);
    }

    pinMode(LIGHT_SENSOR_PIN, INPUT);
}
void loop()
{
  float humidity = dht.readHumidity(); // Đọc độ ẩm
  float temperature = dht.readTemperature(); // Đọc nhiệt độ
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN); // Đọc độ ẩm đất
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  int moisturePercentage = map(soilMoisture, 0, 4095, 0, 100);

  if(!client.connected()) { reconnect(); }
  client.loop();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);
  Serial.print("Soil Moisture Percentage: ");
  Serial.println(moisturePercentage);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  // DHT11
  display.print("Temp: ");
  display.print(temperature);
  display.println(" °C");
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");
  display.print("Soil Moisture: ");
  display.println(soilMoisture);
  display.print("Soil Moisture Percentage: ");
  display.print(moisturePercentage);
  display.println(" %");

  DateTime now = rtc.now();
  display.println("");
  display.print("Date: ");
  printTwoDigits(now.day());
  display.print("/");
  printTwoDigits(now.month());
  display.print("/");
  display.print(now.year(), DEC);
  display.println("");
  display.print("Time: ");
  printTwoDigits(now.hour());
  display.print(":");
  printTwoDigits(now.minute());
  display.print(":");
  printTwoDigits(now.second());

  Serial.print("Analog Value = ");
  Serial.print(analogValue);

  String humidityPayload = String(humidity);
  String temperaturePayload = String(temperature);
  String soilMoisturePayload = String(moisturePercentage);
  String lightsensorPayload = String(analogValue);


 
  client.publish("/sensor/DHT11/humidity", humidityPayload.c_str());
  client.publish("/sensor/DHT11/temperature", temperaturePayload.c_str());
  client.publish("/sensor/soil_moisture", soilMoisturePayload.c_str());
  client.publish("/sensor/LightSensor", lightsensorPayload.c_str());


  // if (soilMoisture < 1500) {
  //   digitalWrite(RELAY_PIN, HIGH); // Bật máy bơm
  // } else {
  //   digitalWrite(RELAY_PIN, LOW); // Tắt máy bơm
  // }
  if (moisturePercentage<40 && moisturePercentage>0){
    digitalWrite(GreenLed, HIGH);
    digitalWrite(RedLed, LOW);
    digitalWrite(YellowLed, LOW);
  }
  else if (moisturePercentage<60){
    digitalWrite(GreenLed, LOW);
    digitalWrite(RedLed, LOW);
    digitalWrite(YellowLed, HIGH);
  }
  else{
    digitalWrite(GreenLed, LOW);
    digitalWrite(RedLed, HIGH);
    digitalWrite(YellowLed, LOW);
  }

  if (analogValue<2500){
    digitalWrite(RELAY_LED_PIN, LOW);
  }
  else{
    digitalWrite(RELAY_LED_PIN, HIGH);
  }

  display.display();
  delay(5000);
}

void printTwoDigits(int number) {
  if (number < 10) {
    Serial.print("0");
    display.print("0");
  }
  Serial.print(number);
  display.print(number, DEC);
}