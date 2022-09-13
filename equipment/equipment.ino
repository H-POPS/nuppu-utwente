//#define SLIDE
//#define SEESAW
//#define SWING
#define CAROUSEL

#include <RF24.h>
#include <RF24Network.h>
#include <nRF24L01.h>
#include <SPI.h>

#include <Adafruit_NeoPixel.h>

unsigned long stoppedMillis;
int freezeTime = 5000;
#define PIN 5
Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, PIN, NEO_GRB + NEO_KHZ800);


#ifdef SLIDE

#define echoPin 2 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 3 // attach pin D3 Arduino to pin Trig of HC-SR04


// defines variables
long duration; // variable for the duration of sound wave travel
int distance;  // variable for the distance measurement
unsigned long lastChangeMilis;
int charge = 0;

#endif
#if defined SEESAW or defined SWING or defined CAROUSEL
#include<Wire.h>

const int MPU_addr = 0x68;
float xa, ya, za, roll, pitch;
bool changed[30];
int change = 15;
int charge = 0;
int side = 0;
#endif


#ifdef CAROUSEL
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
#endif

RF24 radio(9, 10);          // nRF24L01 (CE,CSN)
RF24Network network(radio); // Include the radio in the network

// Address of this node in Octal format ( 04,031, etc)
#if defined SLIDE
const uint16_t this_node = 01;
#elif defined SEESAW
const uint16_t this_node = 02;
#elif defined SWING
const uint16_t this_node = 03;
#elif defined CAROUSEL
const uint16_t this_node = 04;
#endif

int activeColor = 0;
int nextColor = 0;
bool activeColors[] = {true, true, true, true};
int colors[][3] = {
  {150, 0, 0},  // Rood
  {0, 150, 0},  // Groen
  {0, 0, 150},  // Blauw
  {150, 150, 0}, // Geel
  {150, 150, 150} // Wit
};


unsigned long lastUpdateMillis;

void setup()
{

#ifdef SLIDE
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
#endif

#if defined SEESAW or defined SWING
  Wire.begin();                                      //begin the wire communication
  Wire.beginTransmission(MPU_addr);                 //begin, send the slave adress (in this case 68)
  Wire.write(0x6B);                                  //make the reset (place a 0 into the 6B register)
  Wire.write(0);
  Wire.endTransmission(true);                        //end the transmission
#endif

#ifdef CAROUSEL
  /* Initialise the sensor */
  if (!accel.begin())
  {
  Serial.begin(115200);
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1);
  }

  accel.setRange(ADXL345_RANGE_16_G);
#endif

  Serial.begin(115200);
  SPI.begin();
  radio.begin();
  network.begin(95, this_node); //(channel, node address)

  strip.begin();
  strip.show();

  uint32_t stripColor = strip.Color(colors[activeColor][0], colors[activeColor][1], colors[activeColor][2]);

  strip.fill(stripColor);
  strip.show();
  
  Serial.println("Setup complete");


}

unsigned long lastMillis;
unsigned long lastDirChangeMillis;
unsigned long lastChargeMillis;

void loop()
{
  network.update();
  while (network.available())
  { // Is there any incoming data?
    RF24NetworkHeader header;
    byte incomingData;
    Serial.print("Data recieved: ");
    Serial.println(incomingData);
    network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    handleIncommingData(incomingData);

  }

  int next = getNextColor();
  uint32_t other_color  = strip.Color(colors[next][0], colors[next][1], colors[next][2]);
#ifdef SLIDE
  if (millis() - lastUpdateMillis > 100) {
    lastUpdateMillis = millis();
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)

    // Displays the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance > 0 and distance < 30 and (millis() - lastChangeMilis) > 1500)
    {
      strip.setPixelColor(charge, 0-);
      charge = 1;
      lastChangeMilis = millis();
    }

  }

    if (charge > 0 and millis() - lastChargeMillis > 15) {
      lastChargeMillis = millis();
      strip.setPixelColor(charge, other_color);
      strip.show();
      charge += 1;
      if (charge == 30) {
        switchColor();
        charge = 0;
      }
    }
  //delay(200);
#endif

#if defined SEESAW or defined SWING
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true); //get six bytes accelerometer data
  xa = Wire.read() << 8 | Wire.read();
  ya = Wire.read() << 8 | Wire.read();
  za = Wire.read() << 8 | Wire.read();

  roll = atan2(ya , za) * 180.0 / PI;
  pitch = atan2(-xa , sqrt(ya * ya + za * za)) * 180.0 / PI;

  Serial.print("roll = ");
  Serial.print(roll);
  Serial.print(", pitch = ");
  Serial.println(pitch);

#endif
#if defined SEESAW
  if (roll > 10 and change < 29) {
    changed[change] = true;
    strip.setPixelColor(change, other_color);
    strip.show();
    change += 1;
    delay(30);
  }
  if (roll < -10 and change > 0) {
    strip.setPixelColor(change, other_color);
    strip.show();
    change -= 1;
    delay(30);
  }
  changed[change] = true;
  bool full = true;
  for (int i = 0; i < 30; i++)
  {
    if (!changed[i]) {
      full = false;
    }
  }
  if (full) {
    switchColor();
    for (int i = 0; i < 30; i++)
    {
      changed[i] = false;
    }
  }
  strip.setPixelColor(change, strip.Color(255, 255, 255));
  strip.show();
#endif



#if defined SWING
  if (roll < -20 and side != -1) {
    side = -1;
    lastDirChangeMillis = millis();
  }
  if (roll > 20 and side != 1) {
    side = 1;
    lastDirChangeMillis = millis();
  }
  bool shouldCharge = millis() - lastDirChangeMillis < 3000;
#endif

#if defined CAROUSEL

  if (millis() - lastUpdateMillis > 100) {
    lastUpdateMillis = millis();
    /* Get a new sensor event */
    sensors_event_t event;
    accel.getEvent(&event);
    Serial.println(event.acceleration.x);
    if (event.acceleration.x > 3 or event.acceleration.x < -3) {

      lastDirChangeMillis = millis();
    }
  }

#endif



#if defined SWING or defined CAROUSEL

  if (millis() - stoppedMillis < freezeTime) {
    return;
  }
  if (millis() - lastDirChangeMillis < 1500) {

    if (millis() - lastChargeMillis > 100) {

      lastChargeMillis = millis();
      strip.setPixelColor(charge, other_color);
      strip.show();
      charge += 1;
      if (charge == 30) {
        switchColor();
        charge = 0;
      }
    }
  } else {

    if (charge > 0 and millis() - lastChargeMillis > 50) {
      charge -= 1;
      lastChargeMillis = millis();
      uint32_t stripColor = strip.Color(colors[activeColor][0], colors[activeColor][1], colors[activeColor][2]);
      strip.setPixelColor(charge, stripColor);
      strip.show();
    }
  }

#endif
}

int getNextColor()
{
  if (nextColor != activeColor) {
    return nextColor;
  }
  int count = 0;
  int origin[4] = {};
  for (int i = 0; i <= 4; i++)
  {
    if (activeColors[i] and i != activeColor)
    {
      origin[count] = i;
      count += 1;
    }
  }
  int randomIndex = random(count - 1);
  nextColor = origin[randomIndex];
  return nextColor;
}

void switchColor()
{
  if (millis() - stoppedMillis < freezeTime) {
    return;
  }

  activeColor = getNextColor();

  byte data = activeColor << 2;
  Serial.println(activeColor);
  Serial.println(data);

  RF24NetworkHeader header(00);                         // (Address where the data is going)
  bool ok = network.write(header, &data, sizeof(data)); // Send the data

  if (!ok) {
    Serial.print("Failed sending color switch to: ");
    Serial.println(00);
  }

  uint32_t stripColor = strip.Color(colors[activeColor][0], colors[activeColor][1], colors[activeColor][2]);

  strip.fill(stripColor);
  strip.show();
}

void handleIncommingData(byte data)
{
  // To set the radioNumber via the Serial monitor on startup
  Serial.println((int)data);
  byte eventMask = 0b00000011;
  if ((data & eventMask) == 0b00000001)
  { // Change color
    handleColorChange(data);
  }
  if ((data & eventMask) == 0b00000010)
  { // Start of game
    Serial.println("Game started");
    nextColor = 4;
    lastUpdateMillis = 0;
    switchColor();
  }
  if ((data & eventMask) == 0b00000011)
  { // Start of game
    Serial.println("Game stopped");
    stoppedMillis = millis();
    uint32_t stripColor = strip.Color(colors[activeColor][0], colors[activeColor][1], colors[activeColor][2]);
  }
}

void handleColorChange(byte data)
{
  int count = 0;
  for (int i = 0; i < 4; i++)
  {
    bool isActive = data & (0b10000000 >> i);
    if (isActive)
    {
      count += 1;
    }
    activeColors[i] = isActive;
  }

  Serial.print("Change active colors: ");
  Serial.println(count);
}
