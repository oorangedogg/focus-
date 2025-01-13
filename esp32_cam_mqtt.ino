/*
  ArduinoMqttClient - WiFi Advanced Callback

  This example connects to a MQTT broker and subscribes to a single topic,
  it also publishes a message to another topic every 10 seconds.
  When a message is received it prints the message to the Serial Monitor,
  it uses the callback functionality of the library.

  It also demonstrates how to set the will message, get/set QoS, 
  duplicate and retain values of messages.

  The circuit:
  - Arduino MKR 1000, MKR 1010 or Uno WiFi Rev2 board

  This example code is in the public domain.
*/

#include <ArduinoJson.h>
#define LED_PIN    4
/***************需要有一个宏定义*****************/
#define ARDUINO_ARCH_ESP32

#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_GIGA) || defined(ARDUINO_OPTA)
  #include <WiFi.h>
#elif defined(ARDUINO_PORTENTA_C33)
  #include <WiFiC3.h>
#elif defined(ARDUINO_UNOR4_WIFI)
  #include <WiFiS3.h>
#endif

// #include "arduino_secrets.h"
#define SECRET_SSID "ttttry"
#define SECRET_PASS "qwert123"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // wifi网络的名称
char pass[] = SECRET_PASS;    // wifi的密码

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the WiFi module.

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[]    = "iot-06z00f6xb8psxgr.mqtt.iothub.aliyuncs.com";//代理的地址
int        port        = 1883;//端口
//const char willTopic[] = "arduino/will";//遗愿
const char inTopic[]   = "/sys/k28otXwYWjm/esp32-cam/thing/service/property/set";//订阅的topic
const char outTopic[]  = "/sys/k28otXwYWjm/esp32-cam/thing/event/property/post";//发布的topic

const long interval = 10000;//发布消息的频率
unsigned long previousMillis = 0;

int count = 0;

String inputString="";

void setup() {
  pinMode(LED_PIN,OUTPUT);
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to WiFi network:
  Serial.print("尝试连接到WIFI:");
  Serial.println(ssid);
  // while (WiFi.begin(ssid, pass) != WL_CONNECTED) {//连接wifi
  //   // failed, retry
  //   Serial.println("连接失败，再次尝试..");
  //   delay(5000);
  // }
  WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED) {//连接wifi
    // failed, retry
    Serial.println("连接失败，再次尝试..");
    delay(5000);
  }

  Serial.println("已经连接到WIFI");
  Serial.println();

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  // By default the library connects with the "clean session" flag set,
  // you can disable this behaviour by using
  // mqttClient.setCleanSession(false);

  // set a will message, used by the broker when the connection dies unexpectedly
  // you must know the size of the message beforehand, and it must be set before connecting

  /*遗愿机制*/
  // String willPayload = "oh no!";
  // bool willRetain = true;
  // int willQos = 1;

  // mqttClient.beginWill(willTopic, willPayload.length(), willRetain, willQos);
  // mqttClient.print(willPayload);
  // mqttClient.endWill();

  mqttClient.setId("k28otXwYWjm.esp32-cam|securemode=2,signmethod=hmacsha256,timestamp=1736506386558|");//连接客户端id
  mqttClient.setUsernamePassword("esp32-cam&k28otXwYWjm","03ce90ef364389f71b1762649eb4555e2d5c5431272435f5461b28ad7ccde2d0");//连接客户端用户名和密码


  Serial.print("尝试连接到mqtt");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {//mqtt的连接
    Serial.print("MQTT连接失败! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("已经连接到mqtt");
  Serial.println();

  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);//订阅消息，onMqttMessage是回调函数

  Serial.print("Subscribing to topic: ");
  Serial.println(inTopic);
  Serial.println();

  // subscribe to a topic
  // the second parameter sets the QoS of the subscription,
  // the the library supports subscribing at QoS 0, 1, or 2
  int subscribeQos = 1;

  mqttClient.subscribe(inTopic, subscribeQos);//订阅主题，qos可以不用管

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(inTopic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(inTopic);
  Serial.println();
}

void loop() {
  // call poll() regularly to allow the library to receive MQTT messages and
  // send MQTT keep alives which avoids being disconnected by the broker
  mqttClient.poll();

  // to avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    String payload;

    // payload += "hello world!";
    // payload += " ";
    // payload += count;
    payload="{\"params\":{\"temp\":55,\"humi\":65},\"version\":\"1.0.0\"}";

    Serial.print("Sending message to topic: ");
    Serial.println(outTopic);
    Serial.println(payload);

    // send message, the Print interface can be used to set the message contents
    // in this case we know the size ahead of time, so the message payload can be streamed

    bool retained = false;
    int qos = 1;
    bool dup = false;

    mqttClient.beginMessage(outTopic, payload.length(), retained, qos, dup);
    mqttClient.print(payload);
    mqttClient.endMessage();

    Serial.println();

    count++;
  }
}

/*接受到云端发来的数据时的回调函数*/
void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', duplicate = ");
  Serial.print(mqttClient.messageDup() ? "true" : "false");
  Serial.print(", QoS = ");
  Serial.print(mqttClient.messageQoS());
  Serial.print(", retained = ");
  Serial.print(mqttClient.messageRetain() ? "true" : "false");
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    // Serial.print((char)mqttClient.read());

    char inChar=(char)mqttClient.read();
    inputString += inChar;

    if(inputString.length() == messageSize)//读到完整数据
    {
        /*解析json数据*/
        DynamicJsonDocument json_msg(1024);
        DynamicJsonDocument json_item(1024);
        DynamicJsonDocument json_value(1024);

        deserializeJson(json_msg,inputString);
        String items = json_msg["items"];

        deserializeJson(json_item,items);
        String led = json_item["led"];

        deserializeJson(json_value,led);
        bool value = json_value["value"];
        Serial.print("value:");
        Serial.println(value);
        if(value == 0)
        {
          /*灯灭*/
          Serial.println("灯灭");
          digitalWrite(LED_PIN,LOW);
        }
        else
        {
          /*灯亮*/
          Serial.println("灯亮");
          digitalWrite(LED_PIN,HIGH);
        }

        inputString="";
    }
  }
  Serial.println();

  Serial.println();
}
