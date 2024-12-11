# Very basic test of DTLS.

try:
    import tls
    import tls.PROTOCOL_DTLS_CLIENT
    import tls.PROTOCOL_DTLS_SERVER
except (ImportError, AttributeError):
    print("SKIP")
    raise SystemExit


# Test constructing with arguments.
dtls_client = tls.SSLContext(tls.PROTOCOL_DTLS_CLIENT)
dtls_server = tls.SSLContext(tls.PROTOCOL_DTLS_SERVER)
print('OK')