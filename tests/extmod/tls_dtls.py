# Very basic test of DTLS.

try:
    from tls import PROTOCOL_DTLS_CLIENT, PROTOCOL_DTLS_SERVER, SSLContext, CERT_NONE
    import socket
except ImportError:
    print("SKIP")
    raise SystemExit

# Wrap the DTLS Client
dtls_client_ctx = SSLContext(PROTOCOL_DTLS_CLIENT)
dtls_client_ctx.verify_mode = CERT_NONE
dtls_client = dtls_client_ctx.wrap_socket(socket.socket(), do_handshake_on_connect=False)
print('Wrapped DTLS Client')

# Wrap the DTLS Server
dtls_server_ctx = SSLContext(PROTOCOL_DTLS_SERVER)
dtls_server_ctx.verify_mode = CERT_NONE
dtls_server = dtls_server_ctx.wrap_socket(socket.socket(), do_handshake_on_connect=False)
print('Wrapped DTLS Server')

print('OK')
