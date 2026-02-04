#include <ModbusMaster.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= LCD (DITAMBAHKAN) =================
LiquidCrystal_I2C lcd(0x25, 20, 4);

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
float suhu[5]   = {-99, -99, -99, -99, -99};   // init agar LCD tidak blank
float offset[5] = {0, 0, 0, 0, 0};

// --------- Ethernet W5500 pins ---------
const int ETH_CS  = 5;
const int ETH_RST = 26;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x56, 0x60};

// --------- Preferences ---------
Preferences prefs;
const char* PREF_NS = "calib_56_60";

// ---------------- MQTT topics ----------------
const char* T_S1 = "sensor/suhu/id56";
const char* T_S2 = "sensor/suhu/id57";
const char* T_S3 = "sensor/suhu/id58";
const char* T_S4 = "sensor/suhu/id59";
const char* T_S5 = "sensor/suhu/id60";

// discovery number (offset)
const char* DISC_O1 = "homeassistant/number/suhu_offset_id56/config";
const char* DISC_O2 = "homeassistant/number/suhu_offset_id57/config";
const char* DISC_O3 = "homeassistant/number/suhu_offset_id58/config";
const char* DISC_O4 = "homeassistant/number/suhu_offset_id59/config";
const char* DISC_O5 = "homeassistant/number/suhu_offset_id60/config";

// command & state offset
const char* CMD_O1 = "sensor/suhu/offset/id56/set";
const char* CMD_O2 = "sensor/suhu/offset/id57/set";
const char* CMD_O3 = "sensor/suhu/offset/id58/set";
const char* CMD_O4 = "sensor/suhu/offset/id59/set";
const char* CMD_O5 = "sensor/suhu/offset/id60/set";

const char* ST_O1  = "sensor/suhu/offset/id56";
const char* ST_O2  = "sensor/suhu/offset/id57";
const char* ST_O3  = "sensor/suhu/offset/id58";
const char* ST_O4  = "sensor/suhu/offset/id59";
const char* ST_O5  = "sensor/suhu/offset/id60";

// ---------------- Helpers ----------------
void publishDiscovery() {

  client.publish("homeassistant/sensor/suhu_id56/config",
    "{\"name\":\"Suhu ID 56\",\"state_topic\":\"sensor/suhu/id56\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id56\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish("homeassistant/sensor/suhu_id57/config",
    "{\"name\":\"Suhu ID 57\",\"state_topic\":\"sensor/suhu/id57\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id57\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish("homeassistant/sensor/suhu_id58/config",
    "{\"name\":\"Suhu ID 58\",\"state_topic\":\"sensor/suhu/id58\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id58\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish("homeassistant/sensor/suhu_id59/config",
    "{\"name\":\"Suhu ID 59\",\"state_topic\":\"sensor/suhu/id59\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id59\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish("homeassistant/sensor/suhu_id60/config",
    "{\"name\":\"Suhu ID 60\",\"state_topic\":\"sensor/suhu/id60\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id60\",\"force_update\":true,\"expire_after\":30}", true);

  client.publish(DISC_O1,
    "{\"name\":\"Offset Suhu ID 56\",\"command_topic\":\"sensor/suhu/offset/id56/set\",\"state_topic\":\"sensor/suhu/offset/id56\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id56\",\"mode\":\"box\"}", true);

  client.publish(DISC_O2,
    "{\"name\":\"Offset Suhu ID 57\",\"command_topic\":\"sensor/suhu/offset/id57/set\",\"state_topic\":\"sensor/suhu/offset/id57\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id57\",\"mode\":\"box\"}", true);

  client.publish(DISC_O3,
    "{\"name\":\"Offset Suhu ID 58\",\"command_topic\":\"sensor/suhu/offset/id58/set\",\"state_topic\":\"sensor/suhu/offset/id58\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id58\",\"mode\":\"box\"}", true);

  client.publish(DISC_O4,
    "{\"name\":\"Offset Suhu ID 59\",\"command_topic\":\"sensor/suhu/offset/id59/set\",\"state_topic\":\"sensor/suhu/offset/id59\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id59\",\"mode\":\"box\"}", true);

  client.publish(DISC_O5,
    "{\"name\":\"Offset Suhu ID 60\",\"command_topic\":\"sensor/suhu/offset/id60/set\",\"state_topic\":\"sensor/suhu/offset/id60\",\"unit_of_measurement\":\"\\u00b0C\",\"min\":-50,\"max\":50,\"step\":0.1,\"unique_id\":\"suhu_offset_id60\",\"mode\":\"box\"}", true);

  Serial.println("Discovery DONE (ID 56-60)");
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
  for (int i = 0; i < 5; i++) {
    offset[i] = prefs.getFloat(String("o" + String(i)).c_str(), 0.0f);
  }
  prefs.end();
}

void saveOffset(uint8_t idx) {
  prefs.begin(PREF_NS, false);
  prefs.putFloat(String("o" + String(idx)).c_str(), offset[idx]);
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
    Ethernet.begin(mac, IPAddress(192,168,100,56),
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
    if (client.connect("ESP32Client_56_60", mqtt_user, mqtt_pass)) {
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

// ================= LCD (DITAMBAHKAN) =================
void printTemp(float v) {
  if (v < -50) lcd.print("--.-C ");
  else { lcd.print(v,1); lcd.print("C "); }
}

void updateLCD() {
  lcd.setCursor(0,0);  lcd.print("K1 : "); printTemp(suhu[0]);
  lcd.setCursor(11,0); lcd.print("RA : "); printTemp(suhu[3]);

  lcd.setCursor(0,1);  lcd.print("K2 : "); printTemp(suhu[1]);
  lcd.setCursor(11,1); lcd.print("RB : "); printTemp(suhu[4]);

  lcd.setCursor(0,2);  lcd.print("K3 : "); printTemp(suhu[2]);
  lcd.setCursor(0,3);  lcd.print("MODE : LOCAL MONITOR");
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  updateLCD();              // LCD hidup sebelum MQTT

  loadOffsets();

  setup_ethernet();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  reconnect();
}

// ---------------- Loop ----------------
unsigned long lastPublish = 0;

void loop() {

  updateLCD();              // LCD selalu jalan

  for (uint8_t id = 56; id <= 60; id++) {

    uint8_t idx = id - 56;

    Serial1.flush();
    delay(10);

    modbus.begin(id, Serial1);
    modbus.preTransmission(preTransmission);
    modbus.postTransmission(postTransmission);

    uint8_t res = modbus.readHoldingRegisters(0x0000, 1);

    if (res == modbus.ku8MBSuccess) {
      float raw = modbus.getResponseBuffer(0) / 10.0;
      float newVal = raw + offset[idx];

      if (newVal >= 10.0 && newVal <= 90.0) {
        suhu[idx] = newVal;
      }
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
    Serial.println("Publish suhu OK (ID 56-60)");
  }
}
