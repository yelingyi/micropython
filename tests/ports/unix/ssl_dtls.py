# Very basic test of ssl.SSLContext DTLS.

try:
    import ssl
    from ssl import PROTOCOL_DTLS_CLIENT
    from ssl import PROTOCOL_DTLS_SERVER
except ImportError:
    print("SKIP")
    raise SystemExit
except NameError:
    print("SKIP")
    raise SystemExit

# Test constructing with valid arguments
dtls_client = ssl.SSLContext(PROTOCOL_DTLS_CLIENT)
dtls_server = ssl.SSLContext(PROTOCOL_DTLS_SERVER)
