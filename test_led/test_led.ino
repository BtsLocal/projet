

#include <SPI.h>
#include <Ethernet.h>
#define BUZZER_PIN 3
const int serverPort = 4080;
byte mac[]= {0x90, 0xA2, 0xDA, 0x0E, 0xD1, 0xD4 };
EthernetServer server(serverPort);
IPAddress serverIp(192, 168, 2, 72);
const int ledPinEntree = 8; 
const int ledPinSortie = 5; 
char command=2; 

void setup() {
  pinMode(ledPinEntree, OUTPUT);
  pinMode(ledPinSortie, OUTPUT);
  Serial.begin(9600);
  Ethernet.begin(mac, serverIp);
  Serial.print("Arduino loge avec led: ");
  Serial.println(Ethernet.localIP());
  server.begin();
  tone(BUZZER_PIN, 500, 300);
  delay(500);
  tone(BUZZER_PIN, 1000, 300);
  delay(500);
  tone(BUZZER_PIN, 1500, 300);
  delay(500);
  noTone(BUZZER_PIN);
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    char command = client.read();
    Serial.print("- Received command: ");
    Serial.println(command);
    if (command == '1'){
      digitalWrite(ledPinEntree, HIGH);
      delay(100);
      digitalWrite(ledPinEntree, LOW);
      delay(100);
      digitalWrite(ledPinEntree, HIGH);
      delay(100);
      digitalWrite(ledPinEntree, LOW);
    }  
    else if (command == '0'){
      digitalWrite(ledPinSortie, HIGH);
      delay(100);
      digitalWrite(ledPinSortie, LOW);
      delay(100);
      digitalWrite(ledPinSortie, HIGH);
      delay(100);
      digitalWrite(ledPinSortie, LOW);
    }
    else if (command == '3'){
      tone(BUZZER_PIN, 1800, 300);
      delay(500);
      tone(BUZZER_PIN, 1800, 500);
      delay(500);
      tone(BUZZER_PIN, 1800, 300);
      delay(500);
      noTone(BUZZER_PIN);}
  }
  command='2';

}
