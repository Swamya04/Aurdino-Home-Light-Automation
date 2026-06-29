#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "DC Power Monitor"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ACS712.h>

// WiFi credentials
char ssid[] = "YOUR_WIFI";
char pass[] = "YOUR_PASSWORD";

// Pin definitions
#define CURRENT_PIN 34     // ACS712 output
#define VOLTAGE_PIN 35     // Voltage sensor output
#define RELAY_PIN 5        // Relay control pin

// ACS712 object (pin, Vref, ADC resolution, sensitivity for 30A)
ACS712 ACS(CURRENT_PIN, 3.3, 4095, 66);

BlynkTimer timer;

// Voltage divider ratio of voltage sensor
float voltageRatio = 11.0;


// Relay control from Blynk button (V3)
BLYNK_WRITE(V3)
{
  int relayState = param.asInt();   // 0 or 1 from Blynk
  digitalWrite(RELAY_PIN, relayState);
}


// Function to average ADC readings (reduces noise)
int readADC(int pin)
{
  long sum = 0;

  for(int i = 0; i < 20; i++)
  {
    sum += analogRead(pin);
    delay(2);
  }

  return sum / 20;
}


// Read voltage, current and calculate power
void readSensors()
{
  // Read voltage sensor
  int adcVoltage = readADC(VOLTAGE_PIN);

  float sensorVoltage = adcVoltage * (3.3 / 4095.0);
  float voltage = sensorVoltage * voltageRatio;

  // Read current from ACS712
  float current = ACS.getCurrentDC();
  if(current < 0) current = 0;

  // Power calculation
  float power = voltage * current;

  // Print values to Serial Monitor
  Serial.print("Voltage: ");
  Serial.println(voltage);

  Serial.print("Current: ");
  Serial.println(current);

  Serial.print("Power: ");
  Serial.println(power);

  Serial.println("----------------");

  // Send data to Blynk
  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
}


void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);   // Relay OFF at start

  analogReadResolution(12);       // ESP32 ADC resolution

  ACS.autoMidPoint();             // Calibrate ACS712 offset

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Run sensor reading every 2 seconds
  timer.setInterval(2000L, readSensors);
}


void loop()
{
  Blynk.run();
  timer.run();
}