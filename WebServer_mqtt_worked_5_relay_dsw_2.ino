#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define TOPIC_TEMP_F1_LIVING "uno/temp/f1/living"
#define TOPIC_TEMP_F1_KITCHEN "uno/temp/f1/kitchen"
#define TOPIC_TEMP_F1_TOILET "uno/temp/f1/toilet"
#define TOPIC_TEMP_F1_BOILER_ROOM "uno/temp/f1/boiler_room"
#define TOPIC_TEMP_F2_ROOM1 "uno/temp/f2/room1"
#define TOPIC_TEMP_F2_ROOM2 "uno/temp/f2/room1"


#define TOPIC_TEMP_F1_LIVING_SET "uno/temp/f1/living/set"
#define TOPIC_TEMP_F1_KITCHEN_SET "uno/temp/f1/kitchen/set"
#define TOPIC_TEMP_F1_TOILET_SET "uno/temp/f1/toilet/set"
#define TOPIC_TEMP_F1_BOILER_ROOM_SET "uno/temp/f1/boiler_room/set"
#define TOPIC_TEMP_F2_ROOM1_SET "uno/temp/f2/room1/set"
#define TOPIC_TEMP_F2_ROOM2_SET "uno/temp/f2/room1/set"
//#define TOPIC_TEMP_F2_ROOM3_SET "uno/temp/f2/room3/set"

#define TOPIC_RELAY_F1_LIVING_COMMAND "uno/rl/1/cmd"
#define TOPIC_RELAY_F1_KITCHEN_COMMAND "uno/rl/2/cmd"
#define TOPIC_RELAY_F1_BOILER_ROOM_COMMAND "uno/rl/3/cmd"
#define TOPIC_RELAY_F2_ROOM1_COMMAND "uno/rl/4/cmd"
#define TOPIC_RELAY_F2_ROOM2_COMMAND "uno/rl/5/cmd"

#define TOPIC_RELAY_F1_LIVING_STATE "uno/rl/1/st"
#define TOPIC_RELAY_F1_KITCHEN_STATE "uno/rl/2/st"
#define TOPIC_RELAY_F1_BOILER_ROOM_STATE "uno/rl/3/st"
#define TOPIC_RELAY_F2_ROOM1_STATE "uno/rl/4/st"
#define TOPIC_RELAY_F2_ROOM2_STATE "uno/rl/5/st"
      
#define FIRST_START_TIMEOUT 1000

#define READ_TEMP_INTERVAL 10*1000 //30sec

#define TERMOSTAT_DELTA 50 // i.e. 1 grad

#define MAX_DS18B20_SENSORS 3
byte addr_f1_kitchen[8] = {0x28, 0xFF, 0xB8, 0xBA, 0x73, 0x16, 0x05, 0xC1}; 
byte addr_f1_living[8] = {0x28, 0xFF, 0x81, 0xE9, 0x74, 0x16, 0x03, 0x41};
byte addr_f1_boiler_room[8] = {0x28, 0xFF, 0x9B, 0xC3, 0x73, 0x16, 0x05, 0x4C};

#define ONE_WIRE_BUS 8
OneWire  ds(ONE_WIRE_BUS);  // on pin 10
DallasTemperature temp_sensors(&ds);


byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
IPAddress mqtt_server(192, 168, 1, 45);

//*************** PIN's definition ***************************
const byte Relay01_pin = 2;  //pin 2 - floor1 living room
byte Relay01_state = HIGH;
byte Relay01_mode = 0; // 0 - manual, 1- auto

const byte Relay02_pin = 3;  //pin 3 - floor1 kitchen
byte Relay02_state = HIGH;
byte Relay02_mode = 0; // 0 - manual, 1- auto

const byte Relay03_pin = 5;  //pin 5 - floor1 boiler room
byte Relay03_state = HIGH;
byte Relay03_mode = 0; // 0 - manual, 1- auto

const byte Relay04_pin = 6;  //pin 6 - floor2 room1
byte Relay04_state = HIGH;
byte Relay04_mode = 0; // 0 - manual, 1- auto

const byte Relay05_pin = 7;  //pin 7 - floor2 room2
byte Relay05_state = HIGH;
byte Relay05_mode = 0; // 0 - manual, 1- auto

long lastReadingTime = 0;
long lastReadingTime2 = 0;


byte HeatingF1_count = 0;

int TempF1_Living = -12700;
int TempF1_Living_set = 2900;  // get by mqtt

int TempF1_Kitchen = -12700;
int TempF1_Kitchen_set = 2900; // get by mqtt

int TempF1_BoilerRoom = -12700; 
int TempF1_BoilerRoom_set = 2900; // get by mqtt

int TempF1_Toilet = -12700;      // get by mqtt
int TempF1_Toilet_set = 2900;  // get by mqtt

int TempF2_Room1 = -12700;   // get by mqtt
int TempF2_Room1_set = 2900; // get by mqtt

int TempF2_Room2 = -12700;   // get by mqtt
int TempF2_Room2_set = 2900; // get by mqtt

int TempF2_Room3 = -12700;   // get by mqtt
int TempF2_Room3_set = 2900; // get by mqtt

int Temp01 = -12800;
int Temp02 = -12800;
int Temp03 = -12800;
int Hum01 = 0;

EthernetServer server(80);
EthernetClient ethClient;
PubSubClient mqtt_client(ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';

//Serial.println(topic); Serial.println((char)payload);
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
  else if ( strcmp(topic, TOPIC_RELAY_F1_LIVING_COMMAND ) == 0) { switchRelay01(payload); } 
  else if ( strcmp(topic, TOPIC_RELAY_F1_KITCHEN_COMMAND ) == 0) { switchRelay02(payload); } 
  else if ( strcmp(topic, TOPIC_RELAY_F1_BOILER_ROOM_COMMAND ) == 0) { switchRelay03(payload); } 
  else if ( strcmp(topic, TOPIC_RELAY_F2_ROOM1_COMMAND ) == 0) { switchRelay04(payload); } 
  else if ( strcmp(topic, TOPIC_RELAY_F2_ROOM2_COMMAND ) == 0) { switchRelay05(payload); } 

  else if ( strcmp(topic, TOPIC_TEMP_F1_LIVING_SET) == 0) {  
    Serial.println("TOPIC_TEMP_F1_LIVING_SET"); /*TempF1_Living_set = 0;*/ 
    //Serial.println((char*)payload); /*TempF1_Living_set = 0;*/ 
    char *cstring = (char *) payload;
     Serial.println(cstring);
     TempF1_Living_set = atof(cstring)*100;
     Serial.println(TempF1_Living_set);
     }
  else if ( strcmp(topic, TOPIC_TEMP_F1_KITCHEN_SET) == 0) {
        char *cstring = (char *) payload;
     Serial.println(cstring);
     TempF1_Kitchen_set = atof(cstring)*100;
     Serial.println(TempF1_Kitchen_set);
    /*TempF1_Kitchen_set = 0;*/ 
    }
  else if ( strcmp(topic, TOPIC_TEMP_F1_TOILET_SET) == 0) {  /*TempF1_Toilet_set = 0;*/ }  
  else if ( strcmp(topic, TOPIC_TEMP_F1_BOILER_ROOM_SET) == 0) { 
            char *cstring = (char *) payload;
     Serial.println(cstring);
     TempF1_BoilerRoom_set = atof(cstring)*100;
     Serial.println(TempF1_BoilerRoom_set);
    /*TempF1_BoilerRoom_set = 0;*/ 
    }  
  else if ( strcmp(topic, TOPIC_TEMP_F2_ROOM1_SET) == 0) { /*TempF2_Room1_set = 0;*/ }  
  else if ( strcmp(topic, TOPIC_TEMP_F2_ROOM2_SET) == 0) {  /*TempF2_Room2_set = 0;*/ }  
  //else if ( strcmp(topic, TOPIC_TEMP_F2_ROOM3_SET) == 0) { TempF2_Room3_set = 0; }  


  //else {
  //  Serial.println("FAIL!!!");
  // }
  //Serial.println();
}


void relay_state_publish(const char* topic, byte state) {
  mqtt_client.publish( topic, (state == LOW) ? "ON" : "OFF" );
}

void dsw_temp_publish_by_addr(byte addr[8], int value) {
  if ( value < -10000 || value > 8500 ) return;
  char *m = new char[24];
  memset(m, '\0', 24);
  strcat(m, "uno/dsw/");
  char *s = new char[2];
  for( byte i = 0; i < 8; i++) { sprintf(s, "%02X", addr[i]);  strncat(m, s, 2); }
  char *temp = new char[6];
  sprintf(temp, "%d.%02d", (int)value/100, (int)(value)%100);
  mqtt_client.publish( m, temp );

  Serial.println("-------------------------");
  Serial.print(m);
  Serial.print("   ");
  Serial.print(temp);
  Serial.println();
  delete [] temp;
  delete [] s;
  delete [] m;  
}

void dsw_temp_publish_by_topic(const char* topic, int value) {
  if ( value < -10000 || value > 8500 ) return;
  char *temp = new char[6];
  sprintf(temp, "%d.%02d", (int)value/100, (int)(value)%100);
  mqtt_client.publish( topic, temp );
  delete [] temp;
}

byte switchRelay(byte relay_pin, byte idx, byte* payload) {
  byte relay_state = ( strcmp( payload,"OFF") == 0) ? HIGH : LOW;
  EEPROM.write(idx, relay_state);
  digitalWrite(relay_pin, relay_state);
  return relay_state;
}

void switchRelay01(byte* payload) {
  Relay01_state = switchRelay(Relay01_pin, 1, payload);
  Serial.print("Relay01 state: ");
  Serial.print( Relay01_state );
  Serial.println();
  if (Relay01_state) { HeatingF1_count--; } else {HeatingF1_count++;}
  if (HeatingF1_count < 0) HeatingF1_count = 0;
  if (HeatingF1_count > 4) HeatingF1_count = 4;
  delay(50);
}

void switchRelay02(byte* payload) {
  Relay02_state = switchRelay(Relay02_pin, 2, payload);
  Serial.print("Relay02 state: ");
  Serial.print( Relay02_state );  
  Serial.println();
  if (Relay02_state) { HeatingF1_count--; } else {HeatingF1_count++;}
  if (HeatingF1_count < 0) HeatingF1_count = 0;
  if (HeatingF1_count > 4) HeatingF1_count = 4;  
  delay(50);
}

void switchRelay03(byte* payload) {
  Relay03_state = switchRelay(Relay03_pin, 3, payload);
  Serial.print("Relay03 state: ");
  Serial.print( Relay03_state );  
  Serial.println();
  if (Relay03_state) { HeatingF1_count--; } else {HeatingF1_count++;}
  if (HeatingF1_count < 0) HeatingF1_count = 0;
  if (HeatingF1_count > 4) HeatingF1_count = 4; 
  delay(50); 
}


void switchRelay04(byte* payload) {
  Relay04_state = switchRelay(Relay04_pin, 4, payload);
  Serial.print("Relay04 state: ");
  Serial.print( Relay04_state );
  Serial.println();
  if (Relay04_state) { HeatingF1_count--; } else {HeatingF1_count++;}
  if (HeatingF1_count < 0) HeatingF1_count = 0;
  if (HeatingF1_count > 4) HeatingF1_count = 4;  
  delay(50);  
}

void switchRelay05(byte* payload) {
  Relay05_state = switchRelay(Relay05_pin, 5, payload);
  Serial.print("Relay05 state: ");
  Serial.print( Relay05_state );  
  Serial.println();
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

      relay_state_publish( TOPIC_RELAY_F1_LIVING_STATE, Relay01_state);
      relay_state_publish( TOPIC_RELAY_F1_KITCHEN_STATE, Relay02_state);
      relay_state_publish( TOPIC_RELAY_F1_BOILER_ROOM_STATE, Relay03_state);
      relay_state_publish( TOPIC_RELAY_F2_ROOM1_STATE, Relay04_state);
      relay_state_publish( TOPIC_RELAY_F2_ROOM2_STATE, Relay05_state);
       
      mqtt_client.subscribe( "esp-d1-mini/ESP-D1-mini/dsw1");
      mqtt_client.subscribe( "esp-witty/ESP-Witty/#");
      mqtt_client.subscribe( "UNO/#" );
      mqtt_client.subscribe( "uno/#" );
      
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
  HeatingF1_count = 0;
  pinMode(Relay01_pin, OUTPUT);
  //digitalWrite(Relay01_pin, HIGH);
  Relay01_state = EEPROM.read(1);
  if (!Relay01_state) {
    HeatingF1_count++;
    delay(FIRST_START_TIMEOUT);
    digitalWrite(Relay01_pin, Relay01_state); 
  }
  
  
  //switchRelay01(Relay01_state);
  
  pinMode(Relay02_pin, OUTPUT);
  //digitalWrite(Relay02_pin, HIGH);
  Relay02_state = EEPROM.read(2);
  if (!Relay02_state) {
    HeatingF1_count++;
    delay(FIRST_START_TIMEOUT);
    digitalWrite(Relay02_pin, Relay02_state); 
  }
  //switchRelay02(Relay02_state);
    
  pinMode(Relay03_pin, OUTPUT);
  //digitalWrite(Relay03_pin, HIGH);
  Relay03_state = EEPROM.read(3);
  if (!Relay03_state) {
    HeatingF1_count++;
    delay(FIRST_START_TIMEOUT);
    digitalWrite(Relay03_pin, Relay03_state); 
  }
  //switchRelay03(Relay03_state);
    
  pinMode(Relay04_pin, OUTPUT);
  //digitalWrite(Relay04_pin, HIGH);
  Relay04_state = EEPROM.read(4);
  if (!Relay04_state) {
    HeatingF1_count++;
    delay(FIRST_START_TIMEOUT);
    digitalWrite(Relay04_pin, Relay04_state); 
  }
  //switchRelay04(Relay04_state);
    
  pinMode(Relay05_pin, OUTPUT);
  //digitalWrite(Relay05_pin, HIGH);
  Relay05_state = EEPROM.read(5);
  if (!Relay05_state) {
    //HeatingF1_count++;
    delay(FIRST_START_TIMEOUT);
    digitalWrite(Relay05_pin, Relay05_state); 
  }
 //switchRelay05(Relay05_state);

  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  
  temp_sensors.begin();
  
  delay(50);
  
  if (mqtt_client.connected()) {
      relay_state_publish( TOPIC_RELAY_F1_LIVING_STATE, Relay01_state);
      relay_state_publish( TOPIC_RELAY_F1_KITCHEN_STATE, Relay02_state);
      relay_state_publish( TOPIC_RELAY_F1_BOILER_ROOM_STATE, Relay03_state);
      relay_state_publish( TOPIC_RELAY_F2_ROOM1_STATE, Relay04_state);
      relay_state_publish( TOPIC_RELAY_F2_ROOM2_STATE, Relay05_state);
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
  print_relay_info(client, Relay01_state, "Relay1 (Living)", "01"); 
  print_relay_info(client, Relay02_state, "Relay2 (Kitchen)", "02"); 
  print_relay_info(client, Relay03_state, "Relay3 (BoilerRoom)", "03"); 
  print_relay_info(client, Relay04_state, "Relay4 (FL2-Room1)", "04"); 
  print_relay_info(client, Relay05_state, "Relay5 (FL2-Room2)", "05"); 
  
  print_val(client, Temp01, "Temperature01", " *C" );
  print_val(client, Temp02, "Temperature02", " *C");
  print_val(client, Temp03, "Temperature03", " *C");
  print_val(client, Hum01, "Hummidity01", " %");

  
  print_val(client, TempF1_Kitchen, "Kitchen, t: ", " *C");
  print_val(client, TempF1_Living, "Living, t: ", " *C");
  print_val(client, TempF1_BoilerRoom, "Boiler room, t: ", " *C");
  
  client.println(F("<br>"));
  
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
  //if( (temp > -4000) && (temp < 8500)){
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
  //}
}

void updateTemperatures() {
  if (millis() - lastReadingTime2 > READ_TEMP_INTERVAL) {
    lastReadingTime2 = millis();
    Serial.println( lastReadingTime2 / 1000 );
    temp_sensors.requestTemperatures();
    Serial.println(temp_sensors.getTempC(addr_f1_kitchen));  
    Serial.println(temp_sensors.getTempC(addr_f1_living));  
    Serial.println(temp_sensors.getTempC(addr_f1_boiler_room));  

    float v = temp_sensors.getTempC(addr_f1_kitchen);
    TempF1_Kitchen = (int)(v * 100);
    v = temp_sensors.getTempC(addr_f1_living);
    TempF1_Living = (int)(v * 100);
    v = temp_sensors.getTempC(addr_f1_boiler_room);
    TempF1_BoilerRoom = (int)(v * 100);

    //dsw_temp_publish_by_addr(addr_f1_kitchen, TempF1_Kitchen);
    dsw_temp_publish_by_topic( TOPIC_TEMP_F1_KITCHEN, TempF1_Kitchen);
    //dsw_temp_publish_by_addr(addr_f1_living, TempF1_Living);
    dsw_temp_publish_by_topic( TOPIC_TEMP_F1_LIVING, TempF1_Living);
    //dsw_temp_publish_by_addr(addr_f1_boiler_room, TempF1_BoilerRoom);
    dsw_temp_publish_by_topic(TOPIC_TEMP_F1_BOILER_ROOM, TempF1_BoilerRoom);

    termostat();
  }  
}

void heating_turn_on_off(const char* room, byte* state) {
  if ( strcmp(room, "f1_living") == 0 ) {
    switchRelay01( state );
  } else if (strcmp(room, "f1_kitchen") == 0) {
    switchRelay02( state );
  } else if (strcmp(room, "f1_boilerroom") == 0) {
    switchRelay03( state );
  } else if (strcmp(room, "f2_room1" == 0)) {
     switchRelay04( state );
  } else if (strcmp(room, "f2_room2") == 0) {
     switchRelay05( state );
  }
}

void termostat() {
  Serial.println("--- TERMOSTATE START ------");
  if ( (TempF1_Living  > -10000) && (TempF1_Living < (TempF1_Living_set - TERMOSTAT_DELTA)))  {
    Serial.println("Termostate: Living ON");
    Serial.print(TempF1_Living);
    Serial.print("    set: ");
    Serial.println((TempF1_Living_set - TERMOSTAT_DELTA));
    heating_turn_on_off("f1_living", "ON"); //ON - включить конвекторы в зале
  } else if ( TempF1_Living  > (TempF1_Living_set + TERMOSTAT_DELTA)) {
    heating_turn_on_off("f1_living", "OFF"); //OFF - выключить конвекторы в зале
    Serial.println("Termostate: Living OFF");
    Serial.print(TempF1_Living);
    Serial.print("    set: ");
    Serial.print((TempF1_Living_set + TERMOSTAT_DELTA));
    Serial.println();
  } else {
    Serial.println("Termostate: Living NO CHANGE");
    Serial.print(TempF1_Living);
    Serial.print("    set: ");
    Serial.print((TempF1_Living_set - TERMOSTAT_DELTA));        
    Serial.print("  <--> ");
    Serial.print((TempF1_Living_set + TERMOSTAT_DELTA));       
    Serial.println();
  }

  delay(50);
  if ( (TempF1_Kitchen  > -10000) && (TempF1_Kitchen < (TempF1_Kitchen_set - TERMOSTAT_DELTA)))  {
    heating_turn_on_off("f1_kitchen", "ON"); //ON - включить конвекторы на кухне
    Serial.println("Termostate: Kithcen ON");
    Serial.print(TempF1_Kitchen);
    Serial.print("    set: ");
    Serial.println((TempF1_Kitchen_set - TERMOSTAT_DELTA));
  } else if (TempF1_Kitchen  > (TempF1_Kitchen_set + TERMOSTAT_DELTA)) {
    heating_turn_on_off("f1_kitchen", "OFF"); //OFF - выключить конвекторы на кухне
    Serial.println("Termostate: Kithcen OFF");
    Serial.print(TempF1_Kitchen);
    Serial.print("    set: ");
    Serial.println((TempF1_Kitchen_set + TERMOSTAT_DELTA));
  } else {
    Serial.println("Termostate: Kitchen NO CHANGE");
    Serial.print(TempF1_Kitchen);
    Serial.print("    set: ");
    Serial.print((TempF1_Kitchen_set - TERMOSTAT_DELTA));        
    Serial.print("  <--> ");
    Serial.print((TempF1_Kitchen_set + TERMOSTAT_DELTA)); 
    Serial.println();         
  }

  delay(50);
  if ( (TempF1_BoilerRoom  > -10000) && (TempF1_BoilerRoom < (TempF1_BoilerRoom_set - TERMOSTAT_DELTA)))  {
    heating_turn_on_off("f1_boilerroom", "ON"); // ON - включить конвекторы в котельной
    Serial.println("Termostate: BoilerRoom ON");
    Serial.print(TempF1_BoilerRoom);
    Serial.print("    set: ");
    Serial.println((TempF1_BoilerRoom_set - TERMOSTAT_DELTA));
  } else if (TempF1_BoilerRoom  > (TempF1_BoilerRoom_set + TERMOSTAT_DELTA)) {
    heating_turn_on_off("f1_boilerroom", "OFF"); //OFF - выключить конвекторы в котельной
    Serial.println("Termostate: BoilerRoom OFF");
    Serial.print(TempF1_BoilerRoom);
    Serial.print("    set: ");
    Serial.println((TempF1_BoilerRoom_set + TERMOSTAT_DELTA));
  }  else {
    Serial.println("Termostate: BoilerRoom NO CHANGE");
    Serial.print(TempF1_BoilerRoom);
    Serial.print("    set: ");
    Serial.print((TempF1_BoilerRoom_set - TERMOSTAT_DELTA));        
    Serial.print("  <--> ");
    Serial.print((TempF1_BoilerRoom_set + TERMOSTAT_DELTA));   
    Serial.println();
  }


  delay(50);
  if ( HeatingF1_count >= 4) { 
    // включены все конвекторы на 1-м этаже
    // выключить конвекторы на 2-м
     heating_turn_on_off("f2_room1", "OFF"); // OFF - выключить конвекторы в этаж2.комната1
     Serial.println("Termostate: Room1 OFF");
     
     heating_turn_on_off("f2_room2", "OFF"); // OFF - выключить конвекторы в этаж2.комната2
     Serial.println("Termostate: Room2 OFF");
     //heating_turn_on_off("f2_room3", "OFF"); //
  } else {
    // какой-то из конвекторов на этаже 1 не включен, можно включать 2-ой этаж
    // TODO - смотреть общее потребление тока
    // конвектор в этаж2.комната1 можно включить только когда на этаже 1 включено 2 конвектора + этаж2.комната2
    //или на этаже1 включено 3 конвектора и не включен конвектор этаж2.комната2
    if (   //3 любых на 1-м и 1 на втором
          (HeatingF1_count < 3) ||      // если включено менее 3-х на первом и (возможно) этаж2.комната2, то можно включить этаж2.комната1
          (HeatingF1_count = 3 && ! Relay05_state) // если включено 3 любых на первом и не включен этаж2.комната2
        ) 
        {
          if ( (TempF2_Room1  > -10000) && (TempF2_Room1 < (TempF2_Room1_set - TERMOSTAT_DELTA)))  {
            heating_turn_on_off("f2_room1", "ON"); //ON - включить конвекторы в этаж2.комната1
            Serial.println("Termostate: Room1 ON");
            Serial.print(TempF2_Room1);
            Serial.print("    set: ");
            Serial.println(TempF2_Room1_set);
          } else if (TempF2_Room1   > (TempF2_Room1_set + TERMOSTAT_DELTA)) {
            heating_turn_on_off("f2_room1", "OFF");  //OFF - выключить конвекторы в этаж2.комната1
            Serial.println("Termostate: Room1 OFF");
            Serial.print(TempF2_Room1);
            Serial.print("    set: ");
            Serial.println(TempF2_Room1_set);
          } else {
            Serial.println("Termostate: Room1 NO CHANGE");
            Serial.print(TempF2_Room1);
            Serial.print("    set: ");
            Serial.println(TempF2_Room1_set);        
          }
        }

        // конвектор в этаж2.комната2 можно включить только когда на этаже 1 включено 2 конвектора + этаж2.комната1
        //или на этаже1 включено 3 конвектора и не включен конвектор этаж2.комната1
    if (   //3 любых на 1-м и 1 на втором
          (HeatingF1_count < 3) ||      // если включено менее 3-х на первом и (возможно) этаж2.комната1, то можно включить этаж1.комната2
          (HeatingF1_count = 3 && ! Relay04_state) // если включено 3 любых на первом и не включен этаж2.комната1
       ) 
        {
          if ( (TempF2_Room2  > -10000) && (TempF2_Room2 < (TempF2_Room2_set - TERMOSTAT_DELTA)))  {
            heating_turn_on_off("f2_room2", "ON"); //ON - включить конвекторы в этаж2.комната2
            Serial.println("Termostate: Room2 ON");
            Serial.print(TempF2_Room2);
            Serial.print("    set: ");
            Serial.println(TempF2_Room2_set);
          } else if (TempF2_Room2   > (TempF2_Room2_set + TERMOSTAT_DELTA)) {
            heating_turn_on_off("f2_room2", "OFF"); //OFF - выключить конвекторы в этаж2.комната2
            Serial.println("Termostate: Room2 OFF");
            Serial.print(TempF2_Room2);
            Serial.print("    set: ");
            Serial.println(TempF2_Room2_set);
          } else {
            Serial.println("Termostate: Room2 NO CHANGE");
            Serial.print(TempF2_Room2);
            Serial.print("    set: ");
            Serial.println(TempF2_Room2_set);        
          }
        }
  } 
   Serial.println("--- TERMOSTATE END ------");
}

void loop() {
  if (millis() - lastReadingTime > 1000) {
    lastReadingTime = millis();
    //Serial.println( lastReadingTime );
    if (mqtt_client.connected()) {
        relay_state_publish( TOPIC_RELAY_F1_LIVING_STATE, Relay01_state);
        relay_state_publish( TOPIC_RELAY_F1_KITCHEN_STATE, Relay02_state);
        relay_state_publish( TOPIC_RELAY_F1_BOILER_ROOM_STATE, Relay03_state);
        relay_state_publish( TOPIC_RELAY_F2_ROOM1_STATE, Relay04_state);
        relay_state_publish( TOPIC_RELAY_F2_ROOM2_STATE, Relay05_state);

        Serial.print("HeatingF1_count: ");
        Serial.print(HeatingF1_count);
        Serial.println();
      }   
  }

  updateTemperatures();


  
  if (!mqtt_client.connected()) {
    reconnect();
  } 
  
  mqtt_client.loop();
    
  // listen for incoming clients
  //listenForEthernetClients();

}

