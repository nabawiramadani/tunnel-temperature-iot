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
float suhu[5];

// offset tidak di-hardcode; mulai dari 0 dan bisa diubah via HA
float offset[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

// --------- Ethernet W5500 pins & MAC ---------
const int ETH_CS  = 5;
const int ETH_RST = 26;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// --------- Preferences (NVS) ---------
Preferences prefs;
const char* PREF_NS = "calib";

// ---------------- MQTT topics ----------------
const char* T_S1 = "sensor/suhu/id1";
const char* T_S2 = "sensor/suhu/id2";
const char* T_S3 = "sensor/suhu/id3";
const char* T_S4 = "sensor/suhu/id4";
const char* T_S5 = "sensor/suhu/id5";

// discovery number (offset)
const char* DISC_O1 = "homeassistant/number/suhu_offset_id1/config";
const char* DISC_O2 = "homeassistant/number/suhu_offset_id2/config";
const char* DISC_O3 = "homeassistant/number/suhu_offset_id3/config";
const char* DISC_O4 = "homeassistant/number/suhu_offset_id4/config";
const char* DISC_O5 = "homeassistant/number/suhu_offset_id5/config";

// command & state offset
const char* CMD_O1 = "sensor/suhu/offset/id1/set";
const char* CMD_O2 = "sensor/suhu/offset/id2/set";
const char* CMD_O3 = "sensor/suhu/offset/id3/set";
const char* CMD_O4 = "sensor/suhu/offset/id4/set";
const char* CMD_O5 = "sensor/suhu/offset/id5/set";

const char* ST_O1  = "sensor/suhu/offset/id1";
const char* ST_O2  = "sensor/suhu/offset/id2";
const char* ST_O3  = "sensor/suhu/offset/id3";
const char* ST_O4  = "sensor/suhu/offset/id4";
const char* ST_O5  = "sensor/suhu/offset/id5";

// ---------------- Helpers ----------------
void publishDiscovery() {
  // sensor suhu (tambahkan force_update + expire_after agar histori bergerak)
  client.publish("homeassistant/sensor/suhu_id1/config",
    "{\"name\":\"Suhu ID 1\",\"state_topic\":\"sensor/suhu/id1\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id1\",\"force_update\":true,\"expire_after\":30}", true);
  client.publish("homeassistant/sensor/suhu_id2/config",
    "{\"name\":\"Suhu ID 2\",\"state_topic\":\"sensor/suhu/id2\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id2\",\"force_update\":true,\"expire_after\":30}", true);
  client.publish("homeassistant/sensor/suhu_id3/config",
    "{\"name\":\"Suhu ID 3\",\"state_topic\":\"sensor/suhu/id3\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id3\",\"force_update\":true,\"expire_after\":30}", true);
  client.publish("homeassistant/sensor/suhu_id4/config",
    "{\"name\":\"Suhu ID 4\",\"state_topic\":\"sensor/suhu/id4\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id4\",\"force_update\":true,\"expire_after\":30}", true);
  client.publish("homeassistant/sensor/suhu_id5/config",
    "{\"name\":\"Suhu ID 5\",\"state_topic\":\"sensor/suhu/id5\",\"unit_of_measurement\":\"\\u00b0C\",\"device_class\":\"temperature\",\"unique_id\":\"suhu_id5\",\"force_update\":true,\"expire_after\":30}", true);

  // number entity untuk offset → mode: "box" agar input angka manual
  client.publish(DISC_O1,
    "{\"name\":\"Offset Suhu ID 1\",\"command_topic\":\"sensor/suhu/offset/id1/set\",\"state_topic\":\"sensor/suhu/offset/id1\",\"min\":-50,\"max\":50,\"step\":0.1,\"unit_of_measurement\":\"\\u00b0C\",\"unique_id\":\"suhu_offset_id1\",\"mode\":\"box\"}", true);
  client.publish(DISC_O2,
    "{\"name\":\"Offset Suhu ID 2\",\"command_topic\":\"sensor/suhu/offset/id2/set\",\"state_topic\":\"sensor/suhu/offset/id2\",\"min\":-50,\"max\":50,\"step\":0.1,\"unit_of_measurement\":\"\\u00b0C\",\"unique_id\":\"suhu_offset_id2\",\"mode\":\"box\"}", true);
  client.publish(DISC_O3,
    "{\"name\":\"Offset Suhu ID 3\",\"command_topic\":\"sensor/suhu/offset/id3/set\",\"state_topic\":\"sensor/suhu/offset/id3\",\"min\":-50,\"max\":50,\"step\":0.1,\"unit_of_measurement\":\"\\u00b0C\",\"unique_id\":\"suhu_offset_id3\",\"mode\":\"box\"}", true);
  client.publish(DISC_O4,
    "{\"name\":\"Offset Suhu ID 4\",\"command_topic\":\"sensor/suhu/offset/id4/set\",\"state_topic\":\"sensor/suhu/offset/id4\",\"min\":-50,\"max\":50,\"step\":0.1,\"unit_of_measurement\":\"\\u00b0C\",\"unique_id\":\"suhu_offset_id4\",\"mode\":\"box\"}", true);
  client.publish(DISC_O5,
    "{\"name\":\"Offset Suhu ID 5\",\"command_topic\":\"sensor/suhu/offset/id5/set\",\"state_topic\":\"sensor/suhu/offset/id5\",\"min\":-50,\"max\":50,\"step\":0.1,\"unit_of_measurement\":\"\\u00b0C\",\"unique_id\":\"suhu_offset_id5\",\"mode\":\"box\"}", true);

  Serial.println("Konfigurasi MQTT Auto Discovery dikirim.");
}

void publishOffsetStateAll() {
  char buf[16];
  dtostrf(offset[0], 0, 2, buf); client.publish(ST_O1, buf, true);
  dtostrf(offset[1], 0, 2, buf); client.publish(ST_O2, buf, true);
  dtostrf(offset[2], 0, 2, buf); client.publish(ST_O3, buf, true);
  dtostrf(offset[3], 0, 2, buf); client.publish(ST_O4, buf, true);
  dtostrf(offset[4], 0, 2, buf); client.publish(ST_O5, buf, true);
}

// muat offset tersimpan (default 0.0 jika belum ada)
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
  if (idx == 0) prefs.putFloat("o0", offset[0]);
  if (idx == 1) prefs.putFloat("o1", offset[1]);
  if (idx == 2) prefs.putFloat("o2", offset[2]);
  if (idx == 3) prefs.putFloat("o3", offset[3]);
  if (idx == 4) prefs.putFloat("o4", offset[4]);
  prefs.end();
}

// helper: parse payload angka plain atau JSON {"value":x}
float parseOffsetPayload(char* payload, unsigned int length) {
  // buat String tanpa karakter di luar length
  String s;
  s.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) s += (char)payload[i];
  s.trim();

  if (s.length() == 0) return 0.0f;

  if (s[0] == '{') {
    int p = s.indexOf(':');
    int e = s.lastIndexOf('}');
    if (p != -1 && e != -1 && e > p) {
      String num = s.substring(p + 1, e);
      num.trim();
      return num.toFloat();
    }
  }
  return s.toFloat(); // plain "2.5"
}

// ---------------- Ethernet & MQTT ----------------
void setup_ethernet() {
  SPI.begin(18, 19, 23, ETH_CS);
  Ethernet.init(ETH_CS);
  pinMode(ETH_RST, OUTPUT);
  digitalWrite(ETH_RST, LOW); delay(10);
  digitalWrite(ETH_RST, HIGH); delay(50);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP gagal. Coba static IP...");
    IPAddress ip(192, 168, 100, 50);
    IPAddress gateway(192, 168, 100, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(8, 8, 8, 8);
    Ethernet.begin(mac, ip, dns, gateway, subnet);
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

  if (strcmp(topic, CMD_O1) == 0) {
    offset[0] = val; saveOffset(0);
    char b[16]; dtostrf(offset[0], 0, 2, b); client.publish(ST_O1, b, true);
    Serial.printf("Offset ID1 diubah: %.2f\n", offset[0]);
  } else if (strcmp(topic, CMD_O2) == 0) {
    offset[1] = val; saveOffset(1);
    char b[16]; dtostrf(offset[1], 0, 2, b); client.publish(ST_O2, b, true);
    Serial.printf("Offset ID2 diubah: %.2f\n", offset[1]);
  } else if (strcmp(topic, CMD_O3) == 0) {
    offset[2] = val; saveOffset(2);
    char b[16]; dtostrf(offset[2], 0, 2, b); client.publish(ST_O3, b, true);
    Serial.printf("Offset ID3 diubah: %.2f\n", offset[2]);
  } else if (strcmp(topic, CMD_O4) == 0) {
    offset[3] = val; saveOffset(3);
    char b[16]; dtostrf(offset[3], 0, 2, b); client.publish(ST_O4, b, true);
    Serial.printf("Offset ID4 diubah: %.2f\n", offset[3]);
  } else if (strcmp(topic, CMD_O5) == 0) {
    offset[4] = val; saveOffset(4);
    char b[16]; dtostrf(offset[4], 0, 2, b); client.publish(ST_O5, b, true);
    Serial.printf("Offset ID5 diubah: %.2f\n", offset[4]);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("terhubung");
      publishDiscovery();
      subscribeOffsetTopics();
      publishOffsetStateAll(); // publish awal (0.0 jika belum diset)
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// ---------------- Modbus Transmit Hooks ----------------
void preTransmission() { digitalWrite(DE_RE, HIGH); }
void postTransmission() { digitalWrite(DE_RE, LOW); }

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW);

  loadOffsets();  // baca offset yang pernah disimpan (default 0)

  modbus.begin(1, Serial1);
  modbus.preTransmission(preTransmission);
  modbus.postTransmission(postTransmission);

  setup_ethernet();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

// ---------------- Loop ----------------
unsigned long lastPublish = 0;

void loop() {
  // proses tiap ID
  for (uint8_t id = 1; id <= 5; id++) {
    modbus.begin(id, Serial1);
    uint8_t res = modbus.readHoldingRegisters(0x0000, 1);

    if (res == modbus.ku8MBSuccess) {
      suhu[id - 1] = (modbus.getResponseBuffer(0) / 10.0) + offset[id - 1];
      Serial.print("Suhu ID ");
      Serial.print(id);
      Serial.print(": ");
      Serial.println(suhu[id - 1]);
    } else {
      Serial.print("Gagal baca ID ");
      Serial.print(id);
      Serial.print(". Error: ");
      Serial.println(res);
      // tetap publish nilai default agar HA hidup
      suhu[id - 1] = 22.2 + offset[id - 1];
    }

    // jangan blokir MQTT callback terlalu lama
    for (int i = 0; i < 10; i++) { // total ~1s
      if (!client.connected()) reconnect();
      client.loop();
      delay(100);
    }
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - lastPublish > 5000) {
    client.publish(T_S1, String(suhu[0]).c_str());
    client.publish(T_S2, String(suhu[1]).c_str());
    client.publish(T_S3, String(suhu[2]).c_str());
    client.publish(T_S4, String(suhu[3]).c_str());
    client.publish(T_S5, String(suhu[4]).c_str());

    // republish offset (retained) supaya HA konsisten
    publishOffsetStateAll();

    Serial.println("Data suhu dan offset dikirim ke MQTT.");
    lastPublish = millis();
  }

  // loop akhir non-blok agar subscribe cepat diproses
  for (int i = 0; i < 15; i++) { // ~1.5s
    if (!client.connected()) reconnect();
    client.loop();
    delay(100);
  }

  Serial.println("===== Loop selesai =====");
}
