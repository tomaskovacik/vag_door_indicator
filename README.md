This repository holds all hardware and software part for module which retrofits functionality of indication of opened doors on cluster display on VAG cars (only full version display)

<img src="https://raw.githubusercontent.com/tomaskovacik/vag_door_indicator/master/Example.png?token=ABBFEYHKUPULXR4OADIJJGS7JP6B6">

# SW requirements
 - arduino IDE
 - arduino library for reading radio version of 3LB protocol: https://github.com/tomaskovacik/VAGFISReader
 - arduino library for writing to VAG cluster over 3LB (navigation protocol): https://github.com/tomaskovacik/VAGFISWriter
 - arduino sketch: https://github.com/tomaskovacik/vag_door_indicator/tree/master/SW/VAG_DOOR_INDICATOR

# HW requirements:
Arduino compatible board which can have 14.4V as input on Vin (I use my own boards with AM1117-5.0 which has 18V max input voltage).
Diodes on inputs are used as protection against accidential applying of 12V. Also as original wiring puts all switches in door at parallel, four diodes are requiered for spliting this signal


TODO: put picture here!

todo add :
- custom picture at start - how-to add custom one
- module selection (or should we made sedan/avant firmware?
- talk about/link to 3LB ?

