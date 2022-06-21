#include "nRF24L01.h" // NRF24L01 library created by TMRh20 https://github.com/TMRh20/RF24




int SentMessage[1] = {000};
RF24 radio(9, 10); // NRF24L01 used SPI pins + Pin 9 and 10 on the NANO

const uint64_t pipe = 0xE6E6E6E6E6E6; // Needs to be the same for communicating between 2 NRF24L01


void setup() {
  radio.begin(); // Start the NRF24L01
  radio.openWritingPipe(pipe); // Get NRF24L01 ready to transmit
}

void loop() {
  if (digitalRead(SwitchPin) == LOW)    // If switch is pressed
  {
    SentMessage[0] = 111;
    radio.write(SentMessage, 1);      // Send pressed data to NRF24L01
  }
  else
  {
    SentMessage[0] = 000;
    radio.write(SentMessage, 1);      // Send idle data to NRF24L01
  }

}
