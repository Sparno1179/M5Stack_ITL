import numpy as np
import matplotlib.pyplot as plt


def normalize_acc(acc_list):
    array = np.array(acc_list)
    max = np.max(array)
    min = np.min(array)

    acc_amp = max - min
    print(max)
    print(min)
    return array / acc_amp


if __name__ == '__main__':
    print("hello")
    acclist = [1, 3, 20, -2, 31, 34, 9, 2, 1, 0, 0, -5]
    normalize_acc(acclist)

    print(acclist)
    print(normalize_acc(acclist))

    plt.plot(acclist)
    plt.plot(normalize_acc(acclist))

    plt.show()
