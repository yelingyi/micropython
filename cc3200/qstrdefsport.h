/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2015 Daniel Campora
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// qstrs specific to this port
Q(__name__)
Q(help)
Q(pyb)
Q(info)
Q(reset)
Q(main)
Q(sync)
Q(gc)
Q(rng)
Q(delay)
Q(time)
Q(open)
Q(on)
Q(off)
Q(toggle)
Q(write)
Q(read)
Q(readall)
Q(readline)
Q(input)
Q(os)
Q(freq)
Q(unique_id)
Q(repl_info)
Q(disable_irq)
Q(enable_irq)
Q(millis)
Q(micros)
Q(elapsed_millis)
Q(elapsed_micros)
Q(udelay)
Q(flush)
Q(FileIO)
Q(enable)
Q(disable)
Q(repl_uart)
// Entries for sys.path
Q(/flash)
Q(/flash/lib)
#if MICROPY_HW_HAS_SDCARD
Q(/sd)
Q(/sd/lib)
#endif

// for module weak links
Q(struct)
Q(binascii)
Q(re)
Q(json)
Q(heapq)
Q(hashlib)

// for os module
Q(uos)
Q(os)
Q(sysname)
Q(nodename)
Q(release)
Q(version)
Q(machine)
Q(uname)
Q(/)
Q(flash)
#if MICROPY_HW_HAS_SDCARD
Q(sd)
#endif
Q(chdir)
Q(getcwd)
Q(listdir)
Q(mkdir)
Q(rename)
Q(remove)
Q(rmdir)
Q(unlink)
Q(sep)
Q(stat)
Q(urandom)
Q(mkfs)

// for file class
Q(seek)
Q(tell)

// for Pin class
Q(Pin)
Q(board)
Q(init)
Q(value)
Q(toggle)
Q(id)
Q(mode)
Q(pull)
Q(drive)
Q(alt)
Q(alt_list)
Q(IN)
Q(OUT)
Q(OPEN_DRAIN)
Q(ALT)
Q(ALT_OPEN_DRAIN)
Q(PULL_UP)
Q(PULL_DOWN)
Q(LOW_POWER)
Q(MED_POWER)
Q(HIGH_POWER)
Q(IRQ_RISING)
Q(IRQ_FALLING)
Q(IRQ_LOW_LEVEL)
Q(IRQ_HIGH_LEVEL)

// for UART class
Q(UART)
Q(init)
Q(deinit)
Q(any)
Q(sendbreak)
Q(id)
Q(baudrate)
Q(bits)
Q(stop)
Q(parity)
Q(pins)
Q(RX_ANY)

// for I2C class
Q(I2C)
Q(id)
Q(mode)
Q(baudrate)
Q(pins)
Q(addr)
Q(nbytes)
Q(buf)
Q(stop)
Q(memaddr)
Q(addrsize)
Q(init)
Q(deinit)
Q(scan)
Q(readfrom)
Q(readfrom_into)
Q(writeto)
Q(readfrom_mem)
Q(readfrom_mem_into)
Q(writeto_mem)
Q(MASTER)

// for ADC class
Q(ADC)
Q(read)
Q(init)
Q(deinit)

#if MICROPY_HW_HAS_SDCARD
// for SD class
Q(SD)
Q(init)
Q(deinit)
Q(mount)
Q(unmount)
#endif

// for RTC class
Q(RTC)
Q(datetime)

// for time class
Q(utime)
Q(localtime)
Q(mktime)
Q(sleep)
Q(time)

// for select class
Q(select)
Q(uselect)
Q(register)
Q(unregister)
Q(modify)
Q(poll)

// for socket class
Q(socket)
Q(usocket)
Q(getaddrinfo)
Q(family)
Q(type)
Q(send)
Q(sendall)
Q(sendto)
Q(recv)
Q(recvfrom)
Q(listen)
Q(accept)
Q(bind)
Q(settimeout)
Q(setblocking)
Q(setsockopt)
Q(close)
Q(makefile)
Q(protocol)
Q(error)
Q(timeout)
Q(AF_INET)
Q(AF_INET6)
Q(SOCK_STREAM)
Q(SOCK_DGRAM)
Q(SOCK_RAW)
Q(IPPROTO_SEC)
Q(IPPROTO_TCP)
Q(IPPROTO_UDP)
Q(IPPROTO_RAW)

// for ssl class
Q(ssl)
Q(ussl)
Q(wrap_socket)
Q(sock)
Q(keyfile)
Q(certfile)
Q(server_side)
Q(cert_reqs)
Q(ca_certs)
Q(SSLError)
Q(CERT_NONE)
Q(CERT_OPTIONAL)
Q(CERT_REQUIRED)

// for network class
Q(network)
Q(server_running)
Q(server_login)
Q(server_timeout)

// for WLAN class
Q(WLAN)
Q(iwconfig)
Q(key)
Q(security)
Q(ssid)
Q(bssid)
Q(scan)
Q(connect)
Q(isconnected)
Q(disconnect)
Q(channel)
Q(rssi)
Q(ifconfig)
Q(info)
Q(connections)
#if MICROPY_PORT_WLAN_URN
Q(urn)
#endif
Q(mode)
Q(ip)
Q(subnet)
Q(gateway)
Q(dns)
Q(mac)
Q(antenna)
Q(STA)
Q(AP)
Q(OPEN)
Q(WEP)
Q(WPA)
Q(WPA2)
Q(INTERNAL)
Q(EXTERNAL)

// for WDT class
Q(WDT)
Q(kick)

// for HeartBeat class
Q(HeartBeat)
Q(enable)
Q(disable)

// for callback class
Q(init)
Q(enable)
Q(disable)
Q(callback)
Q(handler)
Q(mode)
Q(value)
Q(priority)
Q(wake_from)

// for Sleep class
Q(Sleep)
Q(idle)
Q(suspend)
Q(hibernate)
Q(reset_cause)
Q(wake_reason)
Q(ACTIVE)
Q(SUSPENDED)
Q(HIBERNATING)
Q(POWER_ON)
Q(HARD_RESET)
Q(WDT_RESET)
Q(HIB_RESET)
Q(SOFT_RESET)
Q(WLAN_WAKE)
Q(PIN_WAKE)
Q(RTC_WAKE)

// for SPI class
Q(SPI)
Q(mode)
Q(baudrate)
Q(bits)
Q(polarity)
Q(phase)
Q(cs_polarity)
Q(init)
Q(deinit)
Q(send)
Q(recv)
Q(send_recv)
Q(timeout)
Q(MASTER)
Q(ACTIVE_LOW)
Q(ACTIVE_HIGH)

// for Timer class
Q(Timer)
Q(TimerChannel)
Q(init)
Q(deinit)
Q(freq)
Q(period)
Q(mode)
Q(width)
Q(channel)
Q(polarity)
Q(duty_cycle)
Q(time)
Q(event_count)
Q(event_time)
Q(A)
Q(B)
Q(ONE_SHOT)
Q(PERIODIC)
Q(EDGE_COUNT)
Q(EDGE_TIME)
Q(PWM)
Q(POSITIVE)
Q(NEGATIVE)

// for uhashlib module
Q(uhashlib)
Q(update)
Q(digest)
//Q(md5)
Q(sha1)
Q(sha256)

// for ubinascii module
Q(ubinascii)
Q(hexlify)
Q(unhexlify)
Q(a2b_base64)
Q(b2a_base64)

