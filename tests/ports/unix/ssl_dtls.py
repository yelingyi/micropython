# Very basic test of ssl.SSLContext DTLS.

try:
    import ssl
    import ssl.PROTOCOL_DTLS_CLIENT
    import ssl.PROTOCOL_DTLS_SERVER
    import socket
    import errno
except ImportError:
    print("SKIP")
    raise SystemExit
except NameError:
    print("SKIP")
    raise SystemExit

# Test constructing with arguments.
dtls_client = ssl.SSLContext(ssl.PROTOCOL_DTLS_CLIENT)
dtls_server = ssl.SSLContext(ssl.PROTOCOL_DTLS_SERVER)

# Create test sockets
client_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_sock.bind(('127.0.0.1', 0))
server_addr = server_sock.getsockname()
client_sock.connect(server_addr)

# Wrap sockets with SSL/DTLS
ssl_client = dtls_client.wrap_socket(client_sock, do_handshake_on_connect=False)
ssl_server = dtls_server.wrap_socket(server_sock, server_side=True, do_handshake_on_connect=False)

# Set non-blocking mode to test error paths
ssl_client.setblocking(False)
ssl_server.setblocking(False)

# Test write with WANT_READ and other error conditions (lines 553, 564-568)
try:
    # Configure a small write buffer to force multiple writes
    small_buf = b'test' * 1024  # Create larger data than internal buffer
    ssl_client.write(small_buf)  # This should hit various error paths
except OSError as e:
    assert e.args[0] == errno.EAGAIN

# Force a fatal error by corrupting the SSL state
try:
    # Close underlying socket but keep SSL wrapper to force error
    client_sock.close()
    ssl_client.write(b'test')
except OSError as e:
    # Should hit error path in socket_write
    assert e.args[0] != errno.EAGAIN  # Should be a fatal error, not EAGAIN

# Test pending data checks (lines 572-573)
try:
    # Create new client with proper handshake
    client_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_sock.connect(server_addr)
    ssl_client = dtls_client.wrap_socket(client_sock, do_handshake_on_connect=False)
    ssl_client.setblocking(False)

    # Send data from server to client to create pending data
    server_data = b'server_data' * 100
    try:
        ssl_server.write(server_data)
    except OSError:
        pass  # Expected when non-blocking

    # Now poll client - should show pending data
    from select import poll, POLLIN, POLLOUT
    poller = poll()
    poller.register(ssl_client, POLLIN | POLLOUT)
    
    # Multiple poll/read cycles to ensure we hit pending data paths
    for _ in range(3):
        res = poller.poll(100)
        if res[0][1] & POLLIN:
            try:
                ssl_client.read(100)
            except OSError:
                pass

# Test error case (line 564) by trying to write to closed socket
ssl_client.close()
try:
    ssl_client.write(b'test')
except OSError:
    pass  # Expected error

# Test polling and pending data (lines 572-573)
try:
    # Create new client since we closed the previous one
    client_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_sock.connect(server_addr)
    ssl_client = dtls_client.wrap_socket(client_sock, do_handshake_on_connect=False)
    ssl_client.setblocking(False)

    # Test poll when there's pending data
    from select import poll, POLLIN, POLLOUT
    poller = poll()
    poller.register(ssl_client, POLLIN | POLLOUT)
    
    # Should return POLLOUT since we can write
    res = poller.poll(100)
    assert len(res) > 0

    # Test polling closed socket (line 304)
    ssl_client.close()
    res = poller.poll(100)
    # Should return POLLNVAL for closed socket
    assert len(res) == 1
    assert res[0][1] & select.POLLNVAL

finally:
    ssl_server.close()
