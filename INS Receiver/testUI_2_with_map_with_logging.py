import tkinter as tk
from tkinter import ttk

### LISÄTTY 14.2. ###
import numpy as np
import matplotlib
matplotlib.use("TkAgg")

from matplotlib import pyplot as plt

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib.animation as animation



import time
### ------------- ###


# GLOBAL VARIABLES
selectionbar_color = '#81a2b5'
sidebar_color = '#81a2b5'
header_color = '#81a2b5'
visualisation_frame_color = '#ffffff'


img = plt.imread("GUI Resources/pohjakuva2.PNG") #ladataan kuva # pitää linkittää komentoon "Load Map"

f = Figure(figsize=(10,10), dpi=100)
a = f.add_subplot(111)

def animate(i):
    pullData = open("sampleText.txt","r").read()
    dataArray = pullData.split('\n')
    xar = []
    yar = []
    for eachLine in dataArray:
        if len(eachLine)>1:
                x,y = eachLine.split(',')
                xar.append(int(x))
                yar.append(int(y))
    a.clear()
    a.plot(xar,yar,'o',linewidth=5,color='firebrick')
    #a.axes.imshow(img, extent=[0, 4.0, 0, 4.0])



class TestiSovellus(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        self.title("INS Tracking App")

        # BASIC APP LAYOUT

        self.geometry("1600x800")
        self.resizable(0,0)
        self.config(background=selectionbar_color)
        # icon = tk.PhotoImage(file="images\\INS_logo.png")
        # self.iconphoto(True, icon)

        # HEADER
        self.header = tk.Frame(self, bg=header_color)
        self.header.place(relx=0.3, rely=0, relwidth=0.7, relheight=0.1)

        # SIDEBAR   
        # SIDEBAR FRAME
        self.sidebar = tk.Frame(self, bg=sidebar_color)
        self.sidebar.place(relx=0, rely=0, relwidth=0.3, relheight=1)

        # BRANDING FRAME INS LOGO ETC
        # TO BE ADDED

        
        # SUBMENUS IN SIDEBAR FRAME
        self.submenu_frame = tk.Frame(self.sidebar, bg=sidebar_color)
        self.submenu_frame.place(relx=0,rely=0.2,relwidth=1, relheight=0.85)

        # SUBMENU #1
        submenu1 = sidebarSubMenu(self.submenu_frame, 
                                  sub_menu_heading="VIEW",
                                  sub_menu_options=["Location",
                                                    "Path"])
        submenu1.options["Location"].config(
            command=lambda: self.show_frame(Frame1)
        )

        submenu1.options["Path"].config(
            command=lambda: self.show_frame(Frame2)
        )
        submenu1.place(relx=0, rely=0.025, relwidth=1, relheight=0.3)

        # SUBMENU #2
        submenu2 = sidebarSubMenu(self.submenu_frame, 
                                  sub_menu_heading="SETUP",
                                  sub_menu_options=["Connect Device",
                                                    "Load Map",
                                                    "Set position",
                                                    "Set Scale"])
        submenu2.place(relx=0, rely=0.325, relwidth=1, relheight=0.3)


        # MULTI PAGE SETTINGS
        # MAIN FRAME
        container = tk.Frame(self)
        container.config(highlightbackground="#808080",highlightthickness=0.5)
        container.place(relx=0.3,rely=0.1,relwidth=0.7,relheight=0.9)


        # ADDING FRAMES TO THE MAIN FRAME
        self.frames = {}

        for F in (Frame1, Frame2):
            frame = F(container, self)
            self.frames[F] = frame
            frame.place(relx=0, rely=0, relwidth=1, relheight=1)
        self.show_frame(Frame1)

    def show_frame(self, cont):
        ''' this function enables us to switch between frames'''
        frame = self.frames[cont]
        frame.tkraise()


#---------------------------------------------------------------------------------------------

# LOCATION (ONLY THE ONE AT THE MOMENT)
class Frame1(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self,parent)
        label = tk.Label(self, text="Location",font=("Helvetica",20))
        label.pack()      

        # CREATING FIGURE #1
        #asetetaan akselit (m) pitää linkittää komentoon "Set Axes"   

        canvas = FigureCanvasTkAgg(f, self)
        canvas.draw()
        #canvas.show()
        canvas.get_tk_widget().pack()
        canvas._tkcanvas.pack()


# PATH 
class Frame2(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self,parent)
        label = tk.Label(self, text="Path",font=("Helvetica",20))
        label.pack()  

        # CREATING FIGURE #2
        img = plt.imread("GUI Resources/pohjakuva2.PNG") #ladataan kuva # pitää linkittää komentoon "Load Map" #turha kun kahteen kertaan
        x = np.linspace(0.5, 3.0, num=20)
        y = 0.9*x 

        f2 = Figure(figsize=(10,10), dpi=100)
        a2 = f2.add_subplot(111)

        a2.axes.imshow(img, extent=[0, 4.0, 0, 4.0]) #asetetaan akselit (m) pitää linkittää komentoon "Set Axes"
    
        a2.plot(x,y,'o',color='firebrick')
        canvas = FigureCanvasTkAgg(f2, self)

        canvas.draw()
        canvas.get_tk_widget().pack()
        canvas._tkcanvas.pack()





#--------------------------------------------------------------------------------------------

class sidebarSubMenu(tk.Frame):

    def __init__(self, parent, sub_menu_heading, sub_menu_options):

        tk.Frame.__init__(self,parent)
            
        # setting submenu frame color to sidebar color
        self.config(bg=sidebar_color)

        # creating and placing submenu heading label
        self.sub_menu_heading_label = tk.Label(self,
                                               text=sub_menu_heading,
                                               bg=sidebar_color,
                                               fg="#333333",
                                               font=("Arial",10))
        self.sub_menu_heading_label.place(x=30, y=10, anchor="w")

        # creating and placing submenu heading label
        sub_menu_sep = ttk.Separator(self, orient='horizontal')
        sub_menu_sep.place(x=30,y=30, relwidth=0.8, anchor="w")

        self.options = {}
        for n, x in enumerate(sub_menu_options):
            self.options[x] = tk.Button(self,
                                        text=x,
                                        bg=sidebar_color,
                                        font=("Arial",9,"bold"),
                                        bd=0,
                                        cursor='hand2',
                                        activebackground="#ffffff")
            self.options[x].place(x=30, y=45 * (n + 1), anchor="w")
            


root = TestiSovellus()
ani = animation.FuncAnimation(f, animate, interval=1000)
root.mainloop()

