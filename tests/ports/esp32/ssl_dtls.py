# Very basic test of ssl.SSLContext DTLS.

try:
    import ssl
except ImportError:
    print("SKIP")
    raise SystemExit

# Check that the ssl module has the DTLS constants.
if not hasattr(ssl, 'PROTOCOL_DTLS_CLIENT') or not hasattr(ssl, 'PROTOCOL_DTLS_SERVER'):
    print("SKIP")
    raise SystemExit

# Test constructing with arguments.
dtls_client = ssl.SSLContext(ssl.PROTOCOL_DTLS_CLIENT)
dtls_server = ssl.SSLContext(ssl.PROTOCOL_DTLS_SERVER)
