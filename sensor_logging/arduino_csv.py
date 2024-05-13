import serial
import csv

com = "COM3"
baud = 115200

x = serial.Serial(com, baud, timeout=0.1)

with open('datastream.csv', mode='w', newline='') as sensor_file:
    sensor_writer = csv.writer(sensor_file, delimiter=',')
    sensor_writer.writerow(["time", "acc_x", "acc_y", "acc_z", "gyro_x", "gyro_y", "gyro_z"])

    while x.isOpen() is True:
        data = str(x.readline().decode('utf-8')).rstrip()
        if data is not '':
            data = data.split(',')
            print(data)
            try:
                data = (int(data[0]), float(data[1]), float(data[2]), float(data[3]), float(data[4]), float(data[5]),
                        float(data[6]))
                sensor_writer.writerow(data)
            except Exception as e:
                print(e)
