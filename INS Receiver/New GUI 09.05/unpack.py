import struct

in_data = "1001000101110111000000000000000010000100100011110011001110111110100001011001010110010011001111010111001101100010000111010100000111101101111111110111111100111111000101011001011001011000001110100111100011101010100111111011101010010110000001101101000110111001"
b = bytes(int(in_data[i:i+8], 2) for i in range(0, len(in_data), 8))

timestamp = struct.unpack('<L', b[0:4])[0] / 1000
pos_x = struct.unpack('<f', b[4:8])[0]
pos_y = struct.unpack('<f', b[8:12])[0]
pos_z = struct.unpack('<f', b[12:16])[0]
rot_w = struct.unpack('<f', b[16:20])[0]
rot_x = struct.unpack('<f', b[20:24])[0]
rot_y = struct.unpack('<f', b[24:28])[0]
rot_z = struct.unpack('<f', b[28:32])[0]

print("Time -", timestamp)
print("Position - x:", pos_x, "y:", pos_y, "z:", pos_z)
print("Rotation - w:", rot_w, "x:", rot_x, "y:", rot_y, "z:", rot_z)
