# Event Countdown with Nixies and RFID

![Project Image](https://www.detourner.fr/objects/cde_001.jpg)

This project is an ESP32-based event countdown system that uses Nixie tubes to display the number of days remaining until an event. Each event is represented by an RFID card (tag) that can be placed on the device to trigger the countdown display.

For more information about this object, visit: [https://www.detourner.fr/objects/01-le-temps-qui-reste/](https://www.detourner.fr/objects/01-le-temps-qui-reste/)


## Features

- **Nixie Display**: Uses Nixie tubes to display the remaining days in a retro and elegant way.
- **RFID Event Management**: Each RFID card represents a unique event with a target date.
- **Alarms**: Support for 3 alarms per event (in days before the event).
- **Web Interface**: Responsive web interface to manage events (add, edit, delete) via a browser.
- **Persistence**: Automatic saving of events and preferences (brightness, timeout) to flash memory.
- **Rotary Control**: Uses a rotary encoder to adjust brightness and display timeout.

## Hardware Requirements

- ESP32 (or compatible)
- Nixie tubes (with appropriate driver)
- MFRC522 RFID reader
- Rotary encoder
- Appropriate power supply

## Software

- **Framework**: PlatformIO with Arduino framework
- **Libraries**:
  - ArduinoJson for JSON serialization
  - AsyncWebServer for web server
  - Preferences for flash persistence
  - Other standard ESP32 dependencies

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/detourner/count_down_event.git
   cd count_down_event
   ```

2. Open with PlatformIO:
   ```bash
   platformio run --target upload
   platformio run --target uploadfs
   ```

3. Access the web interface via the ESP32's IP address or cde.local.

## Usage

1. **Add an Event**:
   - Place a new RFID card on the reader.
   - Use the web interface to set the title, date, and alarms.

2. **Display**:
   - Place the RFID card on the reader to display the countdown.
   - The Nixies show the number of days remaining.

3. **Management**:
   - Access the web interface to edit or delete events.
   - Use the rotary encoder to adjust brightness and timeout.

## Web API

- `GET /api/events`: Retrieves the list of events in JSON format.
- `GET /api/event/update?tagId=X&title=Y&day=D&month=M&year=Y&alarm0=Z&...`: Updates an event.
- `GET /api/event/delete?tagId=X`: Deletes an event.

## Project Structure

- `src/`: Main source code
- `data/`: Web files (HTML, CSS, JS)
- `lib/`: Local libraries

## License

See the LICENSE file for details.