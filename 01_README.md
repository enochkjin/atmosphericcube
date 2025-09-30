# atmosphericcube
Weather station with reactive LEDs powered by ESP32 microcontrollers. This is a personal project of mine with the intentions of exploring communication protocols and IoT. The weather station as a whole is made up of a transmitter MCU and a receiver MCU that communicate through SPI protocol. The LEDs are a visually stimulating way of displaying atmospheric data, which allows for a fun and engaging way of visualizing weather data in real time. A PCB was designed for the receiver to allow for an independent battery connection rather than being plugged into a USB source.

## Tech Stack
- C/C++
- Arduino
- ESP32C3-Fx4 Mini Core by WeAct Studio
- BME680
- WS2812B LED Ring

## Features
- SPI communication; ranged deployment
- MCUs are able to be battery powered
- Custom receiver PCB
- Reactive LEDs based off of incoming data received (temperature humidity, pressure, air quality)
