#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "Adafruit_BMP280.h"
#include "DHT20.h"
#include <ArduinoJson.h>
#include <MQ135.h>

// ===================== WiFi =====================
#define WIFI_SSID "SETUP-797C"
#define WIFI_PASSWORD "crumb3771bucket"

// ===================== Azure =====================
#define SAS_TOKEN "SharedAccessSignature sr=cs147IotHub-hkeane.azure-devices.net%2Fdevices%2F147esp32&sig=cLnC56TRtS9n%2F9SJQdaEHwHEkipGBhvonFOlpgFrdH0%3D&se=1766288915"

String iothubName = "cs147IotHub-hkeane";
String deviceName = "147esp32";

String post_url =
	"https://" + iothubName +
	".azure-devices.net/devices/" +
	deviceName +
	"/messages/events?api-version=2021-04-12";

String get_url =
	"https://iot-ingest-func-henry-andzetf6gxafbef0.eastus2-01.azurewebsites.net/api/GetTelemetry?code=zTOCEMAPmJ4yVVbPqW0FZkEeXOh9M9LOJ-AczYnjlCkuAzFuZCuVSg==";

// ===================== TLS Root CA =====================
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
// ===================== Timing =====================
#define TELEMETRY_INTERVAL 60000
#define WIFI_TIMEOUT 15000
#define TIME_TO_SLEEP_SEC 30
#define SEC_TO_US 1000000ULL

// ===================== Pins =====================
#define MQ_SENSOR_PIN 34
#define HMD_LED 25
#define PRESSURE_LED 33
#define AQ_LED 26

// ===================== Thresholds =====================
// These are RELATIVE, not ppm
#define HUMIDITY_MAX 70.0
#define PRESSURE_MIN 100900.0
#define AQ_RELATIVE_THRESHOLD 1.30  // 30% above baseline

#define DATA_POINTS_PER_GET 7

// ===================== Globals =====================
Adafruit_BMP280 bmp;
DHT20 dht;
MQ135 mq135(MQ_SENSOR_PIN);

float aq_baseline = 0.0;
bool baseline_initialized = false;

// ===================== Helpers =====================
float read_air_quality_index() {
	// Raw resistance-based index (stable & relative)
	float r = mq135.getResistance();
	return r;
}

void set_leds(JsonDocument& doc) {
	float hmd_avg = 0.0;
	float pressure_avg = 0.0;
	float aq_avg = 0.0;

	for (int i = 0; i < DATA_POINTS_PER_GET; i++) {
		hmd_avg += float(doc[i]["humidity"]);
		pressure_avg += float(doc[i]["pressure"]);
		aq_avg += float(doc[i]["air_quality_index"]);
	}

	hmd_avg /= DATA_POINTS_PER_GET;
	pressure_avg /= DATA_POINTS_PER_GET;
	aq_avg /= DATA_POINTS_PER_GET;

	Serial.printf("AVG → HMD: %.2f  P: %.2f  AQ_IDX: %.2f\n",
				  hmd_avg, pressure_avg, aq_avg);

	digitalWrite(HMD_LED, hmd_avg > HUMIDITY_MAX);
	digitalWrite(PRESSURE_LED, pressure_avg < PRESSURE_MIN);
	digitalWrite(AQ_LED, aq_avg > aq_baseline * AQ_RELATIVE_THRESHOLD);

	Serial.println("LEDs updated");
}

// ===================== Setup =====================
void setup() {
	Serial.begin(9600);
	delay(500);

	bmp.begin();
	dht.begin();

	pinMode(MQ_SENSOR_PIN, INPUT);
	pinMode(HMD_LED, OUTPUT);
	pinMode(PRESSURE_LED, OUTPUT);
	pinMode(AQ_LED, OUTPUT);

	esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_SEC * SEC_TO_US);

	// Establish AQ baseline (clean-ish air)
	float sum = 0.0;
	for (int i = 0; i < 20; i++) {
		sum += read_air_quality_index();
		delay(200);
	}
	aq_baseline = sum / 20.0;
	baseline_initialized = true;

	Serial.printf("AQ baseline established: %.2f\n", aq_baseline);
}

// ===================== Loop =====================
void loop() {
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	uint32_t start = millis();
	while (WiFi.status() != WL_CONNECTED) {
		if (millis() - start > WIFI_TIMEOUT) {
			WiFi.disconnect();
			esp_light_sleep_start();
			return;
		}
		delay(500);
		Serial.print(".");
	}

	Serial.println("\nWiFi connected");
	Serial.println(WiFi.localIP());

	// ---------- GET historical data ----------
	HTTPClient http;
	http.begin(get_url);

	JsonDocument in_doc;
	if (http.GET() == 200) {
		deserializeJson(in_doc, http.getString());
		set_leds(in_doc);
	}
	http.end();

	// ---------- Read sensors ----------
	dht.read();
	float temp = dht.getTemperature();
	float humidity = dht.getHumidity();
	float pressure = bmp.readPressure();
	float aq_index = read_air_quality_index();

	// ---------- Build JSON ----------
	JsonDocument out_doc;
	out_doc["temperature"] = temp;
	out_doc["humidity"] = humidity;
	out_doc["pressure"] = pressure;
	out_doc["air quality"] = aq_index; // air quality index

	char payload[256];
	serializeJson(out_doc, payload, sizeof(payload));

	Serial.println("\nSending telemetry:");
	Serial.println(payload);

	// ---------- POST to IoT Hub ----------
	WiFiClientSecure client;
	client.setCACert(root_ca);

	http.begin(client, post_url);
	http.addHeader("Content-Type", "application/json");
	http.addHeader("Authorization", SAS_TOKEN);

	int code = http.POST(payload);
	if (code == 204) {
		Serial.println("✓ Telemetry sent");
	} else {
		Serial.printf("✗ Failed (%d)\n", code);
	}

	http.end();
	WiFi.disconnect();

	Serial.flush();
	esp_light_sleep_start();
}
