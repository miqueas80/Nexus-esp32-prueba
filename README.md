# NEXUS-X · Piloto web Bluetooth ESP32

Interfaz web de prueba para leer por Bluetooth Low Energy (BLE) el estado `NEXUS-X LISTO` del firmware de ESP32 instalado y verificado con nRF Connect.

## Publicar con GitHub Pages

1. Entrá en **Settings → Pages** de este repositorio.
2. En **Build and deployment**, elegí **Deploy from a branch**.
3. Seleccioná la rama **main**, carpeta **/(root)** y tocá **Save**.
4. Abrí la dirección HTTPS que te muestre GitHub Pages (normalmente https://miqueas80.github.io/Nexus-esp32-prueba/).

## Probar desde Android

1. Mantené la ESP32 conectada por USB y ejecutando el firmware BLE de la prueba.
2. En nRF Connect tocá **Disconnect** para liberar la conexión.
3. Activá Bluetooth y abrí la página de GitHub Pages en **Chrome para Android** (no desde file://).
4. Tocá **Conectar ESP32** y elegí **NEXUS-ESP32**.
5. Debe mostrarse **NEXUS-X LISTO**. Tocá **Leer estado** para repetir la lectura.

## UUID utilizados

- Servicio: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Lectura: `beb5483e-36e1-4688-b7f5-ea07361b26a8`

## Estado y límites

Esta prueba es **solo lectura**; no controla relés ni transmite sensores. La web necesita HTTPS y un navegador compatible con Web Bluetooth. No incorpora credenciales ni servidores, ni modifica la app NEXUS-X principal.

Para ampliar el firmware con BME280 y MQ-2, prepararemos un protocolo BLE de lectura/escritura y conexiones eléctricas seguras. Nunca conectar la salida de 5 V de un MQ-2 directamente a un GPIO de 3,3 V.
