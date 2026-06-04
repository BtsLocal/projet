#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>

#define SEUIL 80
SoftwareSerial BTSerial(2,3);

byte mac[]= {0x90, 0xA2, 0xDA, 0x0F, 0x1D, 0x88 };
IPAddress ip(192, 168, 2, 73);
IPAddress serverAsterisk(192, 168, 2, 76);
IPAddress serverServo(192, 168, 2, 10);
IPAddress serverLoge(192, 168, 2, 72);
EthernetClient client;

enum Etat {
  IDLE, 
  S1_TRIGGERED,
  S2_TRIGGERED,
};

Etat etat = IDLE;

const int trigPin = 7;
const int echoPin = 6;
const int trigPin2 = 9;
const int echoPin2 = 8;

float temperature;
int distance;
int distance2;
int compteur = 0;

unsigned long lastDetectionTime = 0;
unsigned long lastDetectionTime2 = 0;
unsigned long lastAsteriskKeepAlive = 0;
unsigned long lastTempSend = 0;
unsigned long lastButtonPress = 0;

bool asteriskLoggedIn = false;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loginToAsterisk() {
  if (!client.connected()) {
    if (!client.connect(serverAsterisk, 5038)) {
      Serial.println(F("Failed to connect to Asterisk"));
      return;
    }
  }
  
  client.print("Action: login\r\n");
  client.print("Username: asterisk_user\r\n");
  client.print("Secret: 3615\r\n\r\n");
  
  delay(100);
  while(client.available()) {
    Serial.println(client.readString());
  }
  asteriskLoggedIn = true;
  Serial.println(F("Logged into Asterisk"));
}
////////////////////////////////////////////////////////////////////////
void sendToAsterisk(const char* command) {
  if (!client.connected() || !asteriskLoggedIn) {
    client.stop();
    delay(100);
    if (client.connect(serverAsterisk, 5038)) {
      loginToAsterisk();
    } else {
      Serial.println(F("Cannot send to Asterisk - not connected"));
      return;
    }
  }
  
  client.print(command);
  client.flush();
}
/////////////////////////////////////////////////////////////////////////
void notifyLoge(char eventType, int count) {
  EthernetClient logeClient;
  
  if (logeClient.connect(serverLoge, 4080)) {
    logeClient.write(eventType);
    logeClient.flush();
    
    Serial.print("Notified loge: Event=");
    Serial.print(eventType);
    Serial.print(", Count=");
    Serial.println(count);
    
    logeClient.stop();
  } else {
    Serial.println("Failed to notify loge");
  }
}
///////////////////////////////////////////////////////////////////////////
char queryServoStatus() {
  EthernetClient servoClient;
  
  if (servoClient.connect(serverServo, 4081)) {
    servoClient.print('?');
    servoClient.flush();
    
    if (servoClient.available()) {
      char response = servoClient.read();
      servoClient.stop();
      return response;
    }
    servoClient.stop();
  }
  return '?';
}
////////////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(5, INPUT_PULLUP);
  
  Serial.begin(9600);
  BTSerial.begin(9600);
  
  Ethernet.begin(mac, ip);
  
  Serial.print(F("Arduino IP: "));
  Serial.println(Ethernet.localIP());
  
  if (client.connect(serverAsterisk, 5038)) {
    Serial.println(F("Connected to Asterisk"));
    loginToAsterisk();
  } else {
    Serial.println(F("Failed to connect to Asterisk"));
  }
  
  lastAsteriskKeepAlive = millis();
  lastTempSend = millis();
}
////////////////////////////////////////////////////////////////////////////////
int readDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH);
  return duration * 0.034 / 2;
}
/////////////////////////////////////////////////////////////////////////////////
void loop() {
// 

  
  
  
  if (digitalRead(5)==HIGH) {
    if (millis() - lastButtonPress > 500) {
      
      EthernetClient buttonClient;
      if (buttonClient.connect(serverLoge, 4080)) {
        buttonClient.write('3');
        buttonClient.flush();
        Serial.println("Sent '3' to loge");
        buttonClient.stop();
      } else {
        Serial.println("Failed to connect to loge for button notification");
      }
      
      lastButtonPress = millis();
    }
  }

  
  distance = readDistance(trigPin, echoPin);
  distance2 = readDistance(trigPin2, echoPin2);
  
  bool s1 = (distance < SEUIL);
  bool s2 = (distance2 < SEUIL);
  unsigned long now = millis();

  



  
  switch(etat) {
    case IDLE:
      Serial.println("IDLE");
      if (s1 && !s2) {
        etat = S1_TRIGGERED;
        lastDetectionTime = now;
        Serial.println("-> S1_TRIGGERED");
      } else if (s2 && !s1) {
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
        notifyLoge('1', compteur);
        char servoState = queryServoStatus();
        Serial.print("Servo state: '");
        Serial.print(servoState);
        Serial.println("'");
        
        if (servoState == '1') {  
          sendToAsterisk("Action: originate\r\n"
                         "Channel: SIP/1001\r\n"
                         "Context: perso\r\n"
                         "Exten: 1001\r\n"
                         "CallerId: intrusion local\r\n"
                         "Priority: 1\r\n"
                         "Data:Dial(SIP/1001,3)\r\n"
                         "Async: yes\r\n\r\n");
        }
        
        
        
        etat = IDLE;
      } else if (!s1) {
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
        notifyLoge('0', compteur);
        etat = IDLE;
      } else if (!s2) {
        if (now - lastDetectionTime2 > 1500) {
          etat = IDLE;
          Serial.println("Timeout S2 -> IDLE");
        }
      }
      break;
  }
  
  if (millis() - lastTempSend >= 300000) {
    EthernetClient bddClient;
    if (bddClient.connect(IPAddress(192, 168, 2, 68), 80)) {
      bddClient.print("GET http://192.168.2.68/enregistrementTemp.php?temp=");
      bddClient.print(temperature);
      bddClient.print("&compteur=");
      bddClient.print(compteur);
      bddClient.println(" HTTP/1.0");
      bddClient.println();
      bddClient.println();
      bddClient.stop();
      Serial.println("temp sent");
      lastTempSend = millis();
    }else{
      Serial.println("erreur connect bdd");
      }
    
  }
  
  if (BTSerial.available()) {
    temperature = BTSerial.parseFloat();
    Serial.print(temperature);
  }
  

    BTSerial.println(compteur);
  
}
