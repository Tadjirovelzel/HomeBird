EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Module:Arduino_Nano_v3.x A?
U 1 1 6264AB53
P 9350 2850
F 0 "A?" H 9350 1761 50  0000 C CNN
F 1 "Arduino_Nano_v3.x" H 9350 1670 50  0000 C CNN
F 2 "Module:Arduino_Nano" H 9350 2850 50  0001 C CIN
F 3 "http://www.mouser.com/pdfdocs/Gravitech_Arduino_Nano3_0.pdf" H 9350 2850 50  0001 C CNN
	1    9350 2850
	1    0    0    -1  
$EndComp
$Comp
L voglehuisje_sensors_and_ESP:ESP32-LilyGo-T_PCIE U?
U 1 1 6264F9C2
P 3300 4050
F 0 "U?" H 3350 5865 50  0000 C CNN
F 1 "ESP32-LilyGo-T_PCIE" H 3350 5774 50  0000 C CNN
F 2 "Components:Tity-LilyGo" H 3250 4150 50  0001 C CNN
F 3 "" H 3250 4150 50  0001 C CNN
	1    3300 4050
	1    0    0    -1  
$EndComp
$Comp
L Connector:SD_Card J?
U 1 1 62653AAF
P 6400 2250
F 0 "J?" H 6400 2915 50  0000 C CNN
F 1 "SD_Card" H 6400 2824 50  0000 C CNN
F 2 "" H 6400 2250 50  0001 C CNN
F 3 "http://portal.fciconnect.com/Comergent//fci/drawing/10067847.pdf" H 6400 2250 50  0001 C CNN
	1    6400 2250
	1    0    0    -1  
$EndComp
$Comp
L Sensor:BME280 U?
U 1 1 62653FC9
P 6300 4500
F 0 "U?" H 5871 4546 50  0000 R CNN
F 1 "BME280" H 5871 4455 50  0000 R CNN
F 2 "Package_LGA:Bosch_LGA-8_2.5x2.5mm_P0.65mm_ClockwisePinNumbering" H 7800 4050 50  0001 C CNN
F 3 "https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME280-DS002.pdf" H 6300 4300 50  0001 C CNN
	1    6300 4500
	1    0    0    -1  
$EndComp
$Comp
L Sensor:BME280 U?
U 1 1 626542C0
P 7900 4300
F 0 "U?" H 7471 4346 50  0000 R CNN
F 1 "BME280" H 7471 4255 50  0000 R CNN
F 2 "Package_LGA:Bosch_LGA-8_2.5x2.5mm_P0.65mm_ClockwisePinNumbering" H 9400 3850 50  0001 C CNN
F 3 "https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME280-DS002.pdf" H 7900 4100 50  0001 C CNN
	1    7900 4300
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x08_Male J5
U 1 1 62654811
P 4150 1350
F 0 "J5" H 4258 1831 50  0000 C CNN
F 1 "PMS5003" H 4258 1740 50  0000 C CNN
F 2 "" H 4150 1350 50  0001 C CNN
F 3 "~" H 4150 1350 50  0001 C CNN
	1    4150 1350
	1    0    0    -1  
$EndComp
$EndSCHEMATC
