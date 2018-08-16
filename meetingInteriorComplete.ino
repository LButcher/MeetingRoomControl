#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
#include <Adafruit_NeoPixel.h>
#include <Math.h >



#define PIN D7

// How many NeoPixels are attached to the Arduino?
# define NUMPIXELS 24

int lastFillPixels = 0;
int fillPixels = 0;
int givenSize = 0;


// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 100; // delay for half a second

const char* ssid = "121King5_GOOD";
const char* password = "BronBronIn7";
const char* mqttServer = "192.168.0.19";
const int mqttPort = 1883;
const char * clientName = "RoomNode1Interior";
const char * topic = "11West14Interior";

WiFiClient espClient;
PubSubClient client(espClient);

void ConnectWifi(const char * ssid,
  const char * password)
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to Wifi..");
  }
  Serial.println("Connected to network");
}

void ConnectBroker(PubSubClient client,
  const char * clientName)
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT: ");
    Serial.println(clientName);
    if (client.connect(clientName))
    {
      Serial.println("Connected");
    }
    else
    {
      Serial.print("Failed with state ");
      Serial.println(client.state());
      delay(200);
    }
  }
}

void updateRing(int givenSize)
{
  float multiplier = 10.625; //=255/24
  if(givenSize==0){
      fillPixels = 0;
  }
  else{
    fillPixels = round(givenSize/multiplier);
  }
  
  if (fillPixels<lastFillPixels || fillPixels==0)
  {
    clearColour();
  }

  colourRing(fillPixels);


  lastFillPixels = fillPixels;
}

void colourRing(int fillPixels)
{
  
    for (int i = 0; i < fillPixels; i++)
    {
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255

      pixels.setPixelColor(i, pixels.Color(i * 3, 25 - i, 0)); // Moderately bright green color.
      
      pixels.show(); // This sends the updated pixel color to the hardware.

    }


}


void clearColour()
{
  for (int i = NUMPIXELS; i >= fillPixels; i--)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show(); // This sends the updated pixel color to the hardware.
  }

}

void resetRelay(){
  digitalWrite(D8,HIGH);
  delay(2000);
  digitalWrite(D8,LOW);
  delay(50);
}

void callback(char * topic, byte * payload, unsigned int length2)
{

  StaticJsonBuffer <300> JSONbuffer;

  payload[length2] = 0;
  String inData = String((char * ) payload);
  JsonObject & root = JSONbuffer.parseObject(inData);
  //  if(root["room"]=="room1"){
  String type = root["type"];

  
  if(type=="int"){
     updateRing(root["details"]);
  }
  else{
    resetRelay();
  }



  //}
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (client.connect(clientName))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //  client.publish("Hello World");
      // ... and resubscribe


    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{

  Serial.begin(115200);
  pinMode(D8, OUTPUT);
  pixels.begin(); // This initializes the NeoPixel library.
  ConnectWifi(ssid, password);
  client.setServer(mqttServer, mqttPort);
  ConnectBroker(client, clientName);
  client.setCallback(callback);
  client.subscribe(topic);

  client.loop();
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
    client.subscribe(topic);
  }

  //colourRing(5,15);
  //updateRing(givenSize);
  //updateRing(meetingLength,timeLeft);
  client.loop();
  delay(25);
}
