# Migración XRA1405 → MCP23S17 / MCP23S18 (expansores GPIO, 8 por PCB)

Chip original: MaxLinear (Exar) **XRA1405IL24-F**, 24-QFN

Según los archivos de fabricación de **P331_Rear_IO vx3** (BOM y CPL), el chip que va en la placa es el **MCP23S17T-E/ML** (28-QFN 6×6 mm), designadores Q14, Q16, Q18, Q23, Q25, Q27, Q35 y Q36. El reemplazo que se había mencionado antes era el **MCP23S18-E/MJ** (24-QFN). El firmware soporta los dos.

## Selección del chip

En `P331_PIC18-K42_Unified.X/Hardware.h`:

```c
#define GPIO_EXPANDER      GPIO_EXP_MCP23S17   // P331_Rear_IO vx3 (por defecto)
//#define GPIO_EXPANDER    GPIO_EXP_MCP23S18
//#define GPIO_EXPANDER    GPIO_EXP_XRA1405    // placas con el chip original
```

- MCP23S17 y MCP23S18 usan el mismo código: tienen el mismo mapa de registros y la misma trama SPI.
- Con `GPIO_EXP_XRA1405`, el firmware envía por SPI exactamente los mismos bytes que la versión anterior (lo verifiqué comparando la secuencia completa de init, lectura y escritura).

## Cambios en el firmware

| | XRA1405 | MCP23S17 / MCP23S18 |
|---|---|---|
| Trama SPI | 2 bytes: `(reg<<1) \| R/W` + dato | 3 bytes: `0x40`/`0x41` + dirección + dato |
| Leer pines | GSR1/GSR2 (0x00/0x01) | GPIOA/GPIOB (0x12/0x13) |
| Salidas | OCR1/OCR2 (0x02/0x03) | OLATA/OLATB (0x14/0x15) |
| Dirección (1 = entrada) | GCR1/GCR2 (0x06/0x07) | IODIRA/IODIRB (0x00/0x01) |
| Pull-ups | PUR1/PUR2 (0x08/0x09) | GPPUA/GPPUB (0x0C/0x0D) |
| Configuración | — | IOCON = `0x24` (BANK=0, SEQOP=1, HAEN=0, ODR=1), escrito en los 8 chips al inicio |

- Archivos tocados: `Hardware.h`, `GPIO.h`, `GPIO.c`, `SPI.c`, `SPI.h`, `Controls_Outputs.c`.
- La aplicación usa nombres que no dependen del chip (`GPX_STATE1/2`, `GPX_OUTPUT1/2`, `GPX_DIR1/2`, `GPX_PULLUP1/2`). Las funciones SPI se renombraron a `writeSPI_GPIO_EXP()` / `readSPI_GPIO_EXP()`.
- Con HAEN=0, el MCP23S17 ignora los pines de dirección A0–A2. Cada chip se selecciona solo con su chip select (igual que con el XRA1405).
- Sin cambios en: chip selects (RA0–RA7, uno por chip), reset (RC0, activo en bajo en todos los chips), modo SPI 0 y velocidad del SPI. El SPI1 corre a unos 9 kHz (MFINTOSC, BAUD=27), muy por debajo de los 10 MHz máximos.

## Lo que se pudo verificar con los archivos de hardware (BOM, CPL, Gerbers)

- **Salidas:** el MCP23S17 tiene salidas push-pull. Por eso no aplica el problema de open-drain que tendría el MCP23S18. Las salidas manejan los ULN2803A (U15–U31, 10 piezas) que activan los relés, y eso funciona directo con push-pull.
- **Resistencias de 100 Ω en serie en el RESET** (R_Reset14…R_Reset36): son compatibles con el pulso de reset del firmware (10 ms en bajo).
- Los Gerbers **no traen información de nets** (no son X2 con atributos de red). Por eso no se pudo confirmar desde ellos el mapeo de pines ni cómo están conectados A0–A2.

## Pendiente de confirmar con el esquemático o netlist (.SchDoc / .PrjPcb, o netlist exportado)

1. **Mapeo de pines:** el firmware asume P0–P7 del XRA1405 → GPA0–GPA7 y P8–P15 → GPB0–GPB7, con el mismo orden de chips. CS_A…CS_D son RA0–RA3 para el canal izquierdo y RA4–RA7 para el derecho.
2. **A0–A2 del MCP23S17 a GND.** Con HAEN=0 no deberían importar, pero conviene revisar la errata del MCP23S17 sobre A2 con HAEN=0.
3. **Microcontrolador:** el BOM y el CPL colocan **PIC18F26K20-E/SO** en U8. El PIC18F26K42 aparece solo en el BOM completo como "U?", sin posición en el CPL. Este firmware es para el **PIC18F26K42**: hay que confirmar cuál PIC se monta.

## Pendiente

- Compilar en MPLAB X / XC8 (no hay compilador XC8 en el entorno donde se hizo el cambio).
- Probar en una placa: knobs y switches (CS_A), relés, display y LEDs.
