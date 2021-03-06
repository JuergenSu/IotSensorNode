# IotSensorNode
prototype for a NodeMCU based sensor node exposing temperature, Humidity and a 433 MHZ Controller via REST like interface

## neede hardware
- NodeMCU or compatible
- 433 mhz reciever and sender for example DEBO 433 RX/TX
- BMP280 Sensor
- Boost converter for example MT3608 


## supported endpoints
### http://__[ IP ]__/api/v1/rcswitch/read
This endpoints waits 10 seconds for a 433 mhz signal and returns the recieved values
### http://__[ IP ]__/api/v1/rcswitch/send?code=5204
This endpoint sends the 24 bit 433 mhz signal represented be 5204
### http://__[ IP ]__/api/v1/rcswitch/send?code=5201
This endpoint sends the 24 bit 433 mhz signal represented be 5201
### http://__[ IP ]__/api/v1/environment
This endpoint return the environmental data read from bmp280 sensor
### http://__[ IP ]__/

## Future usage
this node is intendet as an Endpoint for home automation. the 433 MHZ interfaces are intendet to replace time controlled switches. Possible usecases are:

- switching lights on and of whe the sun goes down.
- basic building block for home automation
- what ever i would build of...

## Wiring

| PIN | Wiring | Device   |
| ----- | --------- | ----------- |
| D1  | SCL     | BME 280  |
| D2  | SDA     | BME 280  |
| D8  | DATA    | 433 MHZ Send |
| D4  | DATA    | 433 MHZ Recieve |

## Power Supply
You should consider using a boost converter to power the 433 MHZ sender otherwise range will be limited. NodeMCU modules can typically handle more than 3.3 V at the VIN pin, check the builtin voltage regulator, mine runs fine at 9V. Morover an antenna for the 433 MHZ sender could boost range a lot 
