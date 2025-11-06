# Arduino MIDI NRPN Library

Translates MIDI NRPN from control change (CC) messages and reports NRPN number and value (both 14 bit) to the given callback.  
It can operate in 7 or 14 bit mode where 14 bit mode is default. 

**7 bit mode:** CC DATA_ENTRY_LSB(38) will be skipped. 
- does expect CC NRPN_MSB(99), NRPN_LSB(98), DATA_ENTRY(6). 
- after receiving DATA_ENTRY(6) the callback will be raised with the received 7 bit value << 7.  
**Please note:**  
NRPN values on the callback are 14 bit always!

**14 bit mode** (default)
- does expect CC NRPN_MSB(99), NRPN_LSB(98), DATA_ENTRY(6), DATA_ENTRY_LSB(38). 
- after receiving DATA_ENTRY(6) the callback will be raised with the received 14 bit value.
- if you really don't want to send DATA_ENTRY_LSB(38), you can send CC RPN_MSB(101) and RPN_LSB(100) with a value of 127 each instead as a terminator.

There's a default timeout of 500 milliseconds on incomplete NRPN CC chain.  
If you define NRPN_VERBOSE before adding this library, an aborted state will be printed out for your debugging


### Installation

- Use **Arduino Library Manager** or 
- Or download / use **git** to have latest repository of **MIDI-NRPN** added to Arduino IDE **/libraries** folder  
(File > Preferences > Sketchbook location).


### Usage

- open one of the examples according to your usecase.

## Credits and license

- Licensed MIT License ([text](LICENSE))
