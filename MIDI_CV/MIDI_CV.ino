#include <MIDI.h>
#include "AH_MCP4922.h"

#define LED 13   		    // LED pin on Arduino Uno

//define AnalogOutput (MOSI_pin, SCK_pin, CS_pin, DAC_x, GAIN) 

AH_MCP4922 AnalogOutput1(11,10,12,LOW,LOW);
AH_MCP4922 AnalogOutput2(11,10,12,HIGH,LOW);

AH_MCP4922 AnalogOutput3(8,7,9,LOW,LOW);
AH_MCP4922 AnalogOutput4(8,7,9,HIGH,LOW);

int liveNoteCount = 0;

MIDI_CREATE_DEFAULT_INSTANCE();


void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  liveNoteCount++;
  
  AnalogOutput1.setValue((pitch - 12) * 42);
  AnalogOutput2.setValue(velocity * 32);

  digitalWrite(LED,HIGH);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  liveNoteCount--;
  
  if (liveNoteCount == 0) {
    digitalWrite(LED,LOW);
  }
}

void handleControlChange(byte channel, byte number, byte value)
{
  if (number == 1) {
    AnalogOutput3.setValue(value * 32);
  }
}

void handlePitchBend(byte channel, int bend)
{
    AnalogOutput4.setValue(bend/4.0 + 2048.0);
}


// -----------------------------------------------------------------------------

void setup()
{
    pinMode(LED, OUTPUT);
    
    // Connect the handleNoteOn function to the library,
    // so it is called upon reception of a NoteOn.
    MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

    // Do the same for NoteOffs
    MIDI.setHandleNoteOff(handleNoteOff);
    
    MIDI.setHandleControlChange(handleControlChange);
    MIDI.setHandlePitchBend(handlePitchBend);

    // Initiate MIDI communications, listen to all channels
    MIDI.begin(MIDI_CHANNEL_OMNI);
    
    // C8 at full velocity for calibration on powerup
    handleNoteOn(1, 108, 127);
    liveNoteCount--;
}

void loop()
{
    MIDI.read();
}

