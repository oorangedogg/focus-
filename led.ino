#define LED_PIN   4

int value=0;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN , OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(LED_PIN, value);
  value+=20;
  if(value>=255) value=0;
  delay(500);
}
