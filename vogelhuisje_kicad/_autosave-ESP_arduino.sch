EESchema Schematic File Version 5
EELAYER 36 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 4
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
Comment5 ""
Comment6 ""
Comment7 ""
Comment8 ""
Comment9 ""
$EndDescr
Connection ~ 2100 2700
Connection ~ 2450 2450
Connection ~ 4050 2650
Connection ~ 5250 4650
Connection ~ 5300 4750
Connection ~ 5750 5150
NoConn ~ 6400 4600
NoConn ~ 6400 4700
NoConn ~ 6400 5550
Wire Wire Line
	2000 2850 2450 2850
Wire Wire Line
	2000 3150 3150 3150
Wire Wire Line
	2000 5550 2900 5550
Wire Wire Line
	2100 2700 2000 2700
Wire Wire Line
	2100 2750 2100 2700
Wire Wire Line
	2300 2350 2400 2350
Wire Wire Line
	2400 2350 2400 2700
Wire Wire Line
	2400 2700 2100 2700
Wire Wire Line
	2450 2450 2300 2450
Wire Wire Line
	2450 2850 2450 2450
Wire Wire Line
	2900 4850 3500 4850
Wire Wire Line
	2900 5550 2900 4850
Wire Wire Line
	3150 600  3850 600 
Wire Wire Line
	3150 3150 3150 600 
Wire Wire Line
	3400 4950 3500 4950
Wire Wire Line
	3400 5000 3400 4950
Wire Wire Line
	3850 600  3850 650 
Wire Wire Line
	3950 2650 4050 2650
Wire Wire Line
	4050 2650 4400 2650
Wire Wire Line
	4400 2650 4400 2750
Wire Wire Line
	5000 4650 5250 4650
Wire Wire Line
	5000 4750 5300 4750
Wire Wire Line
	5050 6050 5000 6050
Wire Wire Line
	5050 6100 5050 6050
Wire Wire Line
	5250 4100 6400 4100
Wire Wire Line
	5250 4650 5250 4100
Wire Wire Line
	5250 4650 5250 5050
Wire Wire Line
	5250 5050 6400 5050
Wire Wire Line
	5300 4200 6400 4200
Wire Wire Line
	5300 4750 5300 4200
Wire Wire Line
	5300 4750 5300 5150
Wire Wire Line
	5300 5150 5750 5150
Wire Wire Line
	5750 5150 6400 5150
Wire Wire Line
	5750 5200 5750 5150
Wire Notes Line style solid rgb(194, 14, 12)
	1750 1600 1750 2600
Wire Notes Line style solid rgb(194, 14, 12)
	1750 1600 2100 1600
Wire Notes Line style solid rgb(194, 14, 12)
	1750 2600 2100 2600
Wire Notes Line style solid rgb(194, 14, 12)
	2100 2600 2100 1600
Text Notes 1850 2500 0    65   ~ 0
NO\nNO\nRST\nTXD\nRXD\nSET\nGND\nVCC
Text Label 4450 2050 0    50   ~ 0
SDA
Text Label 4450 2150 0    50   ~ 0
SCL
Text Label 6400 4400 2    50   ~ 0
SCL
Text Label 6400 4500 2    50   ~ 0
SDA
Text Label 6400 5350 2    50   ~ 0
SCL
Text Label 6400 5450 2    50   ~ 0
SDA
Text HLabel 2000 2700 0    50   Input ~ 0
GND
Text HLabel 2000 2850 0    50   Input ~ 0
5V
Text HLabel 2000 3000 0    50   Input ~ 0
Turn_on_boost_converter
Text HLabel 2000 3150 0    50   Input ~ 0
Vbat
Text HLabel 2000 3300 0    50   Input ~ 0
CSI_D0
Text HLabel 2000 3450 0    50   Input ~ 0
CSI_D1
Text HLabel 2000 3600 0    50   Input ~ 0
CSI_D2
Text HLabel 2000 3750 0    50   Input ~ 0
CSI_D3
Text HLabel 2000 3900 0    50   Input ~ 0
CSI_D4
Text HLabel 2000 4050 0    50   Input ~ 0
CSI_D5
Text HLabel 2000 4200 0    50   Input ~ 0
CSI_D6
Text HLabel 2000 4350 0    50   Input ~ 0
CSI_D7
Text HLabel 2000 4500 0    50   Input ~ 0
CSI_MCLK
Text HLabel 2000 4650 0    50   Input ~ 0
CSI_HSYNC
Text HLabel 2000 4800 0    50   Input ~ 0
CSI_VSYNC
Text HLabel 2000 4950 0    50   Input ~ 0
CAM_RST
Text HLabel 2000 5100 0    50   Input ~ 0
TWI_SCK
Text HLabel 2000 5250 0    50   Input ~ 0
TWI_SDA
Text HLabel 2000 5400 0    50   Input ~ 0
V_LED_switch-
Text HLabel 2000 5550 0    50   Input ~ 0
3V3
$Comp
L power:+5V #PWR?
U 1 1 00000000
P 2450 2450
F 0 "#PWR?" H 2450 2300 50  0001 C CNN
F 1 "+5V" H 2450 2650 50  0000 C CNN
F 2 "" H 2450 2450 50  0001 C CNN
F 3 "" H 2450 2450 50  0001 C CNN
	1    2450 2450
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 00000000
P 4150 650
F 0 "#PWR?" H 4150 500 50  0001 C CNN
F 1 "+5V" H 4250 700 50  0000 C CNN
F 2 "" H 4150 650 50  0001 C CNN
F 3 "" H 4150 650 50  0001 C CNN
	1    4150 650 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 626CF2C3
P 2100 2750
F 0 "#PWR?" H 2100 2500 50  0001 C CNN
F 1 "GND" H 2105 2577 50  0000 C CNN
F 2 "" H 2100 2750 50  0001 C CNN
F 3 "" H 2100 2750 50  0001 C CNN
	1    2100 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 626CD9FD
P 3400 5000
F 0 "#PWR?" H 3400 4750 50  0001 C CNN
F 1 "GND" H 3405 4827 50  0000 C CNN
F 2 "" H 3400 5000 50  0001 C CNN
F 3 "" H 3400 5000 50  0001 C CNN
	1    3400 5000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 626D00C2
P 4400 2750
F 0 "#PWR?" H 4400 2500 50  0001 C CNN
F 1 "GND" H 4405 2577 50  0000 C CNN
F 2 "" H 4400 2750 50  0001 C CNN
F 3 "" H 4400 2750 50  0001 C CNN
	1    4400 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 626CD7FE
P 5050 6100
F 0 "#PWR?" H 5050 5850 50  0001 C CNN
F 1 "GND" H 5055 5927 50  0000 C CNN
F 2 "" H 5050 6100 50  0001 C CNN
F 3 "" H 5050 6100 50  0001 C CNN
	1    5050 6100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 626CC30D
P 5750 5200
F 0 "#PWR?" H 5750 4950 50  0001 C CNN
F 1 "GND" H 5755 5027 50  0000 C CNN
F 2 "" H 5750 5200 50  0001 C CNN
F 3 "" H 5750 5200 50  0001 C CNN
	1    5750 5200
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x08_Male J5
U 1 1 62654811
P 2100 2050
F 0 "J5" H 2208 2531 50  0000 C CNN
F 1 "PMS5003" H 2208 2440 50  0000 C CNN
F 2 "Components:PinHeader_1x08_P2.54mm_Vertical_G7" H 2100 2050 50  0001 C CNN
F 3 "~" H 2100 2050 50  0001 C CNN
	1    2100 2050
	1    0    0    -1  
$EndComp
$Comp
L voglehuisje_sensors_and_ESP:BME280Breakout U?
U 1 1 62697B14
P 6850 4450
AR Path="/62697B14" Ref="U?"  Part="1" 
AR Path="/6263B902/62697B14" Ref="U?"  Part="1" 
F 0 "U?" H 7228 4526 50  0000 L CNN
F 1 "BME280Breakout" H 7228 4435 50  0000 L CNN
F 2 "Components:Breakout-6" H 6850 4450 50  0001 C CNN
F 3 "" H 6850 4450 50  0001 C CNN
	1    6850 4450
	1    0    0    -1  
$EndComp
$Comp
L voglehuisje_sensors_and_ESP:BME280Breakout U?
U 1 1 626985F8
P 6850 5400
AR Path="/626985F8" Ref="U?"  Part="1" 
AR Path="/6263B902/626985F8" Ref="U?"  Part="1" 
F 0 "U?" H 7228 5476 50  0000 L CNN
F 1 "BME280Breakout" H 7228 5385 50  0000 L CNN
F 2 "Components:Breakout-6" H 6850 5400 50  0001 C CNN
F 3 "" H 6850 5400 50  0001 C CNN
	1    6850 5400
	1    0    0    -1  
$EndComp
$Comp
L Connector:SD_Card J?
U 1 1 62653AAF
P 7150 2700
F 0 "J?" H 7150 3365 50  0000 C CNN
F 1 "SD_Card" H 7150 3274 50  0000 C CNN
F 2 "" H 7150 2700 50  0001 C CNN
F 3 "http://portal.fciconnect.com/Comergent//fci/drawing/10067847.pdf" H 7150 2700 50  0001 C CNN
	1    7150 2700
	1    0    0    -1  
$EndComp
$Comp
L MCU_Module:Arduino_Nano_v3.x A?
U 1 1 6264AB53
P 3950 1650
F 0 "A?" H 3950 561 50  0000 C CNN
F 1 "Arduino_Nano_v3.x" H 3950 470 50  0000 C CNN
F 2 "Module:Arduino_Nano" H 3950 1650 50  0001 C CIN
F 3 "http://www.mouser.com/pdfdocs/Gravitech_Arduino_Nano3_0.pdf" H 3950 1650 50  0001 C CNN
	1    3950 1650
	1    0    0    -1  
$EndComp
$Comp
L voglehuisje_sensors_and_ESP:ESP32-LilyGo-T_PCIE U?
U 1 1 6264F9C2
P 4200 6000
AR Path="/6264F9C2" Ref="U?"  Part="1" 
AR Path="/6263B902/6264F9C2" Ref="U?"  Part="1" 
F 0 "U?" H 4250 7815 50  0000 C CNN
F 1 "ESP32-LilyGo-T_PCIE" H 4250 7724 50  0000 C CNN
F 2 "Components:Tity-LilyGo" H 4150 6100 50  0001 C CNN
F 3 "" H 4150 6100 50  0001 C CNN
	1    4200 6000
	1    0    0    -1  
$EndComp
$EndSCHEMATC
