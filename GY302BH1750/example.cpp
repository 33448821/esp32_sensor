#include <Wire.h>
#include <math.h>

// BH1750 I2C地址
int BH1750address = 0x23;
byte buff[2];

/*
============= BH1750 接线说明 ESP32-C3 =============
BH1750 VCC  --> ESP32-C3 3.3V
BH1750 GND  --> ESP32-C3 GND
BH1750 SDA  --> ESP32-C3 GPIO8
BH1750 SCL  --> ESP32-C3 GPIO9
BH1750 ADDR --> 悬空（地址0x23），接GND则地址改为0x5C
=====================================================
串口打印波特率：115200
*/
void setup()
{
  // 初始化硬件I2C（默认SDA=8 SCL=9）
  Wire.begin();
  // 修改串口输出波特率为115200
  Serial.begin(115200);
  Serial.println("BH1750 光照传感器 启动");
}

void loop()
{
  uint16_t val = 0;
  BH1750_Init(BH1750address);
  delay(200); // 等待传感器采集完成

  if (BH1750_Read(BH1750address) == 2)
  {
    val = ((buff[0] << 8) | buff[1]) / 1.2;
    Serial.print("光照强度：");
    Serial.print(val, DEC);
    Serial.println(" lx");
  }
  delay(150);
}

// 读取BH1750原始2字节数据
int BH1750_Read(int address)
{
  int i = 0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while (Wire.available())
  {
    buff[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();
  return i;
}

// BH1750初始化：1lx高精度模式，转换时间120ms
void BH1750_Init(int address)
{
  Wire.beginTransmission(address);
  Wire.write(0x10);
  Wire.endTransmission();
}