#include <MIDI.h>
#include "AH_MCP4922.h"

#define LED 13   		    // LED pin on Arduino Uno

#define GATE_PIN 3
#define DELAYED_GATE_PIN 2


//define AnalogOutput (MOSI_pin, SCK_pin, CS_pin, DAC_x, GAIN) 

AH_MCP4922 AnalogOutput1(11,10,12,LOW,LOW);
AH_MCP4922 AnalogOutput2(11,10,12,HIGH,LOW);

AH_MCP4922 AnalogOutput3(8,7,9,LOW,LOW);
AH_MCP4922 AnalogOutput4(8,7,9,HIGH,LOW);

int liveNoteCount = 0;
int pitchbendOffset = 0;
int baseNoteFrequency;

MIDI_CREATE_DEFAULT_INSTANCE();

byte selectedChannel = 17;

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  if (selectedChannel == 17) {
    selectedChannel = channel;
  }
  else if (channel != selectedChannel) {
    return;
  }
  
  liveNoteCount++;
  
  baseNoteFrequency = (pitch - 12) * 42;
  AnalogOutput1.setValue(baseNoteFrequency + pitchbendOffset);
  AnalogOutput2.setValue(velocity * 32);

  digitalWrite(LED,HIGH);

  digitalWrite(GATE_PIN, HIGH);
  digitalWrite(DELAYED_GATE_PIN, HIGH);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  if (channel != selectedChannel) {
    return;
  }
  liveNoteCount--;
  
  if (liveNoteCount == 0) {
    digitalWrite(LED,LOW);

    digitalWrite(DELAYED_GATE_PIN, LOW);
    digitalWrite(GATE_PIN, LOW);
  }
}

void handleControlChange(byte channel, byte number, byte value)
{
  if (channel != selectedChannel) {
    return;
  }
  if (number == 1) {
    AnalogOutput4.setValue(value * 32);
  }
}

void handleChannelPressure(byte channel, byte value)
{
  if (channel != selectedChannel) {
    return;
  }

  AnalogOutput3.setValue(value * 32);
}





void handlePitchBend(byte channel, int bend)
{
  pitchbendOffset = bend >> 4;

  AnalogOutput1.setValue(baseNoteFrequency + pitchbendOffset);
}


// -----------------------------------------------------------------------------

void setup()
{
    pinMode(LED, OUTPUT);

    pinMode(GATE_PIN, OUTPUT);
    pinMode(DELAYED_GATE_PIN, OUTPUT);
    
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    
    MIDI.setHandleControlChange(handleControlChange);
    MIDI.setHandleAfterTouchChannel(handleChannelPressure);

    MIDI.begin(10);
    
    // C8 at full velocity for calibration on powerup
    handleNoteOn(17, 108, 127);

    liveNoteCount--;
}

void loop()
{
    MIDI.read();
}

