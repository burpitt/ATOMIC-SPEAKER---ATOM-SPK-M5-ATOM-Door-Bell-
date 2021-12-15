/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with Atom-Lite sample source code
*                          配套  Atom-Lite 示例源代码
* Visit the website for more information：  https://docs.m5stack.com/en/atom/atom_spk
* 获取更多资料请访问：                         https://docs.m5stack.com/zh_CN/atom/atom_spk
* ESP8266Audio:                             https://github.com/earlephilhower/ESP8266Audio
* Debouncer:                                https://esp32io.com/tutorials/esp32-button-debounce)
*
* describe: ATOM SPK-Doorbell v1.0
* date：15/12/2021
* author: burpitt
*******************************************************************************
  On short or long button press, play MP3 from SD card once then stand by for next press.
  
  v0.2 - 24/11/21
  -Added button LED states for visual representation
  
  v1.0 - 15/12/21
  -Added short/long press function
  -Added debounce timer for accuracy 

  This code is adapted and updated from the original example(s) provided by M5Stack &
  ESP8266Audio.

  Also with thanks to the MsX helpers.
  
*/

#include "M5Atom.h"
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourceID3.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;

#define SCK 23
#define MISO 33
#define MOSI 19
#define BUTTON_PIN     39                                   // GIOP39 pin connected to button
#define DEBOUNCE_TIME  50                                   // the debounce time in millisecond, increase this time if it still chatters
#define LONG_PRESS   2000                                   // time in mili

// Variables will change:
int lastSteadyState = LOW;                                  // the previous steady state from the input pin
int lastFlickerableState = LOW;                             // the previous flickerable state from the input pin
int currentState;                                           // the current reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;                         // the last time the output pin was toggled
unsigned long PressTime = 0;                                // Button usage timers
unsigned long ReleaseTime = 0;                              // Button usage timers

int ringFlag;                                               // Flags for button status
int sbFlag;                                                 // Flags for button status

uint8_t DisBuff[2 + 5 * 5 * 3]; 

void setBuff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata){  //Set the colors of LED, and save the relevant data to DisBuff[].
    DisBuff[0] = 0x05;
    DisBuff[1] = 0x05;
    for (int i = 0; i < 25; i++){
        DisBuff[2 + i * 3 + 0] = Rdata;
        DisBuff[2 + i * 3 + 1] = Gdata;
        DisBuff[2 + i * 3 + 2] = Bdata;
    }
}

void setup() {
  M5.begin(true,false,true);
  Serial.begin(115200);
      SPI.begin(SCK, MISO, MOSI, -1);
        if(!SD.begin(-1, SPI, 40000000)){                    // Check for a card, if not one in throw up some moaning
        Serial.println();
        Serial.println("Put a memory card in & reset");
        while(1);
      }
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    out = new AudioOutputI2S();
    out->SetPinout(22, 21, 25);
    out->SetGain(3.9);                                       // Set gain on audio 0.0-4.0
    mp3 = new AudioGeneratorMP3();
    ringFlag = 0x02;                                         // Flags for button status
    sbFlag = 0x02;                                           // Flags for button status
    Serial.println("ATOM SPK-Doorbell v0.2 Initialised:");
    Serial.println("Standing by...");
}

void loop() {

 buttonpress();
 
  if (mp3->isRunning()) {                                      //"Running" returned from loop
        setBuff(0x00, 0x40, 0x00);                             // Set LED to red
        M5.dis.displaybuff(DisBuff);
        if (ringFlag == 0x00){
        Serial.println("|>PRESSED<|");
        ringFlag = 0x01;                                       // Ring confirmed, set 1 byte to stop serial spam
      }
        if (!mp3->loop()) mp3->stop();                         // Whilst mp3 plays, 'mp3' returns running, if the loop is over then stop MP3
    } 
    else {
          setBuff(0x40, 0x00, 0x00);                           // Set LED to green
          M5.dis.displaybuff(DisBuff);  
          if (sbFlag == 0x00){
          Serial.println("|>STANDBY<|");
          sbFlag = 0x01;                                       // Set 1 byte to stop serial spaam
          }
        }   
      

}

void buttonpress () {

    // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch/button changed, due to noise or pressing:
  if (currentState != lastFlickerableState) {
    // reset the debouncing timer
    lastDebounceTime = millis();

    // save the the last flickerable state
    lastFlickerableState = currentState;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if((lastSteadyState == HIGH && currentState == LOW)) {
      Serial.println("The button is pressed");
      PressTime = millis();
      Serial.println(PressTime); }
    else if(lastSteadyState == LOW && currentState == HIGH) {
      Serial.println("The button is released");
      ReleaseTime = millis();
      Serial.println(ReleaseTime);
      process();
    }
    
    // save the the last steady state
    lastSteadyState = currentState;

  }

}

void process () {

  delete file;                                            // Clear file buffer for new MP3

  if ((ReleaseTime - PressTime) > LONG_PRESS) {           // Decide short or long press
  
    Serial.println("Long press");
    ringFlag = 0x00;                                      // Flags for button status
    sbFlag = 0x00;                                        // Flags for button status
    file = new AudioFileSourceSD("/hello.mp3");           // MP3 file call
    mp3->begin(file, out);                                // Play that sucker

  }

    else  {
      Serial.println("Short press");
      ringFlag = 0x00;                                      // Flags for button status
      sbFlag = 0x00;                                        // Flags for button status
      file = new AudioFileSourceSD("/hello.mp3");           // MP3 file call
      mp3->begin(file, out);                                // Play that sucker

    }
    
}
