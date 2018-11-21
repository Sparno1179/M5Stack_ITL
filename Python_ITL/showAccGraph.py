import pandas as pd
import os
import matplotlib.pyplot as plot
import numpy as np
import glob

accX = []
accY = []
accZ = []
gyroX = []
gyroY = []
gyroZ = []

movement = "stdown"

file_list = glob.glob("acc/{}/*_*_*.csv".format(movement))

for file in file_list:
    csvFile = pd.read_csv(file, names=('accX', 'accY', 'accZ', 'gyroX', 'gyroY', 'gyroZ'))

    accX.extend(csvFile.accX.values.tolist())
    accY.extend(csvFile.accY.values.tolist())
    accZ.extend(csvFile.accZ.values.tolist())
    gyroX.extend(csvFile.gyroX.values.tolist())
    gyroY.extend(csvFile.gyroY.values.tolist())
    gyroZ.extend(csvFile.gyroZ.values.tolist())
    print("extended {}.csv".format(file_list.index(file)))

    # print("accX = " + "".join(str(a) for a in accX))
    # print("accY = " + "".join(str(b) for b in accY))
    # print("accZ = " + "".join(str(c) for c in accZ))

print(accX.__len__()/len(file_list)/30)
plot.title(movement)
plot.plot(accX)
plot.plot(accY)
plot.plot(accZ)
plot.plot(gyroX)
plot.plot(gyroY)
plot.plot(gyroZ)
plot.show()

