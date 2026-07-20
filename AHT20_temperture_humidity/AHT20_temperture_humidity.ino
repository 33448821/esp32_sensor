#include <Wire.h>

#define SDA 5
#define SCL 6
#define AHT20_ADDR 0x38

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  delay(100);
  // AHT20初始化命令
  Wire.beginTransmission(AHT20_ADDR);
  Wire.write(0xBE);
  Wire.write(0x08);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(20);
}

void loop() {
  // 触发一次温湿度测量
  Wire.beginTransmission(AHT20_ADDR);
  Wire.write(0xAC);
  Wire.write(0x33);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(80);

  // 读取6字节数据
  Wire.requestFrom(AHT20_ADDR, 6);
  uint8_t buf[6];
  for(int i=0;i<6;i++) buf[i] = Wire.read();

  // 修正此处括号错误
  uint32_t raw_h = ((buf[1] << 12) | (buf[2] << 4) | (buf[3] >> 4));
  uint32_t raw_t = (((buf[3] & 0x0F) << 16) | (buf[4] << 8) | buf[5]);

  // 换算实际温湿度
  float humi = raw_h / 1048576.0 * 100;
  float temp = raw_t / 1048576.0 * 200 - 50;

  Serial.print("温度:");
  Serial.print(temp,2);
  Serial.print(" ℃  湿度:");
  Serial.print(humi,2);
  Serial.println(" %RH");

  delay(2000);
}