import asyncio
import csv
from bleak import BleakScanner
from bleak import BleakClient
import time
import struct

address = "64:E8:33:00:66:B6" #ATA_S 

SERVICE_UUID = "2e5dc756-78bd-405c-bb72-9641a6848842"   #service channel
DATA_UUID = "a20eebe5-dfbf-4428-bb7b-84e40d102681"      #data channel
CONTROL_UUID = "0cf0cef9-ec1a-495a-a007-4de6037a303b"   #config channel

device_connected = False

async def connect(address):
    
    client = BleakClient(address)
    
    try: 
        await client.connect()
        device_connected = True
        print(device_connected) # true jos yhdistetty
        
        # writing control 1
        await client.write_gatt_char(CONTROL_UUID,b'\x01')
        
        # odotetaan 4 sekuntia
        time.sleep(4)

        # avataan csv tiedosto mihin kirjoitetaan
        with open('DATA_COLLECTION.csv', 'w') as csvfile:

            csvWriter = csv.writer(csvfile)
            csvWriter.writerow(['TIME','X', 'Y', 'Z', 'ROT W', 'ROT X', 'ROT Y', 'ROT Z'])
        
            for i in range(100):
                
                in_data = await client.read_gatt_char(DATA_UUID)
                #print(in_data)

                timestamp = struct.unpack('<L', in_data[0:4])[0]
                pos_x = struct.unpack('<f', in_data[4:8])[0]
                pos_y = struct.unpack('<f', in_data[8:12])[0]
                pos_z = struct.unpack('<f', in_data[12:16])[0]
                rot_w = struct.unpack('<f', in_data[16:20])[0]
                rot_x = struct.unpack('<f', in_data[20:24])[0]
                rot_y = struct.unpack('<f', in_data[24:28])[0]
                rot_z = struct.unpack('<f', in_data[28:32])[0]

                csvWriter.writerows([[timestamp, pos_x, pos_y, pos_z, rot_w, rot_x, rot_y, rot_z]])

                #print("Time -", timestamp)
                #print("Position - x:", pos_x, "y:", pos_y, "z:", pos_z)
                #print("Rotation - w:", rot_w, "x:", rot_x, "y:", rot_y, "z:", rot_z)

        
 
        print(device_connected) # true, jos laite oli yhdistettynä  --->  poista myöhemmin

    except Exception as e:
        print(e)
    
    finally:
        # print("Kytken pois") # poista myöhemmin
        await client.disconnect() # kytkee laitteen lopuksi pois


async def disconnect(address):
    client = BleakClient(address)
    await client.disconnect()
    device_connected = False 
    print(device_connected) #poista myöhemmin 

def main():
    # Main function
    asyncio.run(connect(address)) 
    asyncio.run(disconnect(address))

if __name__ == '__main__':
    main()
