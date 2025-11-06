/* NRPNHandler.h
 *
 * Copyright (c) 2025 by Uli Schmidt steinundfloete@online.de
 * V1.0 11/06/2025 
 *
 * Translates MIDI NRPN from control change (CC) messages and reports NRPN number and value (both 14 bit) to the given callback. 
 * It can operate in 7 or 14 bit mode where 14 bit mode is default. 
 *
 * 7 bit mode: CC DATA_ENTRY_LSB(38) will be skipped. 
 * - does expect CC NRPN_MSB(99), NRPN_LSB(98), DATA_ENTRY(6). 
 * - after receiving DATA_ENTRY(6) the callback will be raised with the received 7 bit value << 7.
 * Please note: 
 * NRPN values on the callback are 14 bit always!
 * 
 * 14 bit mode (default)
 * - does expect CC NRPN_MSB(99), NRPN_LSB(98), DATA_ENTRY(6), DATA_ENTRY_LSB(38). 
 * - after receiving DATA_ENTRY(6) the callback will be raised with the received 14 bit value.
 * - if you really don't want to send DATA_ENTRY_LSB(38), you can send CC RPN_MSB(101) and RPN_LSB(100) with a value of 127 each instead as a terminator.
 * 
 * There's a default timeout of 500 milliseconds on incomplete NRPN CC chain
 * If you define NRPN_VERBOSE before adding this library, an aborted state will be printed out for your debugging
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _NRPNHANDLER_INCLUDED_H_
#define _NRPNHANDLER_INCLUDED_H_

class NRPNHandler {
public:
  // callback prototype
  typedef void (*HANDLE_NRPN)(uint16_t, uint16_t);

  // instanciate a NRPNHandler.
  // mode7Bit: set to true if you want to skip DATA_ENTRY_LSB(38).
  // callback: sets the callback or NULL.
  // timeout: timeout for incomplete NRPN control change chain in milliseconds.
  NRPNHandler(bool mode7Bit = false, HANDLE_NRPN callback = NULL, uint32_t timeout = 500) {
    this->timeout = timeout;
    this->mode7Bit = mode7Bit;
    this->callback = callback;
    this->state = NRPNState::NRPN_OFF;
  }

  // set the callback or NULL
  void setHandleNRPN(HANDLE_NRPN callback) {
    this->callback = callback;
  }

  // must be called from your MIDI callback
  void handleControlChange(byte channel, byte number, byte value) {
    switch (number) {
      case 99:  //NRPN_MSB
        setState(NRPN_NUMBER_MSB, value);
        break;
      case 98:  //NRPN_LSB
        setState(NRPN_NUMBER_LSB, value);
        break;
      case 6:  //DataEntryMSB
        setState(NRPN_VALUE_MSB, value);
        break;
      case 38:  //DataEntryLSB
        setState(NRPN_VALUE_LSB, value);
        break;
      case 101:  //RPN_MSB
        setState(RPN_NUMBER_MSB, value);
        break;
      case 100:  //RPN_LSB
        setState(RPN_NUMBER_LSB, value);
        break;
      default:
        break;
    }
  }

  // for use without callback.
  // returns true if a complete NRPN message was received
  bool isValid() {
    return state == NRPN_READY;
  }

  // return the last received NRPN number or 0 if !isValid()
  uint16_t getNumber() {
    return number;
  }

  // return the last received NRPN value or 0 if !isValid()
  uint16_t getValue() {
    return value;
  }

private:
  HANDLE_NRPN callback;
  uint32_t timeout;
  uint32_t startTime;
  bool mode7Bit;
  enum NRPNState : uint8_t {
    NRPN_OFF = 0,
    NRPN_NUMBER_MSB,
    NRPN_NUMBER_LSB,
    NRPN_VALUE_MSB,
    NRPN_VALUE_LSB,
    RPN_NUMBER_MSB,
    RPN_NUMBER_LSB,
    NRPN_READY
  };

  NRPNState state;
  uint16_t number;
  uint16_t value;

  void init() {
    number = 0;
    value = 0;
    state = NRPN_OFF;
    startTime = 0;
  }

  bool isTimeout() {
    return millis() - startTime >= timeout;
  }

  void print() {
    Serial.printf("State %d number %d value %d\n", state, number, value);
  }

  void setState(NRPNState s, uint8_t value = 0) {
    switch (s) {
      case NRPN_OFF:
#ifdef NRPN_VERBOSE
        Serial.print("cancelled from state ");
        Serial.println(this->state);
#endif
        init();
        break;
      case NRPN_NUMBER_MSB:
        init();
        startTime = millis();
        this->number = value << 7;
        break;
      case NRPN_NUMBER_LSB:
        if (this->state != NRPN_NUMBER_MSB || isTimeout()) {
          setState(NRPN_OFF);
          return;
        }
        this->number |= value;
        break;
      case NRPN_VALUE_MSB:
        if (this->state != NRPN_NUMBER_LSB || isTimeout()) {
          setState(NRPN_OFF);
          return;
        }
        this->value = value;
        this->value <<= 7;
        if (mode7Bit) {
          setState(NRPN_READY);
        }
        break;
      case NRPN_VALUE_LSB:
        if (this->state != NRPN_VALUE_MSB || isTimeout()) {
          setState(NRPN_OFF);
          return;
        }
        this->value |= value;
        setState(NRPN_READY);
        break;
      case RPN_NUMBER_MSB:
        // terminator 1 for value MSB or LSB
        if (this->state == NRPN_VALUE_MSB && value == 0x7f && !isTimeout()) {
          break;
        }
        if (this->state == NRPN_VALUE_LSB && value == 0x7f && !isTimeout()) {
          break;
        }
        if (this->state != NRPN_READY) {
          setState(NRPN_OFF);
        }
        return;
      case RPN_NUMBER_LSB:
        // terminator 2 for value MSB or LSB
        if (this->state == RPN_NUMBER_MSB && value == 0x7f && !isTimeout()) {
          setState(NRPN_READY);
          break;
        } else if (this->state != NRPN_READY) {
          setState(NRPN_OFF);
        }
        return;
      case NRPN_READY:
        if (callback != NULL) {
          callback(this->number, this->value);
        }
        break;
      default:
        return;
    }
    this->state = s;
    print();
  }
};

#endif  //#ifndef _NRPNHANDLER_INCLUDED_H_