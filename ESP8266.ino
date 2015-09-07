
#include <SoftwareSerial.h>
SoftwareSerial serial2(8, 9); // RX, TX
int ALLOWOUTPUT = 0;

void setup()  
{
  // Open serial communications and wait for port to open:
  if( ALLOWOUTPUT ) Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // set the data rate for the SoftwareSerial port
  serial2.begin(9600);
  
  int tm = millis();
  while(1) {
    serial2.println("AT+RST");
    if( serial2.find("ready") )
      break;
    if( millis() - tm > 30000 )
      return;
  }
  Serial.println("Device ready");
  
  serial2.println("AT+GMR");
  waitForResponse("OK", 2000);
  
  serial2.println("AT+CWMODE=1");
  waitForResponse("OK", 2000);
  
  serial2.println("AT+CWJAP=\"SSID\",\"wifipassword\"");
  waitForResponse("OK", 30000);
  
  serial2.println("AT+CWJAP?");
  waitForResponse("OK", 5000);
  
  serial2.println("AT+CIPSTATUS");
  waitForResponse("OK", 2000);
  
  serial2.println("AT+CIFSR");
  waitForResponse("OK", 30000);
  
  
}

void loop() {
  //while( serial2.available() ) Serial.write(serial2.read());
  //while( Serial.available() ) serial2.write(Serial.read());
  
  sendEvent("FRAME/millis", String(millis()) );
  delay(60000);
}

void sendEvent( char *name, String value ) {
  char* buff = (char*)malloc(value.length()+1);
  value.toCharArray(buff, value.length());

  sendEvent(name, buff);
  free(buff);
  
}


void sendEvent(char *name, char *value ) {
  serial2.println("AT+CIPMUX=0");
  waitForResponse("OK", 30000);
  
  //byte server[] = {23,21,201,35};
  String command;
  String server = "photon-ruby.herokuapp.com";
  String server_ip = "23.23.138.248";
  String url = "GET /events/add?event_name=" + String(name) + String("&event_value=") + String(value) + String(" HTTP/1.0");  
  url = url + "\r\nHost: " + server + "\r\n\r\n";
  
  Serial.print("About to send: " );
  Serial.println(url);
  
  int length = url.length();

  command = "AT+CIPSTART=\"TCP\",\"" + server_ip + "\",80";
  serial2.println(command);
  waitForResponse("ready", 60000);
  
  command = "AT+CIPSEND=";
  serial2.print(command);
  serial2.println(length);
  waitForResponse(">", 30000);
  
  Serial.println("\r\nSending command");
  serial2.print(url);
  waitForData(5000);
  
  Serial.println("All done");
  serial2.println("AT+CIPCLOSE");
  
}

void waitForData(int tmout) {
  unsigned int timeout = millis();
  while(1) {
    while( serial2.available() ) {
      serial2.read();
      //Serial.write(serial2.read());
      timeout = millis();
    }
    if( millis()-timeout > tmout){
      Serial.println("timeout");
      break;
    }
  } 
}

void waitForResponse(char *resp, int tmout) {
  unsigned int timeout = millis();
  char string[255];
  char string_index = 0;
  int done = 0;
  int incomingByte;
  
  String t;
  
  memset(&string[0], 0, 255);
  
  while( !done ) {
    if( serial2.available() )  {
      incomingByte = serial2.read();
      string[++string_index]=incomingByte;
      if( incomingByte == 10 ) {
        if(t.length() > 0 ) {
          Serial.print("Line=");
          Serial.println(t);
        }
        t="";
      }
      if( incomingByte >13) {
        t += char(incomingByte);
      }
      //Serial.write(incomingByte);
    }
    if( t.compareTo(resp) == 0) {Serial.println(":"+String(resp)); done=1; continue;}
    if( t.compareTo("OK") == 0) {Serial.println(":OK");done = 1;continue;}
    if( t.compareTo("ERROR") == 0) {Serial.println(":error");done = 1;continue;}
    if( t.compareTo("no change") == 0) {Serial.println(":no change"); done = 1;continue;}
    if( t.compareTo("ready") == 0) {Serial.println(":ready"); done = 1;continue;}
    
    if( millis()-timeout > tmout){
      Serial.println("timeout");
      Serial.println(t);
      done = 1;
    }
  }
  
  
  while( serial2.available() ) Serial.write(serial2.read());
  
}
