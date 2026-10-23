# NEXUS-X · Hardware Bridge BLE v2 — piloto bidireccional

**Objetivo alcanzable sin cables Dupont:** enviar `PING`, `DEMO_ON`, `DEMO_OFF` y `STATUS` desde Chrome para Android a la ESP32, y recibir confirmaciones reales de vuelta.

> **Importante:** `DEMO_ON` y `DEMO_OFF` solamente cambian una variable **virtual** dentro de la ESP32. Este firmware **no activa pines**, no maneja relés ni lee el BME280/MQ-2. Es una prueba segura del protocolo.

## 1. Cargar el firmware v2 por USB

1. En la computadora abrí Arduino IDE 2.x.
2. Entrá en [firmware/NEXUS_ESP32_Bridge_V2/NEXUS_ESP32_Bridge_V2.ino](firmware/NEXUS_ESP32_Bridge_V2/NEXUS_ESP32_Bridge_V2.ino).
3. Copiá **todo** el código del archivo (botón de copiar contenido, o vista Raw) y reemplazá el código del sketch en Arduino IDE. Alternativamente descargá el archivo y abrilo en Arduino IDE. Si pregunta si querés mover el .ino a una carpeta con su nombre, aceptá.
4. Confirmá la placa **ESP32 Dev Module** y el puerto de la ESP32 (antes aparecía COM6).
5. Asegurate de haber instalado **esp32 by Espressif Systems** desde el Gestor de placas. Este sketch usa las bibliotecas BLE incluidas en ese paquete; no requiere bibliotecas de sensores.
6. Tocá **Verificar** y luego **Subir**.
7. Abrí **Monitor serie** a **115200 baudios**. Si no se imprime el inicio, presioná **EN** una sola vez.

Mensajes esperados:

```text
NEXUS-X HARDWARE BRIDGE BLE v2
MODO PRUEBA: sin control de pines
[BLE] NEXUS-ESP32 anunciándose
[BLE] Servicio v1 + control bidireccional v2
```

## 2. Conectar desde Chrome

1. Dejá la ESP32 alimentada por USB.
2. Desconectá `NEXUS-ESP32` desde **nRF Connect**, si esa app está conectada.
3. En el celular abrí **Chrome**, no el navegador interno de otra app, y entrá a: https://miqueas80.github.io/Nexus-esp32-prueba/
4. Recargá la página para usar la nueva interfaz.
5. Tocá **Conectar ESP32** y elegí **NEXUS-ESP32**.
6. Al detectar firmware v2 se habilitan cuatro comandos:
   - **Enviar PING** → la placa debe contestar `PONG`.
   - **Activar demo** → `ACK:DEMO_ON`.
   - **Desactivar demo** → `ACK:DEMO_OFF`.
   - **Consultar estado** → `STATE:ON` o `STATE:OFF`.
7. También podés observar cada confirmación en el monitor serie de Arduino IDE.

## 3. Cómo saber si sigue instalado el firmware antiguo

Si la web muestra «**Firmware anterior detectado (solo lectura)**» y podés leer `NEXUS-X LISTO` pero no se habilitan comandos, significa que la ESP32 todavía tiene el firmware v1 o que falta la característica BLE de escritura. **Primero subí el sketch v2 en Arduino IDE**; actualizar únicamente la página web no agrega capacidades a la placa.

## 4. UUID BLE

| Característica | UUID | Propiedades |
| --- | --- | --- |
| Servicio | `4fafc201-1fb5-459e-8fcc-c5c9c331914b` | BLE GATT |
| Estado (conserva v1) | `beb5483e-36e1-4688-b7f5-ea07361b26a8` | READ + NOTIFY |
| Órdenes v2 | `3a1e2bb8-531e-4d55-bc0c-b83b650e9069` | WRITE |

## 5. Solución de problemas

- **No aparece el dispositivo:** cerrá la conexión en nRF Connect, activá Bluetooth y pulsá **EN** de la ESP32.
- **Botón Conectar no abre la selección:** usá Chrome para Android desde **HTTPS**, no una copia `file://` del HTML. Revisá permisos de Bluetooth/dispositivos cercanos.
- **Se conecta pero muestra solo lectura:** falta cargar firmware v2.
- **Característica no encontrada:** comprobá que no ejecutás otro sketch y que la carga finalizó sin errores.
- **Caracteres extraños en el monitor serie:** seleccioná 115200 baud.
- **No llega ACK:** revisá «Diagnóstico» en la página y el monitor serie. Desconectá, reiniciá la placa y reconectá.

## 6. Seguridad y próximos pasos

Por ahora **no conectes** el MQ-2 ni relés a la ESP32. Algunos módulos MQ-2 operan a 5 V y su salida analógica **no debe ir directamente** a una entrada ESP32 de 3,3 V. Antes de conectar sensores será necesario comprobar pines, tensión real del BME280, adaptación de niveles y alimentación.

Este piloto no modifica `miqueas80/laboratorio-universal` ni la versión de producción. El enlace BLE es local, pero una web pública con controles **no constituye autenticación del hardware**: no usar este protocolo demostrativo para automatizar equipos peligrosos o críticos.

Referencias oficiales: [Espressif Arduino BLE](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ble.html), [Web Bluetooth](https://developer.mozilla.org/en-US/docs/Web/API/Web_Bluetooth_API).
