from matplotlib import pyplot as plt
import numpy as np

time, data = np.loadtxt("BumpTest_2019-12-19T15:18:43_998219.csv", skiprows=6, unpack=True, delimiter=',')

plt.plot(time, data)
plt.show()
