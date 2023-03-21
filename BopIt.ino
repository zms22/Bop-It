/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

//#include "SD.h"
//#define SD_ChipSelectPin 10
#include "PCM.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//TMRpcm tmrpcm;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long threshold = 2000;
int score = 0;

const int background_tone[] = {400, 300, 250, 200, -1, 300, 350};
const unsigned long background_duration[] = {200, 200, 150, 250, 100, 200, 250, 250};

const int defeat_tone[] = {200, 175, 150, 125};
const unsigned long defeat_duration[] = {300, 300, 300, 300};

const int victory_tone[] = {300, 225, 350, 375, -1, 375, -1, 375};
const unsigned long victory_duration[] = {150, 150, 150, 100, 50, 100, 50, 300};

const int success_tone[] = {300, 350, 400, 450};
const unsigned long success_duration[] = {150, 150, 150, 150};

const int reopen_tone[] = {500, -1, 500};
const unsigned long reopen_duration[] = {100, 100, 100};

const int button_tone[] = {-1, 200, -1};
const unsigned long button_duration[] = {100, 100, 100};

const int slide_tone[] = {150, 200, 250, 300, 350, 400};
const unsigned long slide_duration[] = {50, 50, 50, 50, 50, 50};

void setup() {

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;); // Don't proceed, loop forever
  }
 
  display.setTextSize(2); //Setup text size
  display.setTextColor(WHITE); //Setup font color

  //Blue Button
  pinMode(1, OUTPUT);
  pinMode(6, INPUT_PULLUP);

  //Yellow Button
  pinMode(A0, OUTPUT);
  pinMode(5, INPUT_PULLUP);

  //Red Button
  pinMode(A1, OUTPUT);
  pinMode(7, INPUT_PULLUP);

  //Green Button
  pinMode(A2, OUTPUT);
  pinMode(0, INPUT_PULLUP);

  //Lever Switch
  pinMode(8, INPUT_PULLUP);

  //Wait for game to be opened
  while (digitalRead(8) == LOW);
}

void success(void){
  //Make sure to turn off all buttons
  digitalWrite(1, LOW);
  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);

  //First flash sequence (Blue -> Yellow -> Red -> Green)
  digitalWrite(1, HIGH);
  tone(9, success_tone[0], success_duration[0]);
  delay(50);
  digitalWrite(1, LOW);
  digitalWrite(A0, HIGH);
  delay(50);
  digitalWrite(A0, LOW);
  digitalWrite(A1, HIGH);
  delay(50);
  noTone(9);
  tone(9, success_tone[1], success_duration[1]);
  digitalWrite(A1, LOW);
  digitalWrite(A2, HIGH);
  delay(50);
  digitalWrite(A2, LOW);


  //Second flash sequence (Red -> Yellow -> Green -> Blue)
  digitalWrite(A1, HIGH);
  delay(50);
  digitalWrite(A1, LOW);
  digitalWrite(A0, HIGH);
  delay(50);
  noTone(9);
  tone(9, success_tone[2], success_duration[2]);
  digitalWrite(A0, LOW);
  digitalWrite(A2, HIGH);
  delay(50);
  digitalWrite(A2, LOW);
  digitalWrite(1, HIGH);
  delay(50);
  digitalWrite(1, LOW);

  //Third flash sequence (Yellow -> Green -> Blue -> Red)
  digitalWrite(A0, HIGH);
  delay(50);
  noTone(9);
  tone(9, success_tone[3], success_duration[3]);
  digitalWrite(A0, LOW);
  digitalWrite(A2, HIGH);
  delay(50);
  digitalWrite(A2, LOW);
  digitalWrite(1, HIGH);
  delay(50);
  digitalWrite(1, LOW);
  digitalWrite(A1, HIGH);
  delay(50);
  noTone(9);
  digitalWrite(A1, LOW);
}

boolean playing(void)
{
  return TIMSK1 & _BV(OCIE1A);
}

int buttonstatus(){
  if (digitalRead(6) == LOW) return 1;
  if (digitalRead(5) == LOW) return 2;
  if (digitalRead(7) == LOW) return 3;
  if (digitalRead(0) == LOW) return 4;
  return -1;
}

int sliderthreshold(int current_threshold){
  int current_value = analogRead(A3);
  int next_threshold = current_threshold;
  if (current_value > current_threshold && current_value < current_threshold + 100) next_threshold = current_value + 100;
  if (next_threshold >= 650) next_threshold = -1;
  return next_threshold;
}

void loop() {
  bool game_end = false; //This var controls end of game logic

  randomSeed(millis()); //Set seed each time

  int number = random(1, 4); //Pick random int between 1 and 3
  int action = -1;

  if (number == 1){
    action = random(1, 5); //Pick random int between 1 to 4
  }
  if (number == 2){
    action = 5;
  }
  if (number == 3){
    action = 6;
  }

  if (action == 1 || action == 2 || action == 3 || action == 4){
    display.clearDisplay();
    display.display();

    //Play button tones 
    unsigned long last_tone_start = millis();
    int current_tone = 0;
    tone(9, button_tone[current_tone], button_duration[current_tone]);
    while (current_tone != -1){
      if (millis() - last_tone_start > button_duration[current_tone]){
        last_tone_start = millis();
        if (current_tone < 2){
          current_tone++;
          if (button_tone[current_tone] > 0) tone(9, button_tone[current_tone], button_duration[current_tone]);
          else noTone(9);
        }
        else current_tone = -1;
      }
    }

    noTone(9);
    delay(250);
    display.clearDisplay();
    display.setCursor(17, 24);
    display.println(F("Push It!"));
    display.display();
    if (action==1) digitalWrite(1, HIGH);
    if (action==2) digitalWrite(A0, HIGH);
    if (action==3) digitalWrite(A1, HIGH);
    if (action==4) digitalWrite(A2, HIGH);
  }
 
  if (action == 5){
    display.clearDisplay();
    display.display();

    //Play reopen tones 
    unsigned long last_tone_start = millis();
    int current_tone = 0;
    tone(9, reopen_tone[current_tone], reopen_duration[current_tone]);
    while (current_tone != -1){
      if (millis() - last_tone_start > reopen_duration[current_tone]){
        last_tone_start = millis();
        if (current_tone < 2){
          current_tone++;
          if (reopen_tone[current_tone] > 0) tone(9, reopen_tone[current_tone], reopen_duration[current_tone]);
          else noTone(9);
        }
        else current_tone = -1;
      }
    }
    noTone(9);
    delay(250);

    display.clearDisplay();
    display.setCursor(7, 24);
    display.println(F("Reopen It!"));
    display.display();     
  }

  if (action == 6){
    display.clearDisplay();
    display.display();

    //Play slide tones 
    unsigned long last_tone_start = millis();
    int current_tone = 0;
    tone(9, slide_tone[current_tone], slide_duration[current_tone]);
    while (current_tone != -1){
      if (millis() - last_tone_start > slide_duration[current_tone]){
        last_tone_start = millis();
        if (current_tone < 5){
          current_tone++;
          if (slide_tone[current_tone] > 0) tone(9, slide_tone[current_tone], slide_duration[current_tone]);
          else noTone(9);
        }
        else current_tone = -1;
      }
    }
    noTone(9);
    delay(250);

    display.clearDisplay();
    display.setCursor(13, 24);
    display.println(F("Slide It!"));
    display.display();     
  }

  int button = -1;
  int condition = 1;
  unsigned long starttime = millis();
  bool lever = false;
  int leverswitch = 0;
  bool nextlever = false;
  int slider_threshold = 450;

  //Start Background Music
  unsigned long last_tone_start = millis();
  int current_tone = 7;

  while (condition == 1){
    if (millis() - starttime > threshold) condition = -1;

    //Check if we need to advance to the next tone of the background music       
    if (millis() - last_tone_start > background_duration[current_tone]){
      if (current_tone < 6) current_tone++;
      else current_tone = 0;
      last_tone_start = millis();
      if (background_tone[current_tone] > 0) tone(9, background_tone[current_tone], background_duration[current_tone]);
      else noTone(9);
    }    
    

    button = buttonstatus();

    int next_slider_threshold = sliderthreshold(slider_threshold);
    if (next_slider_threshold != slider_threshold) starttime = millis();
    slider_threshold = next_slider_threshold;

    nextlever = (digitalRead(8) == LOW);
    
    if (nextlever != lever) leverswitch++;
    else leverswitch = 0;

    if (button != -1){
      if (button == action) condition = 0;
      else condition = -1;
    }
    else if (lever == true && nextlever == false && leverswitch == 30){
			if (action == 5) condition = 0;
			else condition = -1;
		}
    else if (slider_threshold == -1){
			if (action == 6) condition = 0;
			else condition = -1;
    }

    if (leverswitch == 30){
      lever = nextlever;
      leverswitch = 0;
      starttime = millis();      
    }
  }

  delay(200);
  int response_start = millis();

  if (action > 0 || action < 4){
    digitalWrite(1, LOW);
    digitalWrite(A0, LOW);
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
  }

  if (condition == 0 && score < 99){
    score++;
    display.clearDisplay();
    display.display();

    char c[30];
    sprintf(c, "Success!\n Score: %d", score);

    display.setCursor(17, 24);
    display.println(c);
    display.display();

    success();
  }
  else if (condition == 0){
    display.clearDisplay();
    display.setCursor(5, 24);
    display.println(F("You Win!!!"));
    display.display();

    digitalWrite(1, HIGH);
    digitalWrite(A0, HIGH);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, HIGH);

    //Play victory sound
    last_tone_start = millis();
    current_tone = 0;
    tone(9, victory_tone[current_tone], victory_duration[current_tone]);
    while (current_tone != -1){
      if (millis() - last_tone_start > victory_duration[current_tone]){
        last_tone_start = millis();
        if (current_tone < 7){
          current_tone++;
          if (victory_tone[current_tone] > 0) tone(9, victory_tone[current_tone], victory_duration[current_tone]);
          else noTone(9);
        }
        else current_tone = -1;
      }
    }
    noTone(9);
    game_end = true;
  }
  
  if (condition == -1){
    display.clearDisplay();

    char c[30];
    sprintf(c, "You Lose...\n   Final Score: %d", score);

    display.setTextSize(1);
    display.setCursor(30, 24);
    display.println(c);
    display.display();
    display.setTextSize(2);    
    
    //Play defeat sound
    last_tone_start = millis();
    current_tone = 0;
    tone(9, defeat_tone[current_tone], defeat_duration[current_tone]);
    while (current_tone != -1){
      if (millis() - last_tone_start > defeat_duration[current_tone]){
        last_tone_start = millis();
        if (current_tone < 3){
          current_tone++;
          if (defeat_tone[current_tone] > 0) tone(9, defeat_tone[current_tone], defeat_duration[current_tone]);
          else noTone(9);
        }
        else current_tone = -1;
      }
    }
    noTone(9);
    game_end = true;
  }

  if (game_end == false){
    threshold = threshold * 0.9;
  	if (threshold < 650) threshold = 650;
    while (millis() - response_start < threshold);
  }
  else{
    score = 0;
    threshold = 2000;
    delay(3000);
    display.setTextSize(1);
    display.clearDisplay();
    display.setCursor(4, 15);
    display.println(F("To restart the game,\nplease press and hold\n  all four buttons\n  until a beep is\n       heard."));
    display.display();
    display.setTextSize(2);

    digitalWrite(1, HIGH);
    digitalWrite(A0, HIGH);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, HIGH);
    
    int breakout_streak = 0;
    while (breakout_streak < 33){
      if (digitalRead(6) == LOW && digitalRead(5) == LOW && digitalRead(7) == LOW && digitalRead(0) == LOW) breakout_streak++;
      else breakout_streak = 0;
      delay(10);
    }

    digitalWrite(1, LOW);
    digitalWrite(A0, LOW);
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);

    tone(9, 400, 100);
    delay(100);
    while (digitalRead(6) == LOW || digitalRead(5) == LOW || digitalRead(7) == LOW || digitalRead(0) == LOW);
    delay(1000);
    noTone(9);
  }
}