# ESP8266 Utility Meter monitor

Project is a cheap approach for semi real-time power consumption monitoring based
on led indicator on utility meter. Data is populated via WiFi LAN every predefined time frame using MQTT protocol.

## Problems

* PGE (Polish electricity provider) has installed two-way energy meter
in the cheapest possible version, with disabled communication via optical interface.
* No room in the distribution box to install additional 3-phase smart energy meter.
* No room in wall to install bigger distribution box, due to nearby fresh and waste water installations, additionally wall is made of bricks.

## Why custom solution

* Ready-solutions (i.e. iNode Energy Meter) are expensive, and require additional BLE to LAN gateway
* It was a only a PoC for smart storage of PV energy in utility water tank.

## Solution description

* NodeMCUv3 connecting via WiFi to LAN with home automation
* MQTT protocol for data distribution
* sensor resilient for light noise (sunlight, neighboring led indicators, artificial light in the room)
* cheap
* Powered with 3.3V

### Energy meter

Utility meter installed by PGE is GAMA 350 series

![photo of GAMA 350 utility meter with led indicators and DLMS port marked](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/GAMA350_utility_meter.jpg)

Following important parts are marked:

1. Infrared optical port, DLMS protocol
2. Active power consumption LED indicator (1000 pulses for 1 kilo-Watt hour)
3. Reactive power consumption LED indicator (1000 pulse for 1 kilo-Var hour)

### The sensor

The sensor is based on the light detection resistor (LDR) mounted to monitor active power LED indicator.

![photo of GAMA 350 utility meter LDR mounted opposite to active power LED indicator](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/LDR_mounted.jpg)

#### MOSFET approach

First approach to sensor is based on 2N7000 N-MOSFET.

![schematics of sensor based on 2N7000](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/2N7000_pulse_detector.png)

LDR with the resistance around 100kΩ in the dark in pair with potentiometer RP1 work as a voltage divider. The Gate potential is around the ground, and the MOSFET is cut-off. Resistor R1 works as pull-up. Potential on DOut is VCC. 

When resistance on LDR drops to less than hundred Ohms (utility meter LED indicator is on), almost all VCC voltage is dropped across PR1. The gate has near VCC potential, and the MOSFET is saturated. The current flows through R1 resistor, so voltage on VOut is down to ground. The pulse is sent to the microcontroler.

Pros:
+ low power consumption during: max around 0.1W,
+ compact circuit,
+ very cheap components: around $1.3

Cons:
- slow falling pulses (gentle slope)
- very difficult sensitivity regulation, in practice neighboring LED was causing false impulses.

#### Comparator approach

Final solution is based on LM393 comparator.

![schematics of sensor based on LM393 comparator](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/LM393P_pulse_detector.png)

The comparator's output (Open Collector) is pulled out to VCC via resistor R1, when the voltage dropped across the LDR is greater then the one set on PR1 divider (in dark).

During the utility meter LED's pulse, the voltage's drop across LDR goes to zero, and the comparator's output is shorted to ground.

Pros:

+ high resolution of sensitivity regulation,
+ can be set to ignore pulses from neighboring LED indicator,
+ fast falling pulse (steep slope),
+ still very cheap: around $1.7.

Cons:
- higher power consumption: around 0.33W.


### Sensor adjusting

Sensor must be adjusted individually to utility meter, so it's reading is not interfered with light in the room, or neighboring indicators.

The voltage drop must detect fast changing signal, so no transient states are present on comparator's output.

Using oscilloscope is a must have. Check probe points on schematics below:

![schematics of sensor based on LM393 comparator with scope connection points](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/LM939P_scope.png)

![Screenshot of the scope view vith sensor's pulse](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/active_power_pulse.png)

Regulation must be done using potentiometer PR1, to achieve the following:

1. neighboring LED pulse does not trigger pulse,
![Screenshot of the scope view with neighboring indicator LED on not triggering sensor's pulse](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/passive_power_pulse.png)

2. pulse is triggered on the fast falling sigal,
![Screenshot of the scope view with slow-changing LDR voltage marked](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/LDR_voltage_area_to_low.png)


3. pulse is triggered on as low voltage drop on LDR as possible.
![Screenshot of the scope view with markers on LDR voltage drop that ttriggers sensor's pulse](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/LDR_voltage_treshold.png)

### NodeMCUv3 connection

Circuit requires stabilized 3.3V power supply to work properly, connect it to VCC pins on both circuits. DOut is connected to the D6 pin on NodeMCUv3. GND is common ground, connected to negative connection on power supply.

![Schematics of pinout of NodeMCUv3 with sensor's connection points marked.](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/NodeMCUv3_connection.png)

### Dirty hack on digital pin interrupt

Pulse generated by sensor is mirroring utility meter's LED pulse length. It is too wide for ESP-family hardware interrupt processing logic (30ms).

![Screenshot of the scope view with measurement on sensor's pulse](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/LM393P_pulse_measurement.png)

One pulse was triggering multiple interrupts, due to transient states, [and long-lasting low state](https://github.com/espressif/arduino-esp32/issues/4172).

The solution is to apply mono-stable multivibrator on the output stage, but this will complicate the circuit and increase power consumption.

As a workaround, the dirty hack was added, so the pulse is counted only when:
* logical low state is present on pulse input, and
* after delay of 10ms in the interrupt routine.

```
delay(10);
if (digitalRead(pulse_sensor)==LOW){
    counter++;
}
```

## Results

Measured pulses are counted, converted to consumed power in kilo Watts, and sent via WiFi and MQTT to home automation management software in LAN.

Encryption is not used in current solution, due to negligible security gain and high complication of code because of handling CA certificate's pinning.

![Screenshot of the RRD graph with daily power consumption](https://raw.githubusercontent.com/malipek/esp8266_utility_meter_mon/master/assets/power_consumed-day.png)

Total cost (without power source) is $8.99.

### ToDos:

* Powering the circuit with AGM battery will require voltage monitoring and cut-off when voltage drops below 10.5V; implementation on NodeMCUv3 is needed.
