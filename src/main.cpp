/*
  Modified Code: Device returns data every second with higher precision
*/
#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <version.h>
#include "led_control.h"

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

const float MAX_VOLTAGE = 400.0; // Maximum voltage
const float MAX_CURRENT = 100.0; // Maximum current

char inBuf[20];
float voltageCalibration = 0.92;
float currentCalibration = 2.52;
float powerFactorCalibration = 1.0;
byte isPaused = 0;
unsigned long lastMeasurementTime = 0;

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

  if (!(voltage >= 0.0 && voltage <= MAX_VOLTAGE && current >= 0.0 && current <= MAX_CURRENT))
  {
    return;
  }

  if ((voltage == 0 && current > 0) || (voltage > 0 && current == 0))
  {
    return;
  }

  // Rewritten: avoid %f in snprintf (not supported on AVR -> produced '?')
  char jsonBuf[96];
  char vBuf[12], cBuf[12], fBuf[12];
  dtostrf(voltage, 0, 3, vBuf);
  dtostrf(current, 0, 3, cBuf);
  dtostrf(frequency, 0, 3, fBuf);

  int n = snprintf(jsonBuf, sizeof(jsonBuf),
                   "{\"type\":\"data\",\"voltage\":%s,\"current\":%s,\"frequency\":%s}",
                   vBuf, cBuf, fBuf);
  if (n > 0 && n < (int)sizeof(jsonBuf))
  {
    Serial.println(jsonBuf);
  }
}

void printCoefficients()
{
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

void processNewVoltageCoefficient(char *str)
{
  float v = atof(str);
  voltageCalibration = v;
  Serial.print("{");
  Serial.print("\"type\":");
  Serial.print("\"coefficients\"");
  Serial.print(",");
  Serial.print("\"voltage\":");
  Serial.print(voltageCalibration, 3);
  Serial.println("}");
}

void processNewPowerCoefficient(char *str)
{
  float v = atof(str);
  powerFactorCalibration = v;
  Serial.print("{");
  Serial.print("\"type\":");
  Serial.print("\"coefficients\"");
  Serial.print(",");
  Serial.print("\"powerFactor\":");
  Serial.print(powerFactorCalibration, 3);
  Serial.println("}");
}

void processNewCurrentCoefficient(char *str)
{
  float v = atof(str);
  currentCalibration = v;
  Serial.print("{");
  Serial.print("\"type\":");
  Serial.print("\"coefficients\"");
  Serial.print(",");
  Serial.print("\"current\":");
  Serial.print(currentCalibration, 3);
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

void printBoardInfo()
{
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
}

void printHelp()
{
  Serial.println("Usage");
  Serial.println("i - calibrations values: voltage:current:power factor");
  Serial.println("v - voltage calibration");
  Serial.println("p - power factor calibration");
  Serial.println("c - current calibration");
  Serial.println("d - build date");
  Serial.println("w - pause monitor");
}

void setup()
{
  Serial.begin(57600); // Debugging serial
  Serial.println("Starting PowerMonitor board...");
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
      // Prevent buffer overflow (previously could corrupt memory -> malformed JSON)
      if (i < (int)sizeof(inBuf) - 1)
      {
        inBuf[i++] = (char)inChar;
      }
      else
      {
        // Overflow: reset buffer
        i = 0;
        memset(inBuf, 0, sizeof(inBuf));
      }
    }
    else
    {
      switch (inBuf[0])
      {
      case 'i':
        printCoefficients();
        break;
      case 'v':
        processNewVoltageCoefficient(inBuf + 1);
        break;
      case 'p':
        processNewPowerCoefficient(inBuf + 1);
        break;
      case 'c':
        processNewCurrentCoefficient(inBuf + 1);
        break;
      case 's':
        processNewCoefficients(inBuf + 1);
        break;
      case 'h':
        printHelp();
        break;
      case 'd':
        printBoardInfo();
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
