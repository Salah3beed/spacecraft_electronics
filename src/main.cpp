// Include the libraries
#include <Arduino.h>
#include <TM1637Display.h>
#include "MS5611.h"

// Define the connections pins
#define CLK 3
#define DIO 4
#define FILTER_SIZE 10 // The size of the moving average filter

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

// Kalman filter variables
float x_hat;         // Estimated state
float P;             // Estimated error covariance
float Q;             // Process noise covariance
float R;             // Measurement noise covariance
float K;             // Kalman gain

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

// Initialize Kalman filter parameters
void kalmanFilterInit(float initialPressure, float initialError) {
    x_hat = initialPressure;  // Initial estimate
    P = initialError;         // Initial error covariance
    Q = 0.001;                // Process noise covariance (adjust as needed)
    R = 0.1;                  // Measurement noise covariance (adjust as needed)
}

// Update Kalman filter with new pressure measurement
float kalmanFilterUpdate(float pressureMeasurement) {
    // Prediction step
    x_hat = x_hat;  // In a more complex scenario, you would predict the next state based on system dynamics

    // Update step
    K = P / (P + R);  // Kalman gain
    x_hat = x_hat + K * (pressureMeasurement - x_hat);  // Update estimate
    P = (1 - K) * P;  // Update error covariance

    return x_hat;
}


float movingAverageFilter(float rawValue) {
    static float buffer[FILTER_SIZE];
    static int index = 0;
    float sum = 0;

    // Update the buffer with the latest raw value
    buffer[index] = rawValue;

    // Calculate the sum of the values in the buffer
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += buffer[i];
    }

    // Move to the next position in the buffer
    index = (index + 1) % FILTER_SIZE;

    // Return the filtered value
    return sum / FILTER_SIZE;
}

float getAltitudeHypsometric(float pressure, float temprature)
{
  return (((pow((SEA_PRESSURE / pressure), 1 / 5.257) - 1.0) * (temprature + 273.15)) / 0.0065) - HEIGHT_REFERENCE;
}

float getAltitudeBarometric(float pressure) {
return (44330.0f * (1.0f - pow((double)pressure / (double)SEA_PRESSURE, 0.1902949f)))- HEIGHT_REFERENCE;
}

float getAltitudeBarometricFiltered(float pressure) {
    return getAltitudeBarometric(movingAverageFilter(pressure));
}
float getAltitudeHypsometricFiltered(float pressure, float temprature) {
    return getAltitudeHypsometric(movingAverageFilter(pressure), temprature);
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
  // return getAltitudeBarometric(current_pressure);
  return getAltitudeBarometricFiltered(current_pressure);
}
float getAltitude(float current_pressure, float current_temprature){
  // return getAltitudeHypsometric(current_pressure, current_temprature);
  return getAltitudeHypsometricFiltered(current_pressure, current_temprature);
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
  float another_pressure = MS5611.getPressure();
  kalmanFilterInit(current_pressure, current_pressure-another_pressure);  // Initialize Kalman filter
  float current_altitude_hypo = getAltitude(current_pressure, current_temprature);
  float current_altitude_baro=   getAltitude(current_pressure);
  int altitude_output = getFirst4Digits(current_altitude_baro);

  // Just print out the raw values for debugging purposes
  Serial.print("T:\t");
  Serial.print(current_temprature, 3);
  Serial.print("\tP:\t");
  Serial.print(current_pressure, 3);
  Serial.println();
  // Serial.println("Shown in the 4-Digit Disply is: "+ String(altitude_output));
  Serial.println("Barometric Altitude Filtered: "+ String(current_altitude_baro));
  Serial.println("Hypsometric Altitude Filtered: "+ String(current_altitude_hypo));


  float kalman_pressure = kalmanFilterUpdate(current_pressure);
  Serial.println("Kalman Filtered Altitude: "+String(getAltitude(kalman_pressure)));

  altitude_output = getFirst4Digits(getAverage(AVG_SIZE));
  Serial.println(altitude_output);

  // Prints altitude
  display.showNumberDecEx(altitude_output, 0b00100000, false, 4, 0);
  delay(DELAY);
}
