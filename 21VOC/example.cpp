// 项目1：21VOC五合一环境检测代码
// ====== 官方协议解析结果 ======
// TVOC   : 6 μg/m³
// 甲醛   : 0 μg/m³
// eCO₂   : 362 ppm
// 温度   : 29.4 °C
// 湿度   : 48.3 %RH
// ==============================

// RX->GPIO2
// TX->GPIO3
// VCC +3.3V


#include <HardwareSerial.h>

#define RX_PIN 2
#define TX_PIN 3

HardwareSerial sensorSerial(1);
uint8_t buffer[12];
uint8_t bufIndex = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("21VOC 五合一传感器解析启动（官方协议版）");
  sensorSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
}

void loop() {
  while (sensorSerial.available()) {
    uint8_t c = sensorSerial.read();
    
    // 帧头同步：仅保留以0x2C开头的数据包
    if (bufIndex == 0 && c != 0x2C) {
      continue;
    }
    
    buffer[bufIndex++] = c;
    
    // 收到完整12字节后解析
    if (bufIndex >= 12) {
      if (checkSum(buffer)) {
        parseFrame(buffer);
      } else {
        Serial.println("校验失败，丢弃无效帧");
      }
      bufIndex = 0;
    }
  }
}

// 校验和验证
bool checkSum(uint8_t *data) {
  uint8_t sum = 0;
  for (int i = 0; i < 11; i++) {
    sum += data[i];
  }
  uint8_t check = (~sum) + 1; // 取反+1
  return (check == data[11]);
}

// 解析数据帧
void parseFrame(uint8_t *data) {
  // 解析各字段
  uint16_t tvoc = (data[1] << 8) | data[2];
  uint16_t hcho = (data[3] << 8) | data[4];
  uint16_t co2 = (data[5] << 8) | data[6];
  uint16_t temp_raw = (data[7] << 8) | data[8];
  uint16_t hum_raw = (data[9] << 8) | data[10];

  // 温度处理（支持负数）
  float temperature;
  if (temp_raw & 0x8000) { // 最高位为1表示负数
    temperature = -(0xFFFF - temp_raw) * 0.1;
  } else {
    temperature = temp_raw * 0.1;
  }

  // 湿度处理
  float humidity = hum_raw * 0.1;

  // 打印解析结果
  Serial.println("====== 官方协议解析结果 ======");
  Serial.print("TVOC   : "); Serial.print(tvoc); Serial.println(" μg/m³");
  Serial.print("甲醛   : "); Serial.print(hcho); Serial.println(" μg/m³");
  Serial.print("eCO₂   : "); Serial.print(co2); Serial.println(" ppm");
  Serial.print("温度   : "); Serial.print(temperature, 1); Serial.println(" °C");
  Serial.print("湿度   : "); Serial.print(humidity, 1); Serial.println(" %RH");
  Serial.println("==============================");
}
