#include <RF24.h>
#include <RF24Network.h>
#include <nRF24L01.h>
#include <SPI.h>

RF24 radio(9, 10);               // nRF24L01 (CE,CSN)
RF24Network network(radio);      // Include the radio in the network
const uint16_t this_node = 00;   // Address of this node in Octal format ( 04,031, etc)
int amountOfTools = 3;
int toolColors[] = { -1, -1, -1, -1};
int winning;
int startingMillis;

bool magnets[] = {true, true, true, true};
bool activeColors[] = {true, true, true, true};


// Rood, Groed, Blauw, Geel
int colorPins[] = {6, 5, 4, 3};
int colorOutput[] = {A0, A2, A3, A1};

bool gameActive = false;
int startedMillis;
int lastUpdateSend;
int gameDuration = 1000 * 10;


void setup() {
  Serial.begin(115200);
  SPI.begin();
  radio.begin();
  network.begin(95, this_node);  //(channel, node address)

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

  // send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:
    int i = Serial.read() - '0';
    if (i == 5) {
      startGame();
    }
  }

  // Update colors when game is not active
  if (!gameActive) {

    // send data only when you receive data:
    if (Serial.available() > 0) {
      // read the incoming byte:
      int i = Serial.read() - '0';
      if (i > 1 and i <= 4) {
        setColor(i - 1, !activeColors[i - 1]);
      }
    }

    for (int i = 0; i < 4; i++) {
      if (colorPins[i] > 0) {
        bool active = !digitalRead(colorPins[i]);   // read the input pin
        if ( magnets[i] != active) {
          setColor(i, active);
          magnets[i] = active;
        }
      }
    }
  }

  if (gameActive) {
    if (millis() - startedMillis > gameDuration) {
      endGame();
    }
  }
}

void setColor(int i, bool active) {
  activeColors[i] = active;

  analogWrite(colorOutput[i], active * 254);
  Serial.print("Color ");
  Serial.print(i);
  Serial.print(": ");
  Serial.println(active);
  sendColors();
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
  Serial.print("Tool  ");
  Serial.print(toolIndex);
  Serial.print(": ");
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



void sendStartOfGame() {
  byte data = 0b00000010;
  for (int i = 1; i < amountOfTools; i++) {
    RF24NetworkHeader header2(i);     // (Address where the data is going)
    bool ok = network.write(header2, &data, sizeof(data)); // Send the data
    if (!ok) {
      Serial.println("Fail");
    }
  }
}

void sendEndOfGame() {
  byte data = 0b00000011;
  for (int i = 1; i < amountOfTools; i++) {
    RF24NetworkHeader header2(i);     // (Address where the data is going)
    bool ok = network.write(header2, &data, sizeof(data)); // Send the data
    if (!ok) {
      Serial.println("Fail");
    }
  }
}

void startGame() {
  Serial.print("Count down" );
   for (int i = 0; i < 4; i++) {
    if (colorOutput[i] > 0) {
      analogWrite(colorOutput[i], 0);
    }
  }
  delay(900)
      analogWrite(colorOutput[], 0);
  
  startedMillis = millis();
  gameActive = true;
  sendStartOfGame();
  Serial.print("Game started:" );
  Serial.println(gameDuration / 1000);
}

void endGame() {
  gameActive = false;
  sendEndOfGame();
  Serial.println("Game ended");
}
