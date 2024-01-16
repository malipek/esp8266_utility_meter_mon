# ESP8266 Utility Meter monitor

Project is a cheap approach for semi real-time power consumption monitoring based
on led indicator on utility meter. Data is populated via WiFi LAN every predefined time frame using MQTT protocol.

## Problems

* PGE (Polish electricity provider) has installed two-way energy meter
in the cheapest possible version, with disabled communication via optical interface.
* No room in the distribution box to install additional 3-phase smart energy meter.
* No room in wall to install bigger distribution box, due to nearby fresh and waste water installations, additionally wall is made of bricks.

## Why custom solution

* Ready-solutions (i.e. iNode Energy Meter) are expensive, and require additional BLE to LAN gateway.
* It was a only a PoC for smart storage of PV energy in utility water tank.

## Solution

* NodeMCUv3 connecting via WiFi to LAN with home automation.
* MQTT protocol for data distribution.
* Sensor resilient for light noise (sunlight, neighboring led indicators, artificial light in the room).
* Cheap.
* Powered with 3.3V.

### Table of contents

* [Energy meter](https://github.com/malipek/esp8266_utility_meter_mon/blob/master/SOLUTION.md#energy-meter)
* [The sensor](https://github.com/malipek/esp8266_utility_meter_mon/blob/master/SOLUTION.md#the-sensor)
  * [MOSFET approach](https://github.com/malipek/esp8266_utility_meter_mon/blob/master/SOLUTION.md#mosfet-approach)
  * [Comparator approach](https://github.com/malipek/esp8266_utility_meter_mon/blob/master/SOLUTION.md#comparator-approach)
  * [Dirty hack on digital pin interrupt](https://github.com/malipek/esp8266_utility_meter_mon/blob/master/SOLUTION.md#dirty-hack-on-digital-pin-interrupt)
  * [Final: comparator with integrator and inverter](https://github.com/malipek/esp8266_utility_meter_mon/blob/master/SOLUTION.md#comparator-with-integrator-and-inverter)
* [Sensor adjusting](https://github.com/malipek/esp8266_utility_meter_mon/blob/master/SOLUTION.md#sensor-adjusting)
* [NodeMCUv3 connection](https://github.com/malipek/esp8266_utility_meter_mon/blob/master/SOLUTION.md#nodemcuv3-connection)

## Results

Measured pulses are counted, converted to consumed power in kilo Watts, and sent via WiFi and MQTT to home automation management software in LAN.

Encryption is not used in current solution, due to negligible security gain and high complication of code because of handling CA certificate's pinning.

![Screenshot of the RRD graph with daily power consumption](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/power_consumed-day.png)

Total cost (without power source) is $9.49.

### ToDos:

* Powering the circuit with AGM battery will require voltage monitoring and cut-off when voltage drops below 10.5V; implementation on NodeMCUv3 is needed.
