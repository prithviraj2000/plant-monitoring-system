#define BLYNK_PRINT Serial
#include <OneWire.h>
#include <SPI.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
String apiKey = "2AQXSYZ973KEGS7F";
char auth[] = "IM-hC4qKWItS_LbfxLXkdpmy2r9FD5SS";
char ssid[] = "div";
char pass[] = "tarp1234";
const char* server = "api.thingspeak.com";

#define pirPin D1
int pirValue;
int pinValue;
#define rainPin D5
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
SimpleTimer timer;
const int pumpPin = 14 ;

BLYNK_WRITE(V0)
{
  pinValue = param.asInt();
}

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.println("temperature is:");
  Serial.println(t);
  Serial.println("humidity is:");
  Serial.println(h);
  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V5, h); //V5 is for Humidity
  Blynk.virtualWrite(V6, t); //V6 is for Temperature
}

void setup()
{
  pinMode(pumpPin, OUTPUT);

  Serial.begin(115200);
  delay(10);
  Blynk.begin(auth, ssid, pass);
  pinMode(rainPin, INPUT);
  pinMode(pirPin, INPUT);
  dht.begin();
  Serial.println("Connecting to ");
  Serial.println(ssid);


  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  timer.setInterval(1000L, sendSensor);
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  sensors.begin();
}

int sensor = 0;
int sensor1 = 0;
int output = 0;


void sendTemps()
{
  sensor = analogRead(A0);
  output = (100 - map(sensor, 0, 1023, 0, 100));
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  Serial.print("moisture = ");
  Serial.print(output);
  Serial.println("%");
  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, output);
  delay(1000);
}

void getPirValue(void)        //Get PIR Data
{
  pirValue = digitalRead(pirPin);
  if (pirValue)
  { Serial.println(pirValue);
    Serial.println("Motion detected");
    Blynk.notify("Motion detected");
  }
}

void loop()
{
  Blynk.run();
  timer.run();
  sendTemps();
  if (output <= 10) {
    digitalWrite(pumpPin, HIGH);
    Serial.println("needs water, send notification");
    Blynk.notify("Water your plant");
    delay(100);
    //send notification

  }
  else {
    digitalWrite(pumpPin, LOW);
    Serial.println("does not need water");
    delay(100);
  }

  int rainState = digitalRead(rainPin);
  Serial.println(rainState);

  if (rainState == 0) {
    Serial.println("Raining!");
    Blynk.notify(" Raining!,so no need to water");
    delay(1000);
    //send notification

  }
  else if (rainState == 1) {
    Serial.println("Not Raining");
    delay(1000);
  }
  else {
    delay(100);
  }


  if (pinValue == HIGH)
  {
    getPirValue();
  }


  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (client.connect(server, 80))  //   "184.106.153.149" or api.thingspeak.com
  {

    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(rainState);
    postStr += "&field4=";
    postStr += String(pirValue);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" degrees Celcius, Humidity: ");
    Serial.print(h);
    Serial.println("%");
    Serial.print("rain-staus");
    Serial.print(rainState);
    Serial.print("movement status");
    Serial.print(pirValue);
    Serial.println("Send to Thingspeak.");
    

  }
  client.stop();
  

  Serial.println("Waiting...");

  // thingspeak needs minimum 15 sec delay between updates
  delay(1000);



}
