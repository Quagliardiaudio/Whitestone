# Migración XRA1405 → MCP23S18 (expansores GPIO, 8 por PCB)

Chip original: MaxLinear (Exar) **XRA1405IL24-F**, 24-QFN
Reemplazo: Microchip **MCP23S18-E/MJ**, 24-QFN

## Selección del chip

En `P331_PIC18-K42_Unified.X/Hardware.h`:

```c
#define GPIO_EXPANDER      GPIO_EXP_MCP23S18   // placas nuevas
//#define GPIO_EXPANDER    GPIO_EXP_XRA1405    // placas con el chip original
```

Con `GPIO_EXP_XRA1405`, el firmware envía por SPI exactamente los mismos bytes que la versión anterior (lo verifiqué comparando la secuencia completa de init, lectura y escritura).

## Cambios en el firmware

| | XRA1405 | MCP23S18 |
|---|---|---|
| Trama SPI | 2 bytes: `(reg<<1) \| R/W` + dato | 3 bytes: `0x40`/`0x41` + dirección + dato |
| Leer pines | GSR1/GSR2 (0x00/0x01) | GPIOA/GPIOB (0x12/0x13) |
| Salidas | OCR1/OCR2 (0x02/0x03) | OLATA/OLATB (0x14/0x15) |
| Dirección (1 = entrada) | GCR1/GCR2 (0x06/0x07) | IODIRA/IODIRB (0x00/0x01) |
| Pull-ups | PUR1/PUR2 (0x08/0x09) | GPPUA/GPPUB (0x0C/0x0D) |
| Configuración | — | IOCON = `0x24` (BANK=0, SEQOP=1, ODR=1), escrito en los 8 chips al inicio |

- Archivos tocados: `Hardware.h`, `GPIO.h`, `GPIO.c`, `SPI.c`, `SPI.h`, `Controls_Outputs.c`.
- La aplicación usa nombres que no dependen del chip (`GPX_STATE1/2`, `GPX_OUTPUT1/2`, `GPX_DIR1/2`, `GPX_PULLUP1/2`). Las funciones SPI se renombraron a `writeSPI_GPIO_EXP()` / `readSPI_GPIO_EXP()`.
- Sin cambios en: chip selects (RA0–RA7, uno por chip), reset (RC0, activo en bajo en ambos chips), modo SPI 0 y velocidad del SPI. El SPI1 corre a unos 9 kHz (MFINTOSC, BAUD=27), muy por debajo de los 10 MHz máximos del MCP23S18.

## Revisar en hardware antes de producir

1. **Las salidas del MCP23S18 son open-drain.** Solo pueden llevar el pin a GND; un "1" deja el pin flotando. Todas las salidas activas en alto de CS_B, CS_C y CS_D (relés del OUTLEVEL, XFORMER, etc.) necesitan pull-ups externas o drivers que funcionen así. Los segmentos del display (CS_D P8–P15, activos en bajo) funcionan directo.
2. **Mapeo de pines:** el firmware asume P0–P7 → GPA0–GPA7 y P8–P15 → GPB0–GPB7. Hay que confirmarlo con el esquemático del PCB nuevo.
3. **Footprint:** los dos chips vienen en 24-QFN, pero el pinout es distinto, así que hace falta el PCB rediseñado.
4. **Valores de reset:** el XRA1405 arranca con OCR = 0xFF y el MCP23S18 con OLAT = 0x00. El firmware escribe las salidas antes de configurarlas como salidas, así que esto no cambia el comportamiento.
5. El MCP23S18 no tiene el filtro de entrada del XRA1405 (IFR). El debounce por software ya cubre esto.

## Pendiente

- Compilar en MPLAB X / XC8 (no hay compilador XC8 en el entorno donde se hizo el cambio).
- Probar en una placa con MCP23S18: knobs y switches (CS_A), relés, display y LEDs.
