#include <WiFi.h>
#include <PubSubClient.h>

// ===================== WiFi信息 =====================
const char* WIFI_SSID = "SSID";
const char* WIFI_PWD  = "WIFI Password";

// ===================== MQTT(Mosquitto)配置 =====================
const char* MQTT_BROKER = "Server Host"; // 替换成你的Mosquitto服务器IP
uint16_t MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "esp32c3_rain_sensor";
const char* PUB_TOPIC = "sensor/rain/data";    // 设备上传雨量数据
const char* SUB_TOPIC = "sensor/rain/cmd";     // 设备订阅指令主题

// ===================== 雨滴传感器引脚 =====================
#define PIN_DO 7
#define PIN_AO 1

WiFiClient espClient;
PubSubClient client(espClient);

// 订阅消息回调函数（收到SUB_TOPIC下发消息自动执行）
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("收到订阅主题 [");
  Serial.print(topic);
  Serial.print("] 消息：");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("\n----------------------------------");
}

// 连接WiFi
void connectWiFi() {
  Serial.print("正在连接WiFi：");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功，本机IP：");
  Serial.println(WiFi.localIP());
}

// MQTT断线重连
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("MQTT连接Mosquitto服务器...");
    // 无账号密码直接client.connect(客户端ID)
    if (client.connect(MQTT_CLIENT_ID)) {
      Serial.println("MQTT连接成功");
      client.subscribe(SUB_TOPIC); // 连接后自动订阅主题
      Serial.print("已订阅指令主题：");
      Serial.println(SUB_TOPIC);
    } else {
      Serial.print("连接失败，错误码：");
      Serial.print(client.state());
      Serial.println("，5秒后重试");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_DO, INPUT);

  connectWiFi();
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(mqttCallback); // 绑定订阅回调
}

void loop() {
  // MQTT保活+处理订阅消息
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // 读取雨量数据
  int rainDigital = digitalRead(PIN_DO);
  int rainAnalog = analogRead(PIN_AO);
  float rainPercent = (4095.0 - rainAnalog) / 4095.0 * 100;

  // 组装JSON上传
  char sendBuf[96];
  snprintf(sendBuf, sizeof(sendBuf),
    "{\"rain_digit\":%d,\"rain_adc\":%d,\"rain_percent\":%.2f}",
    rainDigital, rainAnalog, rainPercent
  );

  // 发布数据到MQTT
  client.publish(PUB_TOPIC, sendBuf);
  Serial.print("已发布雨量数据：");
  Serial.println(sendBuf);

  delay(1000);
}
