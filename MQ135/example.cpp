/*
硬件接线 ESP32-C3
MQ135 VCC --> 5V
MQ135 GND --> GND
MQ135 AO  --> GPIO1
调试串口：115200
WiFi：****** 密码******
MQTT：127.0.0.1:1883 主题 sensor/gas
仅上传气体，时间由Java后端自动生成
*/
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi配置
const char WIFI_SSID[] = "******";
const char WIFI_PWD[]  = "******";

// MQTT配置
const char MQTT_BROKER[] = "127.0.0.1******";
const uint16_t MQTT_PORT = 1883;
const char MQTT_USER[] = "admin";
const char MQTT_PASS[] = "public";
const char MQTT_TOPIC_UPLOAD[] = "sensor/gas";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// MQ135参数
#define MQ135_PIN 1
#define MAX_ADC 4095
float g_gasLel = 0.0f;
uint16_t baseADC = 0;

// 均值采样防抖
uint16_t getAvgADC(uint8_t times = 10)
{
  uint32_t sum = 0;
  for (uint8_t i = 0; i < times; i++)
  {
    sum += analogRead(MQ135_PIN);
    delay(15);
  }
  return sum / times;
}

// WiFi重连
void connectWiFi()
{
  Serial.print("连接WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功 IP:" + WiFi.localIP().toString());
}

// MQTT重连
void connectMqtt()
{
  while (!mqttClient.connected())
  {
    String clientId = "esp32c3_gas_" + String(random(1000, 9999));
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS))
    {
      Serial.println("MQTT连接成功");
    }
    else
    {
      Serial.print("MQTT失败 错误码:");
      Serial.print(mqttClient.state());
      delay(1000);
    }
  }
}

// 上传报文，只发gas，时间后端生成
void uploadGasData()
{
  String json = "{";
  json += "\"gas\":" + String(g_gasLel, 1);
  json += "}";
  mqttClient.publish(MQTT_TOPIC_UPLOAD, json.c_str());
  Serial.print("上传:");
  Serial.println(json);
}

// 定时全局变量
unsigned long gasReadTimer = 0;
const unsigned long gasReadInterval = 500;

unsigned long mqttSendTimer = 0;
const unsigned long mqttSendInterval = 2000;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== MQ135采集启动 ===");

  pinMode(MQ135_PIN, INPUT);
  analogSetAttenuation(ADC_11db);

  connectWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  // 预热校准基线
  Serial.println("MQ135预热5s校准...");
  delay(5000);
  baseADC = getAvgADC(20);
  Serial.print("基准ADC:");
  Serial.println(baseADC);
  Serial.println("=====================\n");
}

void loop()
{
  // 断网自动重连
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqttClient.connected()) connectMqtt();
  mqttClient.loop();

  unsigned long now = millis();

  // 定时读取MQ135
  if (now - gasReadTimer >= gasReadInterval)
  {
    gasReadTimer = now;
    uint16_t adcRaw = getAvgADC(8);
    g_gasLel = 0.0f;
    if (adcRaw > baseADC)
    {
      g_gasLel = (float)(adcRaw - baseADC) / (MAX_ADC - baseADC) * 100.0f;
    }
    Serial.print("ADC:");
    Serial.print(adcRaw);
    Serial.print(" gas:");
    Serial.print(g_gasLel, 1);
    Serial.println("%LEL");
  }

  // 定时上传MQTT
  if (now - mqttSendTimer >= mqttSendInterval)
  {
    mqttSendTimer = now;
    uploadGasData();
  }

  delay(50);
}