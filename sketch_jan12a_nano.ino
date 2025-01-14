#include <DHT.h>

// Define DHT sensor
#define DHTPIN 7        // Pin connected to DHT sensor
#define DHTTYPE DHT11   // DHT sensor type (DHT11)

// Define MQ135 sensor
#define MQ135PIN A0     // Pin connected to MQ135 sensor

// Constants for MQ135 calculations
const float RLOAD = 10.0;          // Load resistance on the MQ135 board
const float RZERO = 3.62;          // Base resistance at fresh air
const float PARA = 116.6020682;    // CO2 parameter
const float PARB = 2.769034857;    // CO2 parameter
const float ETHYLENE_COEFFICIENT = 0.8;  // Assumed relative sensitivity to ethylene (scale factor)
const float AMMONIA_COEFFICIENT = 0.6;   // Assumed relative sensitivity to ammonia (scale factor)

// Variables for smoothing
const int SMOOTHING_WINDOW = 10;  // Number of readings for moving average
float co2Buffer[SMOOTHING_WINDOW] = {0};
float ethyleneBuffer[SMOOTHING_WINDOW] = {0};
float ammoniaBuffer[SMOOTHING_WINDOW] = {0};
int bufferIndex = 0;

// Initialize DHT object
DHT dht(DHTPIN, DHTTYPE);

void resetSensors() {
  // Simulating sensor reset by reinitializing or flushing buffers
  for (int i = 0; i < SMOOTHING_WINDOW; i++) {
    co2Buffer[i] = 0;
    ethyleneBuffer[i] = 0;
    ammoniaBuffer[i] = 0;
  }
  bufferIndex = 0;
  delay(500); // Simulate a small delay to mimic hardware reset
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize DHT sensor
  dht.begin();

  // Print CSV header to Serial Monitor
  Serial.println("Time(ms),Temperature(°C),Humidity(%),CO2(ppm),Ethylene(ppm),Ammonia(ppm),Ripeness");
}

void loop() {
  // Allow sensors to stabilize
  delay(2000);

  // Read DHT11 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Validate DHT11 readings
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    resetSensors();
    return;
  }

  // Read MQ135 sensor
  int mq135Raw = analogRead(MQ135PIN);
  float mq135Voltage = (mq135Raw / 1023.0) * 5.0;

  if (mq135Voltage < 0.1) {
    Serial.println("Sensor voltage too low, skipping calculation.");
    resetSensors();
    return;
  }

  float resistance = ((5.0 * RLOAD) / mq135Voltage) - RLOAD;
  if (resistance < 0) {
    Serial.println("Calculated resistance is invalid, skipping.");
    resetSensors();
    return;
  }

  float ratio = resistance / RZERO;
  float co2ppm = PARA * pow(ratio, -PARB);
  if (co2ppm < 0 || isinf(co2ppm)) {
    Serial.println("Invalid CO2 value, skipping.");
    resetSensors();
    return;
  }

  float ethylenePpm = co2ppm * ETHYLENE_COEFFICIENT;
  float ammoniaPpm = co2ppm * AMMONIA_COEFFICIENT;

  // Smooth readings using a moving average
  co2Buffer[bufferIndex] = co2ppm;
  ethyleneBuffer[bufferIndex] = ethylenePpm;
  ammoniaBuffer[bufferIndex] = ammoniaPpm;
  bufferIndex = (bufferIndex + 1) % SMOOTHING_WINDOW;

  float co2Smoothed = 0;
  float ethyleneSmoothed = 0;
  float ammoniaSmoothed = 0;
  for (int i = 0; i < SMOOTHING_WINDOW; i++) {
    co2Smoothed += co2Buffer[i];
    ethyleneSmoothed += ethyleneBuffer[i];
    ammoniaSmoothed += ammoniaBuffer[i];
  }
  co2Smoothed /= SMOOTHING_WINDOW;
  ethyleneSmoothed /= SMOOTHING_WINDOW;
  ammoniaSmoothed /= SMOOTHING_WINDOW;

  // Determine ripeness status
  String ripenessStatus = "Not Yet Ripened";
  if (temperature > 20 && humidity > 70 && ethyleneSmoothed > 10) {
    ripenessStatus = "Likely Ripened";
  }

  // Log data to Serial Monitor
  Serial.print(millis());
  Serial.print(",");
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
  Serial.print(co2Smoothed);
  Serial.print(",");
  Serial.print(ethyleneSmoothed);
  Serial.print(",");
  Serial.print(ammoniaSmoothed);
  Serial.print(",");
  Serial.println(ripenessStatus);

  // Reset sensors before the next reading
  resetSensors();

  // Wait before next reading
  delay(10000);  // 10 seconds
}
