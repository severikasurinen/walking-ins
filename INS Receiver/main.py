import asyncio
import os
import csv
from bleak import BleakScanner
from bleak import BleakClient
import struct
from datetime import datetime
import tkinter as tk
from tkinter.constants import DISABLED, NORMAL
from tkinter import ttk
from matplotlib.backends.backend_tkagg import (
    FigureCanvasTkAgg, NavigationToolbar2Tk)
from matplotlib.backend_bases import key_press_handler
from matplotlib import pyplot as plt, animation

address = "64:E8:33:00:66:B6"  # ATA_S

SERVICE_UUID = "2e5dc756-78bd-405c-bb72-9641a6848842"  # service channel
DATA_UUID = "a20eebe5-dfbf-4428-bb7b-84e40d102681"  # data channel
CONTROL_UUID = "0cf0cef9-ec1a-495a-a007-4de6037a303b"  # config channel

fig_range = (20, 20)  # Plot range from center point in meters (x, y)
fig_size = (600, 600)  # Plot size in pixels (x, y)

device_connected = False
is_measuring = False
device_client = None
data_points = []


async def connect(conn_address):
    global device_connected, device_client

    device_client = BleakClient(conn_address)

    try:
        await device_client.connect()

        return True

    except Exception as e:
        print(e)
        device_client = None
        return False


async def disconnect(conn_address):
    global device_connected, device_client

    try:
        await device_client.disconnect()
        return True
    except Exception as e:
        print(e)
        return False


async def stop_measuring():
    global is_measuring

    try:
        await device_client.write_gatt_char(CONTROL_UUID, b'\x00')
        return True

    except Exception as e:
        print(e)
        return False


async def start_measuring():
    global is_measuring

    try:
        await device_client.write_gatt_char(CONTROL_UUID, b'\x01')
        return True

    except Exception as e:
        print(e)
        return False


class App:
    async def exec(self):
        self.window = Window(asyncio.get_event_loop())
        await self.window.show()


class Window(tk.Tk):
    def __init__(self, loop):
        self.loop = loop
        self.root = tk.Tk()
        self.root.title("INS Receiver")

        self.canvas = tk.Canvas(self.root, width=fig_size[0], height=fig_size[1], bg="white",
                                highlightthickness=2, highlightbackground="black")
        self.canvas.pack()
        self.canvas.grid(row=1, columnspan=2, padx=(8, 8), pady=(16, 0))

        self.label = tk.Label(text="")
        self.label.grid(row=0, column=0, padx=(8, 8), pady=(16, 0))

        self.conn_button = tk.Button(text="Connect", width=15, bg='green',
                                     command=lambda: self.loop.create_task(self.toggle_connection()))
        self.conn_button.grid(row=2, column=0, sticky=tk.W, padx=8, pady=8)
        self.control_button = tk.Button(text="Start", width=15, bg='gray',
                                        command=lambda: self.loop.create_task(self.toggle_measurement()))
        self.control_button["state"] = DISABLED
        self.control_button.grid(row=2, column=1, sticky=tk.W, padx=8, pady=8)

    async def show(self):
        global is_measuring
        while True:
            timestamp = "None"
            if is_measuring:
                in_data = await device_client.read_gatt_char(DATA_UUID)
                if len(in_data) == 32:
                    timestamp = struct.unpack('<L', in_data[0:4])[0]
                    pos_x = struct.unpack('<f', in_data[4:8])[0]
                    pos_y = struct.unpack('<f', in_data[8:12])[0]
                    pos_z = struct.unpack('<f', in_data[12:16])[0]
                    rot_w = struct.unpack('<f', in_data[16:20])[0]
                    rot_x = struct.unpack('<f', in_data[20:24])[0]
                    rot_y = struct.unpack('<f', in_data[24:28])[0]
                    rot_z = struct.unpack('<f', in_data[28:32])[0]

                    if data_points[-1][0] != timestamp:
                        print("Timestamp:", timestamp, "| X:", pos_x, ", Y:", pos_y, ", Z:", pos_z,
                              "| W: ", rot_w, ", X: ", rot_x, ", Y: ", rot_y, ", Z: ", rot_z)
                        data_points.append([timestamp, pos_x, pos_y, pos_z, rot_w, rot_x, rot_y, rot_z])

                        scale = ((fig_size[0] / 2) / fig_range[0], (fig_size[1] / 2) / fig_range[1])

                        # Draw data points and lines
                        if len(data_points) > 1:
                            print(data_points)
                            for i in range(len(data_points) - 1):
                                a = data_points[i]
                                b = data_points[i + 1]
                                if a != b:
                                    start_point = (a[1] * scale[0] + fig_size[0] / 2, fig_size[1] / 2 - a[2] * scale[1])
                                    end_point = (b[1] * scale[0] + fig_size[0] / 2, fig_size[1] / 2 - b[2] * scale[1])
                                    self.canvas.create_line(start_point[0], start_point[1], end_point[0], end_point[1]
                                                            , fill="red", width=1)

            self.label["text"] = timestamp

            self.root.update()
            await asyncio.sleep(0.1)

    async def toggle_connection(self):
        global device_connected, device_client
        if device_connected:
            if await disconnect(address):
                device_connected = False
                device_client = None
                self.conn_button["text"] = 'Connect'
                self.conn_button["bg"] = 'green'
                self.control_button["state"] = DISABLED
                self.control_button["bg"] = 'gray'

        else:
            if await connect(address):
                device_connected = True
                self.conn_button["text"] = 'Disconnect'
                self.conn_button["bg"] = 'red'
                self.control_button["state"] = NORMAL
                self.control_button["bg"] = 'green'

        await asyncio.sleep(0)

    async def toggle_measurement(self):
        global is_measuring, data_points

        if is_measuring:
            if await stop_measuring():
                is_measuring = False
                self.control_button["text"] = 'Start'
                self.control_button["bg"] = 'green'
                self.conn_button["bg"] = 'red'
                self.conn_button["state"] = NORMAL

                if not os.path.isdir("datastream/"):
                    os.mkdir("datastream/")

                file_name = "datastream/data_" + datetime.now().strftime("%Y-%m-%d_%H-%M") + ".csv"
                with open(file_name, 'w', newline='') as csvfile:
                    csv_writer = csv.writer(csvfile)
                    csv_writer.writerow(['timestamp', 'pos_x', 'pos_y', 'pos_z', 'rot_w', 'rot_x', 'rot_y', 'rot_z'])
                    csv_writer.writerows(data_points)
        else:
            if await start_measuring():
                self.canvas.delete("all")
                data_points = [[0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0]]
                is_measuring = True
                self.control_button["text"] = 'Stop'
                self.control_button["bg"] = 'red'
                self.conn_button["bg"] = 'gray'
                self.conn_button["state"] = DISABLED


def main():
    asyncio.run(App().exec())


if __name__ == '__main__':
    main()
