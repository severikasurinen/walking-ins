import asyncio
from bleak import BleakScanner
from bleak import BleakClient

address = "64:E8:33:00:66:B6" #ATA_S 

SERVICE_UUID = "2e5dc756-78bd-405c-bb72-9641a6848842"   #service channel
DATA_UUID = "a20eebe5-dfbf-4428-bb7b-84e40d102681"      #data channel
CONTROL_UUID = "0cf0cef9-ec1a-495a-a007-4de6037a303b"   #config channel


# tarvitaan komennot: 
# -> yhdistä (jos ei onnistu tyyliin 30s sisään, palauta "ei onnistu")
# -> disconnectaa (s.e. esp tietää että disconnectattiin)

device_connected = False

async def connect(address):
    client = BleakClient(address)
    
    try: 
        await client.connect()
        device_connected = True

        ## AVATAAN TIEDOSTO, JONNE DATAA RUVETAAN KIRJOITTAMAAN
        file = open("DATA_COLLECTION.txt", "w")
 
        i = 0
        while i < 50: # i kpl lukupisteitä; rakenne voidaan muuttaa myöhemmin "while a == true" -tyyppiseksi
            current_data = list(await client.read_gatt_char(DATA_UUID))
            mills = (current_data[0])*1+(current_data[1])*256+(current_data[2])*(256**2)+(current_data[3])*(256**3)        
            #print("Aikaa on kulunut: " + str(mills) + " ms") ## DEBUGGAUSTA VARTEN
            file.write("Aikaa on kulunut: " + str(mills) + " ms\n") # kysy joltain joka tietää; miten kannattaisi tehdä? ensin puskuri, sitten vasta kirjoitus??
            i += 1
        file.close()
        print(device_connected) # true, jos laite oli yhdistettynä  --->  poista myöhemmin

    except Exception as e:
        print(e)
    
    finally:
        print("Kytken pois") # poista myöhemmin
        await client.disconnect() # kytkee laitteen lopuksi pois


async def disconnect(address):
    client = BleakClient(address)
    await client.unpair()
    device_connected = False 
    print(device_connected) #poista myöhemmin 


def main():
    """Main function."""
    asyncio.run(connect(address)) 


if __name__ == '__main__':
    main()
