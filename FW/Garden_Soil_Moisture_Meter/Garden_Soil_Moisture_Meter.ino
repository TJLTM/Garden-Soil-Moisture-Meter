#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "...";
const char* password = "...";
const char* mqtt_server = "...";

String Name = "SoilMoisture";
String ID;
bool WifiConnected, MQTTConnected = false;
#define MuxAddressPinS0 D1
#define MuxAddressPinS1 D2
#define MuxAddressPinS2 D3
#define MuxAddressPinS3 D4
#define MuxEnablePin D7

int MinsToSleep = 15 * 60000000;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  // Need to add timeout for this
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int StartTime = millis();
  int CurrentTime = millis();
  while (WiFi.status() != WL_CONNECTED && abs(StartTime - CurrentTime) < 60000) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    WifiConnected = true;
  }
  else {
    Serial.println("");
    Serial.println("Can no connect to WiFi");
  }

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  String ClientName = Name + ID;
  int MQttReconnect = 0;
  while (!client.connected() && MQttReconnect < 5) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ClientName.c_str())) {
      Serial.print("connected as ");
      Serial.println(ClientName);
      MQTTConnected = true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      MQttReconnect += 1;
      MQTTConnected = false;
    }
  }
}

void setup() {
  String MAC = WiFi.macAddress();
  for (int x = 9; x < 17; x++) {
    if (MAC.charAt(x) != ':') {
      ID.concat(MAC.charAt(x));
    }
  }

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  if (WifiConnected == true) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    if (MQTTConnected == true) {
      Serial.println(Name + "/" + ID);
      Serial.println("Posting to MQTT");
      for (int x = 1; x < 16; x++) {
        SwitchMuxInputs(x);
        delay(100);
        ReadMoisture(x);
      }
      ReadBatteryVoltage();
    }
    else {
      Serial.println("Could not connect to MQTT Server");
    }
  }

  Serial.println("going to sleep in 5 Seconds");
  digitalWrite(BUILTIN_LED, LOW);
  delay(5000);
  if (WifiConnected == true) {
    ESP.deepSleep(MinsToSleep);
  }
  else {
    ESP.deepSleep(2.5 * 60000000); //try again in 2:30
  }
}

void loop() {
}

void ReadBatteryVoltage() {
  SwitchMuxInputs(0);
  delay(250); // 1/4 second for everything to stablize
  int Sum = 0;
  int Samples = 200;
  for (int x = 0; x < Samples; x++) {
    Sum = Sum + analogRead(A0);
    delay(3); // delay to let the ADC settle before reading again.
  }

  int Average = Sum / Samples;

  String MoistTopic = Name + "/" + ID + "/Voltage/";
  client.publish(MoistTopic.c_str(), String(Average).c_str());
  Serial.println(MoistTopic + "  " + String(Average));

}

void ReadMoisture(int InputNumber) {
  delay(250); // 1/4 second for everything to stablize
  int Sum = 0;
  int Samples = 200;
  for (int x = 0; x < Samples; x++) {
    Sum = Sum + analogRead(A0);
    delay(3); // delay to let the ADC settle before reading again.
  }

  int Average = Sum / Samples;

  String MoistTopic = Name + "/" + ID + "/Moisture Raw/" + String(InputNumber);
  client.publish(MoistTopic.c_str(), String(Average).c_str());
  Serial.println(MoistTopic + "  " + String(Average));
}

void SwitchMuxInputs(int Number) {
  digitalWrite(MuxEnablePin, HIGH);
  switch (Number) {
    case 0:
      digitalWrite(MuxAddressPinS0, LOW);
      digitalWrite(MuxAddressPinS1, LOW);
      digitalWrite(MuxAddressPinS2, LOW);
      digitalWrite(MuxAddressPinS3, LOW);
      break;
    case 1:
      digitalWrite(MuxAddressPinS0, HIGH);
      digitalWrite(MuxAddressPinS1, LOW);
      digitalWrite(MuxAddressPinS2, LOW);
      digitalWrite(MuxAddressPinS3, LOW);
      break;
    case 2:
      digitalWrite(MuxAddressPinS0, LOW);
      digitalWrite(MuxAddressPinS1, HIGH);
      digitalWrite(MuxAddressPinS2, LOW);
      digitalWrite(MuxAddressPinS3, LOW);
      break;
    case 3:
      digitalWrite(MuxAddressPinS0, HIGH);
      digitalWrite(MuxAddressPinS1, HIGH);
      digitalWrite(MuxAddressPinS2, LOW);
      digitalWrite(MuxAddressPinS3, LOW);
      break;
    case 4:
      digitalWrite(MuxAddressPinS0, LOW);
      digitalWrite(MuxAddressPinS1, LOW);
      digitalWrite(MuxAddressPinS2, HIGH);
      digitalWrite(MuxAddressPinS3, LOW);
      break;
    case 5:
      digitalWrite(MuxAddressPinS0, HIGH);
      digitalWrite(MuxAddressPinS1, LOW);
      digitalWrite(MuxAddressPinS2, HIGH);
      digitalWrite(MuxAddressPinS3, LOW);
      break;
    case 6:
      digitalWrite(MuxAddressPinS0, LOW);
      digitalWrite(MuxAddressPinS1, HIGH);
      digitalWrite(MuxAddressPinS2, HIGH);
      digitalWrite(MuxAddressPinS3, LOW);
      break;
    case 7:
      digitalWrite(MuxAddressPinS0, HIGH);
      digitalWrite(MuxAddressPinS1, HIGH);
      digitalWrite(MuxAddressPinS2, HIGH);
      digitalWrite(MuxAddressPinS3, LOW);
      break;
    case 8:
      digitalWrite(MuxAddressPinS0, LOW);
      digitalWrite(MuxAddressPinS1, LOW);
      digitalWrite(MuxAddressPinS2, LOW);
      digitalWrite(MuxAddressPinS3, HIGH);
      break;
    case 9:
      digitalWrite(MuxAddressPinS0, HIGH);
      digitalWrite(MuxAddressPinS1, LOW);
      digitalWrite(MuxAddressPinS2, LOW);
      digitalWrite(MuxAddressPinS3, HIGH);
      break;
    case 10:
      digitalWrite(MuxAddressPinS0, LOW);
      digitalWrite(MuxAddressPinS1, HIGH);
      digitalWrite(MuxAddressPinS2, LOW);
      digitalWrite(MuxAddressPinS3, HIGH);
      break;
    case 11:
      digitalWrite(MuxAddressPinS0, HIGH);
      digitalWrite(MuxAddressPinS1, HIGH);
      digitalWrite(MuxAddressPinS2, LOW);
      digitalWrite(MuxAddressPinS3, HIGH);
      break;
    case 12:
      digitalWrite(MuxAddressPinS0, LOW);
      digitalWrite(MuxAddressPinS1, LOW);
      digitalWrite(MuxAddressPinS2, HIGH);
      digitalWrite(MuxAddressPinS3, HIGH);
      break;
    case 13:
      digitalWrite(MuxAddressPinS0, HIGH);
      digitalWrite(MuxAddressPinS1, LOW);
      digitalWrite(MuxAddressPinS2, HIGH);
      digitalWrite(MuxAddressPinS3, HIGH);
      break;
    case 14:
      digitalWrite(MuxAddressPinS0, LOW);
      digitalWrite(MuxAddressPinS1, HIGH);
      digitalWrite(MuxAddressPinS2, HIGH);
      digitalWrite(MuxAddressPinS3, HIGH);
      break;
    case 15:
      digitalWrite(MuxAddressPinS0, HIGH);
      digitalWrite(MuxAddressPinS1, HIGH);
      digitalWrite(MuxAddressPinS2, HIGH);
      digitalWrite(MuxAddressPinS3, HIGH);
      break;
    default:
      break;
  }
  digitalWrite(MuxEnablePin, LOW);
}
