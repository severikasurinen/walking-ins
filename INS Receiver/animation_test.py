import tkinter
from matplotlib.backends.backend_tkagg import (
    FigureCanvasTkAgg, NavigationToolbar2Tk)
from matplotlib.backend_bases import key_press_handler
from matplotlib import pyplot as plt, animation
import numpy as np

img = plt.imread("GUI Resources/pohjakuva2.PNG")


plt.rcParams["figure.figsize"] = [7.00, 7.00]
plt.rcParams["figure.autolayout"] = True

root = tkinter.Tk()
root.wm_title("Embedding in Tk")

plt.axes(xlim=(0, 4), ylim=(0, 4))
fig = plt.Figure(dpi=100)
ax = fig.add_subplot(xlim=(0, 4), ylim=(0, 4))
line, = ax.plot([], [],'o',linewidth=5,color='firebrick')
ax.imshow(img, extent=[0, 4.0, 0, 4.0])

canvas = FigureCanvasTkAgg(fig, master=root)
canvas.draw()

toolbar = NavigationToolbar2Tk(canvas, root, pack_toolbar=False)
toolbar.update()

canvas.mpl_connect(
    "key_press_event", lambda event: print(f"you pressed {event.key}"))
canvas.mpl_connect("key_press_event", key_press_handler)

button = tkinter.Button(master=root, text="Quit", command=root.quit)
button.pack(side=tkinter.BOTTOM)

toolbar.pack(side=tkinter.BOTTOM, fill=tkinter.X)
canvas.get_tk_widget().pack(side=tkinter.TOP, fill=tkinter.BOTH, expand=1)

def init():
    line.set_data([], [])
    return line,

def animate(i):

    # nyt pisteet lasketaan i:n funktiona. 
    # mitä pitää tehdä: kysytään pisteitä ble laitteelta. 

    xpoint = 0.1 + 0.01*i 
    ypoint = 0.2 + 0.01*i         
    x = np.linspace(0, 2, 1000)
    y = np.sin(2 * np.pi * (x - 0.01 * i))
    line.set_data(xpoint, ypoint)
    return line,

anim = animation.FuncAnimation(fig, animate, init_func=init, frames=200, interval=500, blit=True)

tkinter.mainloop()