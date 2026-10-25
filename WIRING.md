# WIRING  (body ESP32 pin map: `pins.h`)

## Build the BODY PlatformIO project

For the BODY ESP32 (classic ESP32 DevKit):

```sh
pio run
pio run -t upload
pio device monitor
```

The BODY accepts the final HAND controller's packed `V3ControlPacket` (mode
`1` is BODY) on the shared protocol's channel 1. For each valid BODY-mode
packet it sends a sequence-numbered type-7 ACK to the HAND. The existing HAND
firmware does not display or log type-7 ACKs, so use the BODY serial monitor
to verify received controls and ACK radio-delivery results; this project does
not change HAND firmware.

**General rules**
1. **Common ground:** every module, battery and ESP32 shares GND.
2. All ESP32 pins are **3.3 V only**. Never feed 5 V into a GPIO.
3. Motors and servos get their **own power**, not the ESP32's 3.3 V pin.
4. After changing a pin: edit `pins.h` and rebuild with `pio run`.

---
## 1. HAND controller - ESP32-S3
| Part | Part pin | ESP32-S3 GPIO | Note |
|---|---|---|---|
| TFT 2.8" ST7789 | CS / DC / RST | 10 / 9 / 8 | |
| | SCK (CLK) / MOSI (SDA) | 12 / 11 | MISO optional: 13 |
| | VCC, GND, LED/BL | 3V3, GND, 3V3 | backlight straight to 3V3 (`PIN_TFT_BL -1`) |
| Rotary encoder | CLK / DT / SW | 4 / 5 / 6 | module "+" to 3V3, GND to GND |
| Joystick | VRx / VRy | 1 / 2 | **ADC1 pins** (work with Wi-Fi on) |
| | SW | 14 | click = HEAD/BODY mode |
| | +5V pin | **3V3** | power it from 3.3 V so the ADC never sees > 3.3 V |
| Zoom potentiometer | wiper | 7 | outer legs 3V3 and GND |
| Buzzer (passive) | + | 15 | other leg GND (series 100 ohm optional) |
| LED HEAD / BODY / LINK | anode via 330 ohm | 16 / 17 / 18 | cathode to GND |

Avoid on the S3: 0, 3, 45, 46 (boot), 19, 20 (USB), 26-37 (flash/PSRAM), 43, 44 (UART).

## 2. HEAD - ESP32 (A)  (firmware comes in Stage 4)
| Part | Part pin | ESP32 GPIO | Note |
|---|---|---|---|
| Magnetometer GY-271 (QMC5883L) | SDA / SCL | 21 / 22 | **VCC = 3V3** (on 5 V the I2C pull-ups go to 5 V) |
| Servo 1 (pan) / Servo 2 (tilt) | signal | 18 / 19 | servo power from a separate 5 V supply >= 1 A, 470 uF across it, common GND |
| HC-SR04 | TRIG | 23 | |
| | ECHO | 34 | **needs a divider:** ECHO -> 1 kOhm -> GPIO34, and GPIO34 -> 2 kOhm -> GND (or use a 3.3 V sensor) |
| Laser | via transistor | 25 | GPIO -> 1 kOhm -> NPN base (BC547/2N2222), emitter GND, laser between +V and collector |
| Head LED | via transistor | 26 | same circuit as the laser |
| ESP32-CAM power switch | via transistor | 27 | high-side P-MOSFET (e.g. AO3401) is safest; a low-side switch floats the cam ground |
| Buzzer | + | 13 | |
| Status LED | via 330 ohm | 32 | |

## 3. BODY - ESP32 (B) + **L298N motor driver module**  (firmware: this PlatformIO project)
```
        L298N module (red board)                        ESP32 (B)
   +---------------------------------+
   | OUT1 OUT2      OUT3 OUT4        |
   |  [LEFT]         [RIGHT]  motors |
   |                                 |
   | ENA  IN1  IN2  IN3  IN4  ENB    |<----  GPIO25 GPIO26 GPIO27 GPIO32 GPIO18 GPIO33
   |  ^ jumper REMOVED       ^ jumper REMOVED     (ENA  IN1   IN2   IN3   IN4   ENB)
   |                                 |        10k pull-down from ENA and ENB to GND
   | +12V   GND   +5V                |
   +--+------+-----+-----------------+
      |      |     +--- 5 V (module regulator OUT -> ESP32 VIN, if allowed below) 
      |      +--------- GND  ------------- ESP32 GND   (common ground!)
      +---------------- BATTERY +  (7-12 V)
```
| ESP32 GPIO | L298N pin | Function |
|---|---|---|
| 25 | ENA | LEFT motor speed (PWM) |
| 26 | IN1 | LEFT direction 1 |
| 27 | IN2 | LEFT direction 2 |
| 33 | ENB | RIGHT motor speed (PWM) |
| 32 | IN3 | RIGHT direction 1 |
| 18 | IN4 | RIGHT direction 2 |
| 23 | - | status LED (330 ohm to GND) |
| GND | GND | common ground (battery, module, ESP32) |

- **Left motor** on OUT1/OUT2, **right motor** on OUT3/OUT4. Wrong spin direction? Swap that motor's two wires **or** set `MOTOR_L_INVERT` / `MOTOR_R_INVERT` in `body/include/config.h`.
- **REMOVE the ENA and ENB jumpers** (small plastic caps on the pin pairs next to ENA / ENB). If they stay, speed control does not work and the GPIO is tied to +5 V.
- **10 kOhm pull-downs** from ENA and ENB to GND keep the motors off while the ESP32 boots (the module has none once the jumpers are gone).
- **Power:** battery + on the +12V terminal (the module accepts about 7-12 V for this setup), battery - on GND, **same GND to the ESP32**. Put 100-470 uF across +12V/GND near the module.
- **5 V jumper ("5V-EN") and powering the ESP32:** with the jumper ON and a battery of about 7-12 V, the module's +5V terminal can feed the ESP32's VIN/5V pin. Below ~7 V or above 12 V, power the ESP32 separately (USB power bank or a 5 V buck converter) and, above 12 V, remove the jumper.
- **Limits:** 2 A per channel; about 2 V is lost inside the chip (7.4 V pack -> ~5.4 V at the motor). The chip runs hot: keep the heat sink free.

## 4. ESP32-CAM (AI-Thinker)
Fixed board pinout (`cam/include/pins.h`). Flash LED = GPIO4 (shared with the SD card: SD runs in 1-bit mode). Power: **5 V with >= 500 mA** and a 100-470 uF capacitor next to the board (Wi-Fi + camera peaks cause brown-outs). It is powered through the head's transistor switch (section 2).

---
## Checklist before first power-up
- [ ] Common ground everywhere (battery, L298N, ESP32s, servos)
- [ ] HC-SR04 ECHO through the divider
- [ ] Joystick powered from 3.3 V
- [ ] **ENA / ENB jumpers removed** on the L298N, 10 kOhm pull-downs on ENA and ENB, 100-470 uF on the motor supply
- [ ] Motors lifted off the ground for the first test
- [ ] `python3 tools/check_pins.py` shows 0 errors