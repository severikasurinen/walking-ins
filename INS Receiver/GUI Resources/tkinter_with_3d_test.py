import tkinter as tk
from tkinter import ttk
import numpy as np
from matplotlib import pyplot as plt
import matplotlib
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg 
from matplotlib.figure import Figure
matplotlib.use("TkAgg")

# GLOBAL
min_x = 0
max_x = 50

min_y = 0
max_y = 50




class TestiSovellus(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        self.title("INS Tracking App")

        # BASIC APP LAYOUT
        self.geometry("1500x1500")
        #self.geometry()
        self.resizable(1,1)

        # HEADER
        self.header = tk.Frame(self)
        self.header.place(relx=0.3, rely=0, relwidth=0.7, relheight=0.1)

        # CREATING FIGURE #1
        img = plt.imread("pohjakuva2.PNG") #ladataan kuva # pitää linkittää komentoon "Load Map"
        x = np.linspace(10, 25.0, num=200)
        y = -0.01*(x-28)*(x-8)*(x-1)+7

        f = Figure(figsize=(10,10), dpi=100)
        a = f.add_subplot(111)

        a.axes.imshow(img, extent=[min_x, max_x, min_y, max_y]) #asetetaan akselit (m) pitää linkittää komentoon "Set Axes"
    
        a.plot(x,y,'o',color='firebrick')
        canvas = FigureCanvasTkAgg(f, self)

        canvas.draw()
        canvas.get_tk_widget().pack()








root = TestiSovellus()        
root.mainloop()

