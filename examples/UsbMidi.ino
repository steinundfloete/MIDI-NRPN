#include <NRPNHandler.h>


/*
 Example for e.g. Teensy with USB Midi enabled
*/

NRPNHandler nrpn;

void setup() {
  Serial.begin(115200);
  usbMIDI.setHandleControlChange(handleControlChange);
  nrpn.setHandleNRPN(onNRPN);
}

void loop() {
  usbMIDI.read();
}

void handleControlChange(byte channel, byte number, byte value) {
  // feed the NRPNHandler with all control changes
  nrpn.handleControlChange(channel, number, value);
}

// the NRPN callback.
void onNRPN(uint16_t number, uint16_t value) {
  Serial.printf("Received NRPN %d: %d\n", number, value);
}