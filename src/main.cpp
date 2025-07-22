/*
  Modified Code: Device returns data every second with higher precision
*/
#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <version.h>

#if defined(ESP32)
#error "Software Serial is not supported on the ESP32"
#endif

/* Use software serial for the PZEM
   Pin 12 Rx (Connects to the Tx pin on the PZEM)
   Pin 13 Tx (Connects to the Rx pin on the PZEM)
*/
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 10
#define PZEM_TX_PIN 11
#endif

void (*resetFunc)(void) = 0;

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);

char inBuf[20];
int ledState = 0;
float voltageCalibration = 0.92;
float currentCalibration = 2.52;
float powerFactorCalibration = 1.0;
byte isPaused = 0;
unsigned long lastMeasurementTime = 0;

void blinkLed()
{
  // Serial.print("Led State:"); Serial.println(ledState);
  digitalWrite(13, ledState ? LOW : HIGH);
  ledState = !ledState;
}

float prevVoltage = 0;
float prevCurrent = 0;
float prevPower = 0;
float prevPf = 0;
float prevFrequency = 0;
int voltageErrors = 0;

void readPzemData()
{
  // Read data from the sensor
  float voltage = pzem.voltage();
  float current = pzem.current();
  // float power = pzem.power();
  // float pf = pzem.pf();
  float frequency = pzem.frequency();

  // Validate readings and use previous values if needed
  if (!isnan(voltage))
  {
    prevVoltage = voltage;
    voltageErrors = 0;
  }
  else
  {
    voltageErrors++;
    voltage = (voltageErrors < 2) ? prevVoltage : 0;
  }

  if (!isnan(current))
  {
    prevCurrent = current;
  }
  else
  {
    current = (voltageErrors < 2) ? prevCurrent : 0;
  }

  // if (!isnan(power)) {
  //   prevPower = power;
  // } else {
  //   power = (voltageErrors < 2) ? prevPower : 0;
  // }

  // if (!isnan(pf)) {
  //   prevPf = pf;
  // } else {
  //   pf = (voltageErrors < 2) ? prevPf : 0;
  // }

  if (!isnan(frequency))
  {
    prevFrequency = frequency;
  }
  else
  {
    frequency = (voltageErrors < 2) ? prevFrequency : 0;
  }

  // Apply calibration factors
  voltage *= voltageCalibration;
  current *= currentCalibration;

  if (voltage < 0.0 || voltage > 300.0 || current < 0.0 || current > 100.0 || frequency < 45.0 || frequency > 65.0)
  {
    return;
  }
  // Print values with high precision
  // Print values as JSON
  Serial.print("{");
  Serial.print("\"type\":");
  Serial.print("\"data\"");
  Serial.print(",");
  // Serial.print("\"power\":"); Serial.print(power, 3); Serial.print(",");
  Serial.print("\"voltage\":");
  Serial.print(voltage, 3);
  Serial.print(",");
  Serial.print("\"current\":");
  Serial.print(current, 3);
  Serial.print(",");
  Serial.print("\"frequency\":");
  Serial.print(frequency, 3);
  // Serial.print("\"powerFactor\":"); Serial.print(pf, 3);
  Serial.println("}");
}

void processNewCoefficients(char *str)
{
  byte j = 0;
  byte k = 0;
  char buf[10];

  memset(buf, 0, sizeof(buf));
  for (byte i = 0; i < 20; i++)
  {
    if (str[i] == ':' || str[i] == 0)
    {
      switch (k)
      {
      case 0:
        voltageCalibration = atof(buf);
        break;
      case 1:
        currentCalibration = atof(buf);
        break;
      case 2:
        powerFactorCalibration = atof(buf);
      }
      k++;
      j = 0;
      memset(buf, 0, sizeof(buf));
      if (str[i] == 0)
      {
        break;
      }
    }
    else
    {
      buf[j++] = str[i];
    }
  }

  Serial.print("{");
  Serial.print("\"type\":");
  Serial.print("\"coefficients\"");
  Serial.print(",");
  Serial.print("\"voltage\":");
  Serial.print(voltageCalibration, 3);
  Serial.print(",");
  Serial.print("\"current\":");
  Serial.print(currentCalibration, 3);
  Serial.print(",");
  Serial.print("\"powerFactor\":");
  Serial.print(powerFactorCalibration, 3);
  Serial.println("}");
}

void setup()
{
  Serial.begin(57600); // Debugging serial
  Serial.println("Starting...");
  pinMode(13, OUTPUT); // LED pin
}

int i = 0;

void loop()
{
  float v;
  while (Serial.available() > 0)
  {
    byte inChar = Serial.read();
    if (inChar != '\n' && inChar != '\r')
    {
      inBuf[i++] = (char)inChar;
    }
    else
    {
      switch (inBuf[0])
      {
      case 'i':
        Serial.print("{");
        Serial.print("\"type\":");
        Serial.print("\"coefficients\"");
        Serial.print(",");
        Serial.print("\"voltage\":");
        Serial.print(voltageCalibration, 3);
        Serial.print(",");
        Serial.print("\"current\":");
        Serial.print(currentCalibration, 3);
        Serial.print(",");
        Serial.print("\"powerFactor\":");
        Serial.print(powerFactorCalibration, 3);
        Serial.println("}");
        break;
      case 'v':
        v = atof(inBuf + 1);
        voltageCalibration = v;
        Serial.print("{");
        Serial.print("\"type\":");
        Serial.print("\"coefficients\"");
        Serial.print(",");
        Serial.print("\"voltage\":");
        Serial.print(voltageCalibration, 3);
        Serial.println("}");
        break;
      case 'p':
        v = atof(inBuf + 1);
        powerFactorCalibration = v;
        Serial.print("{");
        Serial.print("\"type\":");
        Serial.print("\"coefficients\"");
        Serial.print(",");
        Serial.print("\"powerFactor\":");
        Serial.print(powerFactorCalibration, 3);
        Serial.println("}");
        break;
      case 'c':
        v = atof(inBuf + 1);
        currentCalibration = v;
        Serial.print("{");
        Serial.print("\"type\":");
        Serial.print("\"coefficients\"");
        Serial.print(",");
        Serial.print("\"current\":");
        Serial.print(currentCalibration, 3);
        Serial.println("}");
        break;
      case 's':
        processNewCoefficients(inBuf + 1);
        break;
      case 'h':
        Serial.println("Usage");
        Serial.println("i - calibrations values: voltage:current:power factor");
        Serial.println("v - voltage calibration");
        Serial.println("p - power factor calibration");
        Serial.println("c - current calibration");
        Serial.println("d - build date");
        Serial.println("w - pause monitor");
        break;
      case 'd':
        Serial.print("{");
        Serial.print("\"type\":");
        Serial.print("\"info\"");
        Serial.print(",");
        Serial.print("\"version\":");
        Serial.print("\"");
        Serial.print(VERSION);
        Serial.print("\"");
        Serial.print(",");
        Serial.print("\"date\":");
        Serial.print("\"");
        Serial.print(BUILD_DATE);
        Serial.print("\"");
        Serial.println("}");
        break;
      case 'w':
        isPaused = !isPaused;
        break;
      case 'r':
        Serial.println("Resetting...");
        resetFunc();
        break;
      }
      i = 0;
      memset(inBuf, 0, sizeof(inBuf));
    }
  }

  unsigned long currentTime = millis();
  if (!isPaused && (currentTime - lastMeasurementTime >= 999))
  {
    lastMeasurementTime = currentTime;
    blinkLed();
    readPzemData();
  }
}
