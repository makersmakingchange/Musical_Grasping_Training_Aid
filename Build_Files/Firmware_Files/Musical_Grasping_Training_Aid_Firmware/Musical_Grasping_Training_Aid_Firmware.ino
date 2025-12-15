/*
 * Musical_Grasping_Training_Aid_Firmware (v1.0) 
 * 
 * Copyright 2024 Neil Squire/Makers Making Change. 
 * 
 * License: This work is distributed under the terms of the GNU General Public License.
 * 
 * This program is free software: you can redistribute it and/or modify it under the terms of the 
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
 * or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this program. 
 * If not, see <https://www.gnu.org/licenses/>.
 */


#include <Arduino.h>
#include <Wire.h>
#include <DFRobotDFPlayerMini.h>  //MP3 reader library
#include "Adafruit_MPR121.h"      //Capacitive Touch Sensor library

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

DFRobotDFPlayerMini myDFPlayer;           //Create  DFPlayer object
Adafruit_MPR121 cap = Adafruit_MPR121();  //Create MPR121 object

uint16_t currtouched = 0;  //Variable to store capacitance value from MPR121

const int buttonPin1 = A0;  // Pin for pushbutton 1
const int buttonPin2 = A1;  // Pin for pushbutton 2

const int presetVolumes[] = { 5, 8, 11, 14, 17, 20 };  // Preset volume levels
int currentVolumeIndex = 0;                            // Volume index counter
int currentSongIndex = 0;                              //Song index counter

int buttonState1 = 0;  //Variable to store button 1 state
int buttonState2 = 0;  //Variable to store button 2 state

// dump the registers on MPR121, to get calibration info
void dump_regs() {
  Serial.println("========================================");
  Serial.println("CHAN 00 01 02 03 04 05 06 07 08 09 10 11");
  Serial.println("     -- -- -- -- -- -- -- -- -- -- -- --"); 
  // CDC
  Serial.print("CDC: ");
  for (int chan=0; chan<12; chan++) {
    uint8_t reg = cap.readRegister8(0x5F+chan);
    if (reg < 10) Serial.print(" ");
    Serial.print(reg);
    Serial.print(" ");
  }
  Serial.println();
  // CDT
  Serial.print("CDT: ");
  for (int chan=0; chan<6; chan++) {
    uint8_t reg = cap.readRegister8(0x6C+chan);
    uint8_t cdtx = reg & 0b111;
    uint8_t cdty = (reg >> 4) & 0b111;
    if (cdtx < 10) Serial.print(" ");
    Serial.print(cdtx);
    Serial.print(" ");
    if (cdty < 10) Serial.print(" ");
    Serial.print(cdty);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("========================================");
}

void setup() {

  pinMode(buttonPin1, INPUT_PULLUP);  // Set buttonPin1 as an input
  pinMode(buttonPin2, INPUT_PULLUP);  // Set buttonPin2 as an input

  Serial.begin(115200);  //Setup Serial communication; baud rate is 9600 bps
  Serial.println("Entering Setup");

  //Wire.begin(); //Initialize I2C communication

  Serial1.begin(9600);        //Setup Serial1 communication; baud rate is 9600 bps
  while (!Serial) delay(10);  //Wait for Serial1 to initialize
  //---MPR121 Capacitive Touch Sensor Test----
  Serial.println("Adafruit MPR121 Capacitive Touch Sensor Test");

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D

  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (true)
      ;
  }
  Serial.println("MPR121 found!");

  Serial.println("Initial CDC/CDT values:");
  dump_regs();

  cap.setAutoconfig(true);

  Serial.println("After autoconfig CDC/CDT values:");
  dump_regs();

  //---DFPlayer Test----
  Serial.println("DFRobot DFPlayer Test");
  if (!myDFPlayer.begin(Serial1)) {  // Use Serial1 to communicate with the DFPlayer
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true)
      ;
  }
  Serial.println(F("DFPlayer Mini online."));

  //----Set volume----
  myDFPlayer.volume(presetVolumes[currentVolumeIndex]);  //Set volume value to currentVolumeIndex which corresponds to the first volume setting
  myDFPlayer.volumeUp();                                 //Volume Up
  myDFPlayer.volumeDown();                               //Volume Down
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);                     //Set Equalizer
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);           //Set device (SD as default)
  myDFPlayer.enableLoopAll();                            //Loop files after play through once
  myDFPlayer.play(1);                                    //Play the first music file
  myDFPlayer.pause();                                    //Pause music
}

void loop() {

  currtouched = cap.touched();  //Read the current touched state
  Serial.println(currtouched);
  delay(500);  //Delay for 500 milliseconds

  if (currtouched > 1) {  //The value 1 is the threshold. Value can be changed to modify sensitivity
    myDFPlayer.start();   //start the mp3 from the pause
  } else {
    myDFPlayer.pause();  //pause the mp3
  }

  int buttonState1 = digitalRead(buttonPin1);  //Read value of skip song button
  int buttonState2 = digitalRead(buttonPin2);  //Read value of volume button


  //----Skip Song Button (Push Button 1)----
  if (buttonState1 == LOW) {  //If pushbutton 1 is pressed
    myDFPlayer.next();        //Play next song
    Serial.println("next:");
    Serial.print("Current track:");
    Serial.println(myDFPlayer.readCurrentFileNumber() + 1);
  }

  //----Volume Button (Push Button 2)----
  if (buttonState2 == LOW) {                               //If pushbutton 2 is pressed
    currentVolumeIndex = (currentVolumeIndex + 1) % 6;     // Cycle to next volume value
    myDFPlayer.volume(presetVolumes[currentVolumeIndex]);  // New volume value
    Serial.print("Set volume to ");
    Serial.println(presetVolumes[currentVolumeIndex]);
  }
}
