The project NestWacht in collaboration with the TU Delft and other benefactors aims to design a sensor station in the form of a birdhouse that uses a camera and weather sensors to document bird activity and monitor properties of the local atmosphere such as humidity, pressure and particulate matter levels. 
This file serves to outline the structure and purpose of the folders contained within the project.

The essential project structure is displayed below:

    -Code
        -arduino_firmware
        -esp_firmware
        -test_camera
        -test_video
        -legacy_code
    -Enclosure
    -Kicad
        -main_board
        -camera_board
        -gerbers

    The project also contains the complete material bill and the login information for the Node Developer platform.
    Some irrelevant and older development code is stores in the legacy_code folder. 



The electronical components were initally designed by Maarten van Schagen and Quinten Luyten. Tadjiro Velzel further implemented the design and attempted to correct inital mistakes.
The programming necessary to ensure correct and smooth operation of the board and the server was done by Lennard Duynkerke
The enclosure of the electronics was designed by Thomas van Leeuwen.
The project was overseen and managed by Marit Bogert and Teun Verkerk.


The main board contains the following components, split into sections.

    Power systems section:
        - Three battery connector footprints that lead to battery holders containing three 16650 LiOn batteries
        - Three TP4056 battery charger modules
        - A Solar charger footprint leading to a solar panel
        - A boost converter that regulates the battery power to 5V
        - A Diode to route solar power directly to the boost converter
        - A Diode to regulate the arduino's ability to turn off the boost converter, thus turning off the Esp32 and particulate matter sensor
        - A power FET which turns off the Esp32 and Particulate matter sensor
        - Resistors to regulate the power throughput towards the FET

    Computative section:
        - An Arduino Every that drives the power towards the ESP32 and operates the sensors and an SD card to store sensor data.
        - A LilyGo ESP32 development board

    Sensoric section:
        - Two BME280 breakout boards to monitor air pressure, humidity and temperature both within the birdhouse and towards the outside air
        - A PMS5003 particulate matter sensor

    Data transfer:
        - A Sim LTE module (LilyGo SIM7600E) for data transfer
        - An LTE antenna to transfer data over LTE
        - A Sim-card to route information accross the internet
        - An SD card that stores the sensor data before sending it

The Camera Board contains the following:

    - An OV2640 camera operating within the infrared spectrum 
    - A 24 pin camera connector, soldered to the board
    - A network of capacitors to smooth the voltages of the camera connector and voltage regulators
    - A 2.8V and 1.2V voltage regulator to supply the camera
    - A system of resistors to preload the camera
    - A motion sensor to detect bird activity and trigger the camera
    - Four infrared LEDs to light up for pictures at night, which the birds cannot sense
    - A larger resistor to regulate current to the infrared LEDs

To contain the electronics mentioned above, a plastic enclosure made, printed in PETG by 3d-printer. The design files for this are available in the enclosure folder.

Problems:
    -Sim module will not send large pictures
    -Camera init and capture is inconsistent, bound to inconsistency of the camera connector
    -Pin assignment in code does not correspond to PCB design files: PCB files are incorrect and can never work: pin assignment in software is correct.
    -Complete integration was never tested
    -Likely complete system is too power intensive to drive by solar panel. (Also never tested fully)


    