#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <ThingSpeak.h>
#include <DHT.h>


//WIFI INFO
const char* ssid = "samsungon6";
const char* password = "abcd1234";
//THINKSPEAK INFO
unsigned long channelNo = 1034128;
const char * api = "1E5LWOBCKUOE1J5M";

#define DHTPIN D1     
#define DHTTYPE    DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

float t = 0.0;
float h = 0.0;

AsyncWebServer server(80);

unsigned long previousMillis = 0;   
const long interval = 10000;  

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";


String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  return String();
}
int moisturepin = A0;
int led = D0;



float f = dht.readTemperature(true);
float hif = dht.computeHeatIndex(f, h);
float hic = dht.computeHeatIndex(t, h, false);


void setup(){
 
  Serial.begin(115200);
  ThingSpeak.begin(client);
  dht.begin();
  pinMode(led, OUTPUT);
  Serial.println("Project TEST");
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.println(".");
  }

 
  Serial.println(WiFi.localIP());

  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });

  
  server.begin();
}
 
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    
    previousMillis = currentMillis;
   
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      }
    float newF = dht.readTemperature(true);
    // Read temperature as Fahrenheit (isFahrenheit = true)
    if (isnan(newF)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      f = newF;
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      
    }
  
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  
  float moisture_percentage;
  moisture_percentage = ( 100.00 - ( (analogRead(moisturepin)/1024.00) * 100.00 ) );
  
  float moist = analogRead(moisturepin);
  Serial.print("Soil Moisture");
  Serial.print(moist);
  Serial.println("...");
  Serial.print("Soil Moisture(in Percentage) = ");
  Serial.print(moisture_percentage);
  Serial.println("%");

  if( moisture_percentage > 85)
  {
    digitalWrite(led, LOW);
    Serial.println("motor OFF");
  }
  else{
  digitalWrite(led, HIGH);
  Serial.println("motor ON");
  }

    //Thingspeak
    ThingSpeak.setField(1,h);
    ThingSpeak.setField(2,t);
    ThingSpeak.setField(3,moist);
    ThingSpeak.setField(4,moisture_percentage);
    ThingSpeak.setField(5,hic);
    ThingSpeak.writeFields(channelNo,api);
    }
  }
