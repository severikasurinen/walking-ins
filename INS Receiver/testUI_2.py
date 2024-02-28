import tkinter as tk
from tkinter import ttk

# GLOBAL VARIABLES
selectionbar_color = '#81a2b5'
sidebar_color = '#81a2b5'
header_color = '#81a2b5'
visualisation_frame_color = '#ffffff'

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
                                  sub_menu_options=["Display frame #1",
                                                    "Display frame #2"])
        submenu1.options["Display frame #1"].config(
            command=lambda: self.show_frame(Frame1)
        )

        submenu1.options["Display frame #2"].config(
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

   
        '''
        submenu1.options["Display frame #1"].config(
            command=lambda: self.show_frame(Frame1)
        )

        submenu1.options["Display frame #2"].config(
            command=lambda: self.show_frame(Frame2)
        )
        submenu1.place(relx=0, rely=0.025, relwidth=1, relheight=0.3)
        '''


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

class Frame1(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self,parent)
        label = tk.Label(self, text="Location",font=("Helvetica",20))
        label.pack()      


class Frame2(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self,parent)
        label = tk.Label(self, text="Path",font=("Helvetica",20))
        label.pack()  

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
root.mainloop()

