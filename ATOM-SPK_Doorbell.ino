/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with Atom-Lite sample source code
*                          配套  Atom-Lite 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/atom/atom_spk
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/atom/atom_spk
* ESP8266Audio:  https://github.com/earlephilhower/ESP8266Audio
*
* describe: ATOM SPK-Doorbell v0.2
* date：24/11/2021
* author: burpitt
*******************************************************************************
  On button press, play MP3 from SD card once, then return to wait for next button press.

  -Added button LED states for visual representation

  This code is adapted and updated from the original example(s) provided by M5Stack &
  ESP8266Audio.
  
  Many thanks.
*/

#include "M5Atom.h"
#include <WiFi.h>

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

uint8_t DisBuff[2 + 5 * 5 * 3];                             //To store RBG color values.  
int ringFlag;                                               //ringFlag for serial ring text output
int sbFlag;                                                 //sbFlag for serial standby text output

void setBuff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata){  //Set the colors of LED, and save the relevant data to DisBuff[].
    DisBuff[0] = 0x05;
    DisBuff[1] = 0x05;
    for (int i = 0; i < 25; i++){
        DisBuff[2 + i * 3 + 0] = Rdata;
        DisBuff[2 + i * 3 + 1] = Gdata;
        DisBuff[2 + i * 3 + 2] = Bdata;
    }
}

void setup(){
  
    M5.begin(true,false,true);
    
      SPI.begin(SCK, MISO, MOSI, -1);
        if(!SD.begin(-1, SPI, 40000000)){                    // Check for a card, if not one in throw up some moaning
        Serial.println();
        Serial.println("Put a memory card in your twat");
        while(1);
      }
    file = new AudioFileSourceSD("/hello.mp3");         // MP3 file call
    out = new AudioOutputI2S();
    out->SetPinout(22, 21, 25);
    out->SetGain(3.9);                                       // Set gain on audio 0.0-4.0
    mp3 = new AudioGeneratorMP3();
    ringFlag = 0x02;
    sbFlag = 0x02;
    Serial.println("ATOM SPK-Doorbell v0.2 Initialised:");
    Serial.println("Standing by...");

}

void button() {  
  
  if (M5.Btn.wasPressed()){
        Serial.println("..Someone touched me...");
        ringFlag = 0x00;                                      // Set 0
        sbFlag = 0x00;                                        // Set 0
        file = new AudioFileSourceSD("/hello.mp3");      // MP3 file call
        delay(1000);
        mp3->begin(file, out);                                // Play MP3
        }
}

void loop(){

  button();

  if (mp3->isRunning()) {                                      //"Running" returned from loop
        setBuff(0x00, 0x40, 0x00);                             // Set LED to red
        M5.dis.displaybuff(DisBuff);
        if (ringFlag == 0x00){
        Serial.println("     I did ring!");
        ringFlag = 0x01;                                       // Ring confirmed, set 1 byte to stop serial spam
      }
        if (!mp3->loop()) mp3->stop();                         // Whilst mp3 plays, mp3 returns running, if loop over then stop
    } 
    else {
          setBuff(0x40, 0x00, 0x00);                           // Set LED to green
          M5.dis.displaybuff(DisBuff);  
          if (sbFlag == 0x00){
          Serial.println("|> Now I will wait for another button wacker <|");
          sbFlag = 0x01;                                       // Set 1 byte to stop serial spaam
          }
        }
  M5.update();                                                 // Listen for button press
   }
