
/*

This is just an example of how you can emulate a MIDI interface and use the pedal to send MIDI signals, please customize it to fit your own usecase since I'm in no way a developer and the code might need some fixing up.
You can check and customize pin assignments right at the start of the setup section

As you can see, the sketch features the use of an LCD I2C screen and uses 3 libraries, you will NEED them to compile the code

https://github.com/arduino-libraries/LiquidCrystal
https://github.com/dxinteractive/ResponsiveAnalogRead
https://github.com/tttapa/Control-Surface

*/


#include <LiquidCrystal_I2C.h>
#include <Control_Surface.h>
#include <ResponsiveAnalogRead.h>


const int DEBOUNCE = 250; // EDIT TO YOUR LIKING, BUTTON DEBOUNCE TIME


// == LCD
LiquidCrystal_I2C LCD (0x27, 16, 2);

// == MIDI INTERFACE
USBMIDI_Interface midi;

byte bts1[10];
byte pvbts1[10];
byte encz[3];
byte pvencz[3];
bool hasChanged; 
byte pvpot1, pvpot2; 
byte mappot1, mappot2;
int spot1, spot2;
bool isPot1On;

char *preset[] = {"   [ CLEAN ]", "   [ CRUNCH ]", "   [ HEAVY ]", "   [ SOLO ]"};
int leds[] = {22, 23, 24, 25};

// == RESPONSIVE ANALOG POTS
ResponsiveAnalogRead analogOne(A0, true);
ResponsiveAnalogRead analogTwo(A2, true);
float setSnapMultiplier = 0.3;

// == MIDI ADDRESSES

const MIDIAddress notez[] = {
  {MIDI_Notes::C(4), Channel_1},
  {MIDI_Notes::A(4), Channel_1},
  {MIDI_Notes::B(4), Channel_1},
  {MIDI_Notes::D(4), Channel_1}
};
const MIDIAddress encx[] = {
  {MIDI_Notes::C(4), Channel_2},
  {MIDI_Notes::A(4), Channel_2},
  {MIDI_Notes::B(4), Channel_2},
  {MIDI_Notes::D(4), Channel_2}
};

const MIDIAddress pd1swtON = {MIDI_CC::Sound_Controller_1, Channel_1};
const MIDIAddress faceBt1 = {MIDI_CC::Sound_Controller_2, Channel_1};
const MIDIAddress faceBt2 = {MIDI_CC::Sound_Controller_3, Channel_1};
const MIDIAddress faceBt3 = {MIDI_CC::Sound_Controller_4, Channel_1};
const MIDIAddress faceBt4 = {MIDI_CC::Sound_Controller_5, Channel_1};
const MIDIAddress pd1pot1 = {MIDI_CC::Foot_Controller, Channel_1};
const MIDIAddress pd1pot2 = {MIDI_CC::Effect_Control_1, Channel_1};



void setup() {  // ___________________________________________SETUP____________________________________________________

  pinMode(0, INPUT_PULLUP); //BT1 FOOT 1
  pinMode(1, INPUT_PULLUP); //BT2 FOOT 2
  pinMode(2, INPUT_PULLUP); //BT3 FOOT 3
  pinMode(3, INPUT_PULLUP); //BT4 FOOT 4
  pinMode(6, INPUT_PULLUP); //BT5 FACE 1
  pinMode(7, INPUT_PULLUP); //BT6 FACE 2
  pinMode(8, INPUT_PULLUP); //BT7 FACE 3
  pinMode(9, INPUT_PULLUP); //BT8 FACE 4
  pinMode(10, INPUT_PULLUP); //BT9 ENCODER PRESS
  pinMode(11, INPUT_PULLUP); //BT10 ENCODER A 
  pinMode(12, INPUT_PULLUP); //BT11 ENCODER B
  pinMode(20, INPUT_PULLUP); //BT12 WAH/WHAMMY
  pinMode(21, INPUT_PULLUP); //BT13 POT 1 ON/OFF
  pinMode(A0, INPUT); //PEDALE EXP
  pinMode(A2, INPUT); //BUILTIN POT

  LCD.begin();
  LCD.clear();
  LCD.setCursor(3, 0);
  LCD.print("AveragePedal");
  
  delay(1000);

  midi.begin();
  delay(150);
  LCD.clear();
  LCD.print("USB MODE"); 
  delay(1000);
  LCD.clear();  

}

void loop() {


    midi.update();
    // READ Btns
    bts1[0] = digitalRead(0); 
    bts1[1] = digitalRead(2); 
    bts1[2] = digitalRead(3);
    bts1[3] = digitalRead(1);
    bts1[4] = digitalRead(20);
    bts1[5] = digitalRead(21);
    bts1[6] = digitalRead(6); 
    bts1[7] = digitalRead(7); 
    bts1[8] = digitalRead(8);
    bts1[9] = digitalRead(9);
    //READ encoder
    encz[0] = digitalRead(10);
    encz[1] = digitalRead(11);
    encz[2] = digitalRead(12);

    //UPDATE AND MAP SMOOTHED POTS 
    analogOne.update();
    analogTwo.update();

    spot1 = analogOne.getValue();
    spot1 = constrain(spot1, 1, 835);
    mappot1 = map(spot1, 1, 835, 0, 127);
    
    spot2 = analogTwo.getValue();
    spot2 = constrain(spot2, 2, 1010);
    mappot2 = map(spot2, 2, 1010, 127, 0);
    
    isPot1On = digitalRead(21);

    // == SEND BTNS
    for (byte i = 0; i < 10; i++) {
      if (bts1[i] != pvbts1[i] && bts1[i] == LOW && i != 4 && i <= 3) {
        
        sendNote(notez[i]);
        LCD.clear();
        LCD.print(preset[i]);
        delay(DEBOUNCE);   
      }
      
      else if (bts1[i] != pvbts1[i] && bts1[i] == LOW && i == 4){             // IF SWITCH
        
        midi.sendControlChange(pd1swtON, 127);
        LCD.clear();
        LCD.print("WAH");
        delay(600);
        LCD.clear();

      }
      else if (bts1[i] != pvbts1[i] && bts1[i] == HIGH && i == 4){             // IF SWITCH SWITCH
        
        midi.sendControlChange(pd1swtON, 0);
        LCD.clear();
        LCD.print("WHAMMY");
        delay(600);
        LCD.clear();

      }
      else if (bts1[i] != pvbts1[i] && bts1[i] == LOW && i == 6) {  //
        midi.sendControlChange(faceBt1, 127); 
        delay(DEBOUNCE); 
      }
      else if (bts1[i] != pvbts1[i] && bts1[i] == LOW && i == 7) {
        midi.sendControlChange(faceBt2, 127);
        delay(DEBOUNCE);  
      }
      else if (bts1[i] != pvbts1[i] && bts1[i] == LOW && i == 8) {
        midi.sendControlChange(faceBt3, 127);
        delay(DEBOUNCE);  
      }
      else if (bts1[i] != pvbts1[i] && bts1[i] == LOW && i == 9) {
        midi.sendControlChange(faceBt4, 127);
        delay(DEBOUNCE);   
      }
      
      pvbts1[i] = bts1[i];
      
      
    
    }

    for (byte i = 0; i < 3; i++) {
      if (encz[i] != pvencz[i] && encz[i] == LOW ) {
        
        sendNote(encx[i]); 
        delay(DEBOUNCE);     
      }
    
      pvencz[i] = encz[i];
      
    
    }


    // == POTS
    if (mappot1 != pvpot1 && isPot1On == LOW) { // SEND PEDAL IF SWITCH ON
      midi.sendControlChange(pd1pot1, mappot1);  
    
    }

    if (mappot2 != pvpot2) { // MASTER KNOB
    midi.sendControlChange(pd1pot2, mappot2);  
    
    }
      
    pvpot1 = mappot1;
    pvpot2 = mappot2;
  
}


void sendNote(MIDIAddress addr) {
  midi.sendNoteOn(addr, 127);
  delay(3);
  midi.sendNoteOff(addr, 127);
}
