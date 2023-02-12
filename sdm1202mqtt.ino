// Eastron SDM120 1 phase power meter to MQTT
// Repo: https://github.com/nygma2004/sdm1202mqtt
// author: Csongor Varga, csongor.varga@gmail.com

// Libraries:
// - FastLED by Daniel Garcia
// - ModbusMaster by Doc Walker
// - ArduinoOTA
// - SoftwareSerial
// Hardware:
// - Wemos D1 mini
// - RS485 to TTL converter: https://www.aliexpress.com/item/1005001621798947.html
// - To power from mains: Hi-Link 5V power supply (https://www.aliexpress.com/item/1005001484531375.html), fuseholder and 1A fuse, and varistor
// - Small DC-DC Buck converter: https://www.aliexpress.com/item/1005003512429148.html


#include <SoftwareSerial.h>       // Leave the main serial line (USB) for debugging and flashing
#include <ModbusMaster.h>         // Modbus master library for ESP8266
#include <ESP8266WiFi.h>          // Wifi connection
#include <ESP8266WebServer.h>     // Web server for general HTTP response
#include <PubSubClient.h>         // MQTT support
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include "webpage.h" //Our HTML webpage contents with javascripts
//#include "globals.h"
#include "settings.h"

#include <math.h>
#define BINARY32_MASK_SIGN 0x80000000
#define BINARY32_MASK_EXPO 0x7FE00000
#define BINARY32_MASK_SNCD 0x007FFFFF
#define BINARY32_IMPLIED_BIT 0x800000
#define BINARY32_SHIFT_EXPO 23

os_timer_t myTimer;
ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient mqtt(mqtt_server, 1883, 0, espClient);
// SoftwareSerial modbus(MAX485_RX, MAX485_TX, false, 256); //RX, TX
SoftwareSerial modbus(MAX485_RX, MAX485_TX, false); //RX, TX
ModbusMaster sdm120;
CRGB leds[NUM_LEDS];

void preTransmission() {
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission() {
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void sendModbusError(uint8_t result) {
  String message = "";
  if (result==sdm120.ku8MBIllegalFunction) {
    message = "Illegal function";
  }
  if (result==sdm120.ku8MBIllegalDataAddress) {
    message = "Illegal data address";
  }
  if (result==sdm120.ku8MBIllegalDataValue) {
    message = "Illegal data value";
  }
  if (result==sdm120.ku8MBSlaveDeviceFailure) {
    message = "Slave device failure";
  }
  if (result==sdm120.ku8MBInvalidSlaveID) {
    message = "Invalid slave ID";
  }
  if (result==sdm120.ku8MBInvalidFunction) {
    message = "Invalid function";
  }
  if (result==sdm120.ku8MBResponseTimedOut) {
    message = "Response timed out";
  }
  if (result==sdm120.ku8MBInvalidCRC) {
    message = "Invalid CRC";
  }
  if (message=="") {
    message = result;
  }
  Serial.println(message);   
  strcpy(modbusstatus, message.c_str()); 
  char topic[80];
  char value[30];
  sprintf(topic,"%s/error",topicRoot);
  mqtt.publish(topic, message.c_str());
  delay(5);
}

float binary32_to_float(uint32_t x) {
  // Break up into 3 parts
  bool sign = x & BINARY32_MASK_SIGN;
  int biased_expo = (x & BINARY32_MASK_EXPO) >> BINARY32_SHIFT_EXPO;
  int32_t significand = x & BINARY32_MASK_SNCD;

  float y;
  if (biased_expo == 0xFF) {
    y = significand ? NAN : INFINITY;  // For simplicity, NaN payload not copied
  } else {
    int expo;
    if (biased_expo > 0) {
      significand |= BINARY32_IMPLIED_BIT;
      expo = biased_expo - 127;
    } else {
      expo = 126;
    }
    y = ldexpf((float)significand, expo - BINARY32_SHIFT_EXPO);
  }
  if (sign) {
    y = -y;
  }
  return y;
}    

void ReadInputRegisters() {

  char value[10]; 

  leds[0] = CRGB::Yellow;
  FastLED.show();
  uint8_t result;

  #ifdef BUILTINLED_FOR_MODBUS
    digitalWrite(STATUS_LED, 0);
  #endif

  ESP.wdtDisable();
  switch (readcycle) {
    case 0:
      result = sdm120.readInputRegisters(0,2); // voltage
      break;
    case 1:
      result = sdm120.readInputRegisters(6,2); // current
      break;
    case 2:
      result = sdm120.readInputRegisters(12,2); // active power
      break;
    case 3:
      result = sdm120.readInputRegisters(30,2); // power factor
      break;
    case 4:
      result = sdm120.readInputRegisters(70,2); // frequency
      break;
    case 5:
      result = sdm120.readInputRegisters(342,2); // total active energy
      break;
  }
  ESP.wdtEnable(1);

  if (result == sdm120.ku8MBSuccess)   {

    leds[0] = CRGB::Green;
    FastLED.show();
    lastRGB = millis();
    ledoff = true;

    // save response in global strucutre
    switch (readcycle) {
      case 0:
        modbusdata.voltage = binary32_to_float(((sdm120.getResponseBuffer(0) << 16) | sdm120.getResponseBuffer(1))); // voltage
        break;
      case 1:
        modbusdata.current = binary32_to_float(((sdm120.getResponseBuffer(0) << 16) | sdm120.getResponseBuffer(1))); // current
        break;
      case 2:
        modbusdata.active_power = binary32_to_float(((sdm120.getResponseBuffer(0) << 16) | sdm120.getResponseBuffer(1))); // active power
        break;
      case 3:
        modbusdata.power_factor = binary32_to_float(((sdm120.getResponseBuffer(0) << 16) | sdm120.getResponseBuffer(1))); // power factor
        break;
      case 4:
        modbusdata.frequency = binary32_to_float(((sdm120.getResponseBuffer(0) << 16) | sdm120.getResponseBuffer(1))); // frequency
        break;
      case 5:
        modbusdata.total_active_energy = binary32_to_float(((sdm120.getResponseBuffer(0) << 16) | sdm120.getResponseBuffer(1))); // total active energy
        break;
    }

      //Serial.print(readcycle);
      //Serial.println(" read.");

    // Advance the cycle counter or update MQTT
    if (readcycle<5) {
      // cycle to the next read
      readcycle++;
    } else {
        sprintf(json,"{",json);
        sprintf(json,"%s \"voltage\":%.1f,",json,modbusdata.voltage);
        sprintf(json,"%s \"current\":%.2f,",json,modbusdata.current);
        sprintf(json,"%s \"activepower\":%.1f,",json,modbusdata.active_power);
        sprintf(json,"%s \"powerfactor\":%.2f,",json,modbusdata.power_factor);
        sprintf(json,"%s \"frequency\":%.2f,",json,modbusdata.frequency);
        sprintf(json,"%s \"totalactiveenergy\":%.2f }",json,modbusdata.total_active_energy);
        validdata = true;
        strcpy(modbusstatus,"OK");

        // Reset to the first read
        readcycle=0;
      
    }
    

  } else {
    leds[0] = CRGB::Red;
    FastLED.show();
    lastRGB = millis();
    ledoff = true;

    Serial.print(F("Error: "));
    sendModbusError(result);
    validdata = false;
  }
  #ifdef BUILTINLED_FOR_MODBUS
    digitalWrite(STATUS_LED, 1);
  #endif

   
}


void SendDataMQTT() {
  char topic[80];
  if(validdata) {
    sprintf(topic,"%s/data",topicRoot);
    digitalWrite(STATUS_LED, 0);
    mqtt.publish(topic,json);      
    Serial.println("Data MQTT sent");
    digitalWrite(STATUS_LED, 1);
  }
}


// This is the 1 second timer callback function
void timerCallback(void *pArg) {
  seconds++;
  // Query the modbus device 
  if (seconds % UPDATE_MODBUS==0) {
    SendDataMQTT();
  }

  // Send RSSI and uptime status
  if (seconds % UPDATE_STATUS==0) {
    SendStatusUpdate();
  }
}

// Generate and send the status MQTT message
void SendStatusUpdate() {
  char topic[80];
  sprintf(systemstatus,"{\"rssi\": %d, \"uptime\": %d, \"ssid\": \"%s\", \"ip\": \"%d.%d.%d.%d\", \"clientid\":\"%s\", \"version\":\"%s\"}",WiFi.RSSI(),uptime,WiFi.SSID().c_str(),WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3],newclientid,buildversion);
  sprintf(topic,"%s/%s",topicRoot,"status");
  // Send MQTT update
  if (mqtt_server!="") {
    digitalWrite(STATUS_LED, 0);
    mqtt.publish(topic, systemstatus);
    Serial.println(F("MQTT status sent"));
    digitalWrite(STATUS_LED, 1);
  }  
}


// HTTP status URL request
void handleStatusRequest() {
  char value[2048];
  sprintf(value,"{ \"modbus\": \"%s\" ",modbusstatus);
  if (validdata) {
    sprintf(value,"%s,\"data\": %s ",value,json);
  }
  sprintf(value,"%s, \"status\": %s}",value,systemstatus);

  Serial.println("Sending status json");
  server.send(200, "application/json", value);  
}

// MQTT reconnect logic
void reconnect() {
  //String mytopic;
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    byte mac[6];                     // the MAC address of your Wifi shield
    WiFi.macAddress(mac);
    sprintf(newclientid,"%s-%02x%02x%02x",clientID,mac[2],mac[1],mac[0]);
    Serial.print(F("Client ID: "));
    Serial.println(newclientid);
    // Attempt to connect
    if (mqtt.connect(newclientid, mqtt_user, mqtt_password)) {
      Serial.println(F("connected"));
      // ... and resubscribe
      char topic[80];
      sprintf(topic,"%swrite/#",topicRoot);
      mqtt.subscribe(topic);
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(mqtt.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {

  FastLED.addLeds<LED_TYPE, RGBLED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( BRIGHTNESS );
  leds[0] = CRGB::Pink;
  FastLED.show();

  Serial.begin(SERIAL_RATE);
  Serial.println(F("\nsdm120 Solar Inverter to MQTT Gateway"));
  // Init outputs, RS485 in receive mode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  // Initialize some variables
  uptime = 0;
  seconds = 0;
  leds[0] = CRGB::Pink;
  FastLED.show();

  // Connect to Wifi
  Serial.print(F("Connecting to Wifi"));
  WiFi.mode(WIFI_STA);

  #ifdef FIXEDIP
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  #endif
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
    seconds++;
    if (seconds>180) {
      // reboot the ESP if cannot connect to wifi
      ESP.restart();
    }
  }
  seconds = 0;
  Serial.println("");
  Serial.println(F("Connected to wifi network"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Signal [RSSI]: "));
  Serial.println(WiFi.RSSI());

  // Set up the Modbus line
  sdm120.begin(SLAVE_ID , modbus);
  // Callbacks allow us to configure the RS485 transceiver correctly
  sdm120.preTransmission(preTransmission);
  sdm120.postTransmission(postTransmission);
  Serial.println("Modbus connection is set up");

  // Create the 1 second timer interrupt
  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, 1000, true);

  server.on("/", [](){                        // Dummy page
     String s = webPage; //Read HTML contents
     server.send(200, "text/html", s); //Send web page
  });
  server.on("/status", handleStatusRequest);
  server.begin();
  Serial.println(F("HTTP server started"));

  // Set up the MQTT server connection
  if (mqtt_server!="") {
    mqtt.setServer(mqtt_server, 1883);
    mqtt.setBufferSize(1024);
    mqtt.setCallback(callback);
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  byte mac[6];                     // the MAC address of your Wifi shield
  WiFi.macAddress(mac);
  char value[80];
  sprintf(value,"%s-%02x%02x%02x",clientID,mac[2],mac[1],mac[0]);
  ArduinoOTA.setHostname(value);

  // No authentication by default
  ArduinoOTA.setPassword((const char *)"esp6161");

  ArduinoOTA.onStart([]() {
    os_timer_disarm(&myTimer);
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    os_timer_arm(&myTimer, 1000, true);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  modbus.begin(MODBUS_RATE);
  
  leds[0] = CRGB::Black;
  FastLED.show();
  digitalWrite(STATUS_LED, 1);
  validdata = false;
  SendStatusUpdate();
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Convert the incoming byte array to a string
  String mytopic = (char*)topic;
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;

  Serial.print(F("Message arrived on topic: ["));
  Serial.print(topic);
  Serial.print(F("], "));
  Serial.println(message);


}

void loop() {
  // Handle HTTP server requests
  server.handleClient();
  ArduinoOTA.handle();

  // Handle MQTT connection/reconnection
  if (mqtt_server!="") {
    if (!mqtt.connected()) {
      reconnect();
    }
    mqtt.loop();
  }

  // Cycle modbus read
  if (millis() - lastmodbus >= MODBUS_READ_DELAY) {            
    ReadInputRegisters();   
    lastmodbus=millis();      
  }   

  // Uptime calculation
  if (millis() - lastTick >= 60000) {            
    lastTick = millis();            
    uptime++;            
  }    

 

  if (millis() - lastWifiCheck >= WIFICHECK) {
    // reconnect to the wifi network if connection is lost
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to wifi...");
      WiFi.reconnect();
    }
    lastWifiCheck = millis();
  }

  if (ledoff && (millis() - lastRGB >= RGBSTATUSDELAY)) {
    ledoff = false;
    leds[0] = CRGB::Black;
    FastLED.show();
  }


}
