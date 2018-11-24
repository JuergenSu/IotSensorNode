
#include <SPI.h>
#include <BME280I2C.h>
#include <Wire.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "config.h"

//define Switch 
RCSwitch mySwitch = RCSwitch();


BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
                  

//instantiate server at port 80 (http port)
ESP8266WebServer server(80); 

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Switch of internal default AP and switch to Client Only Mode
  WiFi.mode(WIFI_STA);

  // start the WIFI connection
  WiFi.begin(ssid, password); 
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

  //Init i2c interface
  Wire.begin();

  while(!bme.begin())
  {
    Serial.println(F("Could not find BME280 sensor!"));
    delay(1000);
  }


  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println(F("Found BME280 sensor! Success."));
       break;
     case BME280::ChipModel_BMP280:
       Serial.println(F("Found BMP280 sensor! No Humidity available."));
       break;
     default:
       Serial.println(F("Found UNKNOWN sensor! Error!"));
  }

  //RCSwitch enablen
  //Warnign RC Switch uses GPIO Numbering not PIN numbers
  mySwitch.enableReceive(2);  // Receiver on GPIO 2 => that is pin #4
  mySwitch.enableTransmit(15); // Sender on GPIO 15 => that is pin #8


  server.on("/", [](){

    // den API Index ausliefern
    Serial.println("-> INDEX");

    server.sendHeader("Connection", "close");
    server.send(200, "application/json", "{\"temp\":\"GET /api/v1/environment\", \"switch_read\":\"GET /api/v1/rcswitch/read\", \"switch_send\":\"GET /api/v1/rcswitch/send?code=<value>\"}");
            
  });

  // Send an 433 MHZ  24 Bit Code represented by a integer
  
  server.on("/api/v1/rcswitch/send", [](){           
    Serial.println("-> RC SEND");
    String code = server.arg("code");
    Serial.println("-> code = " + code);
    
    server.sendHeader("Connection", "close");
    mySwitch.setPulseLength(605);
    mySwitch.setProtocol(1);
    mySwitch.send(code.toInt(), 24);
    delay(500);
    Serial.println("-> SENT");

    server.send(200, "application/json", "{\"OK\"}");
  });

  // Puts the Server in listen Mode for 10 Seconds to recieve a codesequence
  server.on("/api/v1/rcswitch/read", [](){
    Serial.println("-> RC_SCAN");
    int timeout=100;
      while (!mySwitch.available() && timeout > 0) {
        delay(100);
        timeout--;
      }
      String result="";
      if (mySwitch.available()) {
        
        Serial.println("-> RC_READ");
        server.sendHeader("Connection", "close");
        int value = mySwitch.getReceivedValue();
        
        if (value == 0) {
          result = "{\"value\":\"Unknown encoding\"}";
        } else {
          result= "{\"value\":\"" + 
            String(mySwitch.getReceivedValue()) + 
            "\", \"bit\":\"" + 
            String(mySwitch.getReceivedBitlength()) + 
            "\", \"Protocol\":\"" + 
            String(mySwitch.getReceivedProtocol()) + 
            "\"}";                  
        }
       
        server.send(200, "application/json", result); 
        mySwitch.resetAvailable();
      } else {
        server.send(200, "application/json", "{\"error\"}"); 
      }
       
       

  });


  // intendet for environment informations and for checking for overheating
  server.on("/api/v1/environment", [](){           
    //Read BME values and send respose
  
            
    Serial.println("-> Temp");
    // send a standard http response header
    server.sendHeader("Content-Type", "application/json");
    server.sendHeader("Connection", "close");

    float temp(NAN), hum(NAN), pres(NAN);
            
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    
    bme.read(pres, temp, hum, tempUnit, presUnit);

    server.send(200, "application/json",      
            "{   \"temp\": { \"value\":\" " + 
            String(temp) + 
            "\", \"unit\":\"°"+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F') + 
            "\"}, \"humidity\": {\"value\":\"" + 
            String(hum) + 
            "\", \"unit\":\"% RH\"},  \"Pressure\": { \"value\":\"" + 
            String(pres) + 
            "\", \"unit\":\"Pa\"}}"
     );
   
  });




  server.begin();
  Serial.println("Web server started!");

  
}


void loop() {

  server.handleClient();
}

