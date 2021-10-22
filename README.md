# MQB steering wheel adapter
*(retrofit MQB steering wheel into PQ46 platform - VW Passat b6/b7, CC)*

The adapter supports on-wheel buttons (media and cruise control). Under the hood it intercepts LIN bus between the steering control unit and the buttons on the wheel. It converts pressed button code from MQB to PQ, accepable by the steering control unit. Also it replaces cruise control lever, converting pressed buttons code on the wheel into the lever signals.

## What versions of steering control units and wheels are supported?
Basically any steering control unit of PQ46 platform that supports on-wheel buttons and cruise control. And steering wheels of MQB platform which match the images in 'firmwares' directory.

The hardware part is based on Arduino Pro Mini (16MHz 5V) and uses couple of optocouplers, LIN transmitters and resistors to fit into car's interface. I share Gerber files, list of components and installation manual.

Software is presented as compiled firmware files. The firmwares are different depending on the steering wheen buttons.

## How to assemble the adapter:
- buy the following parts:
  - Arduino Pro Mini 16MHz 5V
  - 2 optocouplers LTV-847S
  - 2 LIN transmitters MCP2003A-E/SN
  - 5V voltage regulator L7805CV
  - 2 100R resistors in 1206 SMD package
  - 2 150R resistors in 1206 SMD package
  - 1 180R resistor in 1206 SMD package
  - 3 240R resistors in 1206 SMD package
  - 8 330R resistors in 1206 SMD package
  - 3 390R resistors in 1206 SMD package
  - 1 4K7 resistor in 1206 SMD package
  - 2-pin 2.54mm Dupont Connector (https://www.aliexpress.com/item/32905764355.html)
  - 6-pin Edge Card Connector Slot 2.54 Mm Pitch (https://www.aliexpress.com/item/4000573796772.html) - You'll have to cut down 6-pin connector to fit it into the steering control unit nest, or you can skip this item and reuse a connector from your factory cruise control lever
- make the PCB based on the Gerber files
- solder all stuff to get this result:

![adapter front](https://user-images.githubusercontent.com/5708028/138508025-40673c90-a3f3-4b15-9137-88e356b51d86.jpg)
![adapter rear](https://user-images.githubusercontent.com/5708028/138508037-250900c1-fe1d-4c1f-a028-7e979f6e01ea.jpg)


## How to burn a firmware:
- first, find the firmware fitting your steering wheel buttons. Look through images in each firmware directory
- upload the firmware to Arduino using any available programmator


### Follow the manual to install the adapter into your car:
https://github.com/v-ivanyshyn/mqb-steering-wheel-adapter/blob/main/Manual%20EN.pdf
