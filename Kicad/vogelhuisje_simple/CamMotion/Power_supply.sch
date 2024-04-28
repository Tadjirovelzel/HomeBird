EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 4
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
L TP4056-breakout:TP4056breakout P?
U 1 1 6263DE48
P 2550 2300
F 0 "P?" H 2575 2825 50  0000 C CNN
F 1 "TP4056breakout" H 2575 2734 50  0000 C CNN
F 2 "Components:TP4056breakout" H 2500 2700 50  0001 C CNN
F 3 "" H 2500 2700 50  0001 C CNN
	1    2550 2300
	1    0    0    -1  
$EndComp
$Comp
L TP4056-breakout:GenericBoostConverter P?
U 1 1 6263E192
P 5450 2300
F 0 "P?" H 5450 2715 50  0000 C CNN
F 1 "GenericBoostConverter" H 5450 2624 50  0000 C CNN
F 2 "Components:GenericBoostConverter" H 5450 2450 50  0001 C CNN
F 3 "" H 5450 2450 50  0001 C CNN
	1    5450 2300
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 6263EAD6
P 3600 2250
F 0 "R?" H 3670 2296 50  0000 L CNN
F 1 "100k" H 3670 2205 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 3530 2250 50  0001 C CNN
F 3 "~" H 3600 2250 50  0001 C CNN
	1    3600 2250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 2100 3600 1750
Wire Wire Line
	1950 1750 1950 2050
$Comp
L power:GND #PWR?
U 1 1 6263EF5E
P 1950 2400
F 0 "#PWR?" H 1950 2150 50  0001 C CNN
F 1 "GND" H 1955 2227 50  0000 C CNN
F 2 "" H 1950 2400 50  0001 C CNN
F 3 "" H 1950 2400 50  0001 C CNN
	1    1950 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3350 2300 3200 2300
Wire Wire Line
	3200 2450 3600 2450
Wire Wire Line
	3600 2450 3600 2400
Connection ~ 3600 2450
Wire Wire Line
	3600 1750 3850 1750
$Comp
L Connector_Generic:Conn_01x02 J?
U 1 1 62647C4D
P 3250 1200
F 0 "J?" H 3168 875 50  0000 C CNN
F 1 "Conn_01x02" H 3168 966 50  0000 C CNN
F 2 "Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical" H 3250 1200 50  0001 C CNN
F 3 "~" H 3250 1200 50  0001 C CNN
	1    3250 1200
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3200 2150 3250 2150
Wire Wire Line
	3200 2000 3250 2000
Connection ~ 3250 2000
Wire Wire Line
	3600 1750 1950 1750
Connection ~ 3600 1750
Wire Wire Line
	3250 2000 3250 2150
$Comp
L Connector:USB_C_Receptacle_USB2.0 J?
U 1 1 62654366
P 5100 3300
F 0 "J?" V 5253 4029 50  0000 L CNN
F 1 "USB_C_Receptacle_USB2.0" V 5162 4029 50  0000 L CNN
F 2 "Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12" H 5250 3300 50  0001 C CNN
F 3 "https://www.usb.org/sites/default/files/documents/usb_type-c.zip" H 5250 3300 50  0001 C CNN
	1    5100 3300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4500 2700 4500 2600
Wire Wire Line
	4500 2600 6100 2600
Wire Wire Line
	6100 2600 6100 2150
Wire Wire Line
	6100 2150 6050 2150
Wire Wire Line
	6050 2450 6050 3300
Wire Wire Line
	6050 3300 6000 3300
Wire Wire Line
	6050 3300 6050 3600
Wire Wire Line
	6050 3600 6000 3600
Connection ~ 6050 3300
Text HLabel 6300 2150 2    50   Input ~ 0
5V
Wire Wire Line
	6100 2150 6300 2150
Connection ~ 6100 2150
Wire Wire Line
	3600 2450 4850 2450
Connection ~ 6050 2450
Wire Wire Line
	6050 2450 6200 2450
$Comp
L Connector:Conn_01x02_Female J?
U 1 1 62641E6A
P 1650 2250
F 0 "J?" H 1542 1925 50  0000 C CNN
F 1 "Conn_01x02_Female" H 1542 2016 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" H 1650 2250 50  0001 C CNN
F 3 "~" H 1650 2250 50  0001 C CNN
	1    1650 2250
	-1   0    0    1   
$EndComp
Wire Wire Line
	1850 2250 1950 2250
Wire Wire Line
	1950 2250 1950 2400
Connection ~ 1950 2400
Wire Wire Line
	1850 2150 1950 2150
Wire Wire Line
	1950 2150 1950 2050
Connection ~ 1950 2050
$Comp
L power:GND #PWR?
U 1 1 62647069
P 3600 2450
F 0 "#PWR?" H 3600 2200 50  0001 C CNN
F 1 "GND" H 3605 2277 50  0000 C CNN
F 2 "" H 3600 2450 50  0001 C CNN
F 3 "" H 3600 2450 50  0001 C CNN
	1    3600 2450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 626475C4
P 6200 2450
F 0 "#PWR?" H 6200 2200 50  0001 C CNN
F 1 "GND" H 6205 2277 50  0000 C CNN
F 2 "" H 6200 2450 50  0001 C CNN
F 3 "" H 6200 2450 50  0001 C CNN
	1    6200 2450
	1    0    0    -1  
$EndComp
Connection ~ 6200 2450
Wire Wire Line
	6200 2450 6300 2450
Text HLabel 6300 2450 2    50   Input ~ 0
GND
Wire Wire Line
	3850 2050 3850 2150
$Comp
L Device:Q_PMOS_GSD Q?
U 1 1 6264EDFF
P 4400 1750
F 0 "Q?" H 4605 1796 50  0000 L CNN
F 1 "Q_PMOS_GSD" H 4605 1705 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 4600 1850 50  0001 C CNN
F 3 "~" H 4400 1750 50  0001 C CNN
	1    4400 1750
	1    0    0    -1  
$EndComp
$Comp
L Device:D_Zener D?
U 1 1 62643FC2
P 3850 1900
F 0 "D?" V 3896 1821 50  0000 R CNN
F 1 "D_Zener" V 3805 1821 50  0000 R CNN
F 2 "Diode_SMD:D_SMB_Handsoldering" H 3850 1900 50  0001 C CNN
F 3 "~" H 3850 1900 50  0001 C CNN
	1    3850 1900
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4500 1950 4500 2150
Connection ~ 4500 2150
Wire Wire Line
	4500 2150 4850 2150
Wire Wire Line
	3850 2150 4500 2150
Wire Wire Line
	3250 1400 3250 2000
$Comp
L Device:D D?
U 1 1 626DAFFC
P 4000 1750
F 0 "D?" H 4000 1534 50  0000 C CNN
F 1 "D" H 4000 1625 50  0000 C CNN
F 2 "" H 4000 1750 50  0001 C CNN
F 3 "~" H 4000 1750 50  0001 C CNN
	1    4000 1750
	-1   0    0    1   
$EndComp
$Comp
L Device:D D?
U 1 1 626DB777
P 4200 1250
F 0 "D?" V 4246 1171 50  0000 R CNN
F 1 "D" V 4155 1171 50  0000 R CNN
F 2 "" H 4200 1250 50  0001 C CNN
F 3 "~" H 4200 1250 50  0001 C CNN
	1    4200 1250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4500 1550 3350 1550
Wire Wire Line
	3350 1400 3350 1550
Connection ~ 3350 1550
Wire Wire Line
	3350 1550 3350 2300
Connection ~ 3850 1750
Wire Wire Line
	4150 1750 4200 1750
Wire Wire Line
	4200 1400 4200 1750
Connection ~ 4200 1750
Text HLabel 6100 1100 2    50   Input ~ 0
turn_on_boost_converter
Text HLabel 6100 1550 2    50   Input ~ 0
Vbat
Wire Wire Line
	4500 1550 6100 1550
Connection ~ 4500 1550
Wire Wire Line
	4200 1100 6100 1100
$EndSCHEMATC
