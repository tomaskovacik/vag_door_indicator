This repository holds all hardware and software part for module which retrofits functionality of indication of opened doors on cluster display on VAG cars (only full display version)

<img src="https://raw.githubusercontent.com/tomaskovacik/vag_door_indicator/master/Example.png?token=ABBFEYHW7Z4TMMCPZMBLWDK7JQABC">

# SW requirements
 - arduino IDE
 - arduino library for reading messages from radio in 3LB protocol in radio mode: https://github.com/tomaskovacik/VAGFISReader
 - arduino library for writing to VAG cluster over 3LB in navigation mode: https://github.com/tomaskovacik/VAGFISWriter
 - arduino sketch: https://github.com/tomaskovacik/vag_door_indicator/tree/master/SW/VAG_DOOR_INDICATOR

# HW requirements:
Arduino compatible board which can survive 14.4V as input on Vin (I use my own boards with AM1117-5.0 which has 18V max input voltage).
Diodes on inputs are used as protection against accidential applying of 12V (simple 1n4148,1n4007 or similar will do). Also as original wiring in car puts all switches in door at parallel, four diodes are requiered for spliting this signal

# Wiring in car

<img src="https://raw.githubusercontent.com/tomaskovacik/vag_door_indicator/master/how_to_connect.png"/>

# Custom picture on startup

- black and white bitmat 64x88 (WxH) pixels
- convert it to C array using this site: https://www.digole.com/tools/PicturetoC_Hex_converter.php
- copy C array into b5f_orezana varialble in bitmaps.h file, compile upload

# Avant / Sedan picture

 change variable at line 33 to SEDAN or AVANT:
 ```
 bool model = AVANT;
 ```
  ```
 bool model = SEDAN;
 ```
 In source code is commented out detection of model, this can be used, then A0 is used to select model.
 
# Comunication protocol with cluster

Module is using navigation mode to display pictures using <a href="https://github.com/tomaskovacik/VAGFISWriter">VAGFISWriter</a> library, because of this, clasic radion protocol is not functional anymore and stations are will not be displayed on cluster. The module is using <a href="https://github.com/tomaskovacik/VAGFISReader">VAGFISReader</a> library to read data from radio and translating them into navigation protocol.  More about these protocols on <a href="https://github.com/tomaskovacik/vag_door_indicator/wiki/Theory-of-operation">this wiki page</a>. Even more in <a href="https://github.com/tomaskovacik/VAGFISWriter/blob/master/src/VAGFISWriter.cpp">this source code</a>.

