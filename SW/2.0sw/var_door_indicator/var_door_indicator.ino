/*
  Title:
    10.Radio_bridge.ino

  Description:
    Demonstrates how to display data coming from an original radio.
  
  Notes:
    *This demo doesn't initialize/use the rest of the screen.
    *A separate library (https://github.com/domnulvlad/TLBRRLib) is required for receiving data from the original radio.
    *A pull-down resistor is required between the radio's ENA pin and GND (10k recommended).
    *The radio's ENA and CLK lines must be connected to hardware-interrupt or pin-change-interrupt capable pins. It is up to the user to define
    when the RR library thinks an interrupt happened, and what happens when the it wants to attach/detach an interrupt.
    *This demo is designed for the Arduino UNO which has interrupt-capable pins 2 and 3, so the default attachInterrupt() and detachInterrupt()
    are defined to be called when the RR library wants to do those things.
    
    *The radio_is_on variable only monitors whether or not there is valid data in the RR library's buffer.
    *So radio_is_on may be true even if the radio is off, but that's because the last stored message was the "off" message, so it would be correct
    to send that message again, it would just keep the radio mode off.
    *But it seems that sometimes the radio forgets to send the "off" message when it powers down, so a timer is created with the last_message_time
    variable, and "request" attempts will be counted by the request_attempts variable.
    *If too much time passed since the last message and the radio isn't responding to requests, radio_is_on becomes false and the radio mode is
    turned off.
    *The RR library will have the last message stored (which won't be the "off" message, but other text data), and writing it will enable the
    radio mode with previous data even though the radio is off, which is unwanted behavior.
    
    *The only part of the sketch where previous radio data is sent to the cluster is in the errorFunction, if the event gets triggered.
    *In reality, the function will not get triggered if the radio text is cleared and the rest of the screen is not initialized.
    *The scope of the radio_is_on variable is to keep the code correct if you choose to add other functions to this sketch that use the rest of
    the screen. Then it would prevent previous data from being shown if the radio is off but forgot to send the "off" message and the errorFunction
    gets triggered.
*/
#include "bitmaps.h"

//Include the FIS library.
#include <TLBFISLib.h>

//Include the Radio Reader library.
#include <TLBRRLib.h>

//Include the SPI library.
#include <SPI.h>

//Hardware configuration
#define SPI_INSTANCE SPI
#define ENA_PIN       9

#define RADIO_ENA_PIN 2
#define RADIO_CLK_PIN 3
#define RADIO_DAT_PIN 4

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
//#define RADIO 0
//#define NAVI 1
#define OPENED 0
#define CLOSED 1

long last_update = 0;
char radioData[16];
bool model = AVANT;
//bool mode = NAVI;
bool to_upper = 1;
bool logo = 0;

bool displayedCAR = 0;
bool displayedFL = 0;
bool displayedFR = 0;
bool displayedRL = 0;
bool displayedRR = 0;
bool displayedTRUNK = 0;


//Define the function to be called when the FIS library needs to send a byte.
void sendFunction(uint8_t data)
{
  SPI_INSTANCE.beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE3));
  SPI_INSTANCE.transfer(data);
  SPI_INSTANCE.endTransaction();
}

//Define the function to be called when the FIS library is initialized by begin().
void beginFunction()
{
  SPI_INSTANCE.begin();
}

//Declare the functions that will be called when interrupts fire, as they need to use the "Radio" object which isn't defined yet.
void executeInterruptENA();
void executeInterruptCLK();

//Define the function to be called when the RR library needs to attach an interrupt on ENA.
void attachInterruptENA(uint8_t mode)
{
  attachInterrupt(digitalPinToInterrupt(RADIO_ENA_PIN), executeInterruptENA, mode);
}

//Define the function to be called when the RR library needs to detach the interrupt on ENA.
void detachInterruptENA()
{
  detachInterrupt(digitalPinToInterrupt(RADIO_ENA_PIN));
}

//Define the function to be called when the RR library needs to attach an interrupt on CLK.
void attachInterruptCLK(uint8_t mode)
{
  attachInterrupt(digitalPinToInterrupt(RADIO_CLK_PIN), executeInterruptCLK, mode);
}

//Define the function to be called when the RR library needs to detach the interrupt on CLK.
void detachInterruptCLK()
{
  detachInterrupt(digitalPinToInterrupt(RADIO_CLK_PIN));
}

//Create an instance of the FIS library.
TLBFISLib FIS(ENA_PIN, sendFunction, beginFunction);

//Create an instance of the RR library.
TLBRRLib Radio(RADIO_ENA_PIN, RADIO_CLK_PIN, RADIO_DAT_PIN, //ENA, CLK, DAT pins
               attachInterruptENA,                          //attachInterrupt() for ENA
               detachInterruptENA,                          //detachInterrupt() for ENA
               attachInterruptCLK,                          //attachInterrupt() for CLK
               detachInterruptCLK);                         //detachInterrupt() for CLK

//Define the function to be called when an ENA interrupt fires.
void executeInterruptENA()
{
  Radio.executeInterruptENA();
}

//Define the function to be called when a CLK interrupt fires.
void executeInterruptCLK()
{
  Radio.executeInterruptCLK();
}

//Timer to determine how much time has passed since receiving the last message
unsigned long last_message_time;

//Counter for how many requests have been made when too much time passes without receiving messages
uint8_t request_attempts;

//Monitor if the radio was turned off by a timeout (in which case the data stored is the RR library is no longer valid)
bool radio_is_on;


void draw_car(void) {
  if (displayedCAR) return;
  FIS.initScreen();
  //FIS.clear();
  if (model == SEDAN) {
    FIS.drawBitmap(22, 1, 20, 46, sedan);
  } else { // avant
  FIS.drawBitmap(22, 1, 20, 46, avant);
  }
  displayedCAR = 1;
  delay(100);
}

void fl() {
  if (displayedFL) return;
  FIS.setWorkspace(15,19,8,8);
  FIS.setDrawColor(FIS.INVERTED);//mode = 0
  FIS.setBitmapTransparency(FIS.TRANSPARENT); //mode =1
  FIS.drawBitmap(0,0, 8, 8, left_door);
  
  displayedFL = 1;
  delay(100);
}

void fr() {
  if (displayedFR) return;
  FIS.setWorkspace(41,19,8,8);
  FIS.setDrawColor(FIS.INVERTED);//mode =0
  FIS.setBitmapTransparency(FIS.TRANSPARENT); //mode =1
  FIS.drawBitmap(0,0, 8, 8, right_door);
  displayedFR = 1;
  delay(100);
}

void rl() {
  if (displayedRL) return;
  FIS.setWorkspace(15, 27, 8, 8);
  FIS.setDrawColor(FIS.INVERTED);//mode =0
  FIS.setBitmapTransparency(FIS.TRANSPARENT); //mode =1
  FIS.drawBitmap(0, 0, 8, 8, left_door);
  displayedRL = 1;
  delay(100);
}

void rr() {
  if (displayedRR) return;
  FIS.setWorkspace(41, 27, 8, 8);
  FIS.setDrawColor(FIS.INVERTED);
  FIS.setBitmapTransparency(FIS.TRANSPARENT); //mode =1
  FIS.drawBitmap(0, 0, 8, 8, right_door);
  displayedRR = 1;
  delay(100);
}

void draw_trunk() {
  if (displayedTRUNK) return;
  if (model == AVANT) {
        FIS.setWorkspace(25,41,14,4);
        FIS.setDrawColor(FIS.INVERTED);//mode =0
        FIS.setBitmapTransparency(FIS.TRANSPARENT);//mode =1
        FIS.drawBitmap(0, 0, 14, 4, avant_trunc);
    } else {
    FIS.setWorkspace(23,38,18,7);
    FIS.setDrawColor(FIS.INVERTED);//mode =0
    FIS.setBitmapTransparency(FIS.TRANSPARENT);//mode =1
    FIS.drawBitmap(0, 0, 18, 7, sedan_trunc);
   }
  displayedTRUNK = 1;
}

void setup() {
  pinMode(TOUPPER, INPUT_PULLUP);
  //pinMode(MODE, INPUT_PULLUP);
  pinMode(MODEL, INPUT_PULLUP);
  pinMode(DOOR_FRONT_LEFT, INPUT_PULLUP);
  pinMode(DOOR_FRONT_RIGHT, INPUT_PULLUP);
  pinMode(DOOR_REAR_LEFT, INPUT_PULLUP);
  pinMode(DOOR_REAR_RIGHT, INPUT_PULLUP);
  pinMode(DOOR_TRUNK, INPUT_PULLUP);
  model = digitalRead(MODEL); //0 sedan,1avant(default)
  //mode = digitalRead(MODE);
  //to_upper = digitalRead(TOUPPER);


  //Start the libraries.
  FIS.begin();
  Radio.begin();


  if (logo) {
    FIS.initScreen(TLBFISLib::FULLSCREEN);
    FIS.drawBitmap(0, 0, 64, 88, b5f_orezana);
    delay(3000);
    FIS.clear();
    FIS.turnOff();
  }
  //Here, the custom error function is declared directly, as a lambda function.
  //In the example "02.Custom_functions" you can see how to define it as a separate function.
  FIS.errorFunction(
    [](unsigned long duration) {
      //Errors are measured in milliseconds, to offer the possibility of differentiating between events.
      //Here, this value won't be used, so cast it to void to avoid a compiler warning.
      (void) duration;
      
      //Display whatever (old) data is stored in the RR library.
      if (radio_is_on) {
        FIS.writeRadioRawData(Radio.getBuffer());
        Radio.continueReceiving();
      }
    }
  );
  
  //Clear the radio mode text, which will turn the radio section off.
  FIS.clearRadioText();
}

void loop() {
  if (digitalRead(DOOR_FRONT_LEFT) == OPENED) {
    draw_car();
    fl();
  } else {
    if (displayedFL) { //doors are opened, lets close them!
      displayedFL = 0; //this will forceSet a workspace in the middle of the FIS and clear it. "closing" door
      fl();
      displayedFL = 0; //make them closed permanently
    }
  }

  if (digitalRead(DOOR_FRONT_RIGHT) == OPENED) {
    draw_car();
    fr();
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
  } else {
    if (displayedTRUNK) { //doors are opened, lets close them!
      displayedTRUNK = 0; //this will force "closing" door
      draw_trunk();
      displayedTRUNK = 0; //make them closed permanently
    }
  }

  if (displayedCAR && !displayedFL && !displayedFR && !displayedRL && !displayedRR && !displayedTRUNK)
  {
    FIS.turnOff();
    displayedCAR = 0;
  }

  //Maintain the connection.
  FIS.update();
  
  //If there is new radio data available, display it.
  //if (Radio.hasData()) {
    //Calling getBuffer() returns the array containing the data, but it also stops new data from being copied there.
    //When the array is done being used, continueReceiving() must be called to be able to receive new data.
    FIS.writeRadioRawData(Radio.getBuffer());
    Radio.continueReceiving();
    
    //Save the time this message was received and reset the attempt counter.
    last_message_time = millis();
    request_attempts = 0;
    
    //The data in the RR library is valid and can be sent again without Radio.hasData() having to return true.
    radio_is_on = true;
  //}




  //If the timer is set and too much time has passed since the last message, request data from the radio.
  if (last_message_time && (millis() - last_message_time >= 5000))
  {
    //Request data and increment the counter.
    Radio.requestData();
    request_attempts++;
    
    //Wait a little bit before making the next attempt.
    last_message_time += 500;
    
    //If too many requests have been made and received no answer, stop trying.
    if (request_attempts >= 5)
    {
      //Clear the radio text and stop the timer.
      FIS.clearRadioText();
      last_message_time = 0;
      
      //The data in the RR library is no longer valid. Sending it again would activate the radio mode while the radio is off.
      radio_is_on = false;
    }
  }
}
