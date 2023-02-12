# SDM120 Single Phase Energy Meter to MQTT Gateway
For me next project, I took my earlier Modbus RTU to MQTT projects and modified it to work with the Eastron SDM 120 single phase energy meter. This sketch reads the voltage, current, active power, power factor, frequency and total active energy readings from the energy meter and publishes the data over MQTT.

![Setup](/img/board.jpg)

I will use this project to monitor the AC charging of my electric car.

The ESP8266 microcontroller besides handling the modbus and MQTT communication also provides a `http://ip/status` endpoint where all the information is published in JSON format. The ESP also servers a simple webpage that shows data real time.

![Web and MQTT communication](/img/webandmqtt.jpg)

![Status JSON message](/img/status.jpg)

## Libraries
The following libraries are required for this project. These are all listed in the Library Manager within Arduino IDE:
- FastLED by Daniel Garcia
- ModbusMaster by Doc Walker
- ArduinoOTA
- SoftwareSerial

## Parts
You will need the following parts to build this project:
- Wemos D1 Mini ESP8266 module (clone)
- RS-485 to TTL Converter Module: https://www.aliexpress.com/item/1005001621798947.html
- 5.08mm Pitch Panel KF301-3P Screw Terminal Block PCB Connector
- 4 pin DC-DC buck converter power supply module. The module I used looks like is no longer available, but I found a similar module: https://www.aliexpress.com/item/1005003512429148.html. This is configurable for different voltages with solderpads in the back, but the default is 5V.
- WS2812 Neopixel module: https://www.aliexpress.com/item/33026835790.html. This is optional, the sketch works without the neopixel as well

## Install your own version
1. Download the required libraries (see above) and download this project.
2. Set up the modbus communication speed and slave ID of the unit. The documentation is attached, it explains how to change the values on the screen. The default baud rate is 2400. I tested mine with 9600, I recommend changing it to 9600. Slave ID can remain 1.
2. Open the `settings.h` 
  - Set the MODBUS_RATE and SLAVE_ID values to what you set on your unit
  - Further down set up the ssid, password and MQTT details. 
  - If you want dynamix IP comment the '#define FIXEDIP   1` line, otherwise set the IP address details as shown.
3. Build and upload. This is not a big sketch, does not require any special partition.

## Board
To make this project work (modbus communnication and the web bits (MQTT, HTTP) you can use both of my version 1 (https://www.pcbway.com/project/shareproject/ESP8266_Modbus_board.html) and version 2 (https://www.pcbway.com/project/shareproject/Modbus_to_MQTT_board_version_2_338ed64e.html) modbus PCB modules as well. I explained the differences between them in the version 2 PCBWay project page and also in the video. Feel free to order and of the modules and the same code will work on both.

## Video
Youtube video on this project (coming soon):
[![Prototype](https://img.youtube.com/vi/Mz1dJGthIJk/0.jpg)](https://www.youtube.com/watch?v=Mz1dJGthIJk)
