import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import glob
import random
import shutil

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

movement_num = int(input("movement type? -> "))

movement_type = movement_list[movement_num]

csv_list = glob.glob("acc/{}/*.csv".format(movement_type))

for file in csv_list:
    csv = pd.read_csv(file, names=('accX', 'accY', 'accZ', 'gyroX', 'gyroY', 'gyroZ'), dtype='int16')

    accX.extend(csv.accX.values.tolist())
    accY.extend(csv.accY.values.tolist())
    accZ.extend(csv.accZ.values.tolist())
    gyroX.extend(csv.gyroX.values.tolist())
    gyroY.extend(csv.gyroY.values.tolist())
    gyroZ.extend(csv.gyroZ.values.tolist())

x = np.linspace(0, len(accX)-1, len(accX))

for i in range(0, len(accX), 3762):
    xx = x[i:i+3762]
    plt.plot(xx, accX[i:i + 3762])
    plt.plot(xx, accY[i:i + 3762])
    plt.plot(xx, accZ[i:i + 3762])
    plt.plot(xx, gyroX[i:i + 3762])
    plt.plot(xx, gyroY[i:i + 3762])
    plt.plot(xx, gyroZ[i:i + 3762])
    plt.title("{} / {}".format(int(i/3762), int(len(accX)/3762)))
    mng = plt.get_current_fig_manager()
    mng.window.showMaximized()
    plt.show()

print("切りたい範囲をインデックス(x座標)で指定")
print("加速度サイズ: {}".format(len(accX)))

while True:
    print("キャンセルするには-1を入力")
    start = int(input("start ->"))

    if start == -1:
        break

    end = int(input("end ->"))

    if start > end:
        print("\nError: 開始座標より終了座標のほうが小さいです。再入力してください\n")
        continue
    if start > len(accX) or end > len(accX):
        print("\nError: 入力座標が加速度のサイズを超えています。再入力してください。\n")

    for i in range(end - start):
        accX[start + i] = -5000
        accY[start + i] = -5000
        accZ[start + i] = -5000
        gyroX[start + i] = -5000
        gyroY[start + i] = -5000
        gyroZ[start + i] = -5000

plt.plot(accX)
plt.plot(accY)
plt.plot(accZ)
plt.plot(gyroX)
plt.plot(gyroY)
plt.plot(gyroZ)
plt.show()

# 加速度・ジャイロをndarrayに変換
accX = np.array(accX)
accY = np.array(accY)
accZ = np.array(accZ)
gyroX = np.array(gyroX)
gyroY = np.array(gyroY)
gyroZ = np.array(gyroZ)

# 先程指定した部分を切る
index = [i for i in range(len(accX)) if accX[i] == -5000]
print(index)
accX = np.delete(accX, index)
accY = np.delete(accY, index)
accZ = np.delete(accZ, index)
gyroX = np.delete(gyroX, index)
gyroY = np.delete(gyroY, index)
gyroZ = np.delete(gyroZ, index)

acc_gyro_df = pd.DataFrame(columns=['accX', 'accY', 'accZ', 'gyroX', 'gyroY', 'gyroZ'])
acc_gyro_df['accX'] = accX
acc_gyro_df['accY'] = accY
acc_gyro_df['accZ'] = accZ
acc_gyro_df['gyroX'] = gyroX
acc_gyro_df['gyroY'] = gyroY
acc_gyro_df['gyroZ'] = gyroZ
print(acc_gyro_df)

plt.plot(accX)
plt.plot(accY)
plt.plot(accZ)
plt.plot(gyroX)
plt.plot(gyroY)
plt.plot(gyroZ)
plt.show()

print("\n\n本当に変更を適用しますか？")
save_flag = input("適用するなら\"yes\"と入力 -> ")

if save_flag == "yes":

    # csvファイルに書き出し
    acc_gyro_df.to_csv("acc/new_acc/{}/{}{}.csv".format(movement_type, movement_type, random.randint(1, 90000)))
    print("csv書き出し完了")

    # 使用済みcsvファイルを移動
    for file in csv_list:
        shutil.move(file, "acc/trimmed_acc/{}".format(movement_type))
    print("csv移動完了")

else:
    print("\n\nキャンセルされました。変更はすべて破棄されました")
