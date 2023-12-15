
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "credentials.h"


#define LED  0

#define DHTTYPE   DHT11

#define DHTPIN 2

/*#define ONE_WIRE_BUS 2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);*/


#define USER_PWD_LEN 40

const char* host = "api.thingspeak.com";
String writeAPIKey = "TQVK604D9AF8ZIG2";

const char* ssid = WIFI_SSID ;
const char* password = WIFI_PASSWD;

float temperatura = 0;
float humedad = 0;
char unameenc[USER_PWD_LEN];
float oldTemp;
bool bActualiza = true;
const long timerUpdate =3600L;//1h

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  delay(1000);

  dht.begin();


  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(TimingISR);
  timer0_write(ESP.getCycleCount() + 80000000L); // 80MHz == 1sec
  interrupts();

  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  wifiConnect();

}

void loop() {

  float temp ;

  if (bActualiza) {

    //do {
     /* DS18B20.requestTemperatures();
      temp = DS18B20.getTempCByIndex(0);
      Serial.print("Temperatura: ");
      Serial.println(temp);*/
      humedad     = dht.readHumidity();
      temperatura = dht.readTemperature();

      Serial.print("Temperatura:");
      Serial.println(temperatura);
      Serial.print("Humedad:");
      Serial.println(humedad); 

      digitalWrite(LED, LOW); //flashing led
      delay(500);
      digitalWrite(LED, HIGH);
      
      sendTemperatura(temperatura);
  
    bActualiza = false;
  }

}
//********************************************************************************************************************************
void wifiConnect()
{
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("*");

    if (++timeout > 100)
    {
      Serial.println("Sin Conexion WIFI");
      while (1) {
        digitalWrite(LED, LOW);
        delay(100);
        digitalWrite(LED, HIGH);
        delay(100);
      }
    }

  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


}
//***************************************************************************************************************************
void sendTemperatura(float Temperatura)
{


  if (WiFi.status() == WL_CONNECTED) { 

    HTTPClient http;
    String cmd = "http://api.thingspeak.com/update?key=";
    cmd += writeAPIKey;
    cmd += "&field4=";
    cmd += Temperatura;

    http.begin(cmd);
    Serial.print ("Sending: ");
    Serial.println (cmd);

    int httpCode = http.POST(cmd);

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();                  //Get the response payload
      Serial.println (payload);
    }
    else
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());

   

    http.end();
  }
  
  Serial.println();
  Serial.println("Connection closed");


}
//**********************************************************************************************************************
void TimingISR() {

  static long cntTemp = 0L;

  if (++cntTemp > timerUpdate)
  {
    bActualiza = true;

    cntTemp = 0L;
  }

  timer0_write(ESP.getCycleCount() + 80000000L); // 80MHz == 1sec

}

