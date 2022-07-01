#include <RF24.h>
#include <RF24Network.h>
#include <nRF24L01.h>
#include <SPI.h>

RF24 radio(9, 10);               // nRF24L01 (CE,CSN)
RF24Network network(radio);      // Include the radio in the network
const uint16_t this_node = 00;   // Address of this node in Octal format ( 04,031, etc)
int amountOfTools = 3;
int toolColors[] = { -1, -1, -1, -1};

bool activeColors[] = {true, true, true, true};

int colorPins[] = {7, -1, -1, -1};
int colorOutput[] = {6, 5, 3, A1};


void setup() {
  Serial.begin(115200);
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);  //(channel, node address)

  for (int i = 0; i < 4; i++) {
    if (colorPins[i] > 0) {
      pinMode(colorPins[i], INPUT);    // sets the digital pin 7 as input
    }
  }

  for (int i = 0; i < 4; i++) {
    if (colorOutput[i] > 0) {
      pinMode(colorOutput[i], OUTPUT);    // sets the digital pin 7 as input
      analogWrite(colorOutput[i], 254);
    }
  }

  pinMode(6, OUTPUT);
  analogWrite(6, 254);
}

unsigned long lastMillis;

void loop() {
  network.update();
  while ( network.available() ) {     // Is there any incoming data?
    RF24NetworkHeader header;
    byte incomingData;
    network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    // To set the radioNumber via the Serial monitor on startup
    handleIncommingData(incomingData, header);
  }

  for (int i = 0; i < 4; i++) {
    if (colorPins[i] > 0) {
      bool active = !digitalRead(colorPins[i]);   // read the input pin
      if ( activeColors[i] != active) {
        activeColors[i] = active;

        analogWrite(colorOutput[i], active * 254);
        Serial.print("Color ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(active);
        sendColors();
      }
    }
  }

  //is the time up for this task?
  if (millis() - lastMillis >= 10 * 1000UL)
  {
    lastMillis = millis();  //get ready for the next iteration
    sendColors();

  }

}

void handleIncommingData(byte data, RF24NetworkHeader header) {
  // To set the radioNumber via the Serial monitor on startup
  byte eventMask = 0b00000011;
  if ((data & eventMask) == 0b00000000) { // Color changed
    handleColorChanged(data, header);
  }
}

void handleColorChanged(byte data,  RF24NetworkHeader header) {
  int toolIndex = header.from_node - 1;
  int colorIndex = data >> 2;
  toolColors[toolIndex] = colorIndex;
  Serial.print("Tool ");
  Serial.print(toolIndex);
  Serial.print(" changed to color ");
  Serial.println(colorIndex);
}

void sendColors() {
  byte colorData = 0b00000001;

  for (int i = 0; i < 4; i++) {
    if (activeColors[i]) {
      colorData = colorData | (0b10000000 >> i);
    }
  }
  Serial.println(colorData);


  for (int i = 1; i < amountOfTools; i++) {
    RF24NetworkHeader header2(i);     // (Address where the data is going)
    bool ok = network.write(header2, &colorData, sizeof(colorData)); // Send the data
    if (!ok) {
      Serial.println("Fail");
    }
  }
}
