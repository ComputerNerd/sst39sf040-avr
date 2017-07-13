# SST39SF040 Flash Programmer

## Parts required to build this:
* One Arduino Uno
* Three 74HC595.
* One SST39SF040 or lower capacity versions.

## Wiring

### Connections

| Arduino Uno | SST39SF040 |
| ----------- | ---------- |
| A0          | D0         |
| A1          | D1         |
| A2          | D2         |
| A3          | D3         |
| A4          | D4         |
| 5           | D5         |
| 6           | D6         |
| 7           | D7         |
| 11          | CE#        |
| 12          | OE#        |
| 13          | WE#        |


3x 74HC595 means that the pin from the Arudino Uno should be connected to all
three shift registers.

| Arduino Uno | 3x 74HC595              |
| ----------- | ----------------------- |
| 9           | Storage Register (STCP) |
| 10          | Serial Clock     (SHCP) |
| 5V          | Master Reset (MR)       |
| GND         | Output Enable (OE)      |

| Arduino Uno | 74HC595 #1        |
| ----------- | ----------------- |
| 2           | Serial Input (DS) |

| Arduino Uno | 74HC595 #2        |
| ----------- | ----------------- |
| 3           | Serial Input (DS) |

| Arduino Uno | 74HC595 #3        |
| ----------- | ----------------- |
| 4           | Serial Input (DS) |

| 74HC595 #1 | SST39SF040 |
| ---------- | ---------- |
| Q0         | A0         |
| Q1         | A1         |
| Q2         | A2         |
| Q3         | A3         |
| Q4         | A4         |
| Q5         | A5         |
| Q6         | A6         |
| Q7         | A7         |

| 74HC595 #2 | SST39SF040 |
| ---------- | ---------- |
| Q0         | A8         |
| Q1         | A9         |
| Q2         | A10        |
| Q3         | A11        |
| Q4         | A12        |
| Q5         | A13        |
| Q6         | A14        |
| Q7         | A15        |

| 74HC595 #3 | SST39SF040 |
| ---------- | ---------- |
| Q0         | A16        |
| Q1         | A17        |
| Q2         | A18        |


## Compiling

Note that I used avr-gcc instead of the Arduino IDE to compile this if you do
not have avr-gcc it can be modified with ease just change int main() to void
setup() and add void loop() {} at the bottom of the program and it will compile
just fine in the Arduino IDE.

## How to use this
Here is the part that runs on your computer I have only tested it on linux but
I used a cross platform library so it may work on windows
https://github.com/ComputerNerd/sst39sf040-computerside

Also my code can be easily adapted to run on the very similar lower capacity
chips. The sst39sf020a and the sst39sf010a. Just change the main for loop to
run for less bytes.
