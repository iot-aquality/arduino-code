#include <ph4502c_sensor.h>
#include <ArduinoJson.h>

PH4502C_Sensor ph4502c(A0, A1);

// Definir pines sensor distancia y flujo
const int pinSensorDistancia = A3;
const int pinSensorFlujoSalida = 2;

// Variables
int valorSensorDistanciaActual;
int valorSensorDistanciaAnterior;
bool sensorFlujoActivado = false;
int valorFlujoAnterior;

void setup() {
  ph4502c.init();
  Serial.begin(9600);
  pinMode(pinSensorDistancia, INPUT);
  pinMode(pinSensorFlujoSalida, INPUT);
  valorSensorDistanciaAnterior = analogRead(pinSensorDistancia);
}

void loop() {
  // Obtener valores de sensores
  float sensor_ph = ph4502c.read_ph_level();
  float sensor_turbidez = random(0, 20);  // No se tiene el sensor de turbiedad
  int SENSOR_FLUJO_AGUA_SALIDA = analogRead(6);
  int SENSOR_FLUJO_AGUA_ENTRADA = analogRead(7);
  valorSensorDistanciaActual = analogRead(pinSensorDistancia);

  bool perdida_agua = false;


  // Detectar pérdida de agua
  if (valorSensorDistanciaActual < valorSensorDistanciaAnterior) {
    // Verificar si el valor del sensor de flujo ha subido por encima de 20
    if (SENSOR_FLUJO_AGUA_SALIDA > valorFlujoAnterior && SENSOR_FLUJO_AGUA_SALIDA > (valorFlujoAnterior + 20)) {
      sensorFlujoActivado = true;
    } else {
      sensorFlujoActivado = false;
    }

    if (!sensorFlujoActivado) {
      perdida_agua = true;
    }
  }

  valorSensorDistanciaAnterior = valorSensorDistanciaActual;
  valorFlujoAnterior = SENSOR_FLUJO_AGUA_SALIDA;

  // Crear el objeto JSON
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["SENSOR_PH"] = sensor_ph;
  jsonDoc["SENSOR_TURBIDEZ"] = sensor_turbidez;
  jsonDoc["PERDIDA_AGUA"] = perdida_agua;
  jsonDoc["SENSOR_FLUJO_AGUA_ENTRADA"] = SENSOR_FLUJO_AGUA_ENTRADA;
  jsonDoc["SENSOR_FLUJO_AGUA_SALIDA"] = SENSOR_FLUJO_AGUA_SALIDA;

  // Convertir el JSON a una cadena
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Envía los datos al ESP
  Serial.println(jsonString);
  delay(1000);
}