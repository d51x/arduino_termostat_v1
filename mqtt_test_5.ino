#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/pgmspace.h>
//#include <avr/wdt.h>

#define CONST_ON (const char *)F("ON")
#define CONST_OFF (const char *)F("OFF")


//#define WRITE_TO_SERIAL(val1,val2, val3, val4) { Serial.print(val1); Serial.print(val2); Serial.print(val3); Serial.println(val4); }


const char P_TOPIC_ARDUINO_RESTART[] PROGMEM           = "uno/restart";
const char P_TOPIC_ARDUINO_UPTIME[] PROGMEM           = "uno/uptime";

const char P_TOPIC_TEMP_F1_LIVING[] PROGMEM         = "uno/temp/f1/living";
const char P_TOPIC_TEMP_F1_LIVING_SET[] PROGMEM   = "uno/temp/f1/living/set";
const char P_TOPIC_RELAY_F1_LIVING_MODE[] PROGMEM   = "uno/f1/living/heating/mode";
const char P_TOPIC_RELAY_F1_LIVING_COMMAND[] PROGMEM   = "uno/rl/1/cmd";
const char P_TOPIC_RELAY_F1_LIVING_STATE[] PROGMEM   = "uno/rl/1/st";

const char P_TOPIC_TEMP_F1_KITCHEN[] PROGMEM        = "uno/temp/f1/kitchen";
const char P_TOPIC_TEMP_F1_KITCHEN_SET[] PROGMEM        = "uno/temp/f1/kitchen/set";
const char P_TOPIC_RELAY_F1_KITCHEN_MODE[] PROGMEM       = "uno/f1/kitchen/heating/mode";
const char P_TOPIC_RELAY_F1_KITCHEN_COMMAND[] PROGMEM         = "uno/rl/2/cmd";
const char P_TOPIC_RELAY_F1_KITCHEN_STATE[] PROGMEM         = "uno/rl/2/st";

const char P_TOPIC_TEMP_F1_BOILER_ROOM[] PROGMEM    = "uno/temp/f1/boiler_room";
const char P_TOPIC_TEMP_F1_BOILER_ROOM_SET[] PROGMEM    = "uno/temp/f1/boilerroom/set";
const char P_TOPIC_RELAY_F1_BOILER_ROOM_MODE[] PROGMEM    = "uno/f1/boilerroom/heating/mode";
const char P_TOPIC_RELAY_F1_BOILER_ROOM_COMMAND[] PROGMEM     = "uno/rl/3/cmd";
const char P_TOPIC_RELAY_F1_BOILER_ROOM_STATE[] PROGMEM     = "uno/rl/3/st";

const char P_TOPIC_TEMP_F1_TOILET[] PROGMEM         = "uno/temp/f1/toilet";
const char P_TOPIC_TEMP_F1_TOILET_SET[] PROGMEM         = "uno/temp/f1/toilet/set";
const char P_TOPIC_RELAY_F1_TOILET_MODE[] PROGMEM         = "uno/f1/toilet/heating/mode";
const char P_TOPIC_RELAY_F1_TOILET_COMMAND[] PROGMEM         = "uno/rl/6/cmd";
const char P_TOPIC_RELAY_F1_TOILET_ROOM_STATE[] PROGMEM     = "uno/rl/6/st";

const char P_TOPIC_TEMP_F2_ROOM1[] PROGMEM          = "uno/temp/f2/room1";
const char P_TOPIC_TEMP_F2_ROOM1_SET[] PROGMEM          = "uno/temp/f2/room1/set";
const char P_TOPIC_RELAY_F2_ROOM1_MODE[] PROGMEM          = "uno/f2/room1/heating/mode";
const char P_TOPIC_RELAY_F2_ROOM1_COMMAND[] PROGMEM           = "uno/rl/4/cmd";
const char P_TOPIC_RELAY_F2_ROOM1_STATE[] PROGMEM           = "uno/rl/4/st";

const char P_TOPIC_TEMP_F2_ROOM2[] PROGMEM          = "uno/temp/f2/room2";
const char P_TOPIC_TEMP_F2_ROOM2_SET[] PROGMEM          = "uno/temp/f2/room2/set";
const char P_TOPIC_RELAY_F2_ROOM2_MODE[] PROGMEM          = "uno/f2/room2/heating/mode";
const char P_TOPIC_RELAY_F2_ROOM2_COMMAND[] PROGMEM           = "uno/rl/5/cmd";
const char P_TOPIC_RELAY_F2_ROOM2_STATE[] PROGMEM           = "uno/rl/5/st";

#define FIRST_START_TIMEOUT 1000
#define READ_TEMP_INTERVAL 10*1000 //30sec
#define TERMOSTAT_DELTA 50 // i.e. 100 = 1 grad
#define HEATING_MAX 5
#define EEPROM_RELAY_STATE__START_ADDR 0
#define EEPROM_RELAY_MODE__START_ADDR 10
#define EEPROM_TEMP_SET__START_ADDR 20
#define MAX_DS18B20_SENSORS 3

byte addr_f1_kitchen[8] = {0x28, 0xFF, 0xB8, 0xBA, 0x73, 0x16, 0x05, 0xC1};
byte addr_f1_living[8] = {0x28, 0xFF, 0x81, 0xE9, 0x74, 0x16, 0x03, 0x41};
byte addr_f1_boilerroom[8] = {0x28, 0xFF, 0x9B, 0xC3, 0x73, 0x16, 0x05, 0x4C};

byte isRestarted = 1;

#define ONE_WIRE_BUS 8
OneWire  ds(ONE_WIRE_BUS);  // on pin 10
DallasTemperature temp_sensors(&ds);


byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
IPAddress mqtt_server(192, 168, 1, 45);

struct RelayData {
  byte pin;
  byte state;
  byte mode;
  byte inverted;
  byte state_eaddr;
  byte mode_eaddr;
  byte floor;
  
};

struct TempData {
  int current;
  int aim;
  byte set_eaddr;
};

//class RoomControl {
//  RelayData relay;
//  TempData temp;
//
//  public: RoomControl(RelayData relay, TempData temp){
//    
//  }
//
//  void Update(){
//    
//  }
//};

//*************** PIN's definition ***************************
RelayData relay01 = { 2, HIGH, 0, 0, 1, EEPROM_RELAY_MODE__START_ADDR, 1};
TempData temp_f1_living = {-12700, 0, EEPROM_TEMP_SET__START_ADDR};

RelayData relay02 = { 3, HIGH, 0, 0, 2, (EEPROM_RELAY_MODE__START_ADDR + 1), 1};
TempData temp_f1_kitchen = {-12700, 0, (EEPROM_TEMP_SET__START_ADDR + 2)};

RelayData relay03 = { 5, HIGH, 0, 0, 3, (EEPROM_RELAY_MODE__START_ADDR + 2), 1};
TempData temp_f1_boilerroom = {-12700, 0, (EEPROM_TEMP_SET__START_ADDR + 4)};

RelayData relay04 = { 6, HIGH, 0, 0, 4, (EEPROM_RELAY_MODE__START_ADDR + 3), 2};
TempData temp_f2_room1 = {-12700, 0, (EEPROM_TEMP_SET__START_ADDR + 8)};

RelayData relay05 = { 7, HIGH, 0, 1, 5, (EEPROM_RELAY_MODE__START_ADDR + 4), 2};
TempData temp_f2_room2 = {-12700, 0, (EEPROM_TEMP_SET__START_ADDR + 10)};


long lastReadingTime = 0;
long lastReadingTime2 = 0;

int8_t HeatingF1_count = 0;
int8_t HeatingF2_count = 0;

EthernetClient ethClient;
PubSubClient mqtt_client(ethClient);

int EEPROM_read_int(int addr) {
  byte lowByte = EEPROM.read(addr);
  byte highByte = EEPROM.read(addr + 1);
  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void EEPROM_write_int(int addr, int val) {
  byte lowByte = ((val >> 0) & 0xFF);
  byte highByte = ((val >> 8) & 0xFF);
  EEPROM.write(addr, lowByte);
  EEPROM.write(addr + 1, highByte);
}

void callback(char* topic, byte* payload, unsigned int length) {
  byte* payload2 = NULL;
  payload[length] = '\0';
  payload2 = (byte*)malloc(length+1);
  if ( NULL == payload2 ) return;
  memcpy(payload2, payload, length+1);
  //WRITE_TO_SERIAL(F("MQTT topic: "), topic, F("\tmessage: "), (char *)payload2);

  //************** FLOOR1 LIVING ROOM ******************************************
  if ( strcmp_P(topic, P_TOPIC_RELAY_F1_LIVING_COMMAND ) == 0) { 
      switchRelay(&relay01, payload2, P_TOPIC_RELAY_F1_LIVING_STATE);
    }
  else if ( strcmp_P(topic, P_TOPIC_TEMP_F1_LIVING_SET) == 0) {
    temp_f1_living.aim = atof( (char *) payload2 ) * 100;
    if ( EEPROM_read_int(temp_f1_living.set_eaddr) != temp_f1_living.aim )
      EEPROM_write_int(temp_f1_living.set_eaddr, temp_f1_living.aim);
  }
  else if ( strcmp_P(topic, P_TOPIC_RELAY_F1_LIVING_MODE) == 0 ) {  setHeatingMode(&relay01, payload2 ); }

  //************** FLOOR1 KITCHEN ROOM ******************************************
  if ( strcmp_P(topic, P_TOPIC_RELAY_F1_KITCHEN_COMMAND ) == 0) { 
       switchRelay(&relay02, payload2, P_TOPIC_RELAY_F1_KITCHEN_STATE);
    }
  else if ( strcmp_P(topic, P_TOPIC_TEMP_F1_KITCHEN_SET) == 0) {
    temp_f1_kitchen.aim = atof( (char *) payload2 ) * 100;
    if ( EEPROM_read_int((int)temp_f1_kitchen.set_eaddr) != temp_f1_kitchen.aim )
      EEPROM_write_int((int)temp_f1_kitchen.set_eaddr, temp_f1_kitchen.aim);
      //WRITE_TO_SERIAL(F("write int to eeprom: "), temp_f1_kitchen.aim,"", "");
  }
  else if ( strcmp_P(topic, P_TOPIC_RELAY_F1_KITCHEN_MODE) == 0 ) {  setHeatingMode(&relay02, payload2 ); }

  //************** FLOOR1 BOILER ROOM ******************************************
  if ( strcmp_P(topic, P_TOPIC_RELAY_F1_BOILER_ROOM_COMMAND ) == 0) { 
       switchRelay(&relay03, payload2, P_TOPIC_RELAY_F1_BOILER_ROOM_STATE);
    }
  else if ( strcmp_P(topic, P_TOPIC_TEMP_F1_BOILER_ROOM_SET) == 0) {
    temp_f1_boilerroom.aim = atof( (char *) payload2 ) * 100;
    if ( EEPROM_read_int((int)temp_f1_boilerroom.set_eaddr) != temp_f1_boilerroom.aim )
      EEPROM_write_int((int)temp_f1_boilerroom.set_eaddr, temp_f1_boilerroom.aim);
      //WRITE_TO_SERIAL(F("write int to eeprom: "), temp_f1_boilerroom.aim,"", "");
  }
  else if ( strcmp_P(topic, P_TOPIC_RELAY_F1_BOILER_ROOM_MODE) == 0 ) {  setHeatingMode(&relay03, payload2 ); }

  //************** FLOOR2 ROOM1 ******************************************
  if ( strcmp_P(topic, P_TOPIC_RELAY_F2_ROOM1_COMMAND ) == 0) { 
       switchRelay(&relay04, payload2, P_TOPIC_RELAY_F2_ROOM1_STATE);
    }
  else if ( strcmp_P(topic, P_TOPIC_TEMP_F2_ROOM1_SET) == 0) {
    temp_f2_room1.aim = atof( (char *) payload2 ) * 100;
    if ( EEPROM_read_int((int)temp_f2_room1.set_eaddr) != temp_f2_room1.aim )
      EEPROM_write_int((int)temp_f2_room1.set_eaddr, temp_f2_room1.aim);
      //WRITE_TO_SERIAL(F("write int to eeprom: "), temp_f2_room1.aim,"", "");
  }
  else if ( strcmp_P(topic, P_TOPIC_RELAY_F2_ROOM1_MODE) == 0 ) {  setHeatingMode(&relay04, payload2 ); }

  //************** FLOOR2 ROOM2 ******************************************
  if ( strcmp_P(topic, P_TOPIC_RELAY_F2_ROOM2_COMMAND ) == 0) { 
       switchRelay(&relay05, payload2, P_TOPIC_RELAY_F2_ROOM2_STATE);
    }
  else if ( strcmp_P(topic, P_TOPIC_TEMP_F2_ROOM2_SET) == 0) {
    temp_f2_room2.aim = atof( (char *) payload2 ) * 100;
    if ( EEPROM_read_int((int)temp_f2_room2.set_eaddr) != temp_f2_room2.aim )
      EEPROM_write_int((int)temp_f2_room2.set_eaddr, temp_f2_room2.aim);
      //WRITE_TO_SERIAL(F("write int to eeprom: "), temp_f2_room2.aim,"", "");
  }
  else if ( strcmp_P(topic, P_TOPIC_RELAY_F2_ROOM2_MODE) == 0 ) {  setHeatingMode(&relay05, payload2 ); }

  
  //else {
  //  Serial.println("FAIL!!!");
  // }
  free(payload2);
}



void relay_state_publish(const char* topic, byte state) {
  byte l = strlen(topic);
  char* buffer = (char*)malloc( l+1 );
  if ( buffer == NULL) return;
  strcpy_P(buffer, (char*)topic);  
  //WRITE_TO_SERIAL(F("relay_state_publish: "), (char*)buffer, F("\tstate: "), state);
  mqtt_client.publish( buffer, (state == LOW) ? "ON" : "OFF");
  free(buffer);
}

void arduino_restart_publish(const char* topic) {
  if (!isRestarted) return;
  byte l = strlen(topic);
  char* buffer = (char*)malloc( l+1 );
  if ( buffer == NULL) return;
  strcpy_P(buffer, (char*)topic);  
  mqtt_client.publish( buffer, "1");
  free(buffer);
  isRestarted = 1;
  //WRITE_TO_SERIAL(F("************************************"), F("ARDUINO RESTARTED"),F("*******************************"), "");
}

void relay_mode_publish(const char* topic, byte state) {
  byte l = strlen(topic);
  char* buffer = (char*)malloc( l+1 );
  if ( buffer == NULL) return;
  //strcpy_P(buffer, (char*)pgm_read_word(&(topic))); 
  strcpy_P(buffer, (char*)topic);   
  //WRITE_TO_SERIAL(F("relay_mode_publish: "), (char*)buffer, F("\tstate: "), state);
  mqtt_client.publish( buffer, (state) ? "1" : "0");
  free(buffer);
}

void temp_set_publish(const char* topic, int val) {
  char *temp = (char*)malloc(7);
  if ( temp == NULL) return;
  sprintf_P(temp, PSTR("%d.%02d"), (int)val / 100, (int)(val) % 100);
  byte l = strlen(topic);
  char* buffer = (char*)malloc( l+1 );
  if ( buffer == NULL) return;
  strcpy_P(buffer, (char*)topic);   
  //WRITE_TO_SERIAL(F("temp_set_publish: "), (char*)buffer, F("\ttemp: "), (char *)temp);
  mqtt_client.publish( buffer, temp);
  free(buffer);
  free(temp);
}

void dsw_temp_publish_by_topic(const char* topic, int value) {
  if ( value < -10000 || value > 8500 ) return;
  char *temp = (char*)malloc(7);
  if ( temp == NULL) return;
  sprintf_P(temp, PSTR("%d.%02d"), (int)value / 100, (int)(value) % 100);
  byte l = strlen(topic);
  char* buffer = (char*)malloc( l+1 );
  if ( buffer == NULL) return;
  strcpy_P(buffer, (char*)topic);

   //WRITE_TO_SERIAL(F("dsw_temp_publish_by_topic: "), (char*)buffer, F("\ttemp: "), (char *)temp);
  mqtt_client.publish( buffer, temp);
  free(temp);
  free(buffer);
}

void uptime(const char* topic, long val) {
  byte l = strlen(topic);
  char* buffer = (char*)malloc( l+1 );
  char* sval =  (char*)malloc( 10 );
  if ( buffer == NULL) return;
  if ( sval == NULL) { free(buffer); return; }
  ltoa(val, sval, 10);
  strcpy_P(buffer, (char*)topic);   
  mqtt_client.publish( buffer, sval);
  free(buffer);
  free(sval);
}

void setHeatingMode(RelayData *relay, byte* payload) {
  relay->mode =  atoi( (char *) payload );
  if ( EEPROM.read(relay->mode_eaddr) != relay->mode) EEPROM.write(relay->mode_eaddr, relay->mode);
}

void switchRelay(RelayData *relay, byte* payload, const char* topic) {
  char* cstring = (char*)payload;
  byte r = (strcmp( cstring, "OFF") == 0) ? HIGH : LOW;
  if ( relay->state == r   ) {
    return;
  }

  relay->state = ( strcmp( payload, "OFF") == 0) ? HIGH : LOW;
  if ( EEPROM.read(relay->state_eaddr) != relay->state) EEPROM.write(relay->state_eaddr, relay->state);
  digitalWrite(relay->pin, relay->inverted ? !relay->state : relay->state);
  
  relay_state_publish( topic,       relay->state);
  if (relay->state == HIGH) {
    switch (relay->floor) {
      case 1: HeatingF1_count--; break;
      case 2: HeatingF2_count--; break;
    }
  } else {
    switch (relay->floor) {
      case 1: HeatingF1_count++; break;
      case 2: HeatingF2_count++; break;
    }
  }
  if (HeatingF1_count < 0) HeatingF1_count = 0;
  if (HeatingF1_count > HEATING_MAX) HeatingF1_count = HEATING_MAX;

  if (HeatingF2_count < 0) HeatingF2_count = 0;
  if (HeatingF2_count > HEATING_MAX) HeatingF2_count = HEATING_MAX;
  
  delay(50);
}


void initialize_relay(RelayData* relay){
  pinMode(relay->pin, OUTPUT);
  relay->state = EEPROM.read(relay->state_eaddr);
  if (relay->state == LOW) {
      switch (relay->floor) {
        case 1: HeatingF1_count++; break;
        case 2: HeatingF2_count++; break;
      }
  }
  digitalWrite(relay->pin, relay->inverted ? !relay->state : relay->state);
  relay->mode = EEPROM.read(relay->mode_eaddr);
  delay(FIRST_START_TIMEOUT);  
}

void publish_all_data(){
    relay_state_publish( P_TOPIC_RELAY_F1_LIVING_STATE,       relay01.state);
    delay(50);
    temp_set_publish( P_TOPIC_TEMP_F1_LIVING_SET, temp_f1_living.aim);
    delay(50);
    relay_mode_publish( P_TOPIC_RELAY_F1_LIVING_MODE,     relay01.mode);
    delay(50);

    relay_state_publish( P_TOPIC_RELAY_F1_KITCHEN_STATE,       relay02.state);
    delay(50);
    temp_set_publish( P_TOPIC_TEMP_F1_KITCHEN_SET, temp_f1_kitchen.aim);
    delay(50);
    relay_mode_publish( P_TOPIC_RELAY_F1_KITCHEN_MODE,     relay02.mode);
    delay(50);

    relay_state_publish( P_TOPIC_RELAY_F1_BOILER_ROOM_STATE,       relay03.state);
    delay(50);
    temp_set_publish( P_TOPIC_TEMP_F1_BOILER_ROOM_SET, temp_f1_boilerroom.aim);
    delay(50);
    relay_mode_publish( P_TOPIC_RELAY_F1_BOILER_ROOM_MODE,     relay03.mode);    
    delay(50);
    
    relay_state_publish( P_TOPIC_RELAY_F2_ROOM1_STATE,       relay04.state);
    delay(50);
    temp_set_publish( P_TOPIC_TEMP_F2_ROOM1_SET, temp_f2_room1.aim);
    delay(50);
    relay_mode_publish( P_TOPIC_RELAY_F2_ROOM1_MODE,     relay04.mode);        
    delay(50);
    
    relay_state_publish( P_TOPIC_RELAY_F2_ROOM2_STATE,       relay05.state);
    delay(50);
    temp_set_publish( P_TOPIC_TEMP_F2_ROOM2_SET, temp_f2_room2.aim);
    delay(50);
    relay_mode_publish( P_TOPIC_RELAY_F2_ROOM2_MODE,     relay05.mode);            
    delay(50);
}

void setup() {
  ////  MCUSR = 0;
  //wdt_disable();
  // Open serial communications and wait for port to open:
  //Serial.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
 // }
  //wdt_enable(WDTO_4S);
  delay(1500);

  initialize_relay(&relay01);
  initialize_relay(&relay02);
  initialize_relay(&relay03);
  initialize_relay(&relay04);
  initialize_relay(&relay05);
  
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  temp_sensors.begin();
  delay(50);


  temp_f1_living.aim = EEPROM_read_int(temp_f1_living.set_eaddr);
  temp_f1_kitchen.aim = EEPROM_read_int(temp_f1_kitchen.set_eaddr);
  temp_f1_boilerroom.aim = EEPROM_read_int(temp_f1_boilerroom.set_eaddr);
  temp_f2_room1.aim = EEPROM_read_int(temp_f2_room1.set_eaddr);
  temp_f2_room2.aim = EEPROM_read_int(temp_f2_room2.set_eaddr);
 
  if (mqtt_client.connected()) {
    publish_all_data();
    arduino_restart_publish(P_TOPIC_ARDUINO_RESTART);
  }

   //WRITE_TO_SERIAL(F("\n\n********************"), F("START LOOP"), F("\n************* "), "");
}


void updateTemperatures() {

    temp_sensors.requestTemperatures();

    float v;
    v = temp_sensors.getTempC(addr_f1_living);
    temp_f1_living.current = (int)(v * 100);
    temp_f2_room1.current = temp_f1_living.current;
    dsw_temp_publish_by_topic( P_TOPIC_TEMP_F1_LIVING,        temp_f1_living.current);
    dsw_temp_publish_by_topic( P_TOPIC_TEMP_F2_ROOM1,        temp_f2_room1.current);
 
    v = temp_sensors.getTempC(addr_f1_kitchen);
    temp_f1_kitchen.current = (int)(v * 100);
    temp_f2_room2.current = temp_f1_kitchen.current;
    dsw_temp_publish_by_topic( P_TOPIC_TEMP_F1_KITCHEN,        temp_f1_kitchen.current);
    dsw_temp_publish_by_topic( P_TOPIC_TEMP_F2_ROOM2,        temp_f2_room2.current);

    v = temp_sensors.getTempC(addr_f1_boilerroom);
    temp_f1_boilerroom.current = (int)(v * 100);
    dsw_temp_publish_by_topic( P_TOPIC_TEMP_F1_BOILER_ROOM,        temp_f1_boilerroom.current);
        
  
}

void termostat() {
  //WRITE_TO_SERIAL(F("--- TERMOSTATE START ------"), "", "", "");
  //char *t = new char[10];
  //memset(t, '\0', 10);
  //strcpy_P(t, P_F1_LIVING);
  if (relay01.mode) {
    if ( (temp_f1_living.current  > -10000) && (temp_f1_living.current < (temp_f1_living.aim - TERMOSTAT_DELTA)))  {
      //heating_turn_on_off( t, "ON"); //ON - включить конвекторы в зале
      switchRelay(&relay01, "ON", P_TOPIC_RELAY_F1_LIVING_STATE);
    } else if ( temp_f1_living.current  > (temp_f1_living.aim + TERMOSTAT_DELTA)) {
      //heating_turn_on_off( t, "OFF"); //OFF - выключить конвекторы в зале
      switchRelay(&relay01, "OFF", P_TOPIC_RELAY_F1_LIVING_STATE);
    }
    delay(50);
  }
  

  if (relay02.mode) {
    if ( (temp_f1_kitchen.current  > -10000) && (temp_f1_kitchen.current < (temp_f1_kitchen.aim - TERMOSTAT_DELTA)))  {
      //heating_turn_on_off( t, "ON"); //ON - включить конвекторы в зале
      switchRelay(&relay02, "ON", P_TOPIC_RELAY_F1_KITCHEN_STATE);
    } else if ( temp_f1_kitchen.current  > (temp_f1_kitchen.aim + TERMOSTAT_DELTA)) {
      //heating_turn_on_off( t, "OFF"); //OFF - выключить конвекторы в зале
      switchRelay(&relay02, "OFF", P_TOPIC_RELAY_F1_KITCHEN_STATE);
    }
    delay(50);
  }
  
  if (relay03.mode) {
    if ( (temp_f1_boilerroom.current  > -10000) && (temp_f1_boilerroom.current < (temp_f1_boilerroom.aim - TERMOSTAT_DELTA)))  {
      //heating_turn_on_off( t, "ON"); //ON - включить конвекторы в зале
      switchRelay(&relay03, "ON", P_TOPIC_RELAY_F1_BOILER_ROOM_STATE);
    } else if ( temp_f1_boilerroom.current  > (temp_f1_boilerroom.aim + TERMOSTAT_DELTA)) {
      //heating_turn_on_off( t, "OFF"); //OFF - выключить конвекторы в зале
      switchRelay(&relay03, "OFF", P_TOPIC_RELAY_F1_BOILER_ROOM_STATE);
    }
    delay(50);
  }

  if ( (HeatingF1_count >= 4) || 
       ( (HeatingF1_count == 3) && (HeatingF2_count >=2) )
      ) 
  {     // включены все конвекторы на 1-м этаже     // выключить конвекторы на 2-м
        if (relay04.mode) switchRelay(&relay04, "OFF", P_TOPIC_RELAY_F2_ROOM1_STATE);
        if (relay05.mode) switchRelay(&relay05, "OFF", P_TOPIC_RELAY_F2_ROOM2_STATE);
  } else {
    // какой-то из конвекторов на этаже 1 не включен, можно включать 2-ой этаж
    // TODO - смотреть общее потребление тока
    // конвектор в этаж2.комната1 можно включить только когда на этаже 1 включено 2 конвектора + этаж2.комната2
    //или на этаже1 включено 3 конвектора и не включен конвектор этаж2.комната2

      
    if (   //3 любых на 1-м и 1 на втором
      (HeatingF1_count < 3) ||      // если включено менее 3-х на первом и (возможно) этаж2.комната2, то можно включить этаж2.комната1
      ( (HeatingF1_count == 3) && (relay05.state == HIGH) ) // если включено 3 любых на первом и не включен этаж2.комната2
      ) 
        {
          if (relay04.mode) {
            if ( (temp_f2_room1.current  > -10000) && (temp_f2_room1.current < (temp_f2_room1.aim - TERMOSTAT_DELTA)))  {
              //heating_turn_on_off( t, "ON"); //ON - включить конвекторы в этаж2.комната1
              switchRelay(&relay04, "ON", P_TOPIC_RELAY_F2_ROOM1_STATE);
            } else if (temp_f2_room1.current   > (temp_f2_room1.aim + TERMOSTAT_DELTA)) {
              //heating_turn_on_off( t, "OFF");  //OFF - выключить конвекторы в этаж2.комната1
              switchRelay(&relay04, "OFF", P_TOPIC_RELAY_F2_ROOM1_STATE); 
            }
          }
        }

        // конвектор в этаж2.комната2 можно включить только когда на этаже 1 включено 2 конвектора + этаж2.комната1
        //или на этаже1 включено 3 конвектора и не включен конвектор этаж2.комната1
    if (   //3 любых на 1-м и 1 на втором
      (HeatingF1_count < 3) ||      // если включено менее 3-х на первом и (возможно) этаж2.комната1, то можно включить этаж1.комната2
      ( (HeatingF1_count == 3) && (relay04.state == HIGH) ) // если включено 3 любых на первом и не включен этаж2.комната1
    ) 
        {
          if (relay05.mode) {
            if ( (temp_f2_room2.current  > -10000) && (temp_f2_room2.current < (temp_f2_room2.aim - TERMOSTAT_DELTA)))  {
              switchRelay(&relay05, "ON", P_TOPIC_RELAY_F2_ROOM2_STATE); 
            } else if (temp_f2_room2.current   > (temp_f2_room2.aim + TERMOSTAT_DELTA)) {
              //heating_turn_on_off( t, "OFF"); //OFF - выключить конвекторы в этаж2.комната2
              switchRelay(&relay05, "OFF", P_TOPIC_RELAY_F2_ROOM2_STATE); 
            }
          }
        }            
  }
        
  //delete [] t;
  //WRITE_TO_SERIAL(F("--- TERMOSTATE END ------"), "", "", "");
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    if (mqtt_client.connect("uno")) {
      mqtt_client.subscribe( "uno/#" );
      publish_all_data();
      arduino_restart_publish(P_TOPIC_ARDUINO_RESTART);
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(mqtt_client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  //wdt_reset();
  if (millis() - lastReadingTime > 1000) {
    lastReadingTime = millis();
    //Serial.println( lastReadingTime );
    //if (mqtt_client.connected()) {    }
    //WRITE_TO_SERIAL(F("HeatingF1_count: "), HeatingF1_count, F("\tHeatingF2_count: "), HeatingF2_count);
    
  }

  if (millis() - lastReadingTime2 > READ_TEMP_INTERVAL) {
    lastReadingTime2 = millis();
    //Serial.println( lastReadingTime2 / 1000 );
    updateTemperatures(); 
    delay(50);
    termostat();
    delay(50);
    publish_all_data();
    delay(50);
    uptime(P_TOPIC_ARDUINO_UPTIME, lastReadingTime2);
    delay(50);
  }
  
  if (!mqtt_client.connected()) {
    reconnect();
  }

  mqtt_client.loop();

  // listen for incoming clients
  //listenForEthernetClients();

}

