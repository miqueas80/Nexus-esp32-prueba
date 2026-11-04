# NEXUS-X Hardware Bridge v5 — 2 LEDs independientes

**Panel:** https://miqueas80.github.io/Nexus-esp32-prueba/

**Firmware v5:** [Abrir sketch NEXUS_ESP32_v5_Dos_LED.ino](firmware/NEXUS_ESP32_v5_Dos_LED/NEXUS_ESP32_v5_Dos_LED.ino)

El panel permite encender o apagar cada LED por separado. Se conserva la lectura BME280 (temperatura, presión, humedad), la conexión BLE y la demostración.

| Elemento | GPIO | Comandos |
| --- | --- | --- |
| LED 1 | 18 | `LED_ON`, `LED_OFF`, `LED_STATUS` |
| LED 2 | 19 | `LED2_ON`, `LED2_OFF`, `LED2_STATUS` |

Cada LED tiene **su propia resistencia de 330 ohm en serie** y el cátodo (-) a GND común. No conectes LEDs sin resistencia. El firmware deja ambos apagados al inicio y ante desconexión BLE.

## Cómo probar

1. **Desconectá el USB.** Verificá el GND del segundo LED: patita corta del LED 2 en la columna 34 (grupo de `c34`) y jumper `c34` → `i20` si sigue libre y es el grupo de GND compartido (no muevas el BME280). Confirmá que no haya puentes accidentales.
2. En Arduino IDE guardá el sketch v4 anterior y abrí el archivo **v5** desde GitHub, botón **Raw**; copiá el código completo a un sketch nuevo.
3. Con la placa **ESP32 Dev Module**, las bibliotecas de Adafruit BMP280/BME280 instaladas y el puerto USB correcto, presioná **Verificar** y, solo si compila y el circuito está correctamente conectado, **Subir**.
4. Recargá el panel web en **Chrome Android** y conectate a **NEXUS-ESP32**. Probá encender LED 1 y apagar LED 2, y viceversa.
5. El panel deshabilita el control del LED 2 si detecta firmware v4 antiguo. La carga de v5 por USB es obligatoria para usar ambos.

Este piloto controla **solo LEDs de baja tensión**, nunca lámparas de red ni relés directamente. Cambiar la página en GitHub Pages no actualiza automáticamente el firmware del microcontrolador.

---

## Historial — v4, v3 y v2

# NEXUS-X Hardware Bridge v4 — Control del LED y BME280

La web de prueba se actualizó para manejar un LED físico de 3,3 V en GPIO18 y mantener las mediciones ambientales BME280 por Bluetooth.

**Panel web:** https://miqueas80.github.io/Nexus-esp32-prueba/

**Firmware nuevo:** [NEXUS_ESP32_v4_LED.ino](firmware/NEXUS_ESP32_v4_LED/NEXUS_ESP32_v4_LED.ino)

## Instalación

1. En Arduino IDE, guardá una copia del programa v3 que ya funciona.
2. Abrí el firmware v4 desde GitHub, pulsá **Raw** y copiá el código en un sketch nuevo.
3. Conservá la placa **ESP32 Dev Module**, el puerto USB correcto y las bibliotecas Adafruit BMP280 y BME280.
4. Hacé **Verificar (✓)**. Solo si compila correctamente y el LED está conectado con una resistencia de **330 Ω**, hacé **Subir (→)**.
5. Abrí Chrome Android, recargá la web, conectá **NEXUS-ESP32**, tocá **Consultar LED** y probá el encendido/apagado.

## Qué mantiene y agrega

- Mantiene el nombre Bluetooth y los UUID originales, PING, DEMO_ON, DEMO_OFF, STATUS y SENSOR/TEMP/PRESS/HUM.
- Agrega LED_ON, LED_OFF y LED_STATUS para LED de baja tensión en GPIO18.
- El LED inicia apagado y vuelve a apagarse al desconectar BLE.
- Los botones LED solo se habilitan cuando detectan firmware v4; el firmware v3 sigue compatible para las mediciones, pero no puede controlar el LED.
- No controla relés ni tensión de red. La placa debe seguir alimentada por USB o una fuente adecuada.

**Importante:** GitHub Pages actualiza la web, pero **no carga firmware al ESP32**. Hace falta subir v4 desde Arduino IDE.

---

## Historial de la versión v3

# NEXUS-X · Hardware Bridge BLE v3 — panel ambiental

El panel web de [GitHub Pages](https://miqueas80.github.io/Nexus-esp32-prueba/) ahora muestra las lecturas del sensor BMP280/BME280 a través del Bluetooth BLE del ESP32.

## Probar desde el celular

1. Alimentar la ESP32 con el firmware v3 ya cargado.
2. Abrir Chrome para Android en https://miqueas80.github.io/Nexus-esp32-prueba/
3. Recargar la página y tocar **Conectar ESP32**.
4. Elegir **NEXUS-ESP32**. Se consulta el modelo de sensor y sus lecturas.
5. Se actualiza al conectar, cada 15 segundos mientras la página está visible y con **Actualizar mediciones**.

La interfaz reconoce los comandos SENSOR, TEMP, PRESS y HUM, y los mensajes SENSOR:BME280, SENSOR:BMP280, TEMP:valor, PRES:valor y HUM:valor. Humedad solo está disponible con BME280.

Se preservan PING, DEMO_ON, DEMO_OFF, STATUS y los UUID del firmware v2. Si se conecta un firmware v2, funcionan los controles demo, pero la lectura del sensor aparece como no disponible.

**Seguridad:** el sensor usa alimentación 3,3 V e I2C en GPIO21 SDA/GPIO22 SCL. Este piloto no controla relés ni salidas físicas. No conectar módulos MQ de 5 V directamente a GPIO de 3,3 V.

---

## Documentación histórica del piloto BLE v2

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
