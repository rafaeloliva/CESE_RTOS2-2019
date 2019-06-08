import serial
ser = serial.Serial('/dev/ttyUSB1')
print(ser.name)
ser.close()