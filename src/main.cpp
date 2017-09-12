#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>
#include <DallasTemperature.h>
/*
  ssid a heslo k wifi
*/
const char* ssid = "....";
const char* password = ".....";
/*
  Nastaveni pro pripojeni k mqtt brokeru
*/
const char* mqtt_server = "broker.mqttdashboard.com";


WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
float value2 = 0;
/*
 Vlhkost
*/
#define DHTPIN            2
#define DHTTYPE           DHT11     // DHT 11
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)
DHT_Unified dht(DHTPIN, DHTTYPE);
/*
 Teplota
*/
// nastavení čísla vstupního pinu
const int pinCidlaDS = 4;
OneWire oneWireDS(pinCidlaDS);
// vytvoření instance senzoryDS z knihovny DallasTemperature
DallasTemperature senzoryDS(&oneWireDS);
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Pripojuji se k:  ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wifi pripojeno");
  Serial.println("IP adresa: ");
  Serial.println(WiFi.localIP());
}
/*
  Medota k pripojeni k MQTT brokeru
*/
void reconnect() {
  // Opakuj dokud nejsme pripojeni
  while (!client.connected()) {
    Serial.print("Pokus o navazani MQTT pripojeni...");
    // Pokus o pripojeni
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      //Jakmile se pripojime tak to ohlasime v topicu "zarizeni"
    //  client.publish("zarizeni", "Ahoj tady ESP a hlasim ze sem se pripojil :)");
    } else {

      Serial.print("Chyba, rc=");
      Serial.print(client.state());
      Serial.println(" zksusim to znovu za 5s");
      // pocka 5s
      delay(5000);
    }
  }
}

int Vlhkost(){
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
    return -1;
  }
  else {
    int vlhkost = event.relative_humidity;
    Serial.print("Humidity: ");
    Serial.print(vlhkost);
    Serial.println("%");
    return vlhkost;
  }
}
float Teplota(){
  senzoryDS.requestTemperatures();
  // výpis teploty na sériovou linku, při připojení více čidel
  // na jeden pin můžeme postupně načíst všechny teploty
  // pomocí změny čísla v závorce (0) - pořadí dle unikátní adresy čidel
  float teplota = senzoryDS.getTempCByIndex(0);
  Serial.print("Teplota cidla DS18B20: ");
  Serial.print(teplota);
  Serial.println(" stupnu Celsia");
  return teplota;
}
void setup() {
  Serial.begin(9600);
  dht.begin();
  senzoryDS.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
     /*
      Pokud nejsem pripojen tak se pripojim
     */
     if (!client.connected()) {
          reconnect();
      }
      long now = millis();
      if (now - lastMsg > 10000) {

        lastMsg = now;
        value = Vlhkost();
        snprintf (msg, 75, "Vlhkost: %ld %", value);
        value2 = Teplota();
       // snprintf (msg, 75, "Ahoj tady ESP #%ld", value);
        Serial.print("Posilam zpravu: ");
        Serial.println(msg);
       client.publish("zprava", msg);
        snprintf (msg, 75, "Teplota: %g ", value2);
        Serial.print("Posilam zpravu: ");
        String s = String(value2,2);
        s = "Teplota: "+s;
        Serial.println(s);
        s.toCharArray(msg, 75,0);
       client.publish("zprava",msg );
       client.publish("zprava", "Tucnaci jsou super :) ");

      }
}
