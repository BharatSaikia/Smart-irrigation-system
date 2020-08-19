#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

char auth[] = "7Lz2xEgO35Q3augUY7iAzAs6-mBccuuV";
char ssid[] = "samsungon6";
char pass[] = "abcd1234";

#define DHTPIN D1
#define DHTTYPE DHT11 

int sensorpin=A0;
int sensorvalue=0;
int outputvalue=0;

 
WidgetLED led1(V1);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

//LED
void blinkLedWidget()
{
  if (led1.getValue()) {
    led1.off();
    
  } else {
    led1.on();
    
  }
}

//DHT11

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
}

//moisture

void moisture()
{
  sensorvalue=analogRead(sensorpin);
  outputvalue=map(sensorvalue,0,1023,0,100); //100,0 pottupaaru
  delay(1000);

 if(outputvalue>74)
  {
       Serial.println("water your plant");
       Serial.print(outputvalue);
       Blynk.notify("Plants need Water, Switch on the motor");
       delay(1000); 
  }
  else if(outputvalue<45)
  {
       Serial.println("soil is wet");
       Serial.print(outputvalue);
       Blynk.notify("soil is wet , switch off motor");
       delay(1000);
  }
   Blynk.virtualWrite(V2,outputvalue);
}

void setup()
{
 
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  
  dht.begin();
  timer.setInterval(1000L, blinkLedWidget);
  timer.setInterval(1000L, sendSensor);
  timer.setInterval(1000L, moisture);
}

void loop()
{
  
  Blynk.run();
  timer.run();
}
