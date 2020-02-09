// Pulls message from Azure Service bus and flash the corspondig effect and color on the led stribe.
// By gfjakobs - 2020

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "FastLED.h"

WiFiClientSecure client;


// Azure Service Bus Settings
const char* AzUri = "<replace>.servicebus.windows.net";
const char* AzEndPoint = "/<replace>/messages";
const char* AzContentType = "application/atom+xml;type=entry;charset=utf-8";
const char* AzAuthSend = "SharedAccessSignature <replace>";
const char* AzAuthRead = "SharedAccessSignature <replace>";

// Thumbprint of the the cert used for https://<replace>.servicebus.windows.net
const char Thumbprint[] PROGMEM = "ad 72 5d ba 74 78 de 22 a3 b5 e7 53 8b 1b 94 87 c5 d2 3e 94";

// Connect to Wifi
#define ssid      "<replace>"       // WiFi SSID
#define password  "<replace>"        // Password

#define NUM_LEDS      60
#define LED_TYPE   WS2811
#define COLOR_ORDER   GRB
#define DATA_PIN        D1
//#define CLK_PIN       4
#define VOLTS          5
#define MAX_MA       4000
CRGBArray<NUM_LEDS> leds;

void ReadAZ(){
  // Read from Azure ServiceBus
  
  Serial.printf("Using Cert Thumbprint '%s'\n", Thumbprint);
  client.setFingerprint(Thumbprint);
  
  if (!client.connect(AzUri, 443)) {
    Serial.println("connection failed");
    // return;
  }

  // Create request to pull message from ServiceHub Rest
  // Ref https://docs.microsoft.com/en-us/rest/api/servicebus/delete-message
  String delrequest =String("DELETE ") + AzEndPoint + "/head?timeout=30 HTTP/1.1\r\n" +
               "Host: " + AzUri + "\r\n"+
               "Authorization:" + AzAuthRead + "\r\n" +
               "Content-Type: "+ AzContentType + "\r\n" +
               "Connection: close\r\n\r\n";
 
  //Serial.println(delrequest);
  client.print(delrequest);

  String response = "";
  String json = "";
  bool getjson = false;
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    //Serial.println(line);
    response += line;
    if (line == "{\r" ){getjson = true;}
    if (getjson == true){
      json += line;
    }    
    if (line == "}\r" ){getjson = false;}    
  }

  Serial.println();
  Serial.print("Response code DELETE opeation: ");
  Serial.println(response.substring(9, 12));
  
  //Serial.println(response);
  //Serial.println(body);

  if (response.substring(9, 12) == "200"){
    // Response 200, new message turn on led
    const size_t capacity = JSON_OBJECT_SIZE(4) + 100;
    
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, json);

    const char* ColorHTML = doc["Color"];
    const char* Created = doc["Created"];
    const char* Effect = doc["Effect"];
    const char* Env = doc["Env"];
    
    Serial.print("Created: ");
    Serial.println(Created);
    Serial.print("Env: ");
    Serial.println(Env);
    Serial.print("Color: ");
    Serial.println(ColorHTML);
    Serial.print("Effect: ");
    Serial.println(Effect);

    StripeEffect(Effect,ColorHTML);

    }
}

void PostAZ(){
  // Post message to Azure ServiceBus
  
  Serial.printf("Using Cert Thumbprint '%s'\n", Thumbprint);
  client.setFingerprint(Thumbprint);
  
  if (!client.connect(AzUri, 443)) {
    Serial.println("connection failed");
    // return;
  }
  String data = "{Created: '08.02.2020 15.00.17',Env: 'DRM',Color: 'Red',Effect: 'Fade'}";
  
  String request = String("POST ") + AzEndPoint + " HTTP/1.1\r\n" +
               "Host: " + AzUri + "\r\n" +
               "Authorization:" + AzAuthSend + "\r\n" +
               "Content-Type: "+ AzContentType + "\r\n" +
               "Content-Length: " + data.length() + "\r\n\r\n" +
               data +
               "Connection: close\r\n\r\n";
                                
  //Serial.println(request);  
  client.print(request);
  String response = "";
  
  while (client.connected()) {
    response += client.readStringUntil('\n');
  }

  Serial.println();
  Serial.print("Response code PUT Operation: ");
  Serial.println(response.substring(9, 12));  
}

void StripeEffect(const char* Effect, const char* ColorHTML){
  //Covert string color to HEX
  uint32_t ColorHex = strtol(ColorHTML,NULL,16);

  if (strcmp(Effect,"Wipe")==0){
    //Wipe the Color on and off
    Serial.println("Led Effect: Wipe");
    for(int Led = 0; Led < NUM_LEDS; Led = Led + 1) {      
      leds[Led] = ColorHex ;
      FastLED.show();
      delay(10);
      }
    for(int Led = 0; Led < NUM_LEDS; Led = Led + 1) {      
      leds[Led] = CRGB::Black;
      FastLED.show();
      delay(10);
      }
  }
  else if (strcmp(Effect,"Strobe")==0){
    // Blink all led fast 10 times.
    Serial.println("Led Effect: Strobe");
    for(int j = 0; j < 10;j++){    
      for(int Led = 0; Led < NUM_LEDS; Led = Led + 1) {      
        leds[Led] = ColorHex ;
      }
      FastLED.show();
      delay(50);
      for(int Led = 0; Led < NUM_LEDS; Led = Led + 1) {      
        leds[Led] = CRGB::Black ;
      }
      FastLED.show();
      delay(50);
    }             
  }

  else if (strcmp(Effect,"Metor")==0){
    Serial.println("Led Effect: Metor");

    for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
      
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!true) || (random(10)>5) ) {
        //fadeToBlack(j, meteorTrailDecay );
        leds[j].fadeToBlackBy( 64);        
      }
    }
   
    // draw meteor
    int meteorSize = 2;
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        leds[(i-j)] = ColorHex;
      }
    }
   
    FastLED.show();
    delay(30);
    }

    for(int Led = 0; Led < NUM_LEDS; Led = Led + 1) {      
        leds[Led] = CRGB::Black ;
      }
    FastLED.show();
      
  }// End Metor  

}// End StripeEffect

  


void setup() {
  // Serial debug
  Serial.begin(115200);

  //Connect to Wifi 
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to Wifi.");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  Serial.println("Initialize Led stripe");
  FastLED.setMaxPowerInVoltsAndMilliamps( VOLTS, MAX_MA);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  Serial.println("Led stripe ready");  
}

void loop() {
  Serial.println("*******************************************************");
  ReadAZ();
  delay (5000);  //Wait 5 seconds
  //PostAZ();
  

  /*<POST
  
  */

  
  
  //Serial.println(ssid);

  

}
