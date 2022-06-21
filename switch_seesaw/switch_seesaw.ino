#include <Adafruit_NeoPixel.h>
#include<Wire.h>

const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int minVal = 265;
int maxVal = 402;

double x;
double y;
double z;
uint32_t color;

int change = 30;


#define PIN 6

// Parameter 1 = aantal LEDs in de strip
// Parameter 2 = pin nummer
// Parameter 3 = RGB LED vlaggen, combineer indien nodig:
//   NEO_KHZ800  800 KHz bitstream (e meeste NeoPixel producten met WS2812 LEDs)
//   NEO_KHZ400  400 KHz (klassieke 'v1' (niet v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     RGB LED volgens GRB bitstream (de meeste NeoPixel produkten)
//   NEO_RGB     RGB LED volgens RGB bitstream (v1 FLORA pixels, niet v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  strip.begin();
  strip.show(); // initialisatie van alle LEDs (resulteert in UIT zetten hier)
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  color = strip.Color(0, 0, 255);
  strip.fill(color);
  strip.show();
}
void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();

  Serial.println(AcY);

  y = AcY;
  uint32_t other_color = strip.Color(0, 0, 255);
  if (color == strip.Color(0, 0, 255)) // Blauw
  {
    other_color = strip.Color(0, 255, 0);
  }

  if (y > 2000 and change < 60) {
    strip.setPixelColor(change, other_color);
    strip.show();
    change += 1;
  }
  if (y < -2000 and change > 0) {
    strip.setPixelColor(change, other_color);
    strip.show();
    change -= 1;
  }


  strip.setPixelColor(change, strip.Color(0, 0, 0));
  strip.show();
    
  delay(100);

}
