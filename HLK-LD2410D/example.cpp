// 人体存在传感器简化版，只输出高低电平，接线：GPOP6->OUT

#define RADAR_OUT 6
void setup() {
  Serial.begin(115200);
  pinMode(RADAR_OUT, INPUT);
}
void loop() {
  if(digitalRead(RADAR_OUT)){
    Serial.println("检测到人");
  }else{
    Serial.println("无人");
  }
  delay(500);
}