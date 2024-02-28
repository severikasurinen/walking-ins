import numpy as np
from matplotlib import pyplot as plt


# IMPORT BACKGROUND IMG
img = plt.imread("GUI Resources/pohjakuva2.PNG")


fig, ax = plt.subplots()
x = np.linspace(0.5, 3.0, num=20)
y = 0.9*x
ax.imshow(img, extent=[0, 4.0, 0, 4.0])

ax.plot(x, y, 'o', color='firebrick')
#ax.plot(x, x,'--',linewidth=5, color='firebrick') #ensimmäinen demo, paksu viiva jne

plt.show()



#x = np.linspace(2.0, 3.0, num=10)
#y = np.sin(x)

#plt.plot(x,y)
#plt.grid()
#plt.show()
