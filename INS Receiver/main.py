import asyncio
import csv
from bleak import BleakScanner
from bleak import BleakClient
import struct
import tkinter as tk
from tkinter.constants import DISABLED, NORMAL
from tkinter import ttk

address = "64:E8:33:00:66:B6"  # ATA_S

SERVICE_UUID = "2e5dc756-78bd-405c-bb72-9641a6848842"  # service channel
DATA_UUID = "a20eebe5-dfbf-4428-bb7b-84e40d102681"  # data channel
CONTROL_UUID = "0cf0cef9-ec1a-495a-a007-4de6037a303b"  # config channel

device_connected = False
device_client = None
csv_writer = None


async def connect(conn_address):
    global device_connected, device_client

    device_client = BleakClient(conn_address)

    try:
        await device_client.connect()
        device_connected = True

        # writing control 1
        await device_client.write_gatt_char(CONTROL_UUID, b'\x01')

        return True

    except Exception as e:
        print(e)
        return False


async def disconnect(conn_address):
    global device_connected, device_client

    try:
        await device_client.disconnect()
        device_connected = False
        device_client = None
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
        self.label = tk.Label(text="")
        self.label.grid(row=0, columnspan=2, padx=(8, 8), pady=(16, 0))
        self.conn_button = tk.Button(text="Connect", width=15, bg='green',
                                     command=lambda: self.loop.create_task(self.toggle_connection()))
        self.conn_button.grid(row=2, column=1, sticky=tk.W, padx=8, pady=8)
        self.control_button = tk.Button(text="Start", width=15, bg='gray',
                                     command=lambda: self.loop.create_task(self.toggle_connection()))
        self.control_button["state"] = DISABLED
        self.control_button.grid(row=2, column=2, sticky=tk.W, padx=8, pady=8)

    async def show(self):
        while True:
            if device_connected and device_client is not None:
                in_data = await device_client.read_gatt_char(DATA_UUID)

                timestamp = struct.unpack('<L', in_data[0:4])[0]
                pos_x = struct.unpack('<f', in_data[4:8])[0]
                pos_y = struct.unpack('<f', in_data[8:12])[0]
                pos_z = struct.unpack('<f', in_data[12:16])[0]
                rot_w = struct.unpack('<f', in_data[16:20])[0]
                rot_x = struct.unpack('<f', in_data[20:24])[0]
                rot_y = struct.unpack('<f', in_data[24:28])[0]
                rot_z = struct.unpack('<f', in_data[28:32])[0]

                csv_writer.writerow([timestamp, pos_x, pos_y, pos_z, rot_w, rot_x, rot_y, rot_z])
            else:
                timestamp = "None"
            self.label["text"] = self.animation
            self.animation = timestamp
            self.root.update()
            await asyncio.sleep(.1)

    async def toggle_connection(self):
        if device_connected:
            if await disconnect(address):
                self.conn_button["text"] = 'Connect'
                self.conn_button["bg"] = 'green'
        else:
            if await connect(address):
                self.conn_button["text"] = 'Disconnect'
                self.conn_button["bg"] = 'red'

                global csv_writer
                with open('DATA_COLLECTION.csv', 'w') as csvfile:
                    csv_writer = csv.writer(csvfile)
                    csv_writer.writerow(['TIME', 'X', 'Y', 'Z', 'ROT W', 'ROT X', 'ROT Y', 'ROT Z'])
        await asyncio.sleep(0)


def main():
    # Main function
    asyncio.run(App().exec())


if __name__ == '__main__':
    main()
