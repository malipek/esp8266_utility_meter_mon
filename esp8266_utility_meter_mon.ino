// uncomment fot serial debug
// #define DEBUG

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WIFI setup
#define WLAN_SSID       "secret"
#define WLAN_PASS       "evenmoresecret"

// MQTT setup
#define TOPIC           "power_consumed"
#define MQTT_SERVER      "redactedlocalip"
#define MQTT_SERVERPORT  1884 // default 1883 for unencrypted MQTT


/*  number of pulses per 1 kWh
    check utility meter for correct value */
#define IMP_KWH 1000

/*  time in seconds for reporting measures
    the lower value, the worse the accuracy
    accuracy = 1kW * 3600/(IMP_KWH-1)/REPORT_PERIOD
    for 1000imp/kWh and reporting every 30s it gives
    the accuracy of 120W */
#define REPORT_PERIOD 30

const int pulse_sensor = D6; // pin for connected data PIN of pulse sensor
const float pulse_value = 3600000.0 / float(IMP_KWH); // kW * 1000 miliseconds
float power = 0.0;
unsigned long now = millis();
unsigned long last = 0;
unsigned int period = 0;
unsigned short counter = 0; //counter for pulses, watch out for type max values!
unsigned short counter_acc = 0; //counter for pulses, watch out for type max values!
bool published = false;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT);

// anonymous access, change for authentication
Adafruit_MQTT_Publish power_consumed = Adafruit_MQTT_Publish(&mqtt,TOPIC);


void WIFI_connect() {
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  #ifdef DEBUG
    Serial.print(F("Connecting to WiFi... "));
  #endif
  uint8_t retries = 5;
  while (WiFi.status() != WL_CONNECTED) {
       #ifdef DEBUG
        Serial.println(F("Retrying WIFI connection in 5 seconds..."));
       #endif
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for watchdog
         while (1);
       }
  }
#ifdef DEBUG  
  Serial.println(F("WIFI Connected!"));
#endif
}


void MQTT_connect() {
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  #ifdef DEBUG
    Serial.print(F("Connecting to MQTT... "));
  #endif

  uint8_t retries = 1;
  while (mqtt.connect() != 0) { // connect will return 0 for connected
    #ifdef DEBUG
      Serial.println(F("Retrying MQTT connection in 5 seconds..."));
    #endif
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // wait for WatchDog
      while (1);
    }
  }
  #ifdef DEBUG
    Serial.println("MQTT Connected!");
  #endif
}

ICACHE_RAM_ATTR void count_pulses(){
  delay(10); //dirty hack for slow changing signals
  if (digitalRead(pulse_sensor)==LOW){
    //even more dirty hack for slow changing signals
    counter++;
  }
}
  
void setup(){
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  WIFI_connect();
  pinMode(pulse_sensor, INPUT); //LM939 output is OC/GND, external pullup is used
  attachInterrupt(digitalPinToInterrupt(pulse_sensor), count_pulses, FALLING);
  // dirty hack is needed, check https://github.com/espressif/arduino-esp32/issues/4172
}

void loop(){
  now = millis();
  period = (unsigned int)(now - last);
  if (period > REPORT_PERIOD * 1000){
    // perform readings and reset counters
    last = now;
    counter_acc = counter;
    counter = 0;
    power = counter_acc * pulse_value / period;
    #ifdef DEBUG
      Serial.print(F("C:"));
      Serial.print(counter_acc);
      Serial.print(F(",P:"));
      Serial.print(period);
      Serial.print(F(", PulseV:"));
      Serial.print(pulse_value);
      Serial.print(F(", Per:"));
      Serial.print(period);
      Serial.print(F(", Power:"));
      Serial.print(power);
      Serial.println(F("kW"));
    #endif
    MQTT_connect();
    published = power_consumed.publish(power);
    #ifdef DEBUG
      if (!published ) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
    #endif
  }
}