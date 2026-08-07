import socket
import struct

def qstring_bytes(s):
    if s is None:
        return struct.pack('>I', 0xffffffff)
    utf16be = s.encode('utf-16be')
    return struct.pack('>I', len(utf16be)) + utf16be

# START_WEBCAM_STREAM
cmd_bytes = qstring_bytes('START_WEBCAM_STREAM')
data_bytes = qstring_bytes('')
body = cmd_bytes + data_bytes
packet = struct.pack('>I', len(body)) + body

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('127.0.0.1', 8080))
sock.sendall(packet)
response = sock.recv(4096)
print(f"Response received: {len(response)} bytes")
sock.close()
