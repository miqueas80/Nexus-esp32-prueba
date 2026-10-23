/*
  NEXUS-X Hardware Bridge BLE v2 — PRUEBA SEGURA
  ESP32-WROOM-32 / ESP32 Dev Module (BLE compatible)

  Este firmware NO acciona pines ni alimenta sensores.
  DEMO_ON/DEMO_OFF controlan una variable de software.
  Requiere: Arduino-ESP32 (Espressif Systems); BLE incluido en ese paquete.

  Compatible con lector v1: servicio + característica de estado conservados.
  Agrega: characteristic WRITE, NOTIFY y reconexión.
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

static const char *DEVICE_NAME = "NEXUS-ESP32";
static const char *SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
static const char *STATUS_UUID  = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
static const char *COMMAND_UUID = "3a1e2bb8-531e-4d55-bc0c-b83b650e9069";

struct CommandMessage {
  char text[24];
};

BLEServer *bleServer = nullptr;
BLECharacteristic *statusCharacteristic = nullptr;
QueueHandle_t commands = nullptr;
volatile bool deviceConnected = false;
bool previousConnection = false;
bool virtualOn = false;

class ConnectionCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) override {
    deviceConnected = true;
    Serial.println("[BLE] Cliente conectado");
  }

  void onDisconnect(BLEServer *server) override {
    deviceConnected = false;
    Serial.println("[BLE] Cliente desconectado");
  }
};

class CommandCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    String value = characteristic->getValue();
    value.trim();
    value.toUpperCase();

    if (value.length() == 0 || value.length() >= sizeof(CommandMessage::text)) {
      Serial.println("[BLE] Comando vacío o demasiado largo");
      return;
    }

    CommandMessage message = {};
    value.toCharArray(message.text, sizeof(message.text));
    if (xQueueSend(commands, &message, 0) != pdTRUE) {
      Serial.println("[BLE] Cola llena, comando descartado");
    }
  }
};

void publishStatus(const char *value) {
  statusCharacteristic->setValue(String(value));
  Serial.print("[NEXUS] ");
  Serial.println(value);
  if (deviceConnected) {
    statusCharacteristic->notify();
  }
}

void executeCommand(const char *command) {
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
  } else {
    publishStatus("ERR:UNKNOWN_COMMAND");
  }
}

void setup() {
  Serial.begin(115200);
  delay(350);
  Serial.println();
  Serial.println("NEXUS-X HARDWARE BRIDGE BLE v2");
  Serial.println("MODO PRUEBA: sin control de pines");

  commands = xQueueCreate(8, sizeof(CommandMessage));
  if (commands == nullptr) {
    Serial.println("ERROR: no se pudo crear la cola BLE");
    while (true) { delay(1000); }
  }

  BLEDevice::init(DEVICE_NAME);
  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new ConnectionCallbacks());
  BLEService *service = bleServer->createService(SERVICE_UUID);

  statusCharacteristic = service->createCharacteristic(
    STATUS_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  statusCharacteristic->addDescriptor(new BLE2902());
  statusCharacteristic->setValue("NEXUS-X LISTO");

  BLECharacteristic *commandCharacteristic = service->createCharacteristic(
    COMMAND_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  commandCharacteristic->setCallbacks(new CommandCallbacks());

  service->start();
  BLEAdvertising *advertising = bleServer->getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->start();

  Serial.println("[BLE] NEXUS-ESP32 anunciándose");
  Serial.println("[BLE] Servicio v1 + control bidireccional v2");
}

void loop() {
  CommandMessage message;
  if (xQueueReceive(commands, &message, pdMS_TO_TICKS(20)) == pdTRUE) {
    executeCommand(message.text);
  }

  // Reinicio del estado virtual cuando se pierde la conexión.
  // No controla salidas físicas, y así no persisten comandos antiguos.
  if (!deviceConnected && previousConnection) {
    virtualOn = false;
    xQueueReset(commands);
    statusCharacteristic->setValue("NEXUS-X LISTO");
    delay(300);
    bleServer->startAdvertising();
    Serial.println("[BLE] Anuncios reiniciados; listo para reconectar");
    previousConnection = false;
  }
  if (deviceConnected && !previousConnection) {
    previousConnection = true;
  }
}
