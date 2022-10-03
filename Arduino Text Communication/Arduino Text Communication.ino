////////////////////////////////////////////
//      -Arduino Text Communication-      //
////////////////////////////////////////////
//Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>

//Initialize NRF24L01
RF24 radio(9, 10);//(CE, CSN)
const uint64_t ID[2] = {0xE8E8F0F0E1LL, 0xE8E8F0F0E2LL};

//Initialize OLED
//Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

///////////////////////////
//      -VARIABLES-      //
///////////////////////////
//Data to Send
struct dataPackage {
  char msg[30];
  int msgLen[1];
};
dataPackage data;

//Pins
const int leftPin = 2;
const int rightPin = 3;
const int upPin = 4;
const int downPin = 5;
const int selectPin = 6;
const int switchPin = 7;
const int buzzerPin = 8;
const int receiveLED = A2;//red
const int transmitLED = A3;//yellow

//timeDelay
int buttonDelay = 0;
int selectDelay = 0;
int radioDelay = 0;
int blinkDelay = 0;

//drawMenu
bool show = true;
bool letters = true;
bool capitalize = false;

//drawCursor
int x = 0;
int y = 33;
int width = 7;
int height = 11;
bool select = false;
bool transmit = false;

//drawText
int index = 0;
char msgToSend[30];
int num = 0;

//receiveText
int msgFlag = 0;
char convertedMsg[30];
bool displayMsg = false;
bool ringFlag = true;

//runRadio
bool radioON = false;

//drawBlink
int xBlink = 0;
int yBlink1 = 0;
int yBlink2 = 0;

///////////////////////
//      -SETUP-      //
///////////////////////
void setup() {
  atcSetup();
}//setup

//////////////////////
//      -LOOP-      //
//////////////////////
void loop() {
  atcLoop();
}//loop

///////////////////////////
//      -FUNCTIONS-      //
///////////////////////////
void atcSetup() {
  //NRF24L01 Setup
  radio.begin();
  radio.enableAckPayload();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, ID[0]);// <--- (1, ID[#])
  radio.openWritingPipe(ID[1]);// <--- (ID[#])
  //OLED Setup
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.begin(0x3C, true);
  display.display();
  delay(500);
  display.clearDisplay();
  //Pin Setup
  pinMode(leftPin, INPUT_PULLUP);
  pinMode(rightPin, INPUT_PULLUP);
  pinMode(upPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(selectPin, INPUT_PULLUP);
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(receiveLED, OUTPUT);
  pinMode(transmitLED, OUTPUT);
  //Test Buzzer
  digitalWrite(buzzerPin, HIGH);
  delay(50);
  digitalWrite(buzzerPin, LOW);
  //Begin Serial Communication
  //Serial.begin(9600);
}//atcSetup

void atcLoop() {
  display.clearDisplay();
  timeDelay();
  drawMenu();
  drawCursor();
  drawText();
  drawBlink();
  ledControl();
  transmitText();
  receiveText();
  runRadio();
  display.display();
}//atcLoop

void timeDelay() {
  if (buttonDelay <= 3) {
    buttonDelay++;
  }
  if (selectDelay <= 3) {
    selectDelay++;
  }
  if (radioDelay <= 20) {
    radioDelay++;
  }
  if (blinkDelay <= 20) {
    blinkDelay++;
  }
}//timeDelay

void drawMenu() {
  if (show == true) {
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.drawLine(0, 31, 105, 31, SH110X_WHITE);//(x1, y1, x2, y2) horizontal
    display.drawLine(105, 21, 105, 64, SH110X_WHITE);//(x1, y1, x2, y2) verticle
    display.drawLine(0, 21, 128, 21, SH110X_WHITE);//(x1, y1, x2, y2) horizontal
    display.setCursor(109, 25);
    display.println("snd");
    display.setCursor(109, 35);
    display.println("ext");
    display.setCursor(109, 45);
    display.println("cap");
    display.setCursor(109, 55);
    display.println("del");
    if (letters == true && capitalize == false) {
      display.setCursor(1, 35);
      display.println("a b c d e f g h i");
      display.setCursor(1, 45);
      display.println("j k l m n o p q r");
      display.setCursor(1, 55);
      display.println("s t u v w x y z _");
    }
    if (letters == true && capitalize == true) {
      display.setCursor(1, 35);
      display.println("A B C D E F G H I");
      display.setCursor(1, 45);
      display.println("J K L M N O P Q R");
      display.setCursor(1, 55);
      display.println("S T U V W X Y Z _");
    }
    if (letters == false) {
      display.setCursor(1, 35);
      display.println("1 2 3 4 5 6 7 8 9");
      display.setCursor(1, 45);
      display.println("0 # & % + - = / *");
      display.setCursor(1, 55);
      display.println(". , ? ! ' : ; ( )");
    }
  }
}//drawMenu

void drawCursor() {
  if (buttonDelay >= 3 && show == true) {
    if(digitalRead(upPin) == LOW) {
      y -= 10;
      buttonDelay = 0;
      transmit = false;
    }
    if(digitalRead(downPin) == LOW) {
      y += 10;
      buttonDelay = 0;
      transmit = false;
    }
    if(digitalRead(leftPin) == LOW) {
      x -= 12;
      buttonDelay = 0;
      transmit = false;
    }
    if(digitalRead(rightPin) == LOW) {
      x += 12;
      buttonDelay = 0;
      transmit = false;
    }
    //limit cursor movement
    if (x < 0) {
      x = 108;
    }
    if (x > 108) {
      x = 0;
    }
    if (x == 108) {
      if (y < 23) {
        y = 53;
      }
      if (y > 53) {
        y = 23;
      }
    }
    else {
      if (y < 33) {
        y = 53;
      }
      if (y > 53) {
        y = 33;
      }
    }
    if (y == 23) {
      x = 108;
    }
    //change rectangle size based on location
    if (x == 108) {
      width = 20;
    }
    else {
      width = 7;
    }
  }
  if (selectDelay >= 3 && digitalRead(selectPin) == LOW) {
    selectDelay = 0;
    select = true;
    if (digitalRead(switchPin) == LOW) {
      digitalWrite(buzzerPin, HIGH);
    }
  }
  else {
    digitalWrite(buzzerPin, LOW);
  }
  //draw rectangle
  if (show == true) {
    display.drawRect(x, y, width, height, SH110X_WHITE);
  }
}//drawCursor

void drawText() {
  if (show == true) {
    if (capitalize == false && letters == true) {
      switch (y) {
        case 23:
          if (select == true) {
            select = false;
            transmit = true;
            num++;
          }
        case 33:
          switch (x) {
            case 0:
              if (select == true) {
                select = false;
                msgToSend[index] = 'a';
                index++;
              }
            break;
            case 12:
              if (select == true) {
                select = false;
                msgToSend[index] = 'b';
                index++;
              }
            break;
            case 24:
              if (select == true) {
                select = false;
                msgToSend[index] = 'c';
                index++;
              }
            break;
            case 36:
              if (select == true) {
                select = false;
                msgToSend[index] = 'd';
                index++;
              }
            break;
            case 48:
              if (select == true) {
                select = false;
                msgToSend[index] = 'e';
                index++;
              }
            break;
            case 60:
              if (select == true) {
                select = false;
                msgToSend[index] = 'f';
                index++;
              }
            break;
            case 72:
              if (select == true) {
                select = false;
                msgToSend[index] = 'g';
                index++;
              }
            break;
            case 84:
              if (select == true) {
                select = false;
                msgToSend[index] = 'h';
                index++;
              }
            break;
            case 96:
              if (select == true) {
                select = false;
                msgToSend[index] = 'i';
                index++;
              }
            break;
            case 108:
              if (select == true) {
                select = false;
                letters = false;
              }
            break;
          }
        break;
  ///////////////////////////////////////
        case 43:
          switch (x) {
            case 0:
              if (select == true) {
                select = false;
                msgToSend[index] = 'j';
                index++;
              }
            break;
            case 12:
              if (select == true) {
                select = false;
                msgToSend[index] = 'k';
                index++;
              }
            break;
            case 24:
              if (select == true) {
                select = false;
                msgToSend[index] = 'l';
                index++;
              }
            break;
            case 36:
              if (select == true) {
                select = false;
                msgToSend[index] = 'm';
                index++;
              }
            break;
            case 48:
              if (select == true) {
                select = false;
                msgToSend[index] = 'n';
                index++;
              }
            break;
            case 60:
              if (select == true) {
                select = false;
                msgToSend[index] = 'o';
                index++;
              }
            break;
            case 72:
              if (select == true) {
                select = false;
                msgToSend[index] = 'p';
                index++;
              }
            break;
            case 84:
              if (select == true) {
                select = false;
                msgToSend[index] = 'q';
                index++;
              }
            break;
            case 96:
              if (select == true) {
                select = false;
                msgToSend[index] = 'r';
                index++;
              }
            break;
            case 108:
              if (select == true) {
                select = false;
                capitalize = true;
              }
            break;
          }
        break;
  ///////////////////////////////////////
        case 53:
          switch (x) {
            case 0:
              if (select == true) {
                select = false;
                msgToSend[index] = 's';
                index++;
              }
            break;
            case 12:
              if (select == true) {
                select = false;
                msgToSend[index] = 't';
                index++;
              }
            break;
            case 24:
              if (select == true) {
                select = false;
                msgToSend[index] = 'u';
                index++;
              }
            break;
            case 36:
              if (select == true) {
                select = false;
                msgToSend[index] = 'v';
                index++;
              }
            break;
            case 48:
              if (select == true) {
                select = false;
                msgToSend[index] = 'w';
                index++;
              }
            break;
            case 60:
              if (select == true) {
                select = false;
                msgToSend[index] = 'x';
                index++;
              }
            break;
            case 72:
              if (select == true) {
                select = false;
                msgToSend[index] = 'y';
                index++;
              }
            break;
            case 84:
              if (select == true) {
                select = false;
                msgToSend[index] = 'z';
                index++;
              }
            break;
            case 96:
              if (select == true) {
                select = false;
                msgToSend[index] = ' ';
                index++;
              }
            break;
            case 108:
              if (select == true) {
                select = false;
                index--;
                msgToSend[index] = ' ';
              }
            break;
          }//switch x
        break;
      }//switch y
    }//if
  //////////////////////////////////////////////////////////////////////////////////////////////
    else if (capitalize == true && letters == true) {
      switch (y) {
        case 23:
          if (select == true) {
            select = false;
            transmit = true;
            num++;
          }
        case 33:
          switch (x) {
            case 0:
              if (select == true) {
                select = false;
                msgToSend[index] = 'A';
                index++;
              }
            break;
            case 12:
              if (select == true) {
                select = false;
                msgToSend[index] = 'B';
                index++;
              }
            break;
            case 24:
              if (select == true) {
                select = false;
                msgToSend[index] = 'C';
                index++;
              }
            break;
            case 36:
              if (select == true) {
                select = false;
                msgToSend[index] = 'D';
                index++;
              }
            break;
            case 48:
              if (select == true) {
                select = false;
                msgToSend[index] = 'E';
                index++;
              }
            break;
            case 60:
              if (select == true) {
                select = false;
                msgToSend[index] = 'F';
                index++;
              }
            break;
            case 72:
              if (select == true) {
                select = false;
                msgToSend[index] = 'G';
                index++;
              }
            break;
            case 84:
              if (select == true) {
                select = false;
                msgToSend[index] = 'H';
                index++;
              }
            break;
            case 96:
              if (select == true) {
                select = false;
                msgToSend[index] = 'I';
                index++;
              }
            break;
            case 108:
              if (select == true) {
                select = false;
                letters = false;
              }
            break;
          }
        break;
  ///////////////////////////////////////
        case 43:
          switch (x) {
            case 0:
              if (select == true) {
                select = false;
                msgToSend[index] = 'J';
                index++;
              }
            break;
            case 12:
              if (select == true) {
                select = false;
                msgToSend[index] = 'K';
                index++;
              }
            break;
            case 24:
              if (select == true) {
                select = false;
                msgToSend[index] = 'L';
                index++;
              }
            break;
            case 36:
              if (select == true) {
                select = false;
                msgToSend[index] = 'M';
                index++;
              }
            break;
            case 48:
              if (select == true) {
                select = false;
                msgToSend[index] = 'N';
                index++;
              }
            break;
            case 60:
              if (select == true) {
                select = false;
                msgToSend[index] = 'O';
                index++;
              }
            break;
            case 72:
              if (select == true) {
                select = false;
                msgToSend[index] = 'P';
                index++;
              }
            break;
            case 84:
              if (select == true) {
                select = false;
                msgToSend[index] = 'Q';
                index++;
              }
            break;
            case 96:
              if (select == true) {
                select = false;
                msgToSend[index] = 'R';
                index++;
              }
            break;
            case 108:
              if (select == true) {
                select = false;
                capitalize = false;
              }
            break;
          }
        break;
  ///////////////////////////////////////
        case 53:
          switch (x) {
            case 0:
              if (select == true) {
                select = false;
                msgToSend[index] = 'S';
                index++;
              }
            break;
            case 12:
              if (select == true) {
                select = false;
                msgToSend[index] = 'T';
                index++;
              }
            break;
            case 24:
              if (select == true) {
                select = false;
                msgToSend[index] = 'U';
                index++;
              }
            break;
            case 36:
              if (select == true) {
                select = false;
                msgToSend[index] = 'V';
                index++;
              }
            break;
            case 48:
              if (select == true) {
                select = false;
                msgToSend[index] = 'W';
                index++;
              }
            break;
            case 60:
              if (select == true) {
                select = false;
                msgToSend[index] = 'X';
                index++;
              }
            break;
            case 72:
              if (select == true) {
                select = false;
                msgToSend[index] = 'Y';
                index++;
              }
            break;
            case 84:
              if (select == true) {
                select = false;
                msgToSend[index] = 'Z';
                index++;
              }
            break;
            case 96:
              if (select == true) {
                select = false;
                msgToSend[index] = ' ';
                index++;
              }
            break;
            case 108:
              if (select == true) {
                select = false;
                index--;
                msgToSend[index] = ' ';
              }
            break;
          }//switch x
        break;
      }//switch y
    }//else if
  /////////////////////////////////////////////////////////////////////////////////////////////
    else if (letters == false) {
      switch (y) {
        case 23:
          if (select == true) {
            select = false;
            transmit = true;
            num++;
          }
        case 33:
          switch (x) {
            case 0:
              if (select == true) {
                select = false;
                msgToSend[index] = '1';
                index++;
              }
            break;
            case 12:
              if (select == true) {
                select = false;
                msgToSend[index] = '2';
                index++;
              }
            break;
            case 24:
              if (select == true) {
                select = false;
                msgToSend[index] = '3';
                index++;
              }
            break;
            case 36:
              if (select == true) {
                select = false;
                msgToSend[index] = '4';
                index++;
              }
            break;
            case 48:
              if (select == true) {
                select = false;
                msgToSend[index] = '5';
                index++;
              }
            break;
            case 60:
              if (select == true) {
                select = false;
                msgToSend[index] = '6';
                index++;
              }
            break;
            case 72:
              if (select == true) {
                select = false;
                msgToSend[index] = '7';
                index++;
              }
            break;
            case 84:
              if (select == true) {
                select = false;
                msgToSend[index] = '8';
                index++;
              }
            break;
            case 96:
              if (select == true) {
                select = false;
                msgToSend[index] = '9';
                index++;
              }
            break;
            case 108:
              if (select == true) {
                select = false;
                letters = true;
              }
            break;
          }
        break;
  ///////////////////////////////////////
        case 43:
          switch (x) {
            case 0:
              if (select == true) {
                select = false;
                msgToSend[index] = '0';
                index++;
              }
            break;
            case 12:
              if (select == true) {
                select = false;
                msgToSend[index] = '#';
                index++;
              }
            break;
            case 24:
              if (select == true) {
                select = false;
                msgToSend[index] = '&';
                index++;
              }
            break;
            case 36:
              if (select == true) {
                select = false;
                msgToSend[index] = '%';
                index++;
              }
            break;
            case 48:
              if (select == true) {
                select = false;
                msgToSend[index] = '+';
                index++;
              }
            break;
            case 60:
              if (select == true) {
                select = false;
                msgToSend[index] = '-';
                index++;
              }
            break;
            case 72:
              if (select == true) {
                select = false;
                msgToSend[index] = '=';
                index++;
              }
            break;
            case 84:
              if (select == true) {
                select = false;
                msgToSend[index] = '/';
                index++;
              }
            break;
            case 96:
              if (select == true) {
                select = false;
                msgToSend[index] = '*';
                index++;
              }
            break;
            case 108:
              if (select == true) {
                select = false;
                capitalize = true;
              }
            break;
          }
        break;
  ///////////////////////////////////////
        case 53:
          switch (x) {
            case 0:
              if (select == true) {
                select = false;
                msgToSend[index] = '.';
                index++;
              }
            break;
            case 12:
              if (select == true) {
                select = false;
                msgToSend[index] = ',';
                index++;
              }
            break;
            case 24:
              if (select == true) {
                select = false;
                msgToSend[index] = '?';
                index++;
              }
            break;
            case 36:
              if (select == true) {
                select = false;
                msgToSend[index] = '!';
                index++;
              }
            break;
            case 48:
              if (select == true) {
                select = false;
                msgToSend[index] = '\'';
                index++;
              }
            break;
            case 60:
              if (select == true) {
                select = false;
                msgToSend[index] = ':';
                index++;
              }
            break;
            case 72:
              if (select == true) {
                select = false;
                msgToSend[index] = ';';
                index++;
              }
            break;
            case 84:
              if (select == true) {
                select = false;
                msgToSend[index] = '(';
                index++;
              }
            break;
            case 96:
              if (select == true) {
                select = false;
                msgToSend[index] = ')';
                index++;
              }
            break;
            case 108:
              if (select == true) {
                select = false;
                index--;
                msgToSend[index] = ' ';
              }
            break;
          }//switch x
        break;
      }//switch y
    }//else if
    if (index <= 0) {
      index = 0;
    }
    if (index > 29) {
      index = 29;
    }
    display.setCursor(0, 0);
    display.println(msgToSend);
    display.setCursor(0, 23);
    display.println(index);
  }
}//drawText

void ledControl() {
  if (radioON == true) {
    digitalWrite(transmitLED, HIGH);
    digitalWrite(receiveLED, LOW);
  }
  else {
    digitalWrite(transmitLED, LOW);
    digitalWrite(receiveLED, HIGH);
  }
}//ledControl

void transmitText() {
  if (radioON == true) {
    for (int i = 0; i <= sizeof(msgToSend); i++) {
      data.msg[i] = msgToSend[i];
    }
    data.msgLen[0] = num;
    radio.stopListening();
    radio.write(&data, sizeof(dataPackage));
  }
}//sendText

void receiveText() {
  if (radioON == false) {
    radio.startListening();
    if (radio.available()) {
      radio.read(&data, sizeof(dataPackage));
      transmit = false;
      if (data.msgLen[0] != msgFlag) {
        show = false;
        displayMsg = true;
          if (ringFlag == true && digitalRead(switchPin) == LOW) {
          digitalWrite(buzzerPin, HIGH);
          ringFlag = false;
        }
      }
    }
    if (displayMsg == true) {
      display.setCursor(0, 0);
      for (int j = 0; j < 30; j++) {
        convertedMsg[j] = data.msg[j];
        display.print(convertedMsg[j]);
      }
    }
  }
  if (select == true) {
    select = false;
    show = true;
    displayMsg = false;
    msgFlag = data.msgLen[0];
    for (int k = 0; k < 30; k++) {
      convertedMsg[k] = " ";
    }
    ringFlag = true;
  }
}//receiveText

void runRadio() {
  if (transmit == false) {
    radioON = false;
  }
  if (transmit == true) {
    if (radioDelay <= 10) {
      radioON = false;
    }
    if (radioDelay >= 10) {
      radioON = true;
    }
    if (radioDelay >= 20) {
      radioDelay = 0;
    }
  }
}//runRadio

void drawBlink() {
  if (show == true) {
    if (xBlink <= 125) {
      xBlink = index * 6;
      yBlink1 = 0;
      yBlink2 = 6;
    }
    if (xBlink >= 125) {
      xBlink = (index - 21) * 6;
      yBlink1 = 8;
      yBlink2 = 14;
    }
    if (blinkDelay <=10) {
      display.drawLine(xBlink, yBlink1, xBlink, yBlink2, SH110X_WHITE);//(x1, y1, x2, y2) verticle
    }
    if (blinkDelay >= 20) {
      blinkDelay = 0;
    }
  }
}//drawBlink

