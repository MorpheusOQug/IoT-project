#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <RTClib.h>

#define OLED_ADDR   0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1);

#define DHT11_PIN 23 // Chân GPIO kết nối với cảm biến DHT11
#define DHT11_TYPE DHT11 // Loại cảm biến (DHT11 hoặc DHT22)
DHT dht(DHT11_PIN, DHT11_TYPE);

#define SOIL_MOISTURE_PIN 34

#define LIGHT_SENSOR_PIN 25 // ESP32 pin GIOP36 (ADC0)
#define LED_PIN 2

RTC_DS3231 rtc; // Tạo một thể hiện của lớp RTC_DS3231

// Máy bơm + relay
#define RELAY_PIN 32

void setup() {
  // Hiển thị màn hình OLED: SDA-21, SCL-22
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  Serial.begin(9600);
  Serial.println("Sensor Readings");

  dht.begin(); // Khởi động cảm biến
  Wire.begin();
  
  if (!rtc.begin()) {
    Serial.println("Không thể tìm thấy RTC");
    while (1);
  }

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  float humidity = dht.readHumidity(); // Đọc độ ẩm
  float temperature = dht.readTemperature(); // Đọc nhiệt độ
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN); // Đọc độ ẩm đất

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

  // Hiển thị thông tin lên màn hình OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  // DHT11
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");
  // Soil Moisture
  display.print("Soil Moisture: ");
  display.println(soilMoisture);

  // Đọc ngày và giờ hiện tại từ DS3231
  DateTime now = rtc.now();
  //In ngày và giờ trên cùng dòng với hai chữ số cho giờ, phút và giây
  // Serial.print("Ngày hiện tại: ");
  // printTwoDigits(now.day());
  // Serial.print("/");
  // printTwoDigits(now.month());
  // Serial.print("/");
  // Serial.print(now.year(), DEC);
  // Serial.print("  Giờ hiện tại: ");
  // printTwoDigits(now.hour());
  // Serial.print(":");
  // printTwoDigits(now.minute());
  // Serial.print(":");
  // printTwoDigits(now.second());
  // Serial.println();

  // Ngày và giờ hiện tại
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

  //Light sensor
  int analogValue = analogRead(LIGHT_SENSOR_PIN);

  Serial.print("Analog Value = ");
  Serial.print(analogValue);   // the raw analog reading

  // We'll have a few threshholds, qualitatively determined
  if (analogValue > 3500) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN,LOW);
  }
  //   Serial.println(" => Dark");
  // } else if (analogValue < 800) {
  //   Serial.println(" => Dim");
  // } else if (analogValue < 2000) {
  //   Serial.println(" => Light");
  // } else if (analogValue < 3200) {
  //   Serial.println(" => Bright");
  // } else {
  //   Serial.println(" => Very bright");
  // }

  display.display();
  delay(1000); // Update every 1 second

  // Điều khiển máy bơm với độ ẩm đất
  // Kiểm tra độ ẩm đất và điều khiển máy bơm
  if (soilMoisture < 1500) {
    digitalWrite(RELAY_PIN, HIGH); // Bật máy bơm
  } else {
    digitalWrite(RELAY_PIN, LOW); // Tắt máy bơm
  }
}

// Function definition
void printTwoDigits(int number) {
  if (number < 10) {
    Serial.print("0");
    display.print("0");
  }
  Serial.print(number);
  display.print(number, DEC);
}


