// This example uses an ESP32 Development Board
// to connect to shiftr.io.
//
// You can check on your device after a successful
// connection here: https://www.shiftr.io/try.
//
// by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <WiFi.h>
#include <MQTT.h>


/*****************************************/

#include <DHT.h>

#define DHTPIN 13
#define DHTTYPE DHT22

DHT dht(DHTPIN,DHTTYPE);
float temperature;
int counter;

void setup_temperature(){
  //temperature = -300;
  //counter = 0;
  dht.begin();
  Serial.begin(115200);
  dht.begin();
}

float current_temperature(){
  counter = counter + 1;
  Serial.print("Rilevazione temperatura n. : ");
  Serial.println(counter);
  float currentTemperature = dht.readTemperature();
  if ( temperature != currentTemperature ) {
    temperature = currentTemperature;
    Serial.print("Nuova temperatura: ");
    Serial.println(currentTemperature);
  }
  else {
      Serial.println("Temperatura: stabile");
  }
  return currentTemperature;
}

/****************************************/


const char ssid[] = "Bellucci";
const char pass[] = "casabelluccibiocco5865";


#define mqtt_server "130.136.2.70"
#define mqtt_port 1883
#define mqtt_user "IOTuser"
#define mqtt_password "IOTuser"
#define in_topic "temperature/damianobellucci"
#define topic_setting_parameters "settingparameters/damianobellucci"

int sample_frequency = NULL;
float min_temp, max_temp, min_moi, max_moi;

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  //bool connect(const char clientID[], const char username[], const char password[], bool skip = false);
  while (!client.connect("arduino", mqtt_user, mqtt_password)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe(topic_setting_parameters, 2);
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  int ind1 = payload.indexOf(";");
  sample_frequency=payload.substring(0,ind1).toInt();

  int ind2 = payload.indexOf(";", ind1+1 );
  min_temp = payload.substring(ind1+1, ind2+1).toFloat(); 

  int ind3 = payload.indexOf(";", ind2+1 );
  max_temp = payload.substring(ind2+1, ind3+1).toFloat();

  int ind4 = payload.indexOf(";", ind3+1 );
  min_moi = payload.substring(ind3+1, ind4+1).toFloat();

  int ind5 = payload.indexOf(";", ind4+1 );
  max_moi = payload.substring(ind4+1, ind5+1).toFloat();

  Serial.println("Sample frequency:");
  Serial.println(sample_frequency);

  Serial.println("min temp:");
  Serial.println(min_temp);

  Serial.println("max temp:");
  Serial.println(max_temp);

  Serial.println("min_moi:");
  Serial.println(min_moi);

  Serial.println("max_moi:");
  Serial.println(max_moi);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin(mqtt_server,mqtt_port, net);
  client.onMessage(messageReceived);

  connect();
  setup_temperature();
}

void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    String temperature = String(current_temperature(),2);
    char payload[temperature.length()];
    temperature.toCharArray(payload, temperature.length());
    client.publish(in_topic,payload ,strlen(payload), false, 2);
  }
}