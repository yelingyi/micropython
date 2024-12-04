# Very basic test of ssl.SSLContext DTLS.

try:
    import ssl
except ImportError:
    print("SKIP")
    raise SystemExit

try:
    import ssl.PROTOCOL_DTLS_CLIENT
    import ssl.PROTOCOL_DTLS_SERVER
except NameError:
    print("SKIP")
    raise SystemExit


# Test constructing with arguments.
dtls_client = ssl.SSLContext(ssl.PROTOCOL_DTLS_CLIENT)
dtls_server = ssl.SSLContext(ssl.PROTOCOL_DTLS_SERVER)
