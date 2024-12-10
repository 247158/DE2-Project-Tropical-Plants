# DE2-Project-Tropical-Plants

## Team members

* Ivan Efimov (responsible for the soil moisture sensor and the air temperature and humidity sensor (DHT12))
* Ivan Pavlov (responsible for the photoresistor and the air temperature and humidity sensor (DHT12))
* Ruslan Oleinik (responsible for the LED strip and the OLED display)
* Anton Panteleev (responsible for the fan and GitHub repository management)

## Hardware description
### 1. Arduino Uno:
* Central controller for processing data from sensors and managing outputs.

<img src="images/arduino_uno.png" alt="Arduino Uno" width="450" height="500">

### 2. Sensors:
* Soil Moisture Sensor: Measures the moisture level in the soil.

<img src="images/soil_sensor.jpg" alt="Soil Moisture Sensor" width="450" height="500">
  
* DHT11 Sensor: Measures air temperature and humidity.

<img src="images/DHT12-Sensor.jpg" alt="DHT11 Sensor" width="450" height="500">
  
* Photoresistor: Measures ambient light levels.

<img src="images/photoresistor.jpg" alt="Photoresistor" width="450" height="500">

### 3. Output Devices:
* OLED Display (I2C interface): Displays real-time sensor readings (e.g., soil moisture, air temperature, humidity, and light levels).

<img src="images/oled-i2c.jpg" alt="OLED Display" width="450" height="500">
  
* Fan: Operates continuously to simulate airflow.

<img src="images/fan.jpg" alt="Fan" width="450" height="500">

* LED Strip: Illuminates constantly for lighting purposes.

## Software description

```c
   de2_project         // PlatfomIO project
   ├── include         // Included file(s)
   │   └── timer.h
   ├── lib             // Libraries
   │   ├── twi         // Tomas Fryza's TWI/I2C library
   │   │   ├── twi.c
   │   │   └── twi.h
   │   └── oled        // Sylaina's OLED library
   │       ├── font.h
   │       ├── oled.c
   │       └── oled.h
   ├── src             // Source file(s)
   │   └── main.c
   ├── test            // No need this
   └── platformio.ini  // Project Configuration File
```

## Instructions and photos

<img src="images/photo.jpg" alt="photo">

This project enables the monitoring and adjustment of critical environmental parameters. Using our system, you can track real-time data on air humidity and temperature, soil moisture, and light levels (in percentages) displayed on an OLED screen. It provides a convenient way to manage and control these values to maintain optimal conditions for your environment.

## References and tools
[AVR course from GitHub of Tomas Fryza](https://github.com/tomas-fryza/avr-course)
