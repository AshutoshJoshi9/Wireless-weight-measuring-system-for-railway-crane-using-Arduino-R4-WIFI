#include <WiFiS3.h>

const char* ssid = "ArduinoR4";  // ssid of transmitter
const char* password = "mechanica123";  // password
const char* host = "192.168.4.1";  // IP address of accesss point

int DACpin = A0;     // DAC pin

WiFiClient client;

void setup() {
  analogReadResolution(14); // change to 14-bit resolution for ADC
  analogWriteResolution(12); // change to 12-bit resolution for DAC
  Serial.begin(115200);
  pinMode(DACpin, OUTPUT);

  // connect to Wifi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  if (client.connect(host, 80)) {
    Serial.println("Connected to transmitter");

    // HTTP GET request to get sensor data
    client.println("GET /sensor HTTP/1.1");
    client.println("Host: 192.168.4.1");
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        String sensorValueStr = client.readStringUntil('\n');
        int sensorValue = sensorValueStr.toInt();
        // Serial.print("Received sensor value: ");
        // Serial.println(sensorValue);

        //mappinng the sensor value to DAC value (assuming a 14-bit to 12-bit mapping)
        int reading = 0;
        for (int i=0; i < 40; i++){
          int DACvalue = map(sensorValue, 0, 16383, 0, 2047);
          reading = reading + DACvalue;
          delay(100);
        }
        reading = round(reading / 40);
        analogWrite(DACpin, reading);
        break;
      }
    }
    client.stop();
  } else {
    Serial.println("Failed to connect to transmitter");
    analogWrite(DACpin, 0); // so that the reading is zero when not receiving any data
  } 

  delay(1000);  // delay before fetching new data
}
