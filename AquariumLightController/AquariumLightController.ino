// AUTOMATED LIGHT CONTROLLER FOR CURRENT USA SATELLITE FRESHWATER LED+ AQUARIUM LIGHT 
// http://current-usa.com/aquarium-led-lights/satellite-freshwater-led-plus/
//
// Developed targeting a SainSmart UNO R3
//
// Required Libraries:
//   Arduino-IRremote: https://github.com/shirriff/Arduino-IRremote
//   Time: http://www.pjrc.com/teensy/td_libs_Time.html 
//   LiquidCrystal: http://arduino.cc/en/Reference/LiquidCrystal
//
// Copyright (C) 2014  Ryan W Rondeau
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <IRremote.h>
#include <LiquidCrystal.h>
#include <Time.h>

// PIN CONFIGURATIONS
const int RECV_PIN = 11;   // IR reciever 
const int STATUS_PIN = 12; // Status LED 
const int BUTTON_PIN = 2;  // Push button
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

// Represents RBGW levels
struct RGBL
{
  int Red;
  int Green;
  int Blue;
  int White;
};
typedef struct RGBL LightColor;

// TARGET COLORS BY HOUR OF DAY
// 42 IS THE MAXIMUM LEVEL
const LightColor MIDNIGHT = {0,0,1,0};
const LightColor ONE = {0,0,1,0};
const LightColor TWO = {0,0,1,0};
const LightColor THREE = {0,0,2,0};
const LightColor FOUR = {2,0,2,0};
const LightColor FIVE = {16,14,2,0};
const LightColor SIX = {32,24,2,2};
const LightColor SEVEN = {42,42,4,24};
const LightColor EIGHT = {42,42,8,32};
const LightColor NINE = {42,42,16,43};
const LightColor TEN = {42,42,32,42};
const LightColor ELEVEN = {42,42,42,42};
const LightColor TWELVE = {42,42,42,42};
const LightColor THIRTEEN = {32,32,42,42};
const LightColor FOURTEEN = {32,32,42,42};
const LightColor FIFTEEN = {32,32,42,32};
const LightColor SIXTEEN = {24,24,32,24};
const LightColor SEVENTEEN = {24,20,16,12};
const LightColor EIGHTTEEN = {16,12,6,0};
const LightColor NINETEEN = {2,2,2,0};
const LightColor TWENTY = {0,0,1,0};
const LightColor TWENTYONE = {0,0,1,0};
const LightColor TWENTYTWO = {0,0,1,0};
const LightColor TWENTYTHREE = {0,0,1,0};

// REMOTE CONTROL CODES
const unsigned long POWER = 0x20DF02FD;
const unsigned long FULLORANGE = 0x20DF3AC5;
const unsigned long FULLLIGHTBLUE = 0x20DFBA45;
const unsigned long FULLPURPLE = 0x20DF827D;
const unsigned long FULLWHITE = 0x20DF1AE5;
const unsigned long FULLYELLOW = 0x20DF9A65;
const unsigned long FULLBLUE = 0x20DFA25D;
const unsigned long REDUP = 0x20DF2AD5;
const unsigned long REDDOWN = 0x20DF0AF5;
const unsigned long GREENUP = 0x20DFAA55;
const unsigned long GREENDOWN = 0x20DF8A75;
const unsigned long BLUEUP = 0x20DF926D;
const unsigned long BLUEDOWN = 0x20DFB24D;
const unsigned long WHITEUP = 0x20DF12ED;
const unsigned long WHITEDOWN = 0x20DF32CD;
const unsigned long M1 = 0x20DF38C7;
const unsigned long M2 = 0x20DFB847;
const unsigned long M3 = 0x20DF7887;
const unsigned long M4 = 0x20DFF807;
const unsigned long MOONLIGHT = 0x20DF18E7;
const unsigned long MOONDARK = 0x20DF9867;
const unsigned long MOONCLOUDS = 0x20DF58A7;
const unsigned long SUNRISE = 0x20DFD827;
const unsigned long CLOUDS1 = 0x20DF28D7;
const unsigned long CLOUDS2 = 0x20DFA857;
const unsigned long CLOUDS3 = 0x20DF6897;
const unsigned long CLOUDS4 = 0x20DFE817;
const unsigned long CLOUDS5 = 0x20DFC837;
const unsigned long STORM1 = 0x20DF08F7;
const unsigned long STORM2 = 0x20DF8877;
const unsigned long STORM3 = 0x20DF48B7;

LightColor color = {0,0,1,0};       // The current color of the light
LightColor lastColor = {0,0,1,0};   // The previous color of the light
LightColor targetColor = {0,0,1,0}; // The target color of the light

IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results; // IR decoder

long commandData;                             // IR command to be sent
int buttonPushed = 1;                         // Used for setting time
int lastHour = 0;                             // The last hour between 0-23
int printState = 0;                           // Used to prevent clearing and writing the same message.
boolean isTimeSet = false;                    // Has the time been set
boolean isFadeInProgress = false;             // Is there a fade in progress
unsigned long startingSeconds = 0;            // The starting time of the fade
int durationSeconds = 3600;                   // The duration of the fade in seconds

void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);

  pinMode(STATUS_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  resetLights();

  lcd.clear();
  lcd.print("TIME NOT SET");

  startingSeconds = now();
  setTargetColor(hour(), minute());
}

void loop()
{   
  // Use this to test 24 hour cycle in 24 minutes. Comment out the rest of the function.
  /*
  setTargetColor(minute()+4);
   printLCD();
   delay(1000);
   checkColors(second(),0);
   */

  buttonPushed = digitalRead(BUTTON_PIN);

  // If the button is pushed, increase the time by 15 minute increments
  // TODO: Set time using IR remote
  if(buttonPushed == 0)
  {
    // Set the time to the current time + 15 minutes
    setTime(now() + 900);

    // The time has now been set
    isTimeSet = true;

    // Show the time 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TIME SET");
    lcd.setCursor(0, 1);
    lcd.print(hour());
    printDigits(minute());

    // Set an appropriate target color for the time
    setTargetColor(hour(), minute());
    lastColor = color;

    // With a 250ms delay, the time will increment by 1 hour for every second the button has been held down
    delay(250);
  }
  else if(isTimeSet) // If the time has been set
  {
    // Show information on the LCD
    printLCD();

    // Get the time in seconds (since 1970)
    unsigned long rightNow = now();

    // If a color fade has been completed print a message and start a new fade
    if(rightNow > startingSeconds + durationSeconds)
    {
      lcd.clear();
      lcd.print("FADE DONE");
      delay(1000);
      isFadeInProgress = false;
      setTargetColor(hour(), minute());
    }
    else
    {
      // If there is a fade in progress, check if there are any IR commands that should be sent
      checkColors(rightNow - startingSeconds, durationSeconds);
    }

    delay(100); // 100ms delay prevents the LCD from flickering if the same text is written to it
  }
} 

// Check if there should be any IR commands sent for the color fade, based on time elapsed
void checkColors(int secondsElapsed, int durationInSeconds)
{  
  // RED /////////////////////////////////////////////////////////
  if(targetColor.Red != lastColor.Red)
  {
    // Get the change per second for the current color to the target color
    float RTick = (float)durationInSeconds / (float)(targetColor.Red - lastColor.Red);

    // Get the expected change for the time elapsed
    int RValue = round(secondsElapsed / RTick); 

    if(RTick > 0) // If the change is positive
    {
      // And if the current color does not match the expected color
      if(lastColor.Red + RValue > color.Red) 
      {
        // And, for protection, the current color does not exceed the target color.
        if(color.Red < targetColor.Red)
        {
          // Increase the current color
          color.Red++;
          sendCommand("REDUP");
        }
      }
    }
    else // If the change is negative
    {
      // And if the current color does not match the expected color
      if((lastColor.Red + RValue) < color.Red)
      {
        // And, for protection, the current color is not less than the target color.
        if(color.Red > targetColor.Red)
        {
          // Decrease the current color
          color.Red--;
          sendCommand("REDDOWN");
        }
      } 
    }
  }

  // GREEN /////////////////////////////////////////////////////////
  if(targetColor.Green != lastColor.Green)
  {
    // Get the change per second for the current color to the target color
    float GTick = (float)durationInSeconds / (float)(targetColor.Green - lastColor.Green);

    // Get the expected change for the time elapsed
    int GValue = round(secondsElapsed / GTick);
    
    if(GTick > 0) // If the change is positive
    {
      // And if the current color does not match the expected color
      if(lastColor.Green + GValue > color.Green)
      {
        // And, for protection, the current color does not exceed the target color.
        if(color.Green < targetColor.Green)
        {
          // Increase the current color
          color.Green++;
          sendCommand("GREENUP");
        }
      }
    }
    else // If the change is negative
    {
      // And if the current color does not match the expected color
      if((lastColor.Green + GValue) < color.Green)
      {        
        // And, for protection, the current color is not less than the target color.
        if(color.Green > targetColor.Green)
        {
          // Decrease the current color
          color.Green--;
          sendCommand("GREENDOWN");
        }
      } 
    }
  }

  // BLUE /////////////////////////////////////////////////////////
  if(targetColor.Blue != lastColor.Blue)
  {
    // Get the change per second for the current color to the target color
    float BTick = (float)durationInSeconds / (float)(targetColor.Blue - lastColor.Blue);

    // Get the expected change for the time elapsed
    int BValue = round(secondsElapsed / BTick);
    
    if(BTick > 0) // If the change is positive
    {
      // And if the current color does not match the expected color
      if(lastColor.Blue + BValue > color.Blue)
      {
        // And, for protection, the current color does not exceed the target color.
        if(color.Blue < targetColor.Blue)
        {
          // Increase the current color
          color.Blue++;
          sendCommand("BLUEUP");
        }
      }
    }
    else // If the change is negative
    {
      // And if the current color does not match the expected color
      if((lastColor.Blue + BValue) < color.Blue)
      {
        // And, for protection, the current color is not less than the target color.
        if(color.Blue > targetColor.Blue)
        {
          // Decrease the current color
          color.Blue--;
          
          // Sometimes IR codes are missed by the reciever. An assumption is made that once blue brightness = 1
          // That we are at night. Use the M4 preset set to 0,0,1,0 to ensure we are in moonlight mode.
          if(color.Blue == 1) 
          {
            sendCommand("M4");
          }
          else 
          {
            sendCommand("BLUEDOWN");
          }
        }
      }
    }
  }

  // WHITE /////////////////////////////////////////////////////////
  if(targetColor.White != lastColor.White)
  {
    // Get the change per second for the current color to the target color
    float WTick = (float)durationInSeconds / (float)(targetColor.White - lastColor.White);

    // Get the expected change for the time elapsed
    int WValue = round(secondsElapsed / WTick);
    
    if(WTick > 0) // If the change is positive
    {
      // And if the current color does not match the expected color
      if(lastColor.White + WValue > color.White)
      {
        // And, for protection, the current color does not exceed the target color.
        if(color.White < targetColor.White)
        {
          // Increase the current color
          color.White++;
          sendCommand("WHITEUP");
        }
      }
    }
    else // If the change is negative
    {
      // And if the current color does not match the expected color
      if((lastColor.White + WValue) < color.White)
      {
        // And, for protection, the current color is not less than the target color.
        if(color.White > targetColor.White)
        {
          // Decrease the current color
          color.White--;
          sendCommand("WHITEDOWN");
        }
      } 
    }
  }
}

// Print status messages to the LCD
void printLCD()
{
  // For the first 10 seconds of the minute show the target color for this fade
  if(second() < 10)
  {
    if(printState != 0) // If this message has not already been printed
    {
      printState = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Target Color:");
      lcd.setCursor(0, 1);
      lcd.print(targetColor.Red);
      lcd.print(",");
      lcd.print(targetColor.Green);
      lcd.print(",");
      lcd.print(targetColor.Blue);
      lcd.print(",");
      lcd.print(targetColor.White);
    }
  }
  // For the next 10 seconds of the minute show the current color of the lights
  else if(second() < 20)
  {
    if(printState != 1) // If this message has not already been printed
    {
      printState = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Current Color:");
      lcd.setCursor(0, 1);
      lcd.print(color.Red);
      lcd.print(",");
      lcd.print(color.Green);
      lcd.print(",");
      lcd.print(color.Blue);
      lcd.print(",");
      lcd.print(color.White);
    }
  }
  // For the rest of the minute show the current time
  else if(printState != 2) // If this message has not already been printed
  {
    if(printState != 2)
    {
      printState = 2;
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(hour());
      printDigits(minute());
    }
    else if (!isTimeSet)
    {
      printState = -1;
      lcd.clear();
    }
  }
}

void setTargetColor(int thisHour, int thisMinute)
{
  // If a fade is not in progress, pick the next fade!
  if(!isFadeInProgress)
  {
    lastColor = color;
    startingSeconds = now();
    durationSeconds = 3600; // The function currently only sets one hour fades
    isFadeInProgress = true;

    // A target color will be picked based on the hour
    switch(thisHour)
    {
    case 0:
      targetColor = MIDNIGHT;
      break;
    case 1:
      targetColor = ONE;
      break;
    case 2:
      targetColor = TWO;
      break;
    case 3:
      targetColor = THREE;
      break;
    case 4:
      targetColor = FOUR;
      break;
    case 5:
      targetColor = FIVE;
      break;
    case 6:
      targetColor = SIX;
      break;
    case 7:
      targetColor = SEVEN;
      break;
    case 8:
      targetColor = EIGHT;
      break;
    case 9:
      targetColor = NINE;
      break;
    case 10:
      targetColor = TEN;
      break;
    case 11:
      targetColor = ELEVEN;
      break;
    case 12:
      targetColor = TWELVE;
      break;
    case 13:
      targetColor = THIRTEEN;
      break;
    case 14:
      targetColor = FOURTEEN;
      break;
    case 15:
      targetColor = FIFTEEN;
      break;
    case 16:
      targetColor = SIXTEEN;
      break;
    case 17:
      targetColor = SEVENTEEN;
      break;
    case 18:
      targetColor = EIGHTTEEN;
      break;
    case 19:
      targetColor = NINETEEN;
      break;
    case 20:
      targetColor = TWENTY;
      break;
    case 21:
      targetColor = TWENTYONE;
      break;
    case 22:
      targetColor = TWENTYTWO;
      break;
    case 23:
      targetColor = TWENTYTHREE;
      break;
    }
  }
}

// Reset the lights and all vars to 0,0,1,0
void resetLights()
{
  sendCommand("M4");
  color.Red = 0;
  color.Green = 0;
  color.Blue = 1;
  color.White = 0;
  targetColor.Red = 0;
  targetColor.Green = 0;
  targetColor.Blue = 1;
  targetColor.White = 0;
  lastColor.Red = 0;
  lastColor.Blue = 1;
  lastColor.Green = 0;
  lastColor.White = 0;
}

// Send an IR command based on a string command name
void sendCommand(String commandName)
{
  if (commandName == "POWER")
  {
    commandData = POWER;
  }
  else if (commandName == "FULLORANGE")
  {
    commandData = FULLORANGE;
  }
  else if (commandName == "FULLLIGHTBLUE")
  {
    commandData = FULLLIGHTBLUE;
  }
  else if (commandName == "FULLPURPLE")
  {
    commandData = FULLPURPLE;
  }
  else if (commandName == "FULLWHITE")
  {
    commandData = FULLWHITE;
  }
  else if (commandName == "FULLYELLOW")
  {
    commandData = FULLYELLOW;
  }
  else if (commandName == "FULLBLUE")
  {
    commandData = FULLBLUE;
  }
  else if (commandName == "REDUP")
  {
    commandData = REDUP;
  }
  else if (commandName == "REDDOWN")
  {
    commandData = REDDOWN;
  }
  else if (commandName == "GREENUP")
  {
    commandData = GREENUP;
  }
  else if (commandName == "GREENDOWN")
  {
    commandData = GREENDOWN;
  }
  else if (commandName == "BLUEUP")
  {
    commandData = BLUEUP;
  }
  else if (commandName == "BLUEDOWN")
  {
    commandData = BLUEDOWN;
  }
  else if (commandName == "WHITEUP")
  {
    commandData = WHITEUP;
  }
  else if (commandName == "WHITEDOWN")
  {
    commandData = WHITEDOWN;
  }
  else if (commandName == "M1")
  {
    commandData = M1;
  }
  else if (commandName == "M2")
  {
    commandData = M2;
  }
  else if (commandName == "M3")
  {
    commandData = M3;
  }
  else if (commandName == "M4")
  {
    commandData = M4;
  }
  else if (commandName == "MOONLIGHT")
  {
    commandData = MOONLIGHT;
  }
  else if (commandName == "MOONDARK")
  {
    commandData = MOONDARK;
  }
  else if (commandName == "MOONCLOUDS")
  {
    commandData = MOONCLOUDS;
  }
  else if (commandName == "SUNRISE")
  {
    commandData = SUNRISE;
  }
  else if (commandName == "CLOUDS1")
  {
    commandData = CLOUDS1;
  }
  else if (commandName == "CLOUDS2")
  {
    commandData = CLOUDS2;
  }
  else if (commandName == "CLOUDS3")
  {
    commandData = CLOUDS3;
  }
  else if (commandName == "CLOUDS4")
  {
    commandData = CLOUDS4;
  }
  else if (commandName == "CLOUDS5")
  {
    commandData = CLOUDS5;
  }
  else if (commandName == "STORM1")
  {
    commandData = STORM1;
  }
  else if (commandName == "STORM2")
  {
    commandData = STORM2;
  }
  else if (commandName == "STORM3")
  {
    commandData = STORM3;
  }
  sendIRAndPrint(commandName, commandData);
}

// Send the IR command and print a debug message to the LCD
void sendIRAndPrint(String commandName, unsigned long command)
{
  printCommand(commandName);
  irsend.sendNEC(command, 32);
  delay(333);
}

// Print an IR command name to the LCD
void printCommand(String commandName)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending code: ");
  lcd.setCursor(0, 1);
  lcd.print(commandName);
  printState = -1;
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits);
}



