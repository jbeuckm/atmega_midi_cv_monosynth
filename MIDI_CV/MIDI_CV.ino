#include <MIDI.h>
#include "AH_MCP4922.h"

#define LED 13   		    // LED pin on Arduino Uno

#define GATE_PIN 3
#define DELAYED_GATE_PIN 2

#define DELAY_PERIOD_PIN A0

//define AnalogOutput (MOSI_pin, SCK_pin, CS_pin, DAC_x, GAIN) 

AH_MCP4922 AnalogOutput1(11,10,12,LOW,LOW);
AH_MCP4922 AnalogOutput2(11,10,12,HIGH,LOW);

AH_MCP4922 AnalogOutput3(8,7,9,LOW,LOW);
AH_MCP4922 AnalogOutput4(8,7,9,HIGH,LOW);

int liveNoteCount = 0;
int pitchbendOffset = 0;
int baseNoteFrequency;

int delayCounter;
boolean prepareDelayGateOn = false;
boolean prepareDelayGateOff = false;
int delayLength;

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

  digitalWrite(GATE_PIN, HIGH);
  
  delayLength = analogRead(DELAY_PERIOD_PIN);
  delayCounter = 0;
  prepareDelayGateOn = true;  
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  if (channel != selectedChannel) {
    return;
  }
  liveNoteCount--;
  
  if (liveNoteCount == 0) {
    digitalWrite(GATE_PIN, LOW);
  
    delayLength = analogRead(DELAY_PERIOD_PIN);
    delayCounter = 0;
    prepareDelayGateOff = true;
  }
}


ISR(TIMER1_COMPA_vect) {
  if (prepareDelayGateOn) {
    delayCounter++;
    if (delayCounter >= delayLength) {
      digitalWrite(DELAYED_GATE_PIN, HIGH);
      prepareDelayGateOn = false;
    }
  }
  else if (prepareDelayGateOff) {
    delayCounter++;
    if (delayCounter >= delayLength) {
      digitalWrite(DELAYED_GATE_PIN, LOW);
      prepareDelayGateOff = false;
    }
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


    cli();//stop interrupts
    
    //set timer1 interrupt at 1kHz
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0;
    // set timer count for 1khz increments
    OCR1A = 1999;// = (16*10^6) / (1000*8) - 1
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS11 bit for 8 prescaler
    TCCR1B |= (1 << CS11);   
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
    
    sei();//allow interrupts


    MIDI.begin(10);
    
    // C8 at full velocity for calibration on powerup
    handleNoteOn(17, 108, 127);

    liveNoteCount--;
}

void loop()
{
    MIDI.read();
}

