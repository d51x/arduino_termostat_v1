#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <OneWire.h>

#define FIRST_START_TIMEOUT 5 //5000

#define ONE_WIRE_BUS 8
OneWire  ds(ONE_WIRE_BUS);  // on pin 10

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
IPAddress mqtt_server(192, 168, 1, 45);

//*************** PIN's definition ***************************
const byte Relay01_pin = 2;  //pin 2 - floor1 living room
byte Relay01_state = HIGH;

const byte Relay02_pin = 3;  //pin 3 - floor1 kitchen
byte Relay02_state = HIGH;

const byte Relay03_pin = 5;  //pin 5 - floor1 boiler room
byte Relay03_state = HIGH;

const byte Relay04_pin = 6;  //pin 6 - floor2 room1
byte Relay04_state = HIGH;

const byte Relay05_pin = 7;  //pin 7 - floor2 room2
byte Relay05_state = HIGH;

long lastReadingTime = 0;
boolean isFirstStart = true;

int Temp01 = -128;
int Temp02 = -128;
int Temp03 = -128;
int Hum01 = 0;

int Temp_1wire_01 = -128;
int Temp_1wire_02 = -128;
int Temp_1wire_03 = -128;

EthernetServer server(80);
EthernetClient ethClient;
PubSubClient mqtt_client(ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';

  if ( strcmp(topic, "esp-d1-mini/ESP-D1-mini/dsw1") == 0 ) {
    //Serial.write((char*)payload, length);
    String t;
    for (int i=0;i<length;i++) { t += (char)payload[i]; }
     //Serial.println();
     Temp01 = t.toFloat()*10;
     //Serial.println( Temp01 );
  } 
  else if ( strcmp(topic, "esp-witty/ESP-Witty/dsw1") == 0 ) {
    //Serial.write((char*)payload, length);
    String t;
    for (int i=0;i<length;i++) { t += (char)payload[i]; }
     //Serial.println();
     Temp02 = t.toFloat()*10;
     //Serial.println( Temp01 );
  }   
  else if ( strcmp(topic, "esp-witty/ESP-Witty/dhtt1") == 0 ) {
    //Serial.write((char*)payload, length);
    String t;
    for (int i=0;i<length;i++) { t += (char)payload[i]; }
     //Serial.println();
     Temp03 = t.toFloat()*10;
     //Serial.println( Temp01 );
  }   
  else if ( strcmp(topic, "esp-witty/ESP-Witty/dhth1") == 0 ) {
    //Serial.write((char*)payload, length);
    String t;
    for (int i=0;i<length;i++) { t += (char)payload[i]; }
     //Serial.println();
     Hum01 = t.toFloat()*10;
     //Serial.println( Temp01 );
  }   
  else if ( strcmp(topic, "UNO/RL/1/CMD" ) == 0) { switchRelay01(payload); } 
  else if ( strcmp(topic, "UNO/RL/2/CMD" ) == 0) { switchRelay02(payload); } 
  else if ( strcmp(topic, "UNO/RL/3/CMD" ) == 0) { switchRelay03(payload); } 
  else if ( strcmp(topic, "UNO/RL/4/CMD" ) == 0) { switchRelay04(payload); } 
  else if ( strcmp(topic, "UNO/RL/5/CMD" ) == 0) { switchRelay05(payload); } 
  //else {
  //  Serial.println("FAIL!!!");
  // }
  //Serial.println();
}


void relay_state_publish(const char* topic, byte state) {
  mqtt_client.publish( topic, (state == LOW) ? "ON" : "OFF" );
}

void dsw_temp_publish(const char* topic, const char* value) {
  char t[36] = "arduino/dsw/";
  strcat(t, topic);
  mqtt_client.publish( t, value );
}


byte switchRelay(byte relay_pin, byte idx, byte* payload) {
  byte relay_state = ( strcmp( payload,"OFF") == 0) ? HIGH : LOW;
  EEPROM.write(idx, relay_state);
  digitalWrite(relay_pin, relay_state);
  return relay_state;
}

void switchRelay01(byte* payload) {
  Relay01_state = switchRelay(Relay01_pin, 1, payload);
}

void switchRelay02(byte* payload) {
  Relay02_state = switchRelay(Relay02_pin, 2, payload);
}

void switchRelay03(byte* payload) {
  Relay03_state = switchRelay(Relay03_pin, 3, payload);
}


void switchRelay04(byte* payload) {
  Relay04_state = switchRelay(Relay04_pin, 4, payload);
}

void switchRelay05(byte* payload) {
  Relay05_state = switchRelay(Relay05_pin, 5, payload);
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect("uno")) {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      //mqtt_client.publish("outTopic","hello world");
      // ... and resubscribe

      relay_state_publish( "UNO/RL/1/ST", Relay01_state);
      relay_state_publish( "UNO/RL/2/ST", Relay02_state);
      relay_state_publish( "UNO/RL/3/ST", Relay03_state);
      relay_state_publish( "UNO/RL/4/ST", Relay04_state);
      relay_state_publish( "UNO/RL/5/ST", Relay05_state);
       
      mqtt_client.subscribe( "esp-d1-mini/ESP-D1-mini/dsw1");
      mqtt_client.subscribe( "esp-witty/ESP-Witty/dsw1");
      mqtt_client.subscribe( "esp-witty/ESP-Witty/dhtt1");
      mqtt_client.subscribe( "esp-witty/ESP-Witty/dhth1");
      mqtt_client.subscribe( "UNO/RL/1/CMD" );
      mqtt_client.subscribe( "UNO/RL/2/CMD" );
      mqtt_client.subscribe( "UNO/RL/3/CMD" );
      mqtt_client.subscribe( "UNO/RL/4/CMD" );
      mqtt_client.subscribe( "UNO/RL/5/CMD" );
      
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(mqtt_client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  } 
}


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(Relay01_pin, OUTPUT);
  //digitalWrite(Relay01_pin, HIGH);
  Relay01_state = EEPROM.read(1);
 digitalWrite(Relay01_pin, Relay01_state); 
  
  pinMode(Relay02_pin, OUTPUT);
  //digitalWrite(Relay02_pin, HIGH);
  Relay02_state = EEPROM.read(2);
  digitalWrite(Relay02_pin, Relay02_state);
    
  pinMode(Relay03_pin, OUTPUT);
  //digitalWrite(Relay03_pin, HIGH);
  Relay03_state = EEPROM.read(3);
  digitalWrite(Relay03_pin, Relay03_state); 
    
  pinMode(Relay04_pin, OUTPUT);
  //digitalWrite(Relay04_pin, HIGH);
  Relay04_state = EEPROM.read(4);
  digitalWrite(Relay04_pin, Relay04_state);
    
  pinMode(Relay05_pin, OUTPUT);
  //digitalWrite(Relay05_pin, HIGH);
  Relay05_state = EEPROM.read(5);
 digitalWrite(Relay05_pin, Relay05_state);

  isFirstStart = false;
  
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();

  delay(50);
  if (mqtt_client.connected()) {
      relay_state_publish( "UNO/RL/1/ST", Relay01_state);
      relay_state_publish( "UNO/RL/2/ST", Relay02_state);
      relay_state_publish( "UNO/RL/3/ST", Relay03_state);
      relay_state_publish( "UNO/RL/4/ST", Relay04_state);
      relay_state_publish( "UNO/RL/5/ST", Relay05_state);
    }  
}

void listenForEthernetClients() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    //Serial.println("new client");
    // an http request ends with a blank line
    String request;
    boolean currentLineIsBlank = true;
    boolean requestLineReceived = false;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header

          // substract path from a request line
          request = request.substring( request.indexOf(' ') + 1, request.lastIndexOf(' '));
          //Serial.print( "Request: ");
          //Serial.print( request );
          //Serial.println();

          if(request=="/") {
            successHeader(client);
            generatePage(client);
          } 
          else if ( request == F("/relay/01/on") ) {
            switchRelay01( "ON" );
            redirectHeader(client, "/");
          } 
          else if ( request == F("/relay/01/off") ) {
            switchRelay01( "OFF" );
            redirectHeader(client, "/");            
          } 
          else if ( request == F("/relay/02/on") ) {
            switchRelay02( "ON" );
            redirectHeader(client, "/");
          } 
          else if ( request == F("/relay/02/off") ) {
            switchRelay02( "OFF" );
            redirectHeader(client, "/");            
          } 
          else if ( request == F("/relay/03/on") ) {
            switchRelay03( "ON" );
            redirectHeader(client, "/");
          } 
          else if ( request == F("/relay/03/off") ) {
            switchRelay03( "OFF" );
            redirectHeader(client, "/");            
          } 
          else if ( request == F("/relay/04/on") ) {
            switchRelay04( "ON" );
            redirectHeader(client, "/");
          } 
          else if ( request == F("/relay/04/off") ) {
            switchRelay04( "OFF" );
            redirectHeader(client, "/");            
          } 
          else if ( request == F("/relay/05/on") ) {
            switchRelay05( "ON" );
            redirectHeader(client, "/");
          } 
          else if ( request == F("/relay/05/off" )) {
            switchRelay05( "OFF" );
            redirectHeader(client, "/");            
          }

          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          if(!requestLineReceived){
            requestLineReceived = true;
          }          
        } else if (c != '\r') {
          if(!requestLineReceived) {
            request += c;
          }          
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    //Serial.println("client disconnected");
  }
}

void successHeader(EthernetClient client){
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(F("<meta http-equiv='content-type' content='text/html; charset=UTF-8'>"));
  client.println(F("Connnection: close"));
          //client.println("Refresh: 5");  // refresh the page automatically every 5   
          //client.println("<meta http-equiv='refresh' content='5'/>");
  client.println();
}

void redirectHeader(EthernetClient client, String path){
  client.println(F("HTTP/1.1 302 Moved Temporarily"));
  client.println(F("Content-Type: text/html"));
  client.println("Location: " + path);
  client.println(F("Connnection: keep-alive"));
  client.println();
}

void generatePage(EthernetClient client){
  client.println(F("<!DOCTYPE HTML>"));
  client.println(F("<html>"));
  client.println(F("<head>"));
  client.println(F("  <title>Server</title>"));
  client.println(F("</head>"));

  client.println(F("<body>"));


  //print_relay01_info(client);
  print_relay_info(client, Relay01_state, "Relay1", "01"); 
  print_relay_info(client, Relay02_state, "Relay2", "02"); 
  print_relay_info(client, Relay03_state, "Relay3", "03"); 
  print_relay_info(client, Relay04_state, "Relay4", "04"); 
  print_relay_info(client, Relay05_state, "Relay5", "05"); 
  
  print_val(client, Temp01, "Temperature01", " *C" );
  print_val(client, Temp02, "Temperature02", " *C");
  print_val(client, Temp03, "Temperature03", " *C");
  print_val(client, Hum01, "Hummidity01", " %");

  client.println(F("</body>"));
  client.println(F("</html>"));
}

void print_relay_info(EthernetClient client, byte relay_state, const char* title, const char* idx) {
  client.print(F("<strong>"));
  client.print(title);
  client.print(F(" is "));
  client.print( (relay_state == LOW) ? F(" ON ") : F(" OFF "));
  client.print(F("</strong><a href=\"/relay/"));
  client.print(idx);
  client.println((relay_state == LOW) ?  F("/off\">Switch OFF</a>") : F("/on\">Switch ON</a>"));
  client.println(F("<br>"));
}

void print_val(EthernetClient client, int temp, const char* title, const char* val) {
  client.println(F("  <br />"));
  //Serial.println( temp );
  if( (temp > -400) && (temp < 850)){
    char t1 = temp/10;
    char t2 = temp%10;
    char tt[6];
    sprintf(tt, "%d.%d", t1, t2);
    //Serial.println( tt );
    client.print(F("<strong><span>"));
    client.print( title );
    client.print(F("</span> "));
    client.print( tt ); 
    client.print( val );
    client.println(F("</strong>"));
  }
}

void generate_temp_topic_item(byte addr[8]) {
  char rom[24];
  char s[2];
  for( byte i = 0; i < 8; i++) {
    sprintf(s,"%s", addr[i]);
    //Serial.print(addr[i], HEX);
    //strcat(rom, s);
  }
    //Serial.println();
}

void get_onewire_temp(){
  byte i, j;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;



  
  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
    //Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }

  
  //Serial.print("ROM =");
  char *m = new char[16];
  memset(m, '\0', 16);
  char *s = new char[2];
  
  for( i = 0; i < 8; i++) {
    //Serial.write(' ');
    //Serial.print(addr[i], HEX);
    sprintf(s, "%02X", addr[i]);
    strncat(m, s, 2);
  }
  if (OneWire::crc8(addr, 7) != addr[7]) {
      //Serial.println("CRC is not valid!");
      return;
  }
  //Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      //Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  //Serial.print("  Data = ");
  //Serial.print(present, HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.print(" CRC=");
  //Serial.print(OneWire::crc8(data, 8), HEX);
  //Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;

  char *t = new char[6];
  //Serial.print("  Temperature = ");
  //Serial.print(celsius);
    //Serial.println();
    //Serial.println(m);    
    //long V;
    //V = round(celsius*100);
    
  //dtostrf(celsius, 4, 2, t);
   sprintf(t, "%d.%02d", (int)celsius, (int)(celsius*100)%100);
  //strncat(m, t, strlen(t));
    dsw_temp_publish(m, t);
    Serial.println(t);    
    Serial.println(m);    
    Serial.println(); 

    
     delete [] t;
     delete [] s;
    delete [] m;  
}

void loop() {
  // get data from sensors and pins
  // check for a reading no more than once a second.
  if (millis() - lastReadingTime > 1000) {
    lastReadingTime = millis();
    //Serial.println( lastReadingTime );
    if (mqtt_client.connected()) {
        relay_state_publish( "UNO/RL/1/ST", Relay01_state);
        relay_state_publish( "UNO/RL/2/ST", Relay02_state);
        relay_state_publish( "UNO/RL/3/ST", Relay03_state);
        relay_state_publish( "UNO/RL/4/ST", Relay04_state);
        relay_state_publish( "UNO/RL/5/ST", Relay05_state);
      }   
     get_onewire_temp(); 
  }
  
  if (!mqtt_client.connected()) {
    reconnect();
  } 
  
  mqtt_client.loop();
    
  // listen for incoming clients
  listenForEthernetClients();

}

