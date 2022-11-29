////////////////////////////////////
//      -OLED RF Controller-      //
////////////////////////////////////
//Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

//Initialize NRF24L01
RF24 radio(9, 10);//(CE, CSN)

//Initialize OLED
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

///////////////////////////
//      -VARIABLES-      //
///////////////////////////
const byte address[6] = "00001";
struct Data_Package {
  byte X1;
  byte Y1;
  byte X2;
  byte Y2;
  byte b1;
  byte B2;
  byte B3;
  byte B4;
  byte B5;
  byte T1;
  byte T2;
  byte T3;
  byte T4;
  byte T5;
  byte S1;
  byte S2;
  byte S3;
  byte S4;
  byte S5;
};
Data_Package data;

//buzzer pin
const int buzzer_Pin = 5;

//runNRF24L01
const int X1_Pin=A0, Y1_Pin=A1, X2_Pin=A2, Y2_Pin=A3;
const int B1_Pin=3, B2_Pin=2;
int B1_Val=0, B2_Val=0, B3_Val=0, B4_Val=0, B5_Val=0;
int T1_Val=0, T2_Val=0, T3_Val=0, T4_Val=0, T5_Val=0;
int S1_Val=0, S2_Val=0, S3_Val=0, S4_Val=0, S5_Val=0;

//timeDelay
int cursorDelay = 0;
int actionDelay = 0;

//drawCursor
int cursorPot = 0;
int cursorY = 11, yOffset = 0;
int num = 0, num2 = 0, num3 = 0;

//drawMenu
bool B1_Flag = false, B1_Flag2 = false, B2_Flag = false;
bool menu = false, action = false, buzzer = false;
bool toggle1 = false, toggle2 = false, toggle3 = false, toggle4 = false, toggle5 = false;
int sliderPot = 0, slider1 = 0, slider2 = 0, slider3 = 0, slider4 = 0, slider5 = 0;
bool multiply = true;

///////////////////////
//      -SETUP-      //
///////////////////////
void setup() {
  //Begin Serial Communication
  //Serial.begin(9600);

  //Pin Setup
  pinMode(B1_Pin, INPUT_PULLUP);
  pinMode(B2_Pin, INPUT_PULLUP);
  pinMode(buzzer_Pin, OUTPUT);

  //NRF24L01 Setup
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  

  //OLED Setup
  display.begin(0x3C, true);
  display.display();
  delay(500);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  //Initial Values
  data.X1 = 0;
  data.Y1 = 0;
  data.X2 = 0;
  data.Y2 = 0;
  data.b1 = 0;
  data.B2 = 0;
  data.B3 = 0;
  data.T1 = 0;
  data.T2 = 0;
  data.T3 = 0;
  data.S1 = 0;
  data.S2 = 0;
  data.S3 = 0;

}//setup

//////////////////////
//      -LOOP-      //
//////////////////////
void loop() {
  runNRF24L01();
  runOLED();
  timeDelay();
  runBuzzer();
}//loop

///////////////////////////
//      -FUNCTIONS-      //
///////////////////////////
void runNRF24L01() {
  if (menu == false) {
    data.X1 = map(analogRead(X1_Pin), 0, 1023, 0, 255);
    data.Y1 = map(analogRead(Y1_Pin), 0, 1023, 0, 255);
    data.X2 = map(analogRead(X2_Pin), 0, 1023, 0, 255);
    data.Y2 = map(analogRead(Y2_Pin), 0, 1023, 0, 255);
  }
  data.b1 = B1_Val;
  data.B2 = B2_Val;
  data.B3 = B3_Val;
  data.B4 = B4_Val;
  data.B5 = B5_Val;
  data.T1 = T1_Val;
  data.T2 = T2_Val;
  data.T3 = T3_Val;
  data.T4 = T4_Val;
  data.T5 = T5_Val;
  data.S1 = S1_Val;
  data.S2 = S2_Val;
  data.S3 = S3_Val;
  data.S4 = S4_Val;
  data.S5 = S5_Val;
  radio.write(&data, sizeof(Data_Package));
}//runNRF24L01

void runOLED() {
  display.clearDisplay();
  drawCursor();
  drawMenu();
  display.display();
}//runOLED

void timeDelay() {
  if (cursorDelay <= 2) {
    cursorDelay++;
  }
  if (actionDelay <= 10) {
    actionDelay++;
  }
}//timeDelay

void drawText(String Name, int Size, int xPos, int yPos, bool ignore) {
  if (yPos > 11 || ignore == true) {
    display.setTextSize(Size);
    display.setCursor(xPos, yPos);
    display.println(Name);
  }
}//drawText

void drawCursor() {
  if (menu == true) {
    cursorPot = map(analogRead(X1_Pin), 0, 1023, 0, 100);
    
    if (cursorPot <= 30 && cursorDelay >= 2) {
      cursorY += 17;
      if (action == false) {
        num++;
      }
      num3++;
      cursorDelay = 0;
    }
    if (cursorPot >= 70 && cursorDelay >= 2) {
      cursorY -= 17;
      if (action == false) {
        num--;
      }
      num3--;
      cursorDelay = 0;
    }
    
    //limits movement
    if (cursorY > 45) {
      cursorY = 45;
      yOffset -= 17;
    }
    if (cursorY < 11) { 
      cursorY = 11;
      yOffset += 17;
    }
    if (yOffset >= 0) {
      yOffset = 0;
    }
    if (num <= 0) {
      num = 0;
    }
    if (num3 <= 0) {
      num3 = 0;
    }

    if (cursorY >= 28 && num2 == 3) {
      cursorY = 28;
      num3 = 1;
    }
    if (yOffset < -51 && num2 == 0) {
      yOffset = -51;
      num3 = 5;
    }
    if (yOffset < -51 && num2 == 1) {
      yOffset = -51;
      num3 = 5;
    }
    if (yOffset < -51 && num2 == 2) {
      yOffset = -51;
      num3 = 5;
    }
    
    if (yOffset < -17 && action == false) {
      yOffset = -17;
      num = 3;
    }
    
    display.drawRect(0, cursorY, 128, 18, SH110X_WHITE);
  }
}//drawCursor

void drawMenu() {
  //menu button
  if (digitalRead(B2_Pin) == LOW) {
    B2_Flag = true;
  }
  if (B2_Flag == true) {
    if(digitalRead(B2_Pin) == HIGH) {
      B2_Flag = false;
      if (menu == false) {
        menu = true;
      }
      else {
        menu = false;
      }
    }
  }

  display.drawLine(0, 9, 128, 9, SH110X_WHITE);

  if (menu == false) {
    drawText("RADIO", 1, 48, 0, true);
  }

  if (menu == true && action == false) {
    drawText("MAIN MENU", 1, 37, 0, true);
    drawText("Buttons", 2, 3, 13+yOffset, false);
    drawText("Switches", 2, 3, 30+yOffset, false);
    drawText("Sliders", 2, 3, 47+yOffset, false);
    drawText("Buzzer", 2, 3, 64+yOffset, false);
    switch (num) {
      case 0:
        if (digitalRead(B1_Pin) == LOW) {
          B1_Flag = true;
        }
        if (B1_Flag == true) {
          if(digitalRead(B1_Pin) == HIGH) {
            B1_Flag = false;
            num = 0;
            num2 = 0;
            num3 = 0;
            yOffset = 0;
            cursorY = 11;
            action = true;
          }
        }
      break;
      case 1:
        if (digitalRead(B1_Pin) == LOW) {
          B1_Flag = true;
        }
        if (B1_Flag == true) {
          if(digitalRead(B1_Pin) == HIGH) {
            B1_Flag = false;
            num = 0;
            num2 = 1;
            num3 = 0;
            yOffset = 0;
            cursorY = 11;
            action = true;
          }
        }
      break;
      case 2:
        if (digitalRead(B1_Pin) == LOW) {
          B1_Flag = true;
        }
        if (B1_Flag == true) {
          if(digitalRead(B1_Pin) == HIGH) {
            B1_Flag = false;
            num = 0;
            num2 = 2;
            num3 = 0;
            yOffset = 0;
            cursorY = 11;
            action = true;
            if (multiply == false) {
              multiply = true;
            }
            else {
              multiply = false;
            }
          }
        }
      break;
      case 3:
        if (digitalRead(B1_Pin) == LOW) {
          B1_Flag = true;
        }
        if (B1_Flag == true) {
          if(digitalRead(B1_Pin) == HIGH) {
            B1_Flag = false;
            num = 0;
            num2 = 3;
            num3 = 0;
            yOffset = 0;
            cursorY = 11;
            action = true;
          }
        }
      break; 
    }
  }

  if (action == true && menu == true) {
    switch (num2) {
      case 0:
        drawText("BUTTONS", 1, 42, 0, true);
        drawText("B1", 2, 3, 13+yOffset, false);
        drawText("B2", 2, 3, 30+yOffset, false);
        drawText("B3", 2, 3, 47+yOffset, false);
        drawText("B4", 2, 3, 64+yOffset, false);
        drawText("B5", 2, 3, 81+yOffset, false);
        drawText("Back", 2, 3, 98+yOffset, false);
        switch (num3) {
          case 0:
            if (digitalRead(B1_Pin) == LOW) {
              drawText("B1 PRESSED", 1, 60, 16+yOffset, true);
              B1_Val = 1;
            }
            else {
              B1_Val = 0;
            }
          break;
          case 1:
            if (digitalRead(B1_Pin) == LOW) {
              drawText("B2 PRESSED", 1, 60, 33+yOffset, true);
              B2_Val = 1;
            }
            else {
              B2_Val = 0;
            }
          break;
          case 2:
            if (digitalRead(B1_Pin) == LOW) {
              drawText("B3 PRESSED", 1, 60, 50+yOffset, true);
              B3_Val = 1;
            }
            else {
              B3_Val = 0;
            }
          break;
          case 3:
            if (digitalRead(B1_Pin) == LOW) {
              drawText("B4 PRESSED", 1, 60, 67+yOffset, true);
              B4_Val = 1;
            }
            else {
              B4_Val = 0;
            }
          break;
          case 4:
            if (digitalRead(B1_Pin) == LOW) {
              drawText("B5 PRESSED", 1, 60, 84+yOffset, true);
              B5_Val = 1;
            }
            else {
              B5_Val = 0;
            }
          break;
          case 5:
            if (digitalRead(B1_Pin) == LOW) {
              B1_Flag = true;
            }
            if (B1_Flag == true) {
              if(digitalRead(B1_Pin) == HIGH) {
                B1_Flag = false;
                num = 0;
                num2 = -1;
                num3 = 0;
                yOffset = 0;
                cursorY = 11;
                action = false;
              }
            }
          break;
        }
      break;
      case 1:
        drawText("SWITCHES", 1, 36, 0, true);
        drawText("T1", 2, 3, 13+yOffset, false);
        drawText("T2", 2, 3, 30+yOffset, false);
        drawText("T3", 2, 3, 47+yOffset, false);
        drawText("T4", 2, 3, 64+yOffset, false);
        drawText("T5", 2, 3, 81+yOffset, false);
        drawText("Back", 2, 3, 98+yOffset, false);
        if (toggle1 == true) {
          drawText("on", 2, 92, 13+yOffset, false);
          T1_Val = 1;
        }
        else {
          drawText("off", 2, 92, 13+yOffset, false);
          T1_Val = 0;
        }
        if (toggle2 == true) {
          drawText("on", 2, 92, 30+yOffset, false);
          T2_Val = 1;
        }
        else {
          drawText("off", 2, 92, 30+yOffset, false);
          T2_Val = 0;
        }
        if (toggle3 == true) {
          drawText("on", 2, 92, 47+yOffset, false);
          T3_Val = 1;
        }
        else {
          drawText("off", 2, 92, 47+yOffset, false);
          T3_Val = 0;
        }
        if (toggle4 == true) {
          drawText("on", 2, 92, 64+yOffset, false);
          T4_Val = 1;
        }
        else {
          drawText("off", 2, 92, 64+yOffset, false);
          T4_Val = 0;
        }
        if (toggle5 == true) {
          drawText("on", 2, 92, 81+yOffset, false);
          T5_Val = 1;
        }
        else {
          drawText("off", 2, 92, 81+yOffset, false);
          T5_Val = 0;
        }
        switch (num3) {
          case 0:
            if (digitalRead(B1_Pin) == LOW && actionDelay >= 10) {
              if (toggle1 == false) {
                toggle1 = true;
              }
              else {
                toggle1 = false;
              }
              actionDelay = 0;
            }
          break;
          case 1:
            if (digitalRead(B1_Pin) == LOW && actionDelay >= 10) {
              if (toggle2 == false) {
                toggle2 = true;
              }
              else {
                toggle2 = false;
              }
              actionDelay = 0;
            }
          break;
          case 2:
            if (digitalRead(B1_Pin) == LOW && actionDelay >= 10) {
              if (toggle3 == false) {
                toggle3 = true;
              }
              else {
                toggle3 = false;
              }
              actionDelay = 0;
            }
          break;
          case 3:
            if (digitalRead(B1_Pin) == LOW && actionDelay >= 10) {
              if (toggle4 == false) {
                toggle4 = true;
              }
              else {
                toggle4 = false;
              }
              actionDelay = 0;
            }
          break;
          case 4:
            if (digitalRead(B1_Pin) == LOW && actionDelay >= 10) {
              if (toggle5 == false) {
                toggle5 = true;
              }
              else {
                toggle5 = false;
              }
              actionDelay = 0;
            }
          break;
          case 5:
            if (digitalRead(B1_Pin) == LOW) {
              B1_Flag = true;
            }
            if (B1_Flag == true) {
              if(digitalRead(B1_Pin) == HIGH) {
                B1_Flag = false;
                num = 0;
                num2 = -1;
                num3 = 0;
                yOffset = 0;
                cursorY = 11;
                action = false;
              }
            }
          break;
        }
      break;
      case 2:
        drawText("SLIDERS", 1, 42, 0, true);
        drawText("S1", 2, 3, 13+yOffset, false);
        drawText("S2", 2, 3, 30+yOffset, false);
        drawText("S3", 2, 3, 47+yOffset, false);
        drawText("S4", 2, 3, 64+yOffset, false);
        drawText("S5", 2, 3, 81+yOffset, false);
        drawText("Back", 2, 3, 98+yOffset, false);
        sliderPot = map(analogRead(Y2_Pin), 0, 1023, 0, 100);

        drawText(String(slider1), 2, 92, 13+yOffset, false);
        drawText(String(slider2), 2, 92, 30+yOffset, false);
        drawText(String(slider3), 2, 92, 47+yOffset, false);
        drawText(String(slider4), 2, 92, 64+yOffset, false);
        drawText(String(slider5), 2, 92, 81+yOffset, false);

        S1_Val = slider1;
        S2_Val = slider2;
        S3_Val = slider3;
        S4_Val = slider4;
        S5_Val = slider5;

        if (digitalRead(B1_Pin) == LOW) {
          B1_Flag2 = true;
        }
        if (B1_Flag2 == true) {
          if(digitalRead(B1_Pin) == HIGH) {
            B1_Flag2 = false;
            if (multiply == false && num3 != 3) {
              multiply = true;
            }
            else if (num3 != 3) {
              multiply = false;
            }
          }
        }
        
        switch (num3) {
          case 0:
            if (sliderPot <= 30) {
              if (multiply == true) {
                slider1 -= 10;
              }
              else {
                slider1--;
              }
            }
            if (sliderPot >= 70) {
              if (multiply == true) {
                slider1 += 10;
              }
              else {
                slider1++;
              }
            }
            if (slider1 <= 0) {
              slider1 = 0;
            }
            if (slider1 >= 255) {
              slider1 = 255;
            }
          break;
          case 1:
            if (sliderPot <= 30) {
              if (multiply == true) {
                slider2 -= 10;
              }
              else {
                slider2--;
              }
            }
            if (sliderPot >= 70) {
              if (multiply == true) {
                slider2 += 10;
              }
              else {
                slider2++;
              }
            }
            if (slider2 <= 0) {
              slider2 = 0;
            }
            if (slider2 >= 255) {
              slider2 = 255;
            }
          break;
          case 2:
            if (sliderPot <= 30) {
              if (multiply == true) {
                slider3 -= 10;
              }
              else {
                slider3--;
              }
            }
            if (sliderPot >= 70) {
              if (multiply == true) {
                slider3 += 10;
              }
              else {
                slider3++;
              }
            }
            if (slider3 <= 0) {
              slider3 = 0;
            }
            if (slider3 >= 255) {
              slider3 = 255;
            }
          break;
          case 3:
            if (sliderPot <= 30) {
              if (multiply == true) {
                slider4 -= 10;
              }
              else {
                slider4--;
              }
            }
            if (sliderPot >= 70) {
              if (multiply == true) {
                slider4 += 10;
              }
              else {
                slider4++;
              }
            }
            if (slider4 <= 0) {
              slider4 = 0;
            }
            if (slider4 >= 255) {
              slider4 = 255;
            }
          break;
          case 4:
            if (sliderPot <= 30) {
              if (multiply == true) {
                slider5 -= 10;
              }
              else {
                slider5--;
              }
            }
            if (sliderPot >= 70) {
              if (multiply == true) {
                slider5 += 10;
              }
              else {
                slider5++;
              }
            }
            if (slider5 <= 0) {
              slider5 = 0;
            }
            if (slider5 >= 255) {
              slider5 = 255;
            }
          break;
          case 5:
            if (digitalRead(B1_Pin) == LOW) {
              B1_Flag = true;
            }
            if (B1_Flag == true) {
              if(digitalRead(B1_Pin) == HIGH) {
                B1_Flag = false;
                num = 0;
                num2 = -1;
                num3 = 0;
                yOffset = 0;
                cursorY = 11;
                action = false;
              }
            }
          break;
        }
      break;
      case 3:
        drawText("BUZZER", 1, 48, 0, true); 
        drawText("On", 2, 3, 13+yOffset, false);
        drawText("Off", 2, 3, 30+yOffset, false);
        switch (num3) {
          case 0:
            if (digitalRead(B1_Pin) == LOW) {
              B1_Flag = true;
            }
            if (B1_Flag == true) {
              if(digitalRead(B1_Pin) == HIGH) {
                B1_Flag = false;
                num = 0;
                num2 = -1;
                num3 = 0;
                yOffset = 0;
                cursorY = 11;
                action = false;
                buzzer = true;
              }
            }
          break;
          case 1:
            if (digitalRead(B1_Pin) == LOW) {
              B1_Flag = true;
            }
            if (B1_Flag == true) {
              if(digitalRead(B1_Pin) == HIGH) {
                B1_Flag = false;
                num = 0;
                num2 = -1;
                num3 = 0;
                yOffset = 0;
                cursorY = 11;
                action = false;
                buzzer = false;
                digitalWrite(buzzer_Pin, LOW);
              }
            }
          break;
        }
      break;
    }
  }
}//drawMenu

void runBuzzer() {
  if (menu == true && buzzer == true) {
    if (B1_Flag == true) {
      digitalWrite(buzzer_Pin, HIGH);
      analogWrite(buzzer_Pin, 20);
    }
    else {
      digitalWrite(buzzer_Pin, LOW);
    }
  }
}//runBuzzer
