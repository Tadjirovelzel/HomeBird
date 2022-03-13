import numpy as np

t_fine = 0

def BME280_decode_calibrate(raw_BME280_calibration):
    # turn chars into shorts
    raw_BME280_calibration = raw_BME280_calibration[:50] + '00' + raw_BME280_calibration[50:]
    raw_BME280_calibration = raw_BME280_calibration[:58] + '00' + raw_BME280_calibration[58:]

    # seperate dig_E4 and dig_E5
    last_part = raw_BME280_calibration[-8:]
    new_last_part = last_part[1] + last_part[3] + '0' + last_part[0] +    last_part[5] + last_part[2] + '0' + last_part[4] + last_part[6:]
    raw_BME280_calibration = raw_BME280_calibration[:-8] + new_last_part

    # turn last char into short
    raw_BME280_calibration = raw_BME280_calibration[:68] + '00' + raw_BME280_calibration[68:]
    
    # put unsigned into its own array
    unsigned_BME280_calibration_indexes = [0,1,2,3, 12,13,14,15, 48,49,50,51, 56,57,58,59]
    unsigned_BME280_calibration = ''.join([raw_BME280_calibration[i] for i in unsigned_BME280_calibration_indexes])
    signed_BME280_calibration = ''.join([raw_BME280_calibration[i] for i in range(0, 72) if not i in unsigned_BME280_calibration_indexes])

    # convert to numbers
    signed_type = np.dtype(np.int16).newbyteorder("<")
    unsigned_type = np.dtype(np.uint16).newbyteorder("<")
    BME280_calibration_values_unsigned = np.frombuffer(bytes.fromhex(unsigned_BME280_calibration), dtype = unsigned_type)
    BME280_calibration_values_signed = np.frombuffer(bytes.fromhex(signed_BME280_calibration), dtype = signed_type)

    # finish turning char to short by doing a leftshift using division, this is done this way to not mess up the sign
    BME280_calibration_values_signed = np.concatenate((BME280_calibration_values_signed[:-1], [BME280_calibration_values_signed[-1]//(2**8)]), dtype = signed_type)

    return [BME280_calibration_values_signed, BME280_calibration_values_unsigned]

def BME280_compensate_temperature(temperature, BME280_calibration_values):
    global t_fine
    dig_T1 = BME280_calibration_values[1][0]
    dig_T2, dig_T3 = BME280_calibration_values[0][0:2]

    # formulas from BME280 datasheet
    var1 = (np.double(temperature)/16384.0 - (np.double(dig_T1))/1024.0) * (np.double(dig_T2))
    var2 = (((np.double(temperature))/131072.0 - (np.double(dig_T1))/8192.0) * (np.double(temperature)/131072.0 - (np.double(dig_T1))/8192.0)) * (np.double(dig_T3))
    t_fine = np.int32(var1 + var2)
    T = (var1 + var2) / 5120.0
    return T

def BME280_compensate_pressure(pressure, BME280_calibration_values):
    dig_P1 = BME280_calibration_values[1][1]
    dig_P2, dig_P3, dig_P4, dig_P5,dig_P6,dig_P7, dig_P8, dig_P9 = BME280_calibration_values[0][2:10]

    # formulas from BME280 datasheet
    var1 = (np.double(t_fine)/2.0) - 64000.0
    var2 = var1 * var1 * (np.double(dig_P6)) / 32768.0
    var2 = var2 + var1 * (np.double(dig_P5)) * 2.0
    var2 = (var2/4.0)+((np.double(dig_P4)) * 65536.0)
    var1 = ((np.double(dig_P3)) * var1 * var1 / 524288.0 + (np.double(dig_P2)) * var1) / 524288.0
    var1 = (1.0 + var1 / 32768.0)*(np.double(dig_P1))
    if (var1 == 0.0):
        return 0 # avoid exception caused by division by zero
    p = 1048576.0 - np.double(pressure)
    p = (p - (var2 / 4096.0)) * 6250.0 / var1
    var1 = (np.double(dig_P9)) * p * p / 2147483648.0
    var2 = p * (np.double(dig_P8)) / 32768.0
    p = p + (var1 + var2 + (np.double(dig_P7))) / 16.0
    return p

def BME280_compensate_humidity(humidity, BME280_calibration_values):
    dig_H1, dig_H3 = BME280_calibration_values[1][2:4]
    dig_H2, dig_H4, dig_H5, dig_H6 = BME280_calibration_values[0][10:15]

    # formulas from BME280 datasheet
    var_H = ((np.double(t_fine)) - 76800.0)
    var_H = (humidity - ((np.double(dig_H4)) * 64.0 + (np.double(dig_H5)) / 16384.0 * var_H)) * ((np.double(dig_H2)) / 65536.0 * (1.0 + (np.double(dig_H6)) / 67108864.0 * var_H * (1.0 + (np.double(dig_H3)) / 67108864.0 * var_H)))
    var_H = var_H * (1.0 - (np.double(dig_H1)) * var_H / 524288.0)
    if (var_H > 100.0):
        var_H = 100.0
    elif (var_H < 0.0):
        var_H = 0.0
    return var_H


##################################
###########    MAIN    ###########
##################################

data = "CC6D726732008F8F3DD6D00B0F252AFFF9FFAC260AD8BD104B7A0100112A031E7F86A04941D05CF300040008000C03EA011E0035000C0000000000000006"
data = data.replace(" ", "")

raw_BME280_calibration = data[0:32*2]
raw_BME280_temperature = data[32*2:(32+3) * 2]
raw_BME280_pressure = data[(32+3) * 2:(32+6) * 2]
raw_BME280_humidity = data[(32+6) * 2:(32+8) * 2]
raw_PMS5003 = data[(32+8) * 2:(32+8) * 2 + 9 * 4]
raw_count = data[(32+8) * 2 + 9 * 4: ]

count = int(raw_count, 16)
print("count:", count)

BME280_calibration_values = BME280_decode_calibrate(raw_BME280_calibration)

type = np.dtype(np.int32).newbyteorder(">")
BME280_temperature = np.frombuffer(bytes.fromhex("000" + raw_BME280_temperature[:-1]), dtype = type)[0]
BME280_pressure = np.frombuffer(bytes.fromhex("000"+ raw_BME280_pressure[:-1]), dtype = type)[0]
type = np.dtype(np.int16).newbyteorder(">")
BME280_humidity = np.frombuffer(bytes.fromhex(raw_BME280_humidity), dtype = type)[0]

temperature = BME280_compensate_temperature(BME280_temperature, BME280_calibration_values)
pressure = BME280_compensate_pressure(BME280_pressure, BME280_calibration_values)
humidity = BME280_compensate_humidity(BME280_humidity, BME280_calibration_values)

type = np.dtype(np.uint16).newbyteorder(">")
pm01, pm25, pm10, n0p3, n0p5, n1p0, n2p5, n5p0, n10p0 = np.frombuffer(bytes.fromhex(raw_PMS5003), dtype = type)

print(f"temperature: {temperature}")
print(f"pressure: {pressure}")
print(f"humidity: {humidity}")
print(f"pm01: {pm01}")
print(f"pm25: {pm25}")
print(f"pm10: {pm10}")
print(f"n0p3: {n0p3}")
print(f"n0p5: {n0p5}")
print(f"n1p0: {n1p0}")
print(f"n2p5: {n2p5}")
print(f"n5p0: {n5p0}")
print(f"n10p0: {n10p0}")