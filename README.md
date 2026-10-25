# DOOM BODY Firmware

PlatformIO firmware for the BODY ESP32 receiver and two-motor drive. It listens
for joystick packets broadcast by the existing DOOM HAND controller, applies
the configured steering mix, and stops the motors if control packets time out.

## Hardware

- Board: classic ESP32 DevKit (`esp32dev`)
- Motor driver: L298N
- Left motor pins: PWM/ENA GPIO 25, IN1 GPIO 26, IN2 GPIO 27
- Right motor pins: PWM/ENB GPIO 33, IN3 GPIO 32, IN4 GPIO 18
- Optional status LED: GPIO 23
- ESP-NOW channel: 1

Follow [WIRING.md](./WIRING.md) before powering the motor driver. In particular,
remove the L298N ENA/ENB jumpers when using PWM and use a shared ground.

## Build, upload, monitor

Install PlatformIO Core, open this project directory in a terminal, then run:

```sh
pio run
pio run -t upload
pio device monitor
```

The serial monitor uses 115200 baud. The built binary is
`.pio/build/esp32dev/firmware.bin`.

## HAND link and acknowledgement

The receiver accepts the final HAND firmware's packed `V3ControlPacket`
(magic `0xB808`, type `3`, 11 bytes). This layout matches `shared/protocol.h`
in the local controller project at
`~/Documents/PlatformIO/Projects/doom-world`. The packet carries mode
(`0` = HEAD, `1` = BODY), normalized joystick axes (`-100..100`), zoom, and a
32-bit sequence number. The BODY only drives when mode is BODY. The BODY radio
channel is taken directly from `WIFI_CHANNEL` in the shared protocol.

For a valid BODY-mode packet, the BODY sends a unicast type-7 `BodyAckPacket`
back to the sender, echoing the control sequence and joystick values. The ESP-NOW
send callback reports whether the radio link delivered this unicast frame. The
final HAND firmware does not parse or display type 7, so this is BODY-side
diagnostic feedback, not a visible HAND-screen acknowledgement.

On startup, the BODY prints its MAC address. `CONTROLLER_MAC` in [config.h](./config.h)
defaults to all zeros, so control packets are accepted from any sender. Set it
to the HAND's station MAC to restrict control and ACKs to that controller. HAND,
HEAD, and BODY must use the same ESP-NOW channel, defined in
`shared/protocol.h` (currently channel 1).

## Tuning

Edit [config.h](./config.h) to select pivot or tank steering, invert motor
directions, tune PWM limits, acceleration, and the packet failsafe. Edit
[pins.h](./pins.h) if the motor driver wiring differs. The L298N notes and
complete connection tables are in [WIRING.md](./WIRING.md).

See [bodyexplain.md](./bodyexplain.md) for the packet flow, drive behavior,
failsafe, and ACK details.
