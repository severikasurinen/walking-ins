import asyncio
import csv
from bleak import BleakScanner
from bleak import BleakClient
import struct
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

device_connected = False
device_client = None
csv_writer = None

data_points = []


async def connect(conn_address):
    global device_connected, device_client

    device_client = BleakClient(conn_address)

    try:
        await device_client.connect()

        # writing control 1
        await device_client.write_gatt_char(CONTROL_UUID, b'\x01')

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


class App:
    async def exec(self):
        self.window = Window(asyncio.get_event_loop())
        await self.window.show()


class Window(tk.Tk):
    def __init__(self, loop):
        self.loop = loop
        self.root = tk.Tk()
        self.animation = "░▒▒▒▒▒"

        self.canvas = tk.Canvas(self.root, width=1400, height=700, bg="white")
        self.canvas.pack()
        self.canvas.grid(row=1, columnspan=2, padx=(8, 8), pady=(16, 0))

        self.label = tk.Label(text="")
        self.label.grid(row=0, columnspan=2, padx=(8, 8), pady=(16, 0))
        self.conn_button = tk.Button(text="Connect", width=15, bg='green',
                                     command=lambda: self.loop.create_task(self.toggle_connection()))
        self.conn_button.grid(row=2, column=0, sticky=tk.W, padx=8, pady=8)
        self.control_button = tk.Button(text="Start", width=15, bg='gray',
                                     command=lambda: self.loop.create_task(self.toggle_connection()))
        self.control_button["state"] = DISABLED
        self.control_button.grid(row=2, column=1, sticky=tk.W, padx=8, pady=8)

    async def show(self):
        while True:
            if device_connected and device_client is not None:
                in_data = await device_client.read_gatt_char(DATA_UUID)
                if len(in_data) > 4:
                    timestamp = struct.unpack('<L', in_data[0:4])[0]
                    pos_x = struct.unpack('<f', in_data[4:8])[0]
                    pos_y = struct.unpack('<f', in_data[8:12])[0]
                    pos_z = struct.unpack('<f', in_data[12:16])[0]
                    rot_w = struct.unpack('<f', in_data[16:20])[0]
                    rot_x = struct.unpack('<f', in_data[20:24])[0]
                    rot_y = struct.unpack('<f', in_data[24:28])[0]
                    rot_z = struct.unpack('<f', in_data[28:32])[0]

                    data_points.append([timestamp, pos_x, pos_y, pos_z, rot_w, rot_x, rot_y, rot_z])
                    #csv_writer.writerow(data_points[-1])
            else:
                timestamp = "None"
            self.label["text"] = self.animation
            self.animation = timestamp

            x_scale = 20  # Scale for x-axis
            y_scale = 10  # Scale for y-axis
            x_offset = 700
            y_offset = 350

            # Draw data points and lines
            for i in range(len(data_points) - 1):
                a = data_points[i]
                b = data_points[i + 1]
                x1_scaled, y1_scaled = a[1] * x_scale + x_offset, y_offset - a[2] * y_scale
                x2_scaled, y2_scaled = b[1] * x_scale + x_offset, y_offset - b[2] * y_scale
                self.canvas.create_line(x1_scaled, y1_scaled, x2_scaled, y2_scaled, fill="red", width=2)

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
        else:
            if await connect(address):
                device_connected = True
                self.conn_button["text"] = 'Disconnect'
                self.conn_button["bg"] = 'red'
        await asyncio.sleep(0)


def main():
    global csv_writer
    with open('datastream.csv', 'w') as csvfile:
        csv_writer = csv.writer(csvfile)
        csv_writer.writerow(['TIME', 'X', 'Y', 'Z', 'ROT W', 'ROT X', 'ROT Y', 'ROT Z'])
    asyncio.run(App().exec())


if __name__ == '__main__':
    main()
