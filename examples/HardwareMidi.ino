#include <MIDI.h>
#include <MIDI-NRPN.h>


/*
 Example for Hardware MIDI on Serial1
*/

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
MIDI_NRPN nrpn; // initialize with default values

void setup() {
  Serial.begin(115200);
  nrpn.setHandleNRPN(onNRPN);
  
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop() {
  MIDI.read();
}

void handleControlChange(byte channel, byte number, byte value) {
  // feed the NRPNHandler with all control changes
  nrpn.handleControlChange(channel, number, value);
}

// the NRPN callback.
void onNRPN(uint16_t number, uint16_t value) {
  Serial.printf("Received NRPN %d: %d\n", number, value);
}