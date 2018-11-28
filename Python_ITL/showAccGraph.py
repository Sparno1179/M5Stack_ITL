import pandas as pd
import matplotlib.pyplot as plot
import glob
import accelerationfunc as accfunc
import re
import pprint


accX = []
accY = []
accZ = []
gyroX = []
gyroY = []
gyroZ = []

movement_list = ("walk", "stand", "sit", "stdown", "stup", "lying", "vehicle", "vehicle_cycling", "test")

print("0: walk\n"
      "1: stand\n"
      "2: sit\n"
      "3: stair_down\n"
      "4: stair_up\n"
      "5: lying\n"
      "6: vehicle\n"
      "7: vehicleCycling\n"
      "8: test")

movement_id = int(input("movementID -> "))
movement_type = movement_list[movement_id]

print("show gyro?\n"
      "0: no\n"
      "1: yes")
gyro_flag = int(input("gyro? -> "))

print("normalize graph?\n"
      "0: no\n"
      "1: yes")
normalize_flag = int(input("normalize? -> "))


file_list = glob.glob("acc/{}/*.csv".format(movement_type))

for file in file_list:
    csvFile = pd.read_csv(file, names=('accX', 'accY', 'accZ', 'gyroX', 'gyroY', 'gyroZ'))

    accX.extend(csvFile.accX.values.tolist())
    accY.extend(csvFile.accY.values.tolist())
    accZ.extend(csvFile.accZ.values.tolist())
    if gyro_flag:
        gyroX.extend(csvFile.gyroX.values.tolist())
        gyroY.extend(csvFile.gyroY.values.tolist())
        gyroZ.extend(csvFile.gyroZ.values.tolist())
    print("extended {}".format(file))

print(accX.__len__()/len(file_list)/30)

plot.title(movement_list[movement_id])
if normalize_flag:
    accX = accfunc.normalize_acc(accX)
    accY = accfunc.normalize_acc(accY)
    accZ = accfunc.normalize_acc(accZ)
plot.plot(accX)
plot.plot(accY)
plot.plot(accZ)
if gyro_flag:
    plot.plot(gyroX)
    plot.plot(gyroY)
    plot.plot(gyroZ)
plot.show()

