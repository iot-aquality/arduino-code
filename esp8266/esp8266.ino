#include <ESP8266WiFi.h>
#include <Arduino_MQTT_Client.h>
#define THINGSBOARD_ENABLE_PROGMEM 0
#include <ThingsBoard.h>

constexpr char WIFI_SSID[] = "Pancho";
constexpr char WIFI_PASSWORD[] = "2101615111";

//Token de acceso del dispositivo
constexpr char TOKEN[] = "oPZHGB41yhScX3aDKFK4";
// Thingsboard we want to establish a connection too
constexpr char THINGSBOARD_SERVER[] = "iot.ceisufro.cl";
// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port.
constexpr uint16_t THINGSBOARD_PORT = 1883U;

// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;

// Baud rate for the debugging serial connection.
// If the Serial output is mangled, ensure to change the monitor speed accordingly to this variable
constexpr uint32_t SERIAL_DEBUG_BAUD = 9600U;

// Initialize underlying client, used to establish a connection
WiFiClient wifiClient;
// Initalize the Mqtt client instance
Arduino_MQTT_Client mqttClient(wifiClient);
// Initialize ThingsBoard instance with the maximum needed buffer size
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

// Attribute names for attribute request and attribute updates functionality
constexpr char SENSOR_PH_KEY[] = "sensor_ph";
constexpr char SENSOR_TURBIDEZ_KEY[] = "sensor_turbidez";
constexpr char PERDIDA_AGUA_KEY[] = "perdida_agua";
constexpr char SENSOR_FLUJO_AGUA_ENTRADA_KEY[] = "sensor_flujo_agua_entrada";
constexpr char SENSOR_FLUJO_AGUA_SALIDA_KEY[] = "sensor_flujo_agua_salida";

/// @brief Initalizes WiFi connection,
// will endlessly delay until a connection has been successfully established
void InitWiFi() {
  Serial.println("Connecting to WiFi ...");
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been succesfully established
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

/// @brief Reconnects the WiFi uses InitWiFi if the connection has been removed
/// @return Returns true as soon as a connection has been established again
const bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }

  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

void setup() {
  // Initalize serial connection for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);

  delay(1000);
  InitWiFi();
}

void loop() {
  if (Serial.available() > 0) {
    if (!reconnect()) {
      return;
    }

    if (!tb.connected()) {
      // Connect to the ThingsBoard
      Serial.printf("Connecting to: (%s) with token (%s)\n", THINGSBOARD_SERVER, TOKEN);

      if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
        Serial.println("Failed to connect");
        return;
      } else {
        Serial.println("Successfully connected");
      }
    }

    // Lee los datos del Arduino
    String datos = Serial.readStringUntil('\n');

    // Parsea la cadena JSON
    DynamicJsonDocument jsonDoc(200);
    DeserializationError error = deserializeJson(jsonDoc, datos);

    if (error) {
      Serial.print("Error al analizar JSON: ");
      Serial.println(error.c_str());
      return;
    }
    // Obtén los valores específicos
    float sensor_ph = jsonDoc["SENSOR_PH"];
    float sensor_turbidez = jsonDoc["SENSOR_TURBIDEZ"];
    bool perdida_agua = jsonDoc["PERDIDA_AGUA"];
    int sensor_flujo_agua_entrada = jsonDoc["SENSOR_FLUJO_AGUA_ENTRADA"];
    int sensor_flujo_agua_salida = jsonDoc["SENSOR_FLUJO_AGUA_SALIDA"];

    int valorSensorDistanciaAnterior = jsonDoc["DISTANCIA_ANTERIOR"];
    int valorSensorDistanciaActual = jsonDoc["DISTANCIA_ACTUAL"];
    int sensorFlujoActivado = jsonDoc["ESTADO_SENSOR_FLUJO"];

    Serial.print("Distancia anterior: ");
    Serial.println(valorSensorDistanciaAnterior);
    Serial.print("Distancia actual: ");
    Serial.println(valorSensorDistanciaActual);
    Serial.print("Estado sensor flujo: ");
    Serial.println(sensorFlujoActivado);

    tb.sendTelemetryData(SENSOR_PH_KEY, sensor_ph);
    tb.sendTelemetryData(SENSOR_TURBIDEZ_KEY, sensor_turbidez);
    tb.sendTelemetryData(PERDIDA_AGUA_KEY, perdida_agua);
    tb.sendTelemetryData(SENSOR_FLUJO_AGUA_ENTRADA_KEY, sensor_flujo_agua_entrada);
    tb.sendTelemetryData(SENSOR_FLUJO_AGUA_SALIDA_KEY, sensor_flujo_agua_salida);

    tb.loop();
  }

  delay(2000);
}
