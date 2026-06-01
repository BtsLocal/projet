#include <Ethernet.h>
#include <Keypad.h>
#include <SPI.h>

const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 4; //four columns
char keys[ROW_NUM][COLUMN_NUM] = {
{'1','2','3', 'A'},
{'4','5','6', 'B'},
{'7','8','9', 'C'},
{'*','0','#', 'D'}
};
String MDP="";
int numero=0;
byte pin_rows[ROW_NUM] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x1D, 0x98 };
IPAddress serverServo(192, 168, 2, 10);
IPAddress serverBDD(192, 168, 2, 68);
IPAddress ip(192, 168, 2, 11);
EthernetClient client;
EthernetClient clientServo;

/////////////////////////////////////////////////////////////////////////////////////////////
 void envoyerMotDePasse(String MDP) {
    String postData = "code=" + MDP;
    client.println("POST /digicode.php HTTP/1.0");
    client.println("Host: 192.168.2.68");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);
    delay(500);
    Serial.println("Mot de passe envoye !");
     String response = "";
        while (client.available()) {
            char c = client.read();
            response += c;
        }
        
        //client.stop();
        int contentStart = response.indexOf("\r\n\r\n");
        if (contentStart != -1) {
            response = response.substring(contentStart + 4);
        }
        response.trim();
        Serial.print("Reponse : ");
        Serial.println(response);
        if(response=="OK") {
          
            clientServo.write('1');
            //clientServo.stop();
        }
   
    
    else {
          Serial.print("Erreur de connection");
        }
 }



 ///////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  Serial.print("Keypad Arduino IP: ");
  Serial.println(Ethernet.localIP());

  if(client.connect(serverBDD, 80))
    Serial.println("connecté à la bdd");
  if(clientServo.connect(serverServo, 4080))
    Serial.println("connecté au servo");
}

void loop() {
  if(!client.connected()){
    Serial.println("déconnecté de la bdd");
    client.stop();
    if(client.connect(serverBDD, 80))
      Serial.println("reconnecté à la bdd");
  }
  if(!clientServo.connected()){
    Serial.println("déconnecté du servo");
    clientServo.stop();
    if(clientServo.connect(serverServo, 4080))
      Serial.println("reconnecté au servo");
  }

  
  char key = keypad.getKey();
  if(key){
  Serial.println(key);
  MDP+=String(key);
  numero++;
  delay(200);
}
if(numero==4){
      Serial.println(MDP);
      envoyerMotDePasse(MDP);     ///////////////////////////MDP
      numero = 0;
      MDP="";   }

}
