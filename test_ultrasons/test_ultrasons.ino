#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
#define SEUIL 80
SoftwareSerial BTSerial(2,3);

byte mac[]= {0x90, 0xA2, 0xDA, 0x0F, 0x1D, 0x88 };
IPAddress server(192, 168, 2, 72);
IPAddress ip(192, 168, 2, 73);
IPAddress serverAsterisk(192, 168, 2, 76);
EthernetClient client;
EthernetClient clientAsterisk;


enum Etat {
  IDLE, 
  S1_TRIGGERED,
  S2_TRIGGERED,
  CONFIRMING
};

Etat etat = IDLE;

const int serverPort = 4080;

const int trigPin = 7;
const int echoPin = 6;
const int trigPin2 = 9;
const int echoPin2 = 8;


int distance;
int distance2;

int compteur=0;
long tempsPrecedent = 0;
long tempsActuel = 0;
unsigned long lastDetectionTime = 0;

void loginToAsterisk(){
  clientAsterisk.print("Action: login\r\n"); 
  clientAsterisk.print("Username: asterisk_user\r\n");
  clientAsterisk.print("Secret: 3615\r\n\r\n");
  delay(100);
  while(clientAsterisk.available()){
    Serial.println(clientAsterisk.readString());
    }
 }
/////////////////////////////////////////////////////////////////////////////////////////

void setup() {

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  Serial.begin(9600);
  BTSerial.begin(9600);
  Ethernet.begin(mac, ip);

  Serial.print("Arduino local avec capteur: ");
  Serial.println(Ethernet.localIP());

  if (client.connect(server, serverPort))
    Serial.println("Connected to TCP server");
  else
    Serial.println("Failed to connect to TCP server");

  if (clientAsterisk.connect(serverAsterisk, 5038)){
    Serial.println("Connected to Asterisk server");
    loginToAsterisk();
  }
  else
    Serial.println("Failed to connect to Asterisk server");

}
////////////////////////////////////////////////////////////////////////////////////////

int readDistance(int trig, int echo){

  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH);
  return duration * 0.034 / 2;
}

////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  if (!client.connected()) {
    Serial.println("Connection is disconnected");
    client.stop();

    if (client.connect(server, serverPort))
      Serial.println("Reconnected to server");
    else
      Serial.println("Failed to reconnect to server");
  }

  if (!clientAsterisk.connected()) {
    Serial.println("Disconnected to Asterisk Server");
    clientAsterisk.stop();

    if (clientAsterisk.connect(serverAsterisk, 5038)){
      Serial.println("Reconnected to Asterisk server");
      loginToAsterisk();
    }
    else{
      Serial.println("Failed to reconnect to Asterisk server");
    }
  }
  
  distance = readDistance (trigPin, echoPin);
  distance2 = readDistance (trigPin2, echoPin2);
//  Serial.print("distance: ");
//  Serial.println(distance);
//  Serial.print("distance2: ");
//  Serial.println(distance2);
  
  bool s1 = (distance < SEUIL);
  bool s2 = (distance2 < SEUIL);

  unsigned long now = millis();

  switch(etat) {

    case IDLE:
    Serial.println("IDLE");

      if (s1 && !s2){
        etat = S1_TRIGGERED;
        lastDetectionTime = now;
        Serial.println("-> S1_TRIGGERED");
      }
      else if (s2 && !s1){
        etat = S2_TRIGGERED;
        lastDetectionTime = now;
        Serial.println("-> S2_TRIGGERED");
      }
      break;

    case S1_TRIGGERED:
    Serial.println("S1_TRIGGERED");
        if (s2) {
        compteur++;
        Serial.print("Entrée - Compteur: ");
        Serial.println(compteur);
        client.write('1');
        client.flush();
        clientAsterisk.print("Action: originate\r\n");
        clientAsterisk.print("Channel: SIP/1001\r\n");
        clientAsterisk.print("Context: perso\r\n");
        clientAsterisk.print("Exten: 1001\r\n");
        clientAsterisk.print("CallerId: entrée local\r\n");
        clientAsterisk.print("Priority: 1\r\n");
        clientAsterisk.print("Data:Dial(SIP/1001,3)\r\n");
        clientAsterisk.print("Async: yes\r\n\r\n");
        etat = IDLE;
      }else if (!s1) {
        if (now - lastDetectionTime > 1000) {  
          etat = IDLE;
          Serial.println("Timeout S1 -> IDLE");
        }
      }
      
      
      break;

    case S2_TRIGGERED:
      Serial.println("S2_TRIGGERED");
      if (s1) {
        if (compteur > 0) compteur--;
        Serial.print("Sortie - Compteur: ");
        Serial.println(compteur);
        client.write('0');
        client.flush();
        etat = IDLE;
        lastDetectionTime = now;
      }
      else if (!s1) {
        if (now - lastDetectionTime > 1000) {  
          etat = IDLE;
          Serial.println("Timeout S1 -> IDLE");
        }
      }
      break;

//    case CONFIRMING:
//      if (now - lastDetectionTime > 400) {
//        etat = IDLE;
//      }
//      break;
  }

//  tempsActuel=millis();
//  if((tempsActuel-tempsPrecedent) >= 1000){
//    //Serial.println("ok");
//    BTSerial.println("HELLO");
//    tempsPrecedent=millis();
//  }

  BTSerial.print(compteur);
  //if (BTSerial.available()){Serial.println(BTSerial.read());}
}
