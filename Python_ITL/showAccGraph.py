import pandas as pd
import os
import matplotlib.pyplot as plot
import numpy as np
import glob
import accelerationfunc as accfun

accX = []
accY = []
accZ = []
gyroX = []
gyroY = []
gyroZ = []

movement_list = ("walk", "stand", "sit", "stdown", "stup", "lying", "vehicle")

print("0: walk\n"
      "1: stand\n"
      "2: sit\n"
      "3: stair_down\n"
      "4: stair_up\n"
      "5: lying\n"
      "6: vehicle\n")

movement_id = int(input("movementID -> "))

print("file type?\n"
      "0: num only (NUM.csv)\n"
      "1: new format only (MAC_RAND_NUM.csv)\n"
      "2: both")

file_type_ID = int(input("file type -> "))

print("show gyro?\n"
      "0: no\n"
      "1: yes")
gyroflag = int(input("gyro? -> "))

if file_type_ID == 0:
    file_type = "*[0-9]"
elif file_type_ID == 1:
    file_type = "*_*_*"
else:
    file_type = "*"

file_list = glob.glob("acc/{}/{}.csv".format(movement_list[movement_id], file_type))

for file in file_list:
    csvFile = pd.read_csv(file, names=('accX', 'accY', 'accZ', 'gyroX', 'gyroY', 'gyroZ'))

    accX.extend(csvFile.accX.values.tolist())
    accY.extend(csvFile.accY.values.tolist())
    accZ.extend(csvFile.accZ.values.tolist())
    gyroX.extend(csvFile.gyroX.values.tolist())
    gyroY.extend(csvFile.gyroY.values.tolist())
    gyroZ.extend(csvFile.gyroZ.values.tolist())
    print("extended {}".format(file))

print(accX.__len__()/len(file_list)/30)

plot.title(movement_list[movement_id])
plot.plot(accX)
plot.plot(accY)
plot.plot(accZ)
if gyroflag:
    plot.plot(gyroX)
    plot.plot(gyroY)
    plot.plot(gyroZ)
plot.show()

