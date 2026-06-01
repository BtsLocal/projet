
#include <SPI.h>
#include <Ethernet.h>

#include <Servo.h>

Servo myservo;

char command=0;
unsigned long tempsPrecedent=0;


byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x1D, 0x77 };

IPAddress ip(192, 168, 2, 10);
EthernetServer server(4080);

void setup()
{
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  delay(2000);
  myservo.attach(6);
  myservo.write(0); 
  Serial.print("Servo Arduino IP: ");
  Serial.println(Ethernet.localIP());
  server.begin();

}
///////////////////////////////////////////////////////////////////////////////////////////////


void loop()                  
{
  EthernetClient client = server.available();
  if(client){
    command=client.read();
    Serial.println(command);
    if(command == '1'){
      myservo.write(180);
      tempsPrecedent=millis();
    }
    command=2;
    if (myservo.read()!=0)
      client.print("1");
    else
      client.print("0");
  }
  

 if(millis() - tempsPrecedent>=20000){
  myservo.write(1);
  tempsPrecedent=millis();
  }
}

  

            
