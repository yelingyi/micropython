# This will be built into the native mpy module.
# Functions here will be available in the rfcore_transparent module along with
# the native C functions.


def start(stream=None, callback=None):
    import sys
    import micropython
    from bluetooth import BLE

    # Ensure rfcore has been started at least once, then turn off bluetooth.
    BLE().active(1)
    BLE().active(0)

    in_stream = out_stream = stream

    if not in_stream:
        # Disable the ctrl-c interrupt when using repl stream.
        micropython.kbd_intr(-1)

        in_stream = sys.stdin
        out_stream = sys.stdout

    # Start transparant mode C function (never exits).
    _rfcore_transparent_start(in_stream, out_stream, callback)
