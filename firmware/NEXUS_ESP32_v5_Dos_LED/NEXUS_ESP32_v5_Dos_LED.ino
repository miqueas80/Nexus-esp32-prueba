/*
  NEXUS-X Hardware Bridge v5
  ESP32-WROOM-32 + BME280/BMP280 I2C + Bluetooth LE + 2 LED

  LED 1: GPIO18 -> resistor de 330 ohm -> anodo (+) -> catodo (-) -> GND
  LED 2: GPIO19 -> resistor de 330 ohm -> anodo (+) -> catodo (-) -> GND
  SOLO LED DE BAJA TENSION. NO conectar cargas, relés o 220 V.
  Al iniciar y al desconectarse BLE, el LED se apaga.
  Mantiene los UUID y comandos de v2/v3.
*/

#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

static const char *DEVICE_NAME = "NEXUS-ESP32";
static const char *SERVICE_UUID =
    "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
static const char *STATUS_UUID =
    "beb5483e-36e1-4688-b7f5-ea07361b26a8";
static const char *COMMAND_UUID =
    "3a1e2bb8-531e-4d55-bc0c-b83b650e9069";

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t LED_PIN = 18;
constexpr uint8_t LED2_PIN = 19;

struct CommandMessage {
  char text[24];
};

BLEServer *bleServer = nullptr;
BLECharacteristic *statusCharacteristic = nullptr;
QueueHandle_t commands = nullptr;
volatile bool deviceConnected = false;
bool previousConnection = false;
bool virtualOn = false;
bool ledOn = false;
bool led2On = false;

Adafruit_BMP280 bmp;
Adafruit_BME280 bme;

enum SensorType {
  SENSOR_NONE,
  SENSOR_BMP280,
  SENSOR_BME280
};

SensorType sensorType = SENSOR_NONE;
uint8_t sensorAddress = 0;
unsigned long lastMeasurement = 0;

void setLed(bool value) {
  ledOn = value;
  digitalWrite(LED_PIN, value ? HIGH : LOW);
}

void setLed2(bool value) {
  led2On = value;
  digitalWrite(LED2_PIN, value ? HIGH : LOW);
}

const char *sensorName() {
  if (sensorType == SENSOR_BMP280) return "BMP280";
  if (sensorType == SENSOR_BME280) return "BME280";
  return "NO_DETECTADO";
}

bool readChipID(uint8_t address, uint8_t &chipID) {
  Wire.beginTransmission(address);
  Wire.write(0xD0);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(address, (uint8_t)1) != 1) return false;
  chipID = Wire.read();
  return true;
}

void detectSensor() {
  sensorType = SENSOR_NONE;
  sensorAddress = 0;
  const uint8_t addresses[] = {0x76, 0x77};

  for (uint8_t address : addresses) {
    uint8_t chipID = 0;
    if (!readChipID(address, chipID)) continue;

    bool initialized = false;
    if (chipID == 0x60) {
      initialized = bme.begin(address, &Wire);
      if (initialized) sensorType = SENSOR_BME280;
    } else if (chipID == 0x58) {
      initialized = bmp.begin(address);
      if (initialized) sensorType = SENSOR_BMP280;
    }

    if (initialized) {
      sensorAddress = address;
      Serial.print("[SENSOR] Detectado: ");
      Serial.println(sensorName());
      Serial.print("[SENSOR] Direccion: 0x");
      Serial.println(sensorAddress, HEX);
      return;
    }
  }
  Serial.println("[SENSOR] No detectado");
}

float readTemperature() {
  if (sensorType == SENSOR_BME280) return bme.readTemperature();
  if (sensorType == SENSOR_BMP280) return bmp.readTemperature();
  return NAN;
}

float readPressure() {
  if (sensorType == SENSOR_BME280) return bme.readPressure() / 100.0F;
  if (sensorType == SENSOR_BMP280) return bmp.readPressure() / 100.0F;
  return NAN;
}

float readHumidity() {
  if (sensorType == SENSOR_BME280) return bme.readHumidity();
  return NAN;
}

class ConnectionCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) override {
    deviceConnected = true;
    Serial.println("[BLE] Cliente conectado");
  }

  void onDisconnect(BLEServer *server) override {
    deviceConnected = false;
    // Apagado inmediato sin esperar a la proxima orden.
    setLed(false);
    setLed2(false);
    Serial.println("[BLE] Cliente desconectado, LEDs apagados");
  }
};

class CommandCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    String value = characteristic->getValue();
    value.trim();
    value.toUpperCase();
    if (value.length() == 0 ||
        value.length() >= sizeof(CommandMessage::text)) {
      Serial.println("[BLE] Comando invalido");
      return;
    }

    CommandMessage message = {};
    value.toCharArray(message.text, sizeof(message.text));
    if (xQueueSend(commands, &message, 0) != pdTRUE) {
      Serial.println("[BLE] Cola llena");
    }
  }
};

void publishStatus(const char *value) {
  statusCharacteristic->setValue(String(value));
  Serial.print("[NEXUS] ");
  Serial.println(value);
  if (deviceConnected) statusCharacteristic->notify();
}

void publishReading(const char *prefix, float value) {
  if (!isfinite(value)) {
    publishStatus("ERR:READ_FAILED");
    return;
  }
  String response = String(prefix) + String(value, 1);
  publishStatus(response.c_str());
}

void executeCommand(const char *command) {
  // Mantener comandos de demostracion originales.
  if (strcmp(command, "PING") == 0) {
    publishStatus("PONG");
  } else if (strcmp(command, "DEMO_ON") == 0) {
    virtualOn = true;
    publishStatus("ACK:DEMO_ON");
  } else if (strcmp(command, "DEMO_OFF") == 0) {
    virtualOn = false;
    publishStatus("ACK:DEMO_OFF");
  } else if (strcmp(command, "STATUS") == 0) {
    publishStatus(virtualOn ? "STATE:ON" : "STATE:OFF");
  }

  // Lecturas ambientales de la v3.
  else if (strcmp(command, "SENSOR") == 0) {
    String response = String("SENSOR:") + sensorName();
    publishStatus(response.c_str());
  } else if (strcmp(command, "TEMP") == 0) {
    if (sensorType == SENSOR_NONE) publishStatus("ERR:NO_SENSOR");
    else publishReading("TEMP:", readTemperature());
  } else if (strcmp(command, "PRESS") == 0) {
    if (sensorType == SENSOR_NONE) publishStatus("ERR:NO_SENSOR");
    else publishReading("PRES:", readPressure());
  } else if (strcmp(command, "HUM") == 0) {
    if (sensorType != SENSOR_BME280) publishStatus("ERR:NO_HUM");
    else publishReading("HUM:", readHumidity());
  }

  // Control fisico EXCLUSIVO del LED con resistor.
  else if (strcmp(command, "LED_ON") == 0) {
    // Ignorar orden pendiente si se perdió la conexión BLE.
    if (!deviceConnected) {
      setLed(false);
      publishStatus("ERR:DISCONNECTED");
    } else {
      setLed(true);
      publishStatus("ACK:LED_ON");
    }
  } else if (strcmp(command, "LED_OFF") == 0) {
    setLed(false);
    publishStatus("ACK:LED_OFF");
  } else if (strcmp(command, "LED_STATUS") == 0) {
    publishStatus(ledOn ? "LED:ON" : "LED:OFF");
  } else if (strcmp(command, "LED2_ON") == 0) {
    if (!deviceConnected) {
      setLed2(false);
      publishStatus("ERR:DISCONNECTED");
    } else {
      setLed2(true);
      publishStatus("ACK:LED2_ON");
    }
  } else if (strcmp(command, "LED2_OFF") == 0) {
    setLed2(false);
    publishStatus("ACK:LED2_OFF");
  } else if (strcmp(command, "LED2_STATUS") == 0) {
    publishStatus(led2On ? "LED2:ON" : "LED2:OFF");
  } else {
    publishStatus("ERR:UNKNOWN_COMMAND");
  }
}

void setup() {
  Serial.begin(115200);
  delay(350);
  Serial.println();
  Serial.println("NEXUS-X HARDWARE BRIDGE v5");

  // Configuracion segura: ambos LED apagados antes de BLE e I2C.
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  setLed(false);
  setLed2(false);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(100);
  delay(150);
  detectSensor();

  commands = xQueueCreate(8, sizeof(CommandMessage));
  if (commands == nullptr) {
    Serial.println("ERROR: cola BLE");
    while (true) delay(1000);
  }

  BLEDevice::init(DEVICE_NAME);
  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new ConnectionCallbacks());

  BLEService *service = bleServer->createService(SERVICE_UUID);
  statusCharacteristic = service->createCharacteristic(
      STATUS_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_NOTIFY);
  statusCharacteristic->addDescriptor(new BLE2902());
  statusCharacteristic->setValue("NEXUS-X LISTO");

  BLECharacteristic *commandCharacteristic = service->createCharacteristic(
      COMMAND_UUID, BLECharacteristic::PROPERTY_WRITE);
  commandCharacteristic->setCallbacks(new CommandCallbacks());

  service->start();
  BLEAdvertising *advertising = bleServer->getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->start();

  Serial.println("[BLE] NEXUS-ESP32 anunciandose");
  Serial.println("[LED] GPIO18 y GPIO19 apagados");
  Serial.println("[BLE] Comandos LED_ON, LED_OFF, LED_STATUS, LED2_ON, LED2_OFF, LED2_STATUS");
}

void loop() {
  CommandMessage message;
  if (xQueueReceive(commands, &message, pdMS_TO_TICKS(20)) == pdTRUE) {
    executeCommand(message.text);
  }

  // Monitor serie sin enviar telemetria espontanea a BLE.
  if (sensorType != SENSOR_NONE &&
      millis() - lastMeasurement >= 2000) {
    lastMeasurement = millis();
    Serial.print("[TEMP] ");
    Serial.print(readTemperature(), 1);
    Serial.println(" C");
    Serial.print("[PRESION] ");
    Serial.print(readPressure(), 1);
    Serial.println(" hPa");
    if (sensorType == SENSOR_BME280) {
      Serial.print("[HUMEDAD] ");
      Serial.print(readHumidity(), 1);
      Serial.println(" %");
    }
  }

  if (!deviceConnected && previousConnection) {
    setLed(false);
    setLed2(false);
    virtualOn = false;
    xQueueReset(commands);
    statusCharacteristic->setValue("NEXUS-X LISTO");
    delay(300);
    bleServer->startAdvertising();
    Serial.println("[BLE] Esperando reconexion");
    previousConnection = false;
  }
  if (deviceConnected && !previousConnection) {
    previousConnection = true;
  }
}
