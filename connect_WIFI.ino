#include<WiFi.h>

char ssid[]="ttttry";//手机热点名称
char password[]="qwert123";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  Serial.print("连接成功,IP地址:");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:

}
