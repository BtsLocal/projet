#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
#define SEUIL 80
SoftwareSerial BTSerial(2,3);

byte mac[]= {0x90, 0xA2, 0xDA, 0x0F, 0x1D, 0x88 };
IPAddress server(192, 168, 2, 72);
IPAddress ip(192, 168, 2, 73);
IPAddress serverAsterisk(192, 168, 2, 76);
IPAddress serverBDD(192, 168, 2, 68);
IPAddress serverServo(192, 168, 2, 10);
EthernetClient client;
EthernetClient clientAsterisk;
EthernetClient clientBDD;
EthernetClient clientServo;

enum Etat {
  IDLE, 
  S1_TRIGGERED,
  S2_TRIGGERED,
  //CONFIRMING
};

Etat etat = IDLE;

char etatServo='c';

const int trigPin = 7;
const int echoPin = 6;
const int trigPin2 = 9;
const int echoPin2 = 8;

float temperature;
int distance;
int distance2;

int compteur=0;
long tempsPrecedent = 0;
long tempsActuel = 0;
unsigned long lastDetectionTime = 0;
unsigned long lastDetectionTime2 = 0;



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
  pinMode(5, INPUT_PULLUP);
  Serial.begin(9600);
  BTSerial.begin(9600);
  Ethernet.begin(mac, ip);

  Serial.print(F("Arduino local avec capteur: "));
  Serial.println(Ethernet.localIP());

  if (client.connect(server, 4080))
    Serial.println(F("Connected to TCP server"));
  else
    Serial.println(F("Failed to connect to TCP server"));
    
  if (clientServo.connect(serverServo, 4081))
    Serial.println(F("Connected to Servo TCP server"));
  else
    Serial.println(F("Failed to connect to Servo TCP server"));

  if (clientAsterisk.connect(serverAsterisk, 5038)){
    Serial.println(F("Connected to Asterisk server"));
    loginToAsterisk();
  }
  else
    Serial.println(F("Failed to connect to Asterisk server"));
    
  if (clientBDD.connect(serverBDD, 80)){
    Serial.println(F("Connected to Web server"));
  }
  else
    Serial.println(F("Failed to connect to Web server")); 
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
    Serial.println("Server is disconnected");  

    if (client.connect(server, 4080))
      Serial.println("Reconnected to server");
    else
      Serial.println("Failed to reconnect to server");
  }
  
  if (!clientServo.connected()) {
    Serial.println("Server Servo is disconnected");  

    if (clientServo.connect(serverServo, 4081))
      Serial.println("Reconnected to server Servo ");
    else
      Serial.println("Failed to reconnect to server Servo");
  }
  
  if (!clientBDD.connected()) {
    Serial.println("Web Server is disconnected");  

    if (clientBDD.connect(serverBDD, 80))
      Serial.println("Reconnected to Web server");
    else
      Serial.println("Failed to reconnect to Web server");
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
  if (digitalRead(5) == HIGH) {
    client.write('3');
    client.flush();
  }
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
        lastDetectionTime2 = now;
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
        
        etatServo=clientServo.read();
        Serial.println(etatServo);
        if (etatServo=='0'){
          clientAsterisk.print("Action: originate\r\n");
          clientAsterisk.print("Channel: SIP/1001\r\n");
          clientAsterisk.print("Context: perso\r\n");
          clientAsterisk.print("Exten: 1001\r\n");
          clientAsterisk.print("CallerId: entrée local\r\n");
          clientAsterisk.print("Priority: 1\r\n");
          clientAsterisk.print("Data:Dial(SIP/1001,3)\r\n");
          clientAsterisk.print("Async: yes\r\n\r\n");
        }
        
        clientBDD.print("GET http://192.168.2.68/enregistrementTemp.php?temp=+");
        clientBDD.print(temperature);
        clientBDD.print("&compteur=+");
        clientBDD.print(compteur);
        clientBDD.println(" HTTP/1.0");
        clientBDD.println();
        etat = IDLE;
      }else if (!s1) {
        if (now - lastDetectionTime > 1500) {  
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
      }
      else if (!s2) {
        if (now - lastDetectionTime2 > 1500) {
          etat = IDLE;
          Serial.println("Timeout S2 -> IDLE");
        }
      }
      break;

//    case CONFIRMING:
//      if (now - lastDetectionTime > 400) {
//        etat = IDLE;
//      }
//      break;

  tempsActuel=millis();
  if((tempsActuel-tempsPrecedent) >= 1800000){
    clientBDD.print("GET http://192.168.2.68/enregistrementTemp.php?temp=+");
    clientBDD.print(temperature);
    clientBDD.print("&compteur=+");
    clientBDD.print(compteur);
    clientBDD.println(" HTTP/1.0");
    clientBDD.println();
    tempsPrecedent=millis();
  }



  }


  
  BTSerial.println(compteur);
  temperature= BTSerial.parseFloat();
  if (BTSerial.available()){Serial.print(temperature);} 
}
