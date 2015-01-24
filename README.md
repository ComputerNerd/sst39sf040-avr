# SST39SF040 Flash Programmer

## Parts required to build this:
* One arduino uno
* Three 74hc595.
* One SST39SF040 or lower capacity versions.

## Wiring
Pin refers to the numbering on the arduino uno.

I will warn you that this can get messy as there are lots of wires be careful
and check your work. My program verifies every byte if you see any errors check
your wiring.

* The D0:D4 to go A0 to A4
* D5:D7 go to Pins digital pins 5 to 7
* Pin 11 CE#
* Pin 12 OE#
* Pin 13 WE#
* Pin 10 is the serial strobe for the shift register this must be connected to
  all 3 shift registers
* Pin 9 is the storage register this also must be connected to all 3 shift
  registers.
* Pins 2,3 and 4 is the serial output pins 2 is the LSB. Unlike pin 9 and 10
  those are different per each shift register.
* A0:A18 goto the output from the 3 shift registers. Make sure you have which
  one is LSB and MSB right. 

## Compiling

Note that I used avr-gcc instead of the arduino IDE to compile this if you do
not have avr-gcc it can be modified with ease just change int main() to void
setup() and add void loop() {} at the bottom of the program and I think it will
compile just fine in the arduino IDE.

## How to use this
Here is the part that runs on your computer I have only tested it on linux but I
used a cross platform library so it may work on windows
https://github.com/ComputerNerd/sst39sf040-computerside

Also my code can be easily adapted to run on the very similar lower capacity
chips. The sst39sf020a and the sst39sf010a. Just change the main for loop to run
for less bytes.
