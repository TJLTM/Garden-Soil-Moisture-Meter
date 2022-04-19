#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "...";
const char* password = "...";
const char* mqtt_server = "...";

String Name = "SoilMoisture";
String ID;
#define MoistureSensorPin A0
#define MoistureOutputPin 3
#define OutputToCheckVotlage 2

int MinsToSleep = 20 * 60000000;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
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
  pinMode(MoistureOutputPin,OUTPUT);
  pinMode(OutputToCheckVotlage,OUTPUT);
  digitalWrite(OutputToCheckVotlage,LOW);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(BUILTIN_LED,HIGH);
  delay(500);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println(Name + "/" + ID);
  Serial.println("Posting to MQTT");
  ReadMoisture();
  CompletelyStupidVoltageCheck();
  
  Serial.println("going to sleep");
  digitalWrite(BUILTIN_LED,LOW);
  delay(500);
  
  ESP.deepSleep(MinsToSleep);
}

void loop() {
}

void ReadMoisture(){
  digitalWrite(MoistureOutputPin,HIGH); //Turn on the Sensor 
  delay(250); // 1/4 second for everything to stablize
  int Sum = 0;
  int Samples = 200; 
  for (int x = 0; x < Samples; x++){
    Sum = Sum + analogRead(MoistureSensorPin);
  }
  
  digitalWrite(MoistureOutputPin,LOW); //turn off the Sensor
  int Average = Sum/Samples; 

  /*
   * Put in Fudge factor here for your own stuff i'm dealing with multiple 
   * soil types and that changes how you would calibrate your readings 
   * i'm just going to give the raw ADC and convert that to a Percentage 
   */
  
  int MoisturePercentage = map(Average, 0, 1023, 0, 100);
  
  String MoistTopic = Name + "/" + ID + "/Moisture Percentage";
  client.publish(MoistTopic.c_str(),String(MoisturePercentage).c_str());
  Serial.println(MoistTopic + "  " + String(MoisturePercentage));
  MoistTopic = Name + "/" + ID + "/Moisture Raw";
  client.publish(MoistTopic.c_str(),String(Average).c_str());
  Serial.println(MoistTopic + "  " + String(Average));
}

void CompletelyStupidVoltageCheck(){
  digitalWrite(OutputToCheckVotlage,HIGH);
  delay(250); // 1/4 second for everything to stablize
  int Sum = 0;
  int Samples = 200; 
  for (int x = 0; x < Samples; x++){
    Sum = Sum + analogRead(MoistureSensorPin);
  }
  
  digitalWrite(MoistureOutputPin,LOW); //turn off the Sensor
  float Voltage = (Sum/Samples)*(3.3/1023); 
  
  String MoistTopic = Name + "/" + ID + "/Voltage";
  client.publish(MoistTopic.c_str(),String(Voltage).c_str());
  Serial.println(MoistTopic + "  " + String(Voltage));
  digitalWrite(OutputToCheckVotlage,LOW);
}


  


  
