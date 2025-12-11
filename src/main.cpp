#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include "Adafruit_BMP280.h"
#include "DHT20.h"
#include <ArduinoJson.h>
#include <string>
#include <MQ135.h>

#define WIFI_SSID "Goobernet" 
#define WIFI_PASSWORD "givemetheinternet"

// Azure IoT Hub configuration
#define SAS_TOKEN "SharedAccessSignature sr=cs147IotHub-hkeane.azure-devices.net%2Fdevices%2F147esp32&sig=cLnC56TRtS9n%2F9SJQdaEHwHEkipGBhvonFOlpgFrdH0%3D&se=1766288915"

// Root CA certificate for Azure IoT Hub (DigiCert Global Root G2)
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT\n" \
"MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg\n" \
"UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o\n" \
"eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9\n" \
"GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex\n" \
"mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx\n" \
"SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g\n" \
"dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK\n" \
"AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n" \
"VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj\n" \
"buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD\n" \
"AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v\n" \
"b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp\n" \
"Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3\n" \
"oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v\n" \
"dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB\n" \
"AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB\n" \
"klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc\n" \
"kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb\n" \
"eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR\n" \
"penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ\n" \
"QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg\n" \
"q/xKzj3O9hFh/g==\n" \
"-----END CERTIFICATE-----\n";

String iothubName = "cs147IotHub-hkeane"; 
String deviceName = "147esp32";
String url = "https://" + iothubName + ".azure-devices.net/devices/" +
             deviceName + "/messages/events?api-version=2021-04-12";

String get_url = "https://iot-ingest-func-henry-andzetf6gxafbef0.eastus2-01.azurewebsites.net/api/GetTelemetry?code=zTOCEMAPmJ4yVVbPqW0FZkEeXOh9M9LOJ-AczYnjlCkuAzFuZCuVSg==";

// Telemetry interval
#define TELEMETRY_INTERVAL 60000 // Send data every 3 seconds
#define time_to_sleep 30
#define wifi_timeout 15000
#define sec_to_us 1000000ULL

#define MQ_SENSOR_PIN 34
#define RZERO 2870
#define RLOAD 15

#define hmd_led 25
#define pressure_led 33
#define ppm_led 26

#define data_points_per_get 7

#define ppm_safe_range 600
#define hmd_safe_range 70
#define pa_safe_range 100900

uint32_t lastTelemetryTime = 0;
Adafruit_BMP280 bmp;
DHT20 dht;
MQ135 air_quality_sensor(MQ_SENSOR_PIN, RZERO, RLOAD);

void set_leds(JsonDocument doc){
  float hmd_avg;
  float pressure_avg;
  float ppm_avg;
  
  for(int i = 0; i < data_points_per_get; i++){
    hmd_avg += float(doc[i]["humidity"]);
    pressure_avg += float(doc[i]["pressure"]);
    ppm_avg += float(doc[i]["air quality"]);
  }

  hmd_avg /= data_points_per_get;
  pressure_avg /= data_points_per_get;
  ppm_avg /= data_points_per_get;

  Serial.println(String(hmd_avg) + " " + String(pressure_avg) + " " + String(ppm_avg));

  digitalWrite(hmd_led, (hmd_avg > hmd_safe_range) ? HIGH : LOW);
  digitalWrite(ppm_led, (ppm_avg > ppm_safe_range) ? HIGH : LOW);
  digitalWrite(pressure_led, (pressure_avg < pa_safe_range) ? HIGH : LOW);
  Serial.println("set LEDS");
  
}

void setup() {
  // put your setup code here, to run once:
  bmp.begin();
  dht.begin();
  pinMode(MQ_SENSOR_PIN, INPUT);
  pinMode(hmd_led, OUTPUT);
  pinMode(ppm_led, OUTPUT);
  pinMode(pressure_led, OUTPUT);
  
/*  digitalWrite(hmd_led, HIGH);
  digitalWrite(ppm_led, HIGH);
  digitalWrite(pressure_led, HIGH);
*/  
  Serial.begin(9600);

  esp_sleep_enable_timer_wakeup(time_to_sleep * sec_to_us);

  WiFi.mode(WIFI_STA);
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int wifi_start_time = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if(millis() - wifi_start_time >= wifi_timeout){
      WiFi.disconnect();
      Serial.flush();
      esp_deep_sleep_start();
    }

    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  HTTPClient http;

  http.begin(get_url);
  
  ArduinoJson::JsonDocument in_doc;

  int code = http.GET();
  if(code == 200){
    Serial.println("Got data");
    Serial.println("size: " + http.getSize());
    String input_buffer = http.getString();

    DeserializationError error = deserializeJson(in_doc, input_buffer);

    if(error){
      Serial.println("deserialization fail.");
      Serial.println(error.f_str());

    } else {
      Serial.println("deserialization success");
      set_leds(in_doc);
      delay(1000); //delay for one second so LED output is visible for longer
    }
  }

  http.end();

  dht.read();
  float temp = dht.getTemperature();
  float pressure = bmp.readPressure();
  float humidity = dht.getHumidity();

  float air_quality = air_quality_sensor.getCorrectedPPM(temp, humidity);

  Serial.println("\n=== Sending test telemetry ===");
  Serial.println("Temperature: " + String(temp));
  Serial.println("Pressure: " + String(pressure));
  Serial.println("Humidity: " + String(humidity));
  Serial.println("Air quality: " + String(air_quality));

  ArduinoJson::JsonDocument doc;
  doc["temperature"] = temp;
  doc["humidity"] = humidity;
  doc["pressure"] = pressure;
  doc["air quality"] = air_quality;
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  
  Serial.println("JSON payload: " + String(buffer));
  Serial.println("URL: " + url);

  WiFiClientSecure client;
  client.setCACert(root_ca); // Set root CA certificate
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", SAS_TOKEN);
    
  Serial.println("Sending POST request...");
  int httpCode = http.POST(buffer);

  if (httpCode == 204) { // IoT Hub returns 204 No Content for successful telemetry
    Serial.println("✓ SUCCESS! Telemetry sent: " + String(buffer));
  } else {
    Serial.println("✗ FAILED to send telemetry.");
    Serial.println("HTTP code: " + String(httpCode));
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    }
  }
  http.end();

  Serial.flush();
  esp_deep_sleep_start(); //on wakeup, setup is run again.
}

void loop(){}