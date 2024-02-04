// Include the libraries
#include <Arduino.h>
#include <TM1637Display.h>
#include "MS5611.h"

// Define the connections pins
#define CLK 3
#define DIO 4

// Create a display object of type TM1637Display
TM1637Display display = TM1637Display(CLK, DIO);

// Create an array that turns all segments ON
const uint8_t allON[] = {0xff, 0xff, 0xff, 0xff};

// Create an array that turns all segments OFF
const uint8_t allOFF[] = {0x00, 0x00, 0x00, 0x00};

// Create an array that sets individual segments per digit to display the word "dOnE"
const uint8_t done[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,         // d
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G          // E
};


MS5611 MS5611(0x77);
const float SEA_PRESSURE = 1013.25;
const float HEIGHT_REFERENCE = 419.3; //  measured outside at ground level=> 421.3
const int DELAY = 100;
const int AVG_DELAY=40;
const int AVG_SIZE=30;
bool FOUND = false;
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  if (MS5611.begin() == true)
  {
    FOUND=true;
    Serial.println("MS5611 found.");

  }
  else
  {
    FOUND=false;
    Serial.println("MS5611 not found. halt.");
    while (1)
      ;
  }
  MS5611.setOversampling(OSR_STANDARD);

}

float getAltitudeHypsometric(float press, float temp)
{
  return (((pow((SEA_PRESSURE / press), 1 / 5.257) - 1.0) * (temp + 273.15)) / 0.0065) - HEIGHT_REFERENCE;
}

float getAltitudeBarometric(float pressure) {
return (44330.0f * (1.0f - pow((double)pressure / (double)SEA_PRESSURE, 0.1902949f)))- HEIGHT_REFERENCE;
}

int getFirst4Digits(float number)
{
  char buffer[6];
  dtostrf(number, 6, 2, buffer);
  for (int i = 0; i < 5; i++)
  {
    if (buffer[i] == '.')
    {
      buffer[i] = buffer[i + 1];
      buffer[i + 1] = '\0';
      break;
    }
  }

  return atoi(buffer);
}

float getAltitude(float current_pressure){
  return getAltitudeBarometric(current_pressure);
}
float getAltitude(float current_pressure, float current_temprature){
  return getAltitudeHypsometric(current_pressure, current_temprature);
}

float getAverage(int size)
{
  float total=0;
for (int i = 0; i < size; i++)
  {
    float current_temprature = MS5611.getTemperature();
    float current_pressure = MS5611.getPressure();
    total+=getAltitude(current_pressure);
    delay(AVG_DELAY);
  }
  return total/size;
}

void loop()
{
  // Set the brightness to 5 (0=dimmest 7=brightest)
  display.setBrightness(5);

  MS5611.read(); //  note no error checking => "optimistic".
  float current_temprature = MS5611.getTemperature();
  float current_pressure = MS5611.getPressure();
  float current_altitude_hypo = getAltitudeHypsometric(current_pressure, current_temprature);
  float current_altitude_baro=   getAltitudeBarometric(current_pressure);
  int altitude_output = getFirst4Digits(current_altitude_hypo);

  // Just print out the raw values for debugging purposes
  Serial.print("T:\t");
  Serial.print(current_temprature, 3);
  Serial.print("\tP:\t");
  Serial.print(current_pressure, 3);
  Serial.println();
  Serial.println(current_altitude_hypo);
  Serial.println("Shown in the 4-Digit Disply is: "+ String(altitude_output));
  Serial.println(current_altitude_baro);

  
  altitude_output = getFirst4Digits(getAverage(AVG_SIZE));
  Serial.println(altitude_output);

  // Prints altitude
  display.showNumberDecEx(altitude_output, 0b00100000, false, 4, 0);
  delay(DELAY);
}
