#include <SoftwareSerial.h>
#include <PZEM004T.h>
#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include<stdlib.h>
#define OUT_PIN 13
char mqtt_server[40] = "pi1.lan";
char mqtt_port[6] = "1883";
char mqtt_inTopic[40] = "energy/edp/switch";
char mqtt_outTopic[40] = "energy/edp/measures";
char mqtt_deviceName[40] = "EDPEnergyMeter";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
PZEM004T pzem(4,5);  // RX,TX
IPAddress ip(192,168,1,1);
AsyncMqttClient mqttClient;
void setupArduinoOTA();
void onMqttConnect() {
  uint16_t packetIdSub = mqttClient.subscribe(mqtt_inTopic, 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("** Disconnected from the broker **");
  Serial.println("Reconnecting to MQTT...");
  mqttClient.connect();
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("** Subscribe acknowledged **");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("** Unsubscribe acknowledged **");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("** Publish received **");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  Serial.println(payload);
  if(strcmp(payload,"off")==0) {
    Serial.print("turn OFF: ");
    digitalWrite(OUT_PIN, 0);
  }
  else if(strcmp(payload,"on")==0) {
    Serial.print("turn ON: ");
    digitalWrite(OUT_PIN, 1);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("** Publish acknowledged **");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  pinMode(OUT_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
   Serial.println("mounted file system");
   if (SPIFFS.exists("/config.json")) {
     File configFile = SPIFFS.open("/config.json", "r");
     if (configFile) {
       Serial.println("opened config file");
       size_t size = configFile.size();
       std::unique_ptr<char[]> buf(new char[size]);
       configFile.readBytes(buf.get(), size);
       DynamicJsonBuffer jsonBuffer;
       JsonObject& json = jsonBuffer.parseObject(buf.get());
       json.printTo(Serial);
       if (json.success()) {
         Serial.println("\nparsed json");
         strcpy(mqtt_server, json["mqtt_server"]);
         strcpy(mqtt_port, json["mqtt_port"]);
         strcpy(mqtt_inTopic, json["mqtt_inTopic"]);
         strcpy(mqtt_outTopic, json["mqtt_outTopic"]);
         strcpy(mqtt_deviceName, json["mqtt_deviceName"]);

       } else {
         Serial.println("failed to load json config");
       }
     }
   }
 } else {
   Serial.println("failed to mount FS");
  }
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);
  WiFiManagerParameter custom_mqtt_inTopic("inTopic", "mqtt input topic", mqtt_inTopic, 40);
  WiFiManagerParameter custom_mqtt_outTopic("inTopic", "mqtt input topic", mqtt_outTopic, 40);
  WiFiManagerParameter custom_mqtt_deviceName("deviceName", "mqtt device name", mqtt_deviceName, 40);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_inTopic);
  wifiManager.addParameter(&custom_mqtt_outTopic);
  wifiManager.addParameter(&custom_mqtt_deviceName);
  if(!wifiManager.autoConnect("EnergyMeterConfig")) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_inTopic, custom_mqtt_inTopic.getValue());
  strcpy(mqtt_outTopic, custom_mqtt_outTopic.getValue());
  strcpy(mqtt_deviceName, custom_mqtt_deviceName.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_inTopic"] = mqtt_inTopic;
    json["mqtt_outTopic"] = mqtt_outTopic;
    json["mqtt_deviceName"] = mqtt_deviceName;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println(" OK");

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(mqtt_server, atoi(mqtt_port));
  mqttClient.setKeepAlive(5).setWill("topic/online", 2, true, "no").setClientId(mqtt_deviceName);
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
  pzem.setAddress(ip);
  setupArduinoOTA();
  ArduinoOTA.begin();
}
unsigned long lastTime = 0;
float voltage;
float current;
float power;
float energy;
int op = 0;
char energyS[20];
char currentS[20];
char voltageS[20];
char powerS[20];

void loop() {
  ArduinoOTA.handle();
  if(millis() - lastTime >= 500) {
    if(op == 0) {
      voltage = pzem.voltage(ip);
      if (voltage < 0.0) voltage = 0.0;
      Serial.print(voltage);Serial.print("V; ");
      ++op;
      dtostrf(voltage, 0, 1, voltageS);
      char result[100];   // array to hold the result.
      char temp[] = "/voltage";
      strcpy(result,mqtt_outTopic); // copy string one into the result.
      strcat(result,temp); // append string two to the result.
      mqttClient.publish(result, 2, true, voltageS);
    }
    else if(op == 1) {
      current = pzem.current(ip);
      if(current >= 0.0){
        Serial.print(current);Serial.print("A; "); }
      ++op;
      dtostrf(current, 0, 1, currentS);
      char result[100];   // array to hold the result.
      char temp[] = "/current";
      strcpy(result,mqtt_outTopic); // copy string one into the result.
      strcat(result,temp); // append string two to the result.
      mqttClient.publish(result, 2, true, currentS);    }
    else if(op == 2) {
      power = pzem.power(ip);
      if(power >= 0.0){ Serial.print(power);Serial.print("W; "); }
      ++op;
      dtostrf(power, 0, 1, powerS);
      char result[100];   // array to hold the result.
      char temp[] = "/power";
      strcpy(result,mqtt_outTopic); // copy string one into the result.
      strcat(result,temp); // append string two to the result.
      mqttClient.publish(result, 2, true, powerS);    }
    else if(op == 3) {
      energy = pzem.energy(ip);
      if(energy >= 0.0){ Serial.print(energy);Serial.print("Wh; "); }
      op = 0;
      dtostrf(energy, 0, 1, energyS);
      char result[100];   // array to hold the result.
      char temp[] = "/energy";
      strcpy(result,mqtt_outTopic); // copy string one into the result.
      strcat(result,temp); // append string two to the result.
      mqttClient.publish(result, 2, true, energyS);    }
  lastTime = millis();
  if(!mqttClient.connected()) {
    mqttClient.connect();
  }
  }
}
void setupArduinoOTA() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    type = "sketch";
    else // U_SPIFFS
    type = "filesystem";
    SPIFFS.end();
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
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
}
