
#include <SPI.h>
#include <Ethernet.h>
#include <BME280I2C.h>
#include <Wire.h>
#include <RCSwitch.h>


//define Switch 
RCSwitch mySwitch = RCSwitch();


BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
                  

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

// COntainer for the HTTP request
String HTTP_req;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac);
  server.begin();
  Serial.print(F("server is at "));
  Serial.println(Ethernet.localIP());

  //Init i2c interface
  Wire.begin();

  while(!bme.begin())
  {
    Serial.println(F("Could not find BME280 sensor!"));
    delay(1000);
  }

  // bme.chipID(); // Deprecated. See chipModel().
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
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  mySwitch.enableTransmit(7);
  //strom für den Sender  
  pinMode(5, OUTPUT); 
  digitalWrite(5, LOW); // GND 0V
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); // VCC 5V
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean firstline=true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (firstline && c != '\n') { 
          HTTP_req += c;
        }
        if (c == '\n') {
          firstline=false;
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
        
          Serial.println(HTTP_req);  
          if (HTTP_req.indexOf("/api/v1/environment") > -1) {
            //BME Auslesen und in JSON Verpacken 
            
            Serial.println("-> Temp");
            // send a standard http response header
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: application/json"));
            client.println(F("Connection: close"));  // the connection will be closed after completion of the response
            client.println();

            float temp(NAN), hum(NAN), pres(NAN);
            
            BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
            BME280::PresUnit presUnit(BME280::PresUnit_Pa);
            
            bme.read(pres, temp, hum, tempUnit, presUnit);
    
            client.print(F("{")); 
            client.print(F("    \"temp\": { \"value\":\""));
            client.print(temp);
            client.print(F("\", \"unit\":\""));
            client.print("°"+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F'));
            client.print(F("\"}, "));
            client.print(F("    \"humidity\": {\"value\":\""));
            client.print(hum);
            client.print(F("\", \"unit\":\"% RH\"}, "));
            client.print(F("    \"Pressure\": { \"value\":\""));
            client.print(pres);
            client.print(F("\", \"unit\":\"Pa\"}"));
            client.print(F("}"));
         
          } else if (HTTP_req.indexOf(F("/api/v1/rcswitch/send")) > -1) {
            // 24 Bit 433 MHZ senden
            mySwitch.send(HTTP_req.substring(26,30).toInt(), 24);
            delay(500);
            Serial.println("-> SENT");
            client.println(F("HTTP/1.1 204 NO CONTENT"));
            client.println(F("Connection: close"));  // the connection will be closed after completion of the response
            client.println();

          } else if (HTTP_req.indexOf(F("/api/v1/rcswitch/read")) > -1) {
              // 10 Sekunden auf ein 433 MHZ Signal warten und dann zurück liefern
              int timeout=100;
              while (!mySwitch.available() && timeout > 0) {
                delay(100);
                timeout--;
              }
              if (mySwitch.available()) {
                
                Serial.println("-> RC_READ");
                client.println(F("HTTP/1.1 200 OK"));
                client.println(F("Content-Type: application/json"));
                client.println(F("Connection: close"));  // the connection will be closed after completion of the response
                client.println();
                int value = mySwitch.getReceivedValue();
                
                if (value == 0) {
                  client.print(F("{\"value\":\"Unknown encoding\"}"));
                } else {
                  client.print(F("{\"value\":\""));
                  client.print(mySwitch.getReceivedValue());
                  client.print(F("\", \"bit\":\""));
                  client.print(mySwitch.getReceivedBitlength());
                  client.print(F("\", \"Protocol\":\""));
                  client.print(mySwitch.getReceivedProtocol());
                  client.print(F("\"}"));                  
                }
            
                mySwitch.resetAvailable();
              }
          } else  {
            // den API Index ausliefern
            Serial.println("-> INDEX");
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: application/json"));
            client.println(F("Connection: close"));  // the connection will be closed after completion of the response
            client.println();
            client.println(F("{\"temp\":\"GET /api/v1/environment\", \"switch_read\":\"GET /api/v1/rcswitch/read\", \"switch_send\":\"GET /api/v1/rcswitch/send/<value>\"}"));

          } 
            


          HTTP_req = "";
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
  }
}

