
                 
//#define DEBUG_PANNEAU

#define LG_MAX_TRAME    128
#define LG_MAX          16
#define LG_REPONSE      4

#define NUL             0x00    
#define ACK             0x06
#define NACK            0x15

#define DELAI           1000

#include <TimeLib.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>


SoftwareSerial bt(3, 2); // RX, TX
SoftwareSerial Panneau(10,11); // RX, TX

String MDP="";
int capteur_temp = 0;
unsigned long tempsPrecedent=0;
unsigned long tempsPrecedent2=0;
const int PIN_BOUTON = 5;
const int PIN_BUZZER = 8;


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

////////////////////////////////////////////////////////////////////////////////////////////
unsigned char calculerChecksum(String trame)
{
  unsigned char checksum = 0;

  size_t len = trame.length();   //  calcul une seule fois


  for (size_t i = 0; i < len; i++)
    checksum ^= trame.charAt(i);

  
  return checksum;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//void envoyerTemperature(float temperature) {
//  if(client.connect(server, 80)) {
//      client.print("GET http://192.168.2.68/enregistrementTemp.php?temp=+");
//      client.print(temperature);
//      client.println(" HTTP/1.0");
//      client.println();
//      Serial.println("ENVOI TEMP OK"); 
//      client.stop();
//  }
//  else{
//    Serial.println("erreur connect");
//    }
//}
///////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  
  Panneau.begin(9600);
  Serial.begin(9600);
  bt.begin(9600);
  // Ethernet.begin(mac, ip);
  String trameDate;
  String cs = String(calculerChecksum("<SC>26040528103500"),HEX);
  cs.toUpperCase();
  trameDate = "<ID00><SC>26040528103500"+cs+"<E>";
  pinMode(PIN_BOUTON, INPUT_PULLUP);
  Serial.println(trameDate);
  Panneau.println(trameDate);
  delay(10000);
}
///////////////////////////////////////////////////////////////////////////////////////////////


void loop()                   
{
  int c;
  int reading = analogRead(capteur_temp);  
  float volt = reading * 5.0;
  volt = volt / 1023.0; 
  float temperature = (volt - 0.5) * 100 ;  
  
  
 if (bt.available()) { 
    c=bt.parseInt();
    bt.println(temperature);
 }


  String trame;
  String protocole = "<L1><PA><FE><MA><WC><FE>";
  String message ="Bonjour nous sommes le <KD>, il est <KT> Temp = " + String(temperature) + "C,il y a " + String (c) + " personne(s) dans la salle, veuillez entrer le code pour entrer ";
  unsigned char checksum;
  checksum = calculerChecksum(protocole+message);
  String cs = String(checksum,HEX);
  cs.toUpperCase();
  trame = "<ID00>" + protocole + message + cs + "<E>";
  
//  int etat = digitalRead(PIN_BOUTON);
//
//  if (etat == LOW) {           // LOW = pressé avec INPUT_PULLUP
//    tone(PIN_BUZZER, 3500);
//  } else {
//    noTone(PIN_BUZZER);
//  }

  
     
 
 if(millis() - tempsPrecedent>=10000){
  Panneau.println(trame);
  Serial.println(trame);
  tempsPrecedent=millis();
  }
}
                                             
 
 
                                    
