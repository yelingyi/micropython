# Very basic test of ssl.SSLContext DTLS.

try:
    import ssl
    import ssl.PROTOCOL_DTLS_CLIENT
    import ssl.PROTOCOL_DTLS_SERVER
except ImportError:
    print("SKIP")
    raise SystemExit
except NameError:
    print("SKIP")
    raise SystemExit

# Test constructing with valid arguments
dtls_client = ssl.SSLContext(ssl.PROTOCOL_DTLS_CLIENT)
dtls_server = ssl.SSLContext(ssl.PROTOCOL_DTLS_SERVER)

# Test constructing with invalid arguments - should raise an error
try:
    # Use an invalid protocol value that should cause mbedtls_ssl_config_defaults to fail
    invalid_context = ssl.SSLContext(0xff)
    print("FAIL: Should have raised ValueError")
except ValueError:
    print("OK: ValueError raised for invalid protocol")
