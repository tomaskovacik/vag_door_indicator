 #include <VAGFISReader.h>
#include <VAGFISWriter.h>
#include "bitmaps.h"

#define RADIOIN_ENA 2
#define RADIOIN_CLK 3
#define RADIOIN_DATA 4

#define RADIOOUT_ENA 7
#define RADIOOUT_CLK 8
#define RADIOOUT_DATA 9

#define TOUPPER 10
//#define MODE  // 0 = radio, 1 = navi
#define MODEL A0 // 0 = avant, 1 = sedan
#define DOOR_FRONT_LEFT A1 // 0 = opened, 1 = closed
#define DOOR_FRONT_RIGHT A2 // 0 = opened, 1 = closed
#define DOOR_REAR_LEFT A3 // 0 = opened, 1 = closed
#define DOOR_REAR_RIGHT A4 // 0 = opened, 1 = closed
#define DOOR_TRUNK A5 // 0 = opened, 1 = closed

#define SEDAN 0
#define AVANT 1
#define RADIO 0
#define NAVI 1
#define OPENED 0
#define CLOSED 1

VAGFISReader radio_read(RADIOIN_CLK, RADIOIN_DATA, RADIOIN_ENA);
VAGFISWriter radio_write(RADIOOUT_CLK, RADIOOUT_DATA, RADIOOUT_ENA);
long last_update = 0;
char radioData[16];
bool model = AVANT;
bool mode = NAVI;

bool displayedCAR = 0;
bool displayedFL = 0;
bool displayedFR = 0;
bool displayedRL = 0;
bool displayedRR = 0;
bool displayedTRUNK = 0;

void draw_car(void) {
  if (displayedCAR) return;
  radio_write.reset();
  //delay(100);
  radio_write.initMiddleScreen();
  //delay(100);
  //if (model == SEDAN) {
  //  radio_write.GraphicFromArray_P(22, 1, 20, 46, sedan, 2);
  //} else { // avant
  radio_write.GraphicFromArray(22, 1, 20, 46, avant, 2);
  //}
  displayedCAR = 1;
  delay(100);
}

void fl() {
  if (displayedFL) return;
  radio_write.GraphicFromArray(15, 19, 8, 8, left_door, 1);
  displayedFL = 1;
  delay(100);
}

void fr() {
  if (displayedFR) return;
  radio_write.GraphicFromArray(41, 19, 8, 8, right_door, 1);
  displayedFR = 1;
  delay(100);
}

void rl() {
  if (displayedRL) return;
  radio_write.GraphicFromArray(15, 27, 8, 8, left_door, 1);
  displayedRL = 1;
  delay(100);
}

void rr() {
  if (displayedRR) return;
  radio_write.GraphicFromArray(41, 27, 8, 8, right_door, 1);
  displayedRR = 1;
  delay(100);
}

void draw_trunk() {
  if (displayedTRUNK) return;
  //if (model == AVANT) {
  //      radio_write.GraphicFromArray_P(25, 41, 14, 4, avant_trunc, 1);
  //  } else {
  radio_write.GraphicFromArray(23, 38, 18, 7, sedan_trunc, 1);
  //}
  displayedTRUNK = 1;
}

void setup() {
  radio_read.begin();
  radio_write.begin();
//  pinMode(TOUPPER, INPUT_PULLUP);
//  pinMode(MODE, INPUT_PULLUP);
  pinMode(MODEL, INPUT_PULLUP);
  pinMode(DOOR_FRONT_LEFT, INPUT_PULLUP);
  pinMode(DOOR_FRONT_RIGHT, INPUT_PULLUP);
  pinMode(DOOR_REAR_LEFT, INPUT_PULLUP);
  pinMode(DOOR_REAR_RIGHT, INPUT_PULLUP);
  pinMode(DOOR_TRUNK, INPUT_PULLUP);
  model = digitalRead(MODEL);
 // mode = digitalRead(MODE);
 // if (mode == NAVI) {
    radio_write.reset();
    //  delay(1000);
    radio_write.initFullScreen();
    //delay(1000);
    radio_write.GraphicFromArray(0, 0, 64, 88, b5f_orezana, 2);
    delay(3000);
  //}
  radio_write.reset();
}

void loop() {
  if (radio_read.hasNewMsg()) {
    if (!radio_read.msgIsNavi()) {
      // Navi text
      if (radio_read.msgIsRadioText()) {
        //radio msg(upper 2lines) in navi mode
        char tmp;
        for (uint8_t i = 3; i < radio_read.getSize() - 1; i++) { //1st byte is msg ID, second one is packet size,3th is second msg id (sort of) last is checksumm so we skip them
          tmp = radio_read.readData(i);
          //always do conversions if chars are in lower case
          //if (digitalRead(TOUPPER)) //DO CONVERSION TO UPPER CASE
          //{
            if ( tmp > 96 && tmp < 123) // a = 97, Z = 122 , other chars are ommited
              tmp = tmp - 'a' + 'A';
          //}
          radioData[i] = tmp;
        }
      }
//      if (mode == NAVI) {
        if (!radio_write.sendMsg(radioData)) Serial.println(F("send navi msg failed!"));
//      } else {
//        if (!radio_write.sendRadioMsg(radioData)) Serial.println("seng radio msg failed!");
//      }
      last_update = millis();
    } else {
      //radio mode, 16 characters which are important for us
      char tmp;
      for (uint8_t i = 0; i < 16; i++) { //1st byte is msg ID, last is checksumm
        tmp = radio_read.readData(1 + i);
        if (!digitalRead(TOUPPER)) //DO CONVERSION TO UPPER CASE
        {
          if ( tmp > 96 && tmp < 123) // a = 97, Z = 122 , other chars are ommited
            tmp = tmp - 'a' + 'A';
        }
        radioData[i] = tmp;
      }
//      if (mode == NAVI) {
        delay(10);
        radio_write.sendMsg(radioData);
//      } else {
//        radio_write.sendRadioMsg(radioData);
//      }
      last_update = millis();
    }
    radio_read.clearNewMsgFlag();
  }

  if (digitalRead(DOOR_FRONT_LEFT) == OPENED) {
    draw_car();
    fl();
  } else {
    if (displayedFL) { //doors are opened, lets close them!
      displayedFL = 0; //this will force "closing" door
      fl();
      displayedFL = 0; //make them closed permanently
    }
  }

  if (digitalRead(DOOR_FRONT_RIGHT) == OPENED) {
    draw_car();
    fr();
    radio_write.sendKeepAliveMsg();
  } else {
    if (displayedFR) { //doors are opened, lets close them!
      displayedFR = 0; //this will force "closing" door
      fr();
      displayedFR = 0; //make them closed permanently
    }
  }

  if (digitalRead(DOOR_REAR_LEFT) == OPENED) {
    draw_car();
    rl();
    radio_write.sendKeepAliveMsg();
  } else {
    if (displayedRL) { //doors are opened, lets close them!
      displayedRL = 0; //this will force "closing" door
      rl();
      displayedRL = 0; //make them closed permanently
    }
  }

  if (digitalRead(DOOR_REAR_RIGHT) == OPENED) {
    draw_car();
    rr();
    radio_write.sendKeepAliveMsg();
  } else {
    if (displayedRR) { //doors are opened, lets close them!
      displayedRR = 0; //this will force "closing" door
      rr();
      displayedRR = 0; //make them closed permanently
    }
  }

  if (digitalRead(DOOR_TRUNK) == OPENED) {
    draw_car();
    draw_trunk();
    radio_write.sendKeepAliveMsg();
  } else {
    if (displayedTRUNK) { //doors are opened, lets close them!
      displayedTRUNK = 0; //this will force "closing" door
      draw_trunk();
      displayedTRUNK = 0; //make them closed permanently
    }
  }

  if (displayedCAR && !displayedFL && !displayedFR && !displayedRL && !displayedRR && !displayedTRUNK)
  {
    radio_write.reset();
    displayedCAR = 0;
  }

  if ((millis() - last_update) > 10) {
    radio_write.sendKeepAliveMsg();
    radio_read.ACK();
    last_update = millis();
  }
}
