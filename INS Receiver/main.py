import asyncio
import os
import csv
import numpy as np
from PIL import Image, ImageTk
from bleak import BleakScanner
from bleak import BleakClient
import struct
from datetime import datetime
import tkinter as tk
from tkinter.constants import DISABLED, NORMAL
import matplotlib.pyplot as plt

address = "64:E8:33:00:66:B6"  # ATA_S

SERVICE_UUID = "2e5dc756-78bd-405c-bb72-9641a6848842"  # service channel
DATA_UUID = "a20eebe5-dfbf-4428-bb7b-84e40d102681"  # data channel
CONTROL_UUID = "0cf0cef9-ec1a-495a-a007-4de6037a303b"  # config channel

fig_range = (10, 10)  # Plot range from center point in meters (x, y)
fig_size = (600, 480)  # Plot size in pixels (x, y)

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

        self.label = tk.Label(text="")
        self.label.grid(row=0, column=2, padx=8, pady=16)

        self.conn_button = tk.Button(text="Connect", width=15, bg='green',
                                     command=lambda: self.loop.create_task(self.toggle_connection()))
        self.conn_button.grid(row=0, column=0, sticky=tk.E, padx=8, pady=16)
        self.control_button = tk.Button(text="Start", width=15, bg='gray',
                                        command=lambda: self.loop.create_task(self.toggle_measurement()))
        self.control_button["state"] = DISABLED
        self.control_button.grid(row=0, column=1, sticky=tk.W, padx=8, pady=16)

        self.canvas = tk.Canvas(self.root, width=fig_size[0], height=fig_size[1])
        self.canvas.grid(row=1, columnspan=3)

    async def show(self):
        global is_measuring
        while True:
            timestamp = None
            if is_measuring:
                try:
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
                                  "| W:", rot_w, ", X:", rot_x, ", Y:", rot_y, ", Z:", rot_z)
                            data_points.append([timestamp, pos_x, pos_y, pos_z, rot_w, rot_x, rot_y, rot_z])

                            # Draw data points and lines
                            if len(data_points) > 1:
                                x = np.array(data_points)[:, 1]
                                y = np.array(data_points)[:, 2]

                                fig = plt.figure()
                                fig.patch.set_alpha(0.0)
                                ax = fig.add_subplot(111)
                                plt.xlabel("x (m)")
                                plt.ylabel("y (m)")
                                plt.axis((-fig_range[0], fig_range[0], -fig_range[1], fig_range[1]))
                                plt.grid()

                                ax.plot(x, y, color='red')

                                fig.savefig('temp.png', facecolor=fig.get_facecolor(), edgecolor='none')
                                plt.close()
                                img = ImageTk.PhotoImage(Image.open("temp.png"))
                                os.remove("temp.png")
                                self.canvas.create_image(fig_size[0] / 2, 0, anchor=tk.N, image=img)

                        await asyncio.sleep(0.15)
                except Exception as e:
                    print(e)
                    await self.toggle_connection()

            if timestamp is None:
                self.label["text"] = timestamp
            else:
                self.label["text"] = "Timestamp: " + str(timestamp / 1000.0) + " s"

            self.root.update()
            await asyncio.sleep(0.1)

    async def toggle_connection(self):
        global device_connected, device_client

        self.conn_button["state"] = DISABLED

        success = False
        tries = 0
        while not success and tries < 5:
            if device_connected:
                self.conn_button["text"] = 'Disconnecting...'
                if await disconnect(address):
                    device_connected = False
                    device_client = None
                    self.conn_button["text"] = 'Connect'
                    self.conn_button["bg"] = 'green'
                    self.control_button["state"] = DISABLED
                    self.control_button["bg"] = 'gray'
                    success = True

            else:
                self.conn_button["text"] = 'Connecting...'
                if await connect(address):
                    device_connected = True
                    self.conn_button["text"] = 'Disconnect'
                    self.conn_button["bg"] = 'red'
                    self.control_button["state"] = NORMAL
                    self.control_button["bg"] = 'green'
                    success = True
            tries += 1

        self.conn_button["state"] = NORMAL

    async def toggle_measurement(self):
        global is_measuring, data_points

        self.control_button["state"] = DISABLED

        if is_measuring:
            self.control_button["text"] = 'Stopping...'
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
            self.control_button["text"] = 'Starting...'
            if await start_measuring():
                self.canvas.delete("all")
                data_points = [[0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0]]
                is_measuring = True
                self.control_button["text"] = 'Stop'
                self.control_button["bg"] = 'red'
                self.conn_button["bg"] = 'gray'
                self.conn_button["state"] = DISABLED

        self.control_button["state"] = NORMAL


def main():
    asyncio.run(App().exec())


if __name__ == '__main__':
    main()
