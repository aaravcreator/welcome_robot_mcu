# Welcome Robot - Hand Control & Audio System

A PlatformIO project for ESP32 that controls a humanoid robot's hand movements using servo motors and plays greeting audio using a DF Player Mini module.

## Features

- **Servo Motor Control**: Smooth control of 4 servo motors (shoulders and arms)
- **Audio Playback**: Play pre-recorded greetings via DF Player Mini
- **WiFi Web Server**: Remote control via web browser interface
- **Serial Communication**: Command-based control over serial port
- **Servo Calibration**: Store and recall servo position presets
- **Smooth Movements**: Interpolated servo movements for natural motion
- **Pre-defined Sequences**: Greet raise/lower animation sequences

## Hardware Requirements

### Microcontroller

- **ESP32** (or compatible ESP32 board)

### Servo Motor Driver

- **Adafruit PCA9685 PWM Servo Driver** (I2C interface)
  - Drives up to 16 servo motors
  - I2C Address: 0x40 (default)

### Servo Motors

- **4× Standard Servo Motors** (SG90 or similar)
  - Servo 0: Right Shoulder
  - Servo 1: Right Arm
  - Servo 2: Left Shoulder
  - Servo 3: Left Arm

### Audio Module

- **DFRobotDFPlayerMini** - MP3 audio player module
  - Serial Communication (TX/RX)
  - Connected to ESP32 UART2
  - Requires microSD card with MP3 files

### Power Supply

- 5V power supply for servo motors
- 5V power supply for DF Player Mini
- USB power for ESP32

## Pin Configuration

### I2C Interface (Servo Driver)

- **SDA**: GPIO 21 (I2C Data)
- **SCL**: GPIO 22 (I2C Clock)

### UART2 (DF Player Mini)

- **TX**: GPIO 17 → DF Player RX
- **RX**: GPIO 16 ← DF Player TX

### WiFi

- Built-in ESP32 WiFi (802.11 b/g/n)

## Software Dependencies

Project dependencies are managed in `platformio.ini`:

```ini
lib_deps =
  adafruit/Adafruit PWM Servo Driver Library @ ^3.0.3
  dfrobot/DFRobotDFPlayerMini @ ^1.0.6
```

- **Adafruit PWM Servo Driver Library**: Controls servo motors via I2C
- **DFRobotDFPlayerMini**: Handles audio playback via serial

## Setup Instructions

### 1. Hardware Assembly

1. **Connect Servo Driver to ESP32 (I2C)**
   - Servo Driver SDA → ESP32 GPIO 21
   - Servo Driver SCL → ESP32 GPIO 22
   - Servo Driver GND → ESP32 GND
   - Servo Driver VCC → 5V

2. **Connect Servo Motors to Servo Driver**
   - Connect 4 servos to channels 0-3 on the PCA9685
   - Ensure servos receive 5V power

3. **Connect DF Player Mini to ESP32 (UART2)**
   - DF Player RX → ESP32 GPIO 17 (TX)
   - DF Player TX → ESP32 GPIO 16 (RX)
   - DF Player GND → ESP32 GND
   - DF Player VCC → 5V

4. **Prepare microSD Card**
   - Format microSD card as FAT32
   - Place MP3 audio files in root directory
   - Files are indexed by number (001.mp3, 002.mp3, etc.)
   - Insert into DF Player Mini

### 2. Software Setup

1. **Install PlatformIO**

   ```bash
   pip install platformio
   ```

2. **Clone/Open Project**

   ```bash
   cd welcome_robot
   ```

3. **Update Upload Port** (if needed)
   Edit `platformio.ini` and update the upload port:

   ```ini
   upload_port = /dev/cu.usbserial-XXXX  # macOS
   upload_port = COM3                      # Windows
   upload_port = /dev/ttyUSB0              # Linux
   ```

4. **Build and Upload**
   ```bash
   pio run -t upload
   pio device monitor  # View serial output
   ```

## Usage

### Web Server Control

1. **Connect to WiFi**
   - Network: `WelcomeRobot`
   - Password: `12345678`

2. **Access Web Interface**
   - Open browser and navigate to: `http://192.168.4.1`
   - Adjust servo angles using sliders
   - Control greet animations with buttons

3. **Available Commands**
   - **Greet Raise**: Smoothly raise both arms upward
   - **Greet Lower**: Smoothly lower both arms downward
   - Individual servo angle control via sliders

### Serial Commands

Connect via serial monitor (115200 baud) and send commands to control the robot:

```
// Movement Commands
g                  // greet with audio
l                  // lower with audio
```

### Servo Calibration

Servo angles are stored in ESP32 preferences:

- `rsd` = Right Shoulder Down (default: 0°)
- `rsu` = Right Shoulder Up (default: 66°)
- `rad` = Right Arm Down (default: 0°)
- `rau` = Right Arm Up (default: 75°)
- `lsd` = Left Shoulder Down (default: 171°)
- `lsu` = Left Shoulder Up (default: 128°)
- `lad` = Left Arm Down (default: 0°)
- `lau` = Left Arm Up (default: 90°)

Adjust these values in the `loadConfig()` function to calibrate for your specific servo hardware.

## Audio Setup

### Audio Files

1. **Format**: MP3 files (320kbps recommended for quality)
2. **Naming Convention**: `001.mp3`, `002.mp3`, etc.
3. **Placement**: Root directory of microSD card
4. **Content Examples**:
   - 001.mp3 - Welcome greeting
   - 002.mp3 - Thank you message
   - 003.mp3 - Background music

### Playing Audio

Via web interface or serial:

```
// Serial: Play track 1
A 1

// Serial: Set volume to 25
V 25
```

## LED & Debugging

- Check serial monitor (115200 baud) for initialization messages
- Verify I2C communication with servo driver
- Confirm DF Player Mini connection and microSD card detection

## Movement Sequences

### Greet Raise

- Smoothly raises both arms upward simultaneously
- 40 interpolation steps for smooth motion
- 10ms delay between steps for fluid movement

### Greet Lower

- Smoothly lowers both arms to rest position
- Mirror animation of Greet Raise

## Troubleshooting

| Issue               | Solution                                                                               |
| ------------------- | -------------------------------------------------------------------------------------- |
| Servos not moving   | Check I2C connection, verify servo driver address (0x40)                               |
| Audio not playing   | Verify microSD card format (FAT32), check UART2 connections, test with serial commands |
| WiFi not connecting | Check SSID/password in code, verify ESP32 WiFi module                                  |
| Servo jitter        | Reduce delay values, check power supply stability                                      |
| Web UI not loading  | Ensure ESP32 is on WiFi, verify IP address (192.168.4.1)                               |

## Project Structure

```
welcome_robot/
├── src/
│   └── main.cpp              # Main application code
├── include/
│   └── (header files)
├── lib/
│   └── (libraries)
├── platformio.ini            # PlatformIO configuration
└── readme.md                 # This file
```

## Future Enhancements

- [ ] Add more servo channels for complex movements
- [ ] Implement gesture recognition
- [ ] Add voice commands support
- [ ] Multiple animation sequences stored in memory
- [ ] SD card management via web interface
- [ ] Mobile app control interface
- [ ] Real-time telemetry monitoring
- [ ] LLM Intergration for reasoning and conversation

## License

Open source project for educational purposes.

## Support

For issues or questions:

1. Check serial monitor output for error messages
2. Verify hardware connections and power supply
3. Consult Adafruit and DFRobot documentation
4. Review servo driver I2C address configuration

---

**Platform**: ESP32 Dev Module
**Framework**: Arduino (PlatformIO)  
**Last Updated**: May 2026
