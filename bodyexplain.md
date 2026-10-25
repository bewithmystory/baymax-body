# BODY Firmware: What It Does

## Role in the system

The HAND samples the joystick and broadcasts a control packet over ESP-NOW.
HEAD and BODY share the same radio channel and receive the broadcast. HEAD uses
mode `0` for its controls; BODY uses mode `1` for the drive motors. This project
implements the BODY receiver and motor controller only. It does not change the
existing HAND or HEAD firmware.

## Radio receive path

The BODY uses the packed V3 control packet contract from the final HAND
project at `~/Documents/PlatformIO/Projects/doom-world` and the matching
`shared/protocol.h` in the HEAD project:

1. The radio callback copies the sender MAC and packet bytes into a small
   receive ring. It does not parse packets or control motors in the Wi-Fi task.
2. The main loop validates packet length, magic (`0xB808`), type (`3`), mode,
   axis range, and zoom range.
3. A valid packet refreshes the control timestamp and stores mode, `x`, `y`,
   and sequence. Axes are already calibrated percentages from the HAND; BODY
   does not reinterpret them as ADC readings.
4. Packets from other MAC addresses are ignored when `CONTROLLER_MAC` is set.
   The all-zero default accepts any controller.

HAND transmits control packets about every 50 ms on ESP-NOW channel 1. Keep
HAND, HEAD, and BODY on that same channel.

## Motor control

`DriveController` applies the existing steering functions in `DriveMix.h`:

- In pivot mode, pushing forward/back drives both motors; steering sideways
  runs one motor.
- In tank mode, the joystick axes are mixed into left/right motor speeds.
- Values below the configured input threshold are treated as zero.
- `SlewLimiter` applies the configured acceleration ramp. Direction reversals
  pass through zero.

`MotorDriver` maps signed speed to motor direction pins and PWM. Inversion,
PWM frequency/resolution, minimum/maximum duty, and braking are configured in
`config.h`.

## Failsafe

Each valid control packet records its receive time. If no new packet arrives
within `FAILSAFE_MS`, or if the controller selects HEAD mode, BODY resets the
slew state and commands both motors to stop immediately. This prevents a lost
radio link or mode switch from leaving the motors running.

## Acknowledgement

After accepting a valid packet in BODY mode, BODY sends an 11-byte type-7
`BodyAckPacket` unicast to that packet's sender. It includes the original
control sequence, mode, axes, and accepted status. The BODY registers the
sender as an ESP-NOW peer when needed. The ESP-NOW send callback counts the
link-layer delivery result, which is periodically reported in the BODY serial
monitor alongside the most recent control sequence.

The final HAND receiver does not recognize type 7. It ignores this unknown
packet, so the ACK does not currently appear on the HAND screen or HAND serial
monitor. No HAND firmware was changed. The ACK is a BODY-to-controller radio
response and a BODY-side diagnostic; the ESP-NOW send callback can confirm
unicast link delivery, but only a HAND firmware change could display or verify
the application-level ACK.

## Files

- `src/main.cpp` — wires up components and runs their update loop.
- `src/BodyRadio.cpp` — ESP-NOW setup, control validation, and ACK response.
- `src/DriveController.cpp` — mode handling, failsafe, steering and motor output.
- `include/drivers/MotorDriver.h` — direction and PWM control.
- `include/drivers/StatusLed.h` — optional status LED.
- `DriveMix.h` — steering math and speed ramp.
- `shared/protocol.h` — HAND/HEAD packet definitions plus the BODY ACK extension.
- `config.h`, `pins.h` — tunable behavior and hardware pin assignments.

## Verification

Build the BODY firmware with `pio run`. Then test with the robot's drive wheels
off the ground: check BODY-mode forward/back/turn behavior, switch back to HEAD
mode, and stop sending packets to verify the immediate failsafe. The BODY serial
monitor should show accepted controls and ACK delivery counters. A successful
build validates compilation only; confirm radio delivery and motor direction on
the actual hardware before operating the robot.
