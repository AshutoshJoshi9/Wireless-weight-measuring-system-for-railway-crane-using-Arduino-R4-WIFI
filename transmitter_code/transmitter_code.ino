#include <WiFiS3.h>

const char* ssid = "ArduinoR4";
const char* password = "mechanica123";
int sensorPin = A1;  //analog pin where sensor is connected
int DACpin = A0;     //dac pin



WiFiServer server(80);

bool fetchData = false;  // flag to control data fetching

void setup() {
  analogReadResolution(14); //change to 14-bit resolution for adc
  analogWriteResolution(12); //change to 12-bit resolution for dac
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(DACpin, OUTPUT);

  // Start the access point
  Serial.println("Setting up WiFi access point...");
  if (!WiFi.beginAP(ssid, password)) {
    Serial.println("Failed to create access point");
    while (true); // if failed
  }

  // initialize server
  server.begin();
  Serial.println("Server started");

  // to get IP address
  Serial.print("AP IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if a new client connects,
    Serial.println("New client connected");
    String currentLine = "";                // string to hold incoming data from the client
    String request = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // to read from the client,
        char c = client.read();             // reading a byte
        request += c;                       // appending to request string
        Serial.write(c);                    // printing it out to serial monitor
        if (c == '\n') {                    // if byte is a newline character
          if (currentLine.length() == 0) {
            if (request.indexOf("GET /sensor") >= 0) {
              int sensorValue0 = 0;
              for (int i = 0; i < 20; i++) {         // loop to average out 20 readings to avoid fluctuations
                sensorValue0 = sensorValue0 + analogRead(sensorPin);
                delay(100);
              }
              sensorValue0 = round(sensorValue0 / 20);
              String sensorValueStr = String(sensorValue0);
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/plain");
              client.println("Connection: close");
              client.println();
              client.println(sensorValueStr);
            } else if (request.indexOf("GET /start") >= 0) {
              fetchData = true;
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/plain");
              client.println("Connection: close");
              client.println();
              client.println("Started fetching data");
            } else if (request.indexOf("GET /stop") >= 0) {
              fetchData = false;
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/plain");
              client.println("Connection: close");
              client.println();
              client.println("Stopped fetching data");
            } else {
              // Serve the main page
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/html");
              client.println("Connection: close");
              client.println();
              client.println("<!DOCTYPE HTML>");
              client.println("<html>");
              client.println("<head>");
              client.println("<title>Sensor Data</title>");
              client.println("<script>");
              client.println("let intervalId;");
              client.println("function fetchSensorData() {");
              client.println("  fetch('/sensor').then(response => response.text()).then(data => {");
              client.println("    document.getElementById('sensorValue').innerText = data;");
              client.println("  });");
              client.println("}");
              client.println("function startFetching() {");
              client.println("  fetch('/start');");
              client.println("  intervalId = setInterval(fetchSensorData, 1000);");
              client.println("}");
              client.println("function stopFetching() {");
              client.println("  fetch('/stop');");
              client.println("  clearInterval(intervalId);");
              client.println("}");
              client.println("</script>");
              client.println("</head>");
              client.println("<body>");
              client.println("<h1>Sensor Value: <span id='sensorValue'>Loading...</span></h1>");
              client.println("<button onclick='startFetching()'>Start</button>");
              client.println("<button onclick='stopFetching()'>Stop</button>");
              client.println("</body>");
              client.println("</html>");
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') { 
          currentLine += c;      
        }
      }
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
  delay(100);
}
