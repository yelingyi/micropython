# Test DTLS functionality including timeout handling

try:
    from tls import PROTOCOL_DTLS_CLIENT, PROTOCOL_DTLS_SERVER, SSLContext, CERT_NONE
    import socket, io
except ImportError:
    print("SKIP")
    raise SystemExit

class DummySocket(io.IOBase):
    def __init__(self):
        self.write_buffer = bytearray()
        self.read_buffer = bytearray()

    def write(self, data):
        self.write_buffer.extend(data)
        return len(data)

    def readinto(self, buf):
        l = min(len(self.read_buffer), len(buf))
        if l == 0:
            return None
        buf[:l] = self.read_buffer[:l]
        self.read_buffer = self.read_buffer[l:]
        return l

    def ioctl(self, req, arg):
        return 0

# Create dummy sockets for testing
server_socket = DummySocket()
client_socket = DummySocket()

# Wrap the DTLS Server
dtls_server_ctx = SSLContext(PROTOCOL_DTLS_SERVER)
dtls_server_ctx.verify_mode = CERT_NONE
dtls_server = dtls_server_ctx.wrap_socket(server_socket, do_handshake_on_connect=False)
print('Wrapped DTLS Server')

# Wrap the DTLS Client
dtls_client_ctx = SSLContext(PROTOCOL_DTLS_CLIENT)
dtls_client_ctx.verify_mode = CERT_NONE
dtls_client = dtls_client_ctx.wrap_socket(client_socket, do_handshake_on_connect=False)
print('Wrapped DTLS Client')


dtls_client.write(b'test')  # This will start the handshake
data = dtls_server.read(1024)  # This should trigger timing during handshake

print('OK')
