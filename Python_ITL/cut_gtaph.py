import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

csvFile = pd.read_csv("acc/vehicle/807D3AC4635C_b4cPGWIKly_0.csv", names=('accX', 'accY', 'accZ', 'gyroX', 'gyroY', 'gyroZ'))


accX = csvFile.accX.values.tolist()
accY = csvFile.accY.values.tolist()
accZ = csvFile.accZ.values.tolist()

newAccX = []
newAccY = []
newAccZ = []

newAccX.extend(accX[260:410])
newAccY.extend(accY[260:410])
newAccZ.extend(accZ[260:410])

newAccX.extend(accX[1063:1213])
newAccY.extend(accY[1063:1213])
newAccZ.extend(accZ[1063:1213])

newAccX.extend(accX[1677:1877])
newAccY.extend(accY[1677:1877])
newAccZ.extend(accZ[1677:1877])

x = []
for i in range(len(newAccX)):
    x.append(i)

plt.title("Acceleration", fontsize=30)

plt.plot([150, 150],[1280, -2000], "red", linestyle='dashed')
plt.plot([300, 300],[1280, -2000], "red", linestyle='dashed')
plt.plot(x, newAccX, label="X")
plt.plot(x, newAccY, label="Y")
plt.plot(x, newAccZ, label="Z")
plt.legend(fontsize=18)
plt.show()