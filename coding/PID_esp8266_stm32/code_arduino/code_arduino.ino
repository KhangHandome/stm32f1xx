#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

const char* ssid = "Indoor coffee";
const char* password = "camonquykhach";

ESP8266WebServer server(80);
SoftwareSerial uart(D5, D6); // RX, TX

String currentSpeed = "0";

void handleRoot() {
  String html = R"====(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Motor Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
      body { font-family: Arial; margin: 20px; }
      .slider { width: 80%; }
      label { display: block; margin-top: 10px; }
      span { font-weight: bold; }
      canvas { max-width: 100%; height: 80%; }
    </style>
    <script>
      let chart;
      let labels = [];
      let dataPoints = [];

      function updateParams() {
        let speed = document.getElementById("speed").value;
        let kp = document.getElementById("kp").value;
        let ki = document.getElementById("ki").value;
        let kd = document.getElementById("kd").value;

        // Update UI display
        document.getElementById("speedVal").innerText = speed;
        document.getElementById("kpVal").innerText = Number(kp).toFixed(2);
        document.getElementById("kiVal").innerText = Number(ki).toFixed(2);
        document.getElementById("kdVal").innerText = Number(kd).toFixed(2);

        // Send to server
        fetch(`/setpid?kp=${kp}&ki=${ki}&kd=${kd}&setpoint=${speed}`);
      }

      function fetchSpeed() {
        fetch("/speed")
          .then(res => res.text())
          .then(val => {
            document.getElementById("current").innerText = val;

            const now = new Date().toLocaleTimeString();
            labels.push(now);
            dataPoints.push(Number(val));
            if (labels.length > 20) {
              labels.shift();
              dataPoints.shift();
            }

            chart.data.labels = labels;
            chart.data.datasets[0].data = dataPoints;
            chart.update();

            setTimeout(fetchSpeed, 1000);
          });
      }

      window.onload = function() {
        const ctx = document.getElementById('speedChart').getContext('2d');
        chart = new Chart(ctx, {
          type: 'line',
          data: {
            labels: [],
            datasets: [{
              label: 'Speed (%)',
              data: [],
              fill: false,
              borderColor: 'blue',
              tension: 0.1
            }]
          },
          options: {
            scales: {
              y: { beginAtZero: true, max: 100 }
            }
          }
        });
        fetchSpeed();
      };
    </script>
  </head>
  <body>
    <h2>Motor Speed Control</h2>

    <label>Speed: <span id="speedVal">0</span>%</label>
    <input type="range" class="slider" id="speed" min="0" max="100" value="0" oninput="updateParams()"><br>

    <label>Kp: <span id="kpVal">0.00</span></label>
    <input type="range" class="slider" id="kp" min="0" max="10" step="0.01" value="0" oninput="updateParams()"><br>

    <label>Ki: <span id="kiVal">0.00</span></label>
    <input type="range" class="slider" id="ki" min="0" max="10" step="0.01" value="0" oninput="updateParams()"><br>

    <label>Kd: <span id="kdVal">0.00</span></label>
    <input type="range" class="slider" id="kd" min="0" max="10" step="0.01" value="0" oninput="updateParams()"><br>

    <h3>Current Speed:</h3>
    <div id="current">0</div>

    <h3>Speed Chart:</h3>
    <canvas id="speedChart" width="400" height="200"></canvas>
  </body>
  </html>
  )====";
  server.send(200, "text/html", html);
}

void handleSetSpeed() {
  if (server.hasArg("speed")) {
    String speed = server.arg("speed");
    uart.print("SPD:" + speed + "\n");
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing speed argument");
  }
}

void handleSetPID() {
  String kp = server.arg("kp");
  String ki = server.arg("ki");
  String kd = server.arg("kd");
  String setpoint = server.arg("setpoint");
  String pid_data = "PID:" + kp + "," + ki + "," + kd + "," + setpoint + "\n";
  uart.print(pid_data);
  server.send(200, "text/plain", "PID Updated");
}

void handleGetSpeed() {
  server.send(200, "text/plain", currentSpeed);
}

void setup() {
  Serial.begin(115200);
  uart.begin(9600);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/set", handleSetSpeed);
  server.on("/setpid", handleSetPID);
  server.on("/speed", handleGetSpeed);
  server.begin();
}

void loop() {
  server.handleClient();

  // Read UART data
  static String buffer = "";
  while (uart.available()) {
    char c = uart.read();
    if (c == '\n') {
      if (buffer.startsWith("CURRENT:")) {
        currentSpeed = buffer.substring(8);
        currentSpeed.trim();
      }
      buffer = "";
    } else {
      buffer += c;
    }
  }
}
