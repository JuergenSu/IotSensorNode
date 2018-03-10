# IotSensorNode
prototype for a LAN Based sensor node for Arduino exposing temperature, Humidity and a 433 MHZ Controller via REST interface

## neede hardware
- Arduino UNO
- Ethernet Shied
- 433 mhz reciever and sender
- BMP280 Sensor


## supported endpoints
### http://__[ IP ]__/api/v1/rcswitch/read
This endpoints waits 10 seconds for a 433 mhz signal and returns the recieved values
### http://__[ IP ]__/api/v1/rcswitch/send/5204
This endpoint sends the 24 bit 433 mhz signal represented be 5204
### http://__[ IP ]__/api/v1/rcswitch/send/5201
This endpoint sends the 24 bit 433 mhz signal represented be 5201
### http://__[ IP ]__/api/v1/environment
This endpoint return the environmental data read from bmp280 sensor
### http://__[ IP ]__/

## Future usage
this node is intendet as an Endpoint for home automation. the 433 MHZ interfaces are intendet to replace time controlled switches. Possible usecases are:

- switching lights on and of whe the sun goes down.
- basic building block for home automation
- what ever i would build of...
