#include <ModbusMaster.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Preferences.h>

// ---------------- MQTT ----------------
const char* mqtt_server = "emqx.abcfood.app";
const int mqtt_port = 1883;
const char* mqtt_user = "homeassistant";
const char* mqtt_pass = "Hzpw1234#";

EthernetClient espClient;
PubSubClient client(espClient);

// ---------------- Sensor ----------------
const int RXD2 = 16;
const int TXD2 = 17;
const int DE_RE = 4;

ModbusMaster modbus;
float suhu[5]   = {0, 0, 0, 0, 0};
float offset[5] = {0, 0, 0, 0, 0};

// --------- Ethernet W5500 pins ---------
const int ETH_CS  = 5;
const int ETH_RST = 26;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x26, 0x30};

// --------- Preferences ---------
Preferences prefs;
const char* PREF_NS = "calib_26_30";

// ---------------- MQTT topics ----------------
const char* T_S1 = "sensor/suhu/id26";
const char* T_S2 = "sensor/suhu/id27";
const char* T_S3 = "sensor/suhu/id28";
const char* T_S4 = "sensor/suhu/id29";
const char* T_S5 = "sensor/suhu/id30";

// discovery number (offset)
const char* DISC_O1 = "homeassistant/number/suhu_offset_id26/config";
const char* DISC_O2 = "homeassistant/number/suhu_offset_id27/config";
const char* DISC_O3 = "homeassistant/number/suhu_offset_id28/config";
const char* DISC_O4 = "homeassistant/number/suhu_offset_id29/config";
const char* DISC_O5 = "homeassistant/number/suhu_offset_id30/config";

// command & state offset
const char* CMD_O1 = "sensor/suhu/offset/id26/set";
const char* CMD_O2 = "sensor/suhu/offset/id27/set";
const char* CMD_O3 = "sensor/suhu/offset/id28/set";
const char* CMD_O4 = "sensor/suhu/offset/id29/set";
const char* CMD_O5 = "sensor/suhu/offset/id30/set";

const char* ST_O1  = "sensor/suhu/offset/id26";
const char* ST_O2  = "sensor/suhu/offset/id27";
const char* ST_O3  = "sensor/suhu/offset/id28";
const char* ST_O4  = "sensor/suhu/offset/id29";
const char* ST_O5  = "sensor/suhu/offset/id30";

// ---------------- Helpers ----------------
void publishDiscovery() {

  client.publish("homeassistant/sensor/suhu_id26/config",
    "{\"name\":\"Suhu ID 26\",\"state_topic\":\"sensor/suhu/id26\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id26\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish("homeassistant/sensor/suhu_id27/config",
    "{\"name\":\"Suhu ID 27\",\"state_topic\":\"sensor/suhu/id27\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id27\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish("homeassistant/sensor/suhu_id28/config",
    "{\"name\":\"Suhu ID 28\",\"state_topic\":\"sensor/suhu/id28\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id28\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish("homeassistant/sensor/suhu_id29/config",
    "{\"name\":\"Suhu ID 29\",\"state_topic\":\"sensor/suhu/id29\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id29\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish("homeassistant/sensor/suhu_id30/config",
    "{\"name\":\"Suhu ID 30\",\"state_topic\":\"sensor/suhu/id30\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id30\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish(DISC_O1,
    "{\"name\":\"Offset Suhu ID 26\",\"command_topic\":\"sensor/suhu/offset/id26/set\",\"state_topic\":\"sensor/suhu/offset/id26\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id26\",\"mode\":\"box\"}", true);

  client.publish(DISC_O2,
    "{\"name\":\"Offset Suhu ID 27\",\"command_topic\":\"sensor/suhu/offset/id27/set\",\"state_topic\":\"sensor/suhu/offset/id27\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id27\",\"mode\":\"box\"}", true);

  client.publish(DISC_O3,
    "{\"name\":\"Offset Suhu ID 28\",\"command_topic\":\"sensor/suhu/offset/id28/set\",\"state_topic\":\"sensor/suhu/offset/id28\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id28\",\"mode\":\"box\"}", true);

  client.publish(DISC_O4,
    "{\"name\":\"Offset Suhu ID 29\",\"command_topic\":\"sensor/suhu/offset/id29/set\",\"state_topic\":\"sensor/suhu/offset/id29\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id29\",\"mode\":\"box\"}", true);

  client.publish(DISC_O5,
    "{\"name\":\"Offset Suhu ID 30\",\"command_topic\":\"sensor/suhu/offset/id30/set\",\"state_topic\":\"sensor/suhu/offset/id30\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id30\",\"mode\":\"box\"}", true);

  Serial.println("Discovery DONE (ID 26-30)");
}

void publishOffsetStateAll() {
  char buf[16];
  dtostrf(offset[0], 0, 2, buf); client.publish(ST_O1, buf, true);
  dtostrf(offset[1], 0, 2, buf); client.publish(ST_O2, buf, true);
  dtostrf(offset[2], 0, 2, buf); client.publish(ST_O3, buf, true);
  dtostrf(offset[3], 0, 2, buf); client.publish(ST_O4, buf, true);
  dtostrf(offset[4], 0, 2, buf); client.publish(ST_O5, buf, true);
}

void loadOffsets() {
  prefs.begin(PREF_NS, true);
  offset[0] = prefs.getFloat("o0", 0.0f);
  offset[1] = prefs.getFloat("o1", 0.0f);
  offset[2] = prefs.getFloat("o2", 0.0f);
  offset[3] = prefs.getFloat("o3", 0.0f);
  offset[4] = prefs.getFloat("o4", 0.0f);
  prefs.end();
}

void saveOffset(uint8_t idx) {
  prefs.begin(PREF_NS, false);
  prefs.putFloat(idx == 0 ? "o0" :
                 idx == 1 ? "o1" :
                 idx == 2 ? "o2" :
                 idx == 3 ? "o3" : "o4",
                 offset[idx]);
  prefs.end();
}

float parseOffsetPayload(char* payload, unsigned int length) {
  String s;
  for (unsigned int i = 0; i < length; i++) s += (char)payload[i];
  s.trim();
  if (s.startsWith("{")) {
    int p = s.indexOf(':');
    int e = s.indexOf('}');
    return s.substring(p + 1, e).toFloat();
  }
  return s.toFloat();
}

// ---------------- Ethernet & MQTT ----------------
void setup_ethernet() {
  SPI.begin(18, 19, 23, ETH_CS);
  Ethernet.init(ETH_CS);
  pinMode(ETH_RST, OUTPUT);

  digitalWrite(ETH_RST, LOW); delay(10);
  digitalWrite(ETH_RST, HIGH); delay(50);

  if (Ethernet.begin(mac) == 0) {
    Ethernet.begin(mac, IPAddress(192,168,100,54),
                        IPAddress(8,8,8,8),
                        IPAddress(192,168,100,1),
                        IPAddress(255,255,255,0));
  }

  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
}

void subscribeOffsetTopics() {
  client.subscribe(CMD_O1);
  client.subscribe(CMD_O2);
  client.subscribe(CMD_O3);
  client.subscribe(CMD_O4);
  client.subscribe(CMD_O5);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  float val = parseOffsetPayload((char*)payload, length);

  if (strcmp(topic, CMD_O1) == 0) { offset[0] = val; saveOffset(0); }
  if (strcmp(topic, CMD_O2) == 0) { offset[1] = val; saveOffset(1); }
  if (strcmp(topic, CMD_O3) == 0) { offset[2] = val; saveOffset(2); }
  if (strcmp(topic, CMD_O4) == 0) { offset[3] = val; saveOffset(3); }
  if (strcmp(topic, CMD_O5) == 0) { offset[4] = val; saveOffset(4); }

  publishOffsetStateAll();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT retry...");
    if (client.connect("ESP32Client_26_30", mqtt_user, mqtt_pass)) {
      Serial.println("OK");
      publishDiscovery();
      subscribeOffsetTopics();
      publishOffsetStateAll();
    } else {
      Serial.println("Fail, retry...");
      delay(2000);
    }
  }
}

void preTransmission() { digitalWrite(DE_RE, HIGH); }
void postTransmission() { digitalWrite(DE_RE, LOW); }

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW);

  loadOffsets();

  modbus.begin(26, Serial1);
  modbus.preTransmission(preTransmission);
  modbus.postTransmission(postTransmission);

  setup_ethernet();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  reconnect();
}

// ---------------- Loop ----------------
unsigned long lastPublish = 0;

void loop() {
  for (uint8_t id = 26; id <= 30; id++) {

    uint8_t idx = id - 26;

    Serial1.flush();
    delay(10);

    modbus.begin(id, Serial1);
    uint8_t res = modbus.readHoldingRegisters(0x0000, 1);

    if (res == modbus.ku8MBSuccess) {

      float raw = modbus.getResponseBuffer(0) / 10.0;
      float newVal = raw + offset[idx];

      if (newVal < 10.0 || newVal > 90.0) {
        Serial.print("IGNORED ID ");
        Serial.print(id);
        Serial.print(" raw=");
        Serial.print(raw);
        Serial.print(" final=");
        Serial.println(newVal);
      } else {
        suhu[idx] = newVal;
      }

    } else {
      Serial.print("Modbus ERR ID ");
      Serial.println(id);
    }

    if (!client.connected()) reconnect();
    client.loop();

    delay(200);
  }

  if (millis() - lastPublish > 5000) {
    client.publish(T_S1, String(suhu[0]).c_str());
    client.publish(T_S2, String(suhu[1]).c_str());
    client.publish(T_S3, String(suhu[2]).c_str());
    client.publish(T_S4, String(suhu[3]).c_str());
    client.publish(T_S5, String(suhu[4]).c_str());

    lastPublish = millis();
    Serial.println("Publish suhu OK (ID 26-30)");
  }
}
