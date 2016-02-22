#include <stdint.h>
const char mp_frozen_names[] = {
"ModBus\0"
"\0"};
const uint32_t mp_frozen_sizes[] = {
30310,
};
const char mp_frozen_content[] = {
"import pyb\nimport struct\nimport sys\nimport utime\n\nMODE_RTU   = \'rtu\'\nMODE_ASCII = \'ascii\'\n\nCLOSE_PORT_AFTER_EACH_CALL = False\n\n_NUMBER_OF_BYTES_PER_REGISTER = 2\n\n_LATEST_READ_TIMES = {}\n\nclass Instrument():\n\n    def __init__(self, uartObj, slaveaddress, mode=MODE_RTU):\n        self.serial = uartObj\n        self.address = slaveaddress\n\n        self.mode = mode\n\n        self.debug = False\n\n        self.close_port_after_each_call = CLOSE_PORT_AFTER_EACH_CALL\n\n        self.precalculate_read_size = True\n        \n        self.handle_local_echo = False\n\n        self.rtc = pyb.RTC()\n\n        _LATEST_READ_TIMES[str(self.serial)] = 0\n\n    def __repr__(self):\n        return \"{}<id=0x{:x}, address={}, mode={}, close_port_after_each_call={}, precalculate_read_size={}, debug={}, serial={}>\".format(\n            self.__module__,\n            id(self),\n            self.address,\n            self.mode,\n            self.close_port_after_each_call,\n            self.precalculate_read_size,\n            self.debug,\n            self.serial,\n            )\n\n    def read_bit(self, registeraddress, functioncode=2):\n        _checkFunctioncode(functioncode, [1, 2])\n        return self._genericCommand(functioncode, registeraddress)\n\n    def write_bit(self, registeraddress, value, functioncode=5):\n        _checkFunctioncode(functioncode, [5, 15])\n        _checkInt(value, minvalue=0, maxvalue=1, description=\'input value\')\n        self._genericCommand(functioncode, registeraddress, value)\n\n    def read_register(self, registeraddress, numberOfDecimals=0, functioncode=3, signed=False):\n        _checkFunctioncode(functioncode, [3, 4])\n        _checkInt(numberOfDecimals, minvalue=0, maxvalue=10, description=\'number of decimals\')\n        #_checkBool(signed, description=\'signed\')\n        return self._genericCommand(functioncode, registeraddress, numberOfDecimals=numberOfDecimals, signed=signed)\n\n    def write_register(self, registeraddress, value, numberOfDecimals=0, functioncode=16, signed=False):\n        _checkFunctioncode(functioncode, [6, 16])\n        _checkInt(numberOfDecimals, minvalue=0, maxvalue=10, description=\'number of decimals\')\n        #_checkBool(signed, description=\'signed\')\n        _checkNumerical(value, description=\'input value\')\n        self._genericCommand(functioncode, registeraddress, value, numberOfDecimals, signed=signed)\n\n\n\n\n\n\n\n\n    def _genericCommand(self, functioncode, registeraddress, value=None, numberOfDecimals=0, numberOfRegisters=1, signed=False, payloadformat=None):\n        NUMBER_OF_BITS = 1\n        NUMBER_OF_BYTES_FOR_ONE_BIT = 1\n        NUMBER_OF_BYTES_BEFORE_REGISTERDATA = 1\n        ALL_ALLOWED_FUNCTIONCODES = list(range(1, 7)) + [15, 16]  # To comply with both Python2 and Python3\n        MAX_NUMBER_OF_REGISTERS = 255\n\n        # Payload format constants, so datatypes can be told apart.\n        # Note that bit datatype not is included, because it uses other functioncodes.\n        PAYLOADFORMAT_LONG      = \'long\'\n        PAYLOADFORMAT_FLOAT     = \'float\'\n        PAYLOADFORMAT_STRING    = \'string\'\n        PAYLOADFORMAT_REGISTER  = \'register\'\n        PAYLOADFORMAT_REGISTERS = \'registers\'\n\n        ALL_PAYLOADFORMATS = [PAYLOADFORMAT_LONG, PAYLOADFORMAT_FLOAT,PAYLOADFORMAT_STRING, PAYLOADFORMAT_REGISTER, PAYLOADFORMAT_REGISTERS]\n\n        ## Check input values ##\n        _checkFunctioncode(functioncode, ALL_ALLOWED_FUNCTIONCODES)  # Note: The calling facade functions should validate this\n        #_checkRegisteraddress(registeraddress)\n        _checkInt(numberOfDecimals, minvalue=0, description=\'number of decimals\')\n        #_checkInt(numberOfRegisters, minvalue=1, maxvalue=MAX_NUMBER_OF_REGISTERS, description=\'number of registers\')\n        #_checkBool(signed, description=\'signed\')\n\n        if payloadformat is not None:\n            if payloadformat not in ALL_PAYLOADFORMATS:\n                raise ValueError(\'Wrong payload format variable. Given: {0!r}\'.format(payloadformat))\n\n        ## Check combinations of input parameters ##\n        numberOfRegisterBytes = numberOfRegisters * _NUMBER_OF_BYTES_PER_REGISTER\n\n        # Payload format\n        if functioncode in [3, 4, 6, 16] and payloadformat is None:\n            payloadformat = PAYLOADFORMAT_REGISTER\n\n        if functioncode in [3, 4, 6, 16]:\n            if payloadformat not in ALL_PAYLOADFORMATS:\n                raise ValueError(\'The payload format is unknown. Given format: {0!r}, functioncode: {1!r}.\'.\\\n                    format(payloadformat, functioncode))\n        else:\n            if payloadformat is not None:\n                raise ValueError(\'The payload format given is not allowed for this function code. \' + \\\n                    \'Given format: {0!r}, functioncode: {1!r}.\'.format(payloadformat, functioncode))\n\n        # Signed and numberOfDecimals\n        if signed:\n            if payloadformat not in [PAYLOADFORMAT_REGISTER, PAYLOADFORMAT_LONG]:\n                raise ValueError(\'The \"signed\" parameter can not be used for this data format. \' + \\\n                    \'Given format: {0!r}.\'.format(payloadformat))\n\n        if numberOfDecimals > 0 and payloadformat != PAYLOADFORMAT_REGISTER:\n            raise ValueError(\'The \"numberOfDecimals\" parameter can not be used for this data format. \' + \\\n                \'Given format: {0!r}.\'.format(payloadformat))\n\n                    # Number of registers\n        if functioncode not in [3, 4, 16] and numberOfRegisters != 1:\n            raise ValueError(\'The numberOfRegisters is not valid for this function code. \' + \\\n                \'NumberOfRegisters: {0!r}, functioncode {1}.\'.format(numberOfRegisters, functioncode))\n\n        if functioncode == 16 and payloadformat == PAYLOADFORMAT_REGISTER and numberOfRegisters != 1:\n            raise ValueError(\'Wrong numberOfRegisters when writing to a \' + \\\n                \'single register. Given {0!r}.\'.format(numberOfRegisters))\n            # Note: For function code 16 there is checking also in the content conversion functions.\n\n        # Value\n        if functioncode in [5, 6, 15, 16] and value is None:\n            raise ValueError(\'The input value is not valid for this function code. \' + \\\n                \'Given {0!r} and {1}.\'.format(value, functioncode))\n\n        if functioncode == 16 and payloadformat in [PAYLOADFORMAT_REGISTER, PAYLOADFORMAT_FLOAT, PAYLOADFORMAT_LONG]:\n            _checkNumerical(value, description=\'input value\')\n\n        if functioncode == 6 and payloadformat == PAYLOADFORMAT_REGISTER:\n            _checkNumerical(value, description=\'input value\')\n\n        # Value for string\n        if functioncode == 16 and payloadformat == PAYLOADFORMAT_STRING:\n            _checkString(value, \'input string\', minlength=1, maxlength=numberOfRegisterBytes)\n            # Note: The string might be padded later, so the length might be shorter than numberOfRegisterBytes.\n\n                    # Value for registers\n        if functioncode == 16 and payloadformat == PAYLOADFORMAT_REGISTERS:\n            if not isinstance(value, list):\n                raise TypeError(\'The value parameter must be a list. Given {0!r}.\'.format(value))\n\n            if len(value) != numberOfRegisters:\n                raise ValueError(\'The list length does not match number of registers. \' + \\\n                    \'List: {0!r},  Number of registers: {1!r}.\'.format(value, numberOfRegisters))\n\n        ## Build payload to slave ##\n        payloadToSlave = bytearray()\n        if functioncode in [1, 2]:\n            payloadToSlave = _numToTwoByteString(registeraddress) + \\\n                            _numToTwoByteString(NUMBER_OF_BITS)\n\n        elif functioncode in [3, 4]:\n            payloadToSlave = _numToTwoByteString(registeraddress) + \\\n                            _numToTwoByteString(numberOfRegisters)\n\n        elif functioncode == 5:\n            #payloadToSlave = _numToTwoByteString(registeraddress) + _createBitpattern(functioncode, value)\n            payloadToSlave.append(registeraddress>>8)\n            payloadToSlave.append(registeraddress&0xFF)\n            if value == 0:\n                payloadToSlave.append(0x00)\n                payloadToSlave.append(0x00)\n            else:\n                payloadToSlave.append(0xFF)\n                payloadToSlave.append(0x00)\n\n\n        elif functioncode == 6:\n            payloadToSlave = _numToTwoByteString(registeraddress) + \\\n                            _numToTwoByteString(value, numberOfDecimals, signed=signed)\n\n        elif functioncode == 15:\n            payloadToSlave = _numToTwoByteString(registeraddress) + \\\n                            _numToTwoByteString(NUMBER_OF_BITS) + \\\n                            _numToOneByteString(NUMBER_OF_BYTES_FOR_ONE_BIT) + \\\n                            _createBitpattern(functioncode, value)\n\n        elif functioncode == 16:\n            if payloadformat == PAYLOADFORMAT_REGISTER:\n                registerdata = _numToTwoByteString(value, numberOfDecimals, signed=signed)\n\n            elif payloadformat == PAYLOADFORMAT_STRING:\n                registerdata = _textstringToBytestring(value, numberOfRegisters)\n\n            elif payloadformat == PAYLOADFORMAT_LONG:\n                registerdata = _longToBytestring(value, signed, numberOfRegisters)\n\n            elif payloadformat == PAYLOADFORMAT_FLOAT:\n                registerdata = _floatToBytestring(value, numberOfRegisters)\n\n            elif payloadformat == PAYLOADFORMAT_REGISTERS:\n                registerdata = _valuelistToBytestring(value, numberOfRegisters)\n\n            #assert len(registerdata) == numberOfRegisterBytes\n            payloadToSlave = _numToTwoByteString(registeraddress) + _numToTwoByteString(numberOfRegisters) + _numToOneByteString(numberOfRegisterBytes) + registerdata\n\n        print(\"payload to slave\")\n        print(payloadToSlave)\n        print(\"type:\")\n        print(type(payloadToSlave))\n        print(\"________________\")\n\n        #Comunicate\t\t\n        payloadFromSlave = self._performCommand(functioncode, payloadToSlave)\n\n        ## Check the contents in the response payload ##\n        ## TODO \n        print(\"FIN\")\n        return\n\n    def _performCommand(self, functioncode, payloadToSlave):\n        DEFAULT_NUMBER_OF_BYTES_TO_READ = 1000\n\n        _checkFunctioncode(functioncode, None)\n        #_checkString(payloadToSlave, description=\'payload\')\n\n        # Build request\n        request = _embedPayload(self.address, self.mode, functioncode, payloadToSlave)\n        #print(\"request!\")\n        #print(request)\n        #print(\"________\")\n\n        # Calculate number of bytes to read\n        number_of_bytes_to_read = DEFAULT_NUMBER_OF_BYTES_TO_READ\n        if self.precalculate_read_size:\n            try:\n                number_of_bytes_to_read = _predictResponseSize(self.mode, functioncode, payloadToSlave)\n            except:\n                if self.debug:\n                    template = \'MinimalModbus debug mode. Could not precalculate response size for Modbus {} mode. \' + \\\n                        \'Will read {} bytes. request: {!r}\'\n                    _print_out(template.format(self.mode, number_of_bytes_to_read, request))\n\n        # Communicate\n        response = self._communicate(request, number_of_bytes_to_read)\n\n        # Extract payload\n        payloadFromSlave = _extractPayload(response, self.address, self.mode, functioncode)\n        return payloadFromSlave\n\n\n    def _communicate(self, request, number_of_bytes_to_read):\n        _checkInt(number_of_bytes_to_read)\n\n        if self.debug:\n            _print_out(\'\\nMinimalModbus debug mode. Writing to instrument (expecting {} bytes back): {!r} ({})\'. \\\n                format(number_of_bytes_to_read, request, _hexlify(request)))\n\n\n        # Sleep to make sure 3.5 character times have passed\n        minimum_silent_period   = _calculate_minimum_silent_period(self.serial.get_baudrate())\n        time_since_read         = utime.time() - _LATEST_READ_TIMES[str(self.serial)]\n\n        if time_since_read < minimum_silent_period:\n            sleep_time = minimum_silent_period - time_since_read\n\n            if self.debug:\n                template = \'MinimalModbus debug mode. Sleeping for {:.1f} ms. \' + \\\n                        \'Minimum silent period: {:.1f} ms, time since read: {:.1f} ms.\'\n                text = template.format(\n                    sleep_time * _SECONDS_TO_MILLISECONDS,\n                    minimum_silent_period * _SECONDS_TO_MILLISECONDS,\n                    time_since_read * _SECONDS_TO_MILLISECONDS)\n                _print_out(text)\n\n            utime.sleep(sleep_time)\n\n        elif self.debug:\n            template = \'MinimalModbus debug mode. No sleep required before write. \' + \\\n                \'Time since previous read: {:.1f} ms, minimum silent period: {:.2f} ms.\'\n            text = template.format(\n                time_since_read * _SECONDS_TO_MILLISECONDS,\n                minimum_silent_period * _SECONDS_TO_MILLISECONDS)\n            _print_out(text)\n\n        # Write request\n        latest_write_time = utime.time()\n\n        print(\"estoy por enviar request:\")\n        print(request)\n        print(\"largo:\"+str(len(request)))\n                \n        self.serial.write(request)\n\n        # Read and discard local echo\n        if self.handle_local_echo:\n            localEchoToDiscard = self.serial.read(len(request))\n            if self.debug:\n                template = \'MinimalModbus debug mode. Discarding this local echo: {!r} ({} bytes).\' \n                text = template.format(localEchoToDiscard, len(localEchoToDiscard))\n                _print_out(text)\n            if localEchoToDiscard != request:\n                template = \'Local echo handling is enabled, but the local echo does not match the sent request. \' + \\\n                    \'Request: {!r} ({} bytes), local echo: {!r} ({} bytes).\' \n                text = template.format(request, len(request), localEchoToDiscard, len(localEchoToDiscard))\n                raise IOError(text)\n\n        # Read response\n        answer = self.serial.read(number_of_bytes_to_read)\n        _LATEST_READ_TIMES[str(self.serial)] = utime.time()\n\n        if self.close_port_after_each_call:\n            self.serial.close()\n\n        if sys.version_info[0] > 2:\n            #answer = str(answer, encoding=\'latin1\')  # Convert types to make it Python3 compatible\n            answer = str(answer)  # Convert types to make it Python3 compatible\n\n        if self.debug:\n            template = \'MinimalModbus debug mode. Response from instrument: {!r} ({}) ({} bytes), \' + \\\n                \'roundtrip time: {:.1f} ms. Timeout setting: {:.1f} ms.\\n\'\n            text = template.format(\n                answer,\n                _hexlify(answer),\n                len(answer),\n                (_LATEST_READ_TIMES.get(self.serial.port, 0) - latest_write_time) * _SECONDS_TO_MILLISECONDS,\n                self.serial.timeout * _SECONDS_TO_MILLISECONDS)\n            _print_out(text)\n\n        if len(answer) == 0:\n            raise IOError(\'No communication with the instrument (no answer)\')\n\n        return answer\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\ndef _embedPayload(slaveaddress, mode, functioncode, payloaddata):\n    #_checkSlaveaddress(slaveaddress)\n    #_checkMode(mode)\n    #_checkFunctioncode(functioncode, None)\n    \n    #firstPart = _numToOneByteString(slaveaddress) + _numToOneByteString(functioncode) + payloaddata\n    firstPart = bytearray()\n    firstPart.append(slaveaddress)\n    firstPart.append(functioncode)\n    for b in payloaddata:\n        firstPart.append(b)\n\n    if mode == MODE_ASCII:\n        request = _ASCII_HEADER + \\\n                _hexencode(firstPart) + \\\n                _hexencode(_calculateLrcString(firstPart)) + \\\n                _ASCII_FOOTER\n    else:\n        #request = firstPart + _calculateCrcString(firstPart)\n        crc = _calculateCrcString(firstPart)\n        request = bytearray()\n        for b in firstPart:\n            request.append(b)\n        request.append(crc&0xFF)\n        request.append((crc>>8)&0xFF)\n    return request\n\n\n\n\ndef _calculateCrcString(inputstring):\n    # Preload a 16-bit register with ones\n    register = 0xFFFF\n\n    for char in inputstring:\n        register = (register >> 8) ^ _CRC16TABLE[(register ^ int(char)) & 0xFF]\n \n    return register #_numToTwoByteString(register, LsbFirst=True)\n\n\n\ndef _checkNumerical(inputvalue, minvalue=None, maxvalue=None, description=\'inputvalue\'):\n    # Type checking\n    if not isinstance(description, str):\n        raise TypeError(\'The description should be a string. Given: {0!r}\'.format(description))\n\n    if not isinstance(inputvalue, (int, float)):\n        raise TypeError(\'The {0} must be numerical. Given: {1!r}\'.format(description, inputvalue))\n\n    if not isinstance(minvalue, (int, float, type(None))):\n        raise TypeError(\'The minvalue must be numeric or None. Given: {0!r}\'.format(minvalue))\n\n    if not isinstance(maxvalue, (int, float, type(None))):\n        raise TypeError(\'The maxvalue must be numeric or None. Given: {0!r}\'.format(maxvalue))\n\n    # Consistency checking\n    if (not minvalue is None) and (not maxvalue is None):\n        if maxvalue < minvalue:\n            raise ValueError(\'The maxvalue must not be smaller than minvalue. Given: {0} and {1}, respectively.\'.format( \\\n                maxvalue, minvalue))\n\n    # Value checking\n    if not minvalue is None:\n        if inputvalue < minvalue:\n            raise ValueError(\'The {0} is too small: {1}, but minimum value is {2}.\'.format( \\\n                description, inputvalue, minvalue))\n\n    if not maxvalue is None:\n        if inputvalue > maxvalue:\n            raise ValueError(\'The {0} is too large: {1}, but maximum value is {2}.\'.format( \\\n                description, inputvalue, maxvalue))\n\n\ndef _checkInt(inputvalue, minvalue=None, maxvalue=None, description=\'inputvalue\'):\n    if not isinstance(description, str):\n        raise TypeError(\'The description should be a string. Given: {0!r}\'.format(description))\n\n    if not isinstance(inputvalue, (int)):\n        raise TypeError(\'The {0} must be an integer. Given: {1!r}\'.format(description, inputvalue))\n\n    if not isinstance(minvalue, (int, type(None))):\n        raise TypeError(\'The minvalue must be an integer or None. Given: {0!r}\'.format(minvalue))\n\n    if not isinstance(maxvalue, (int, type(None))):\n        raise TypeError(\'The maxvalue must be an integer or None. Given: {0!r}\'.format(maxvalue))\n\n    _checkNumerical(inputvalue, minvalue, maxvalue, description)\n\n\n\ndef _checkFunctioncode(functioncode, listOfAllowedValues=[]):\n    FUNCTIONCODE_MIN = 1\n    FUNCTIONCODE_MAX = 127\n    _checkInt(functioncode, FUNCTIONCODE_MIN, FUNCTIONCODE_MAX, description=\'functioncode\')\n    if listOfAllowedValues is None:\n        return\n    if not isinstance(listOfAllowedValues, list):\n        raise TypeError(\'The listOfAllowedValues should be a list. Given: {0!r}\'.format(listOfAllowedValues))\n    for value in listOfAllowedValues:\n        _checkInt(value, FUNCTIONCODE_MIN, FUNCTIONCODE_MAX, description=\'functioncode inside listOfAllowedValues\')\n    if functioncode not in listOfAllowedValues:\n        raise ValueError(\'Wrong function code: {0}, allowed values are {1!r}\'.format(functioncode, listOfAllowedValues))\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\ndef _numToTwoByteString(value, numberOfDecimals=0, LsbFirst=False, signed=False):\n    _checkNumerical(value, description=\'inputvalue\')\n    _checkInt(numberOfDecimals, minvalue=0, description=\'number of decimals\')\n    #_checkBool(LsbFirst, description=\'LsbFirst\')\n    #_checkBool(signed, description=\'signed parameter\')\n\n    multiplier = 10 ** numberOfDecimals\n    integer = int(float(value) * multiplier)\n\n    if LsbFirst:\n        formatcode = \'<\'  # Little-endian\n    else:\n        formatcode = \'>\'  # Big-endian\n    if signed:\n        formatcode += \'h\'  # (Signed) short (2 bytes)\n    else:\n        formatcode += \'H\'  # Unsigned short (2 bytes)\n\n    outstring = _pack(formatcode, integer)\n    #print(outstring)\n    assert len(outstring) == 2\n    return outstring\n\n\ndef _pack(formatstring, value):\n    _checkString(formatstring, description=\'formatstring\', minlength=1)\n\n    try:\n        result = struct.pack(formatstring, value)\n    except:\n        errortext = \'The value to send is probably out of range, as the num-to-bytestring conversion failed.\'\n        errortext += \' Value: {0!r} Struct format code is: {1}\'\n        raise ValueError(errortext.format(value, formatstring))\n\n    print(\"pack!\")\n    print(\"result en bytes:\")\n    print(result)\n    \n    if sys.version_info[0] > 2:\n        #return str(result, encoding=\'latin1\')  # Convert types to make it Python3 compatible\n        result = str(result,\"utf-8\")  # Convert types to make it Python3 compatible\n    print(\"result en str:\")\n    print(result)\n    return result\n\ndef _createBitpattern(functioncode, value):\n    _checkFunctioncode(functioncode, [5, 15])\n    _checkInt(value, minvalue=0, maxvalue=1, description=\'inputvalue\')\n\n    if functioncode == 5:\n        if value == 0:\n            print(\"aaaaaaaaaaaaaa devuelve 0\")\n            return str(\'\\x00\\x00\',\"utf-8\")\n        else:\n            print(\"bbbbbbbbbbbbbbb devuelve ff\")\n            s =  str(\'\\xff\\x00\',\"utf-8\")\n            print(s)\n            print(\"len:\")\n            print(len(s))\n            return s\n\n    elif functioncode == 15:\n        if value == 0:\n            return str(\'\\x00\',\"utf-8\")\n        else:\n            return str(\'\\x01\',\"utf-8\")  # Is this correct??\n\ndef _numToOneByteString(inputvalue):\n    _checkInt(inputvalue, minvalue=0, maxvalue=0xFF)\n    return chr(inputvalue)\n\n\n\n\n\n\n\n\n\n\n\n\n\ndef _checkString(inputstring, description, minlength=0, maxlength=None):\n    # Type checking\n    if not isinstance(description, str):\n        raise TypeError(\'The description should be a string. Given: {0!r}\'.format(description))\n\n    if not isinstance(inputstring, str):\n        raise TypeError(\'The {0} should be a string. Given: {1!r}\'.format(description, inputstring))\n\n    if not isinstance(maxlength, (int, type(None))):\n        raise TypeError(\'The maxlength must be an integer or None. Given: {0!r}\'.format(maxlength))\n\n    # Check values\n    _checkInt(minlength, minvalue=0, maxvalue=None, description=\'minlength\')\n\n    if len(inputstring) < minlength:\n        raise ValueError(\'The {0} is too short: {1}, but minimum value is {2}. Given: {3!r}\'.format( \\\n            description, len(inputstring), minlength, inputstring))\n\n    if not maxlength is None:\n        if maxlength < 0:\n            raise ValueError(\'The maxlength must be positive. Given: {0}\'.format(maxlength))\n\n        if maxlength < minlength:\n            raise ValueError(\'The maxlength must not be smaller than minlength. Given: {0} and {1}\'.format( \\\n                maxlength, minlength))\n\n        if len(inputstring) > maxlength:\n            raise ValueError(\'The {0} is too long: {1}, but maximum value is {2}. Given: {3!r}\'.format( \\\n                description, len(inputstring), maxlength, inputstring))\n\n\n\n_CRC16TABLE = (\n        0, 49345, 49537,   320, 49921,   960,   640, 49729, 50689,  1728,  1920, \n    51009,  1280, 50625, 50305,  1088, 52225,  3264,  3456, 52545,  3840, 53185, \n    52865,  3648,  2560, 51905, 52097,  2880, 51457,  2496,  2176, 51265, 55297, \n     6336,  6528, 55617,  6912, 56257, 55937,  6720,  7680, 57025, 57217,  8000, \n    56577,  7616,  7296, 56385,  5120, 54465, 54657,  5440, 55041,  6080,  5760, \n    54849, 53761,  4800,  4992, 54081,  4352, 53697, 53377,  4160, 61441, 12480, \n    12672, 61761, 13056, 62401, 62081, 12864, 13824, 63169, 63361, 14144, 62721, \n    13760, 13440, 62529, 15360, 64705, 64897, 15680, 65281, 16320, 16000, 65089, \n    64001, 15040, 15232, 64321, 14592, 63937, 63617, 14400, 10240, 59585, 59777, \n    10560, 60161, 11200, 10880, 59969, 60929, 11968, 12160, 61249, 11520, 60865, \n    60545, 11328, 58369,  9408,  9600, 58689,  9984, 59329, 59009,  9792,  8704, \n    58049, 58241,  9024, 57601,  8640,  8320, 57409, 40961, 24768, 24960, 41281, \n    25344, 41921, 41601, 25152, 26112, 42689, 42881, 26432, 42241, 26048, 25728, \n    42049, 27648, 44225, 44417, 27968, 44801, 28608, 28288, 44609, 43521, 27328, \n    27520, 43841, 26880, 43457, 43137, 26688, 30720, 47297, 47489, 31040, 47873, \n    31680, 31360, 47681, 48641, 32448, 32640, 48961, 32000, 48577, 48257, 31808, \n    46081, 29888, 30080, 46401, 30464, 47041, 46721, 30272, 29184, 45761, 45953, \n    29504, 45313, 29120, 28800, 45121, 20480, 37057, 37249, 20800, 37633, 21440, \n    21120, 37441, 38401, 22208, 22400, 38721, 21760, 38337, 38017, 21568, 39937, \n    23744, 23936, 40257, 24320, 40897, 40577, 24128, 23040, 39617, 39809, 23360, \n    39169, 22976, 22656, 38977, 34817, 18624, 18816, 35137, 19200, 35777, 35457, \n    19008, 19968, 36545, 36737, 20288, 36097, 19904, 19584, 35905, 17408, 33985, \n    34177, 17728, 34561, 18368, 18048, 34369, 33281, 17088, 17280, 33601, 16640, \n    33217, 32897, 16448)\n\ndef _hexencode(bytestring, insert_spaces = False):\n    _checkString(bytestring, description=\'byte string\')\n    separator = \'\' if not insert_spaces else \' \'\n    byte_representions = []\n    for c in bytestring:\n        byte_representions.append( \'{0:02X}\'.format(ord(c)) )\n    return separator.join(byte_representions).strip()\n\n\ndef _hexdecode(hexstring):\n    _checkString(hexstring, description=\'hexstring\')\n\n    if len(hexstring) % 2 != 0:\n        raise ValueError(\'The input hexstring must be of even length. Given: {!r}\'.format(hexstring))\n\n    try:\n        return hexstring.decode(\'hex\')\n    except TypeError as err:\n        raise TypeError(\'Hexdecode reported an error: {}. Input hexstring: {}\'.format(err.message, hexstring))\n\n\ndef _calculate_minimum_silent_period(baudrate):\n    _checkNumerical(baudrate, minvalue=1, description=\'baudrate\')  # Avoid division by zero\n    BITTIMES_PER_CHARACTERTIME = 11\n    MINIMUM_SILENT_CHARACTERTIMES = 3.5\n    bittime = 1 / float(baudrate)\n    return bittime * BITTIMES_PER_CHARACTERTIME * MINIMUM_SILENT_CHARACTERTIMES\n\ndef _extractPayload(response, slaveaddress, mode, functioncode):\n    BYTEPOSITION_FOR_ASCII_HEADER          = 0  # Relative to plain response\n\n    BYTEPOSITION_FOR_SLAVEADDRESS          = 0  # Relative to (stripped) response\n    BYTEPOSITION_FOR_FUNCTIONCODE          = 1\n\n    NUMBER_OF_RESPONSE_STARTBYTES          = 2  # Number of bytes before the response payload (in stripped response)\n    NUMBER_OF_CRC_BYTES                    = 2\n    NUMBER_OF_LRC_BYTES                    = 1\n    BITNUMBER_FUNCTIONCODE_ERRORINDICATION = 7\n\n    MINIMAL_RESPONSE_LENGTH_RTU            = NUMBER_OF_RESPONSE_STARTBYTES + NUMBER_OF_CRC_BYTES\n    MINIMAL_RESPONSE_LENGTH_ASCII          = 9\n\n    # Argument validity testing\n    _checkString(response, description=\'response\')\n    #_checkSlaveaddress(slaveaddress)\n    #_checkMode(mode)\n    _checkFunctioncode(functioncode, None)\n\n    plainresponse = response\n\n    # Validate response length\n    if mode == MODE_ASCII:\n        if len(response) < MINIMAL_RESPONSE_LENGTH_ASCII:\n            raise ValueError(\'Too short Modbus ASCII response (minimum length {} bytes). Response: {!r}\'.format( \\\n                MINIMAL_RESPONSE_LENGTH_ASCII,\n                response))\n    elif len(response) < MINIMAL_RESPONSE_LENGTH_RTU:\n            raise ValueError(\'Too short Modbus RTU response (minimum length {} bytes). Response: {!r}\'.format( \\\n                MINIMAL_RESPONSE_LENGTH_RTU,\n                response))\n\n    # Validate the ASCII header and footer.\n    if mode == MODE_ASCII:\n        if response[BYTEPOSITION_FOR_ASCII_HEADER] != _ASCII_HEADER:\n            raise ValueError(\'Did not find header ({!r}) as start of ASCII response. The plain response is: {!r}\'.format( \\\n                _ASCII_HEADER,\n                response))\n        elif response[-len(_ASCII_FOOTER):] != _ASCII_FOOTER:\n            raise ValueError(\'Did not find footer ({!r}) as end of ASCII response. The plain response is: {!r}\'.format( \\\n                _ASCII_FOOTER,\n                response))\n\n        # Strip ASCII header and footer\n        response = response[1:-2]\n\n        if len(response) % 2 != 0:\n            template = \'Stripped ASCII frames should have an even number of bytes, but is {} bytes. \' + \\\n                    \'The stripped response is: {!r} (plain response: {!r})\'\n            raise ValueError(template.format(len(response), response, plainresponse))\n\n        # Convert the ASCII (stripped) response string to RTU-like response string\n        response = _hexdecode(response)\n\n    # Validate response checksum\n    if mode == MODE_ASCII:\n        calculateChecksum = _calculateLrcString\n        numberOfChecksumBytes = NUMBER_OF_LRC_BYTES\n    else:\n        calculateChecksum = _calculateCrcString\n        numberOfChecksumBytes = NUMBER_OF_CRC_BYTES\n\n    receivedChecksum = response[-numberOfChecksumBytes:]\n    responseWithoutChecksum = response[0 : len(response) - numberOfChecksumBytes]\n    calculatedChecksum = calculateChecksum(responseWithoutChecksum)\n\n    if receivedChecksum != calculatedChecksum:\n        template = \'Checksum error in {} mode: {!r} instead of {!r} . The response is: {!r} (plain response: {!r})\'\n        text = template.format(\n                mode,\n                receivedChecksum,\n                calculatedChecksum,\n                response, plainresponse)\n        raise ValueError(text)\n\n    # Check slave address\n    responseaddress = ord(response[BYTEPOSITION_FOR_SLAVEADDRESS])\n\n    if responseaddress != slaveaddress:\n        raise ValueError(\'Wrong return slave address: {} instead of {}. The response is: {!r}\'.format( \\\n            responseaddress, slaveaddress, response))\n\n    # Check function code\n    receivedFunctioncode = ord(response[BYTEPOSITION_FOR_FUNCTIONCODE])\n\n    if receivedFunctioncode == _setBitOn(functioncode, BITNUMBER_FUNCTIONCODE_ERRORINDICATION):\n        raise ValueError(\'The slave is indicating an error. The response is: {!r}\'.format(response))\n\n    elif receivedFunctioncode != functioncode:\n        raise ValueError(\'Wrong functioncode: {} instead of {}. The response is: {!r}\'.format( \\\n            receivedFunctioncode, functioncode, response))\n\n    # Read data payload\n    firstDatabyteNumber = NUMBER_OF_RESPONSE_STARTBYTES\n\n    if mode == MODE_ASCII:\n        lastDatabyteNumber = len(response) - NUMBER_OF_LRC_BYTES\n    else:\n        lastDatabyteNumber = len(response) - NUMBER_OF_CRC_BYTES\n\n    payload = response[firstDatabyteNumber:lastDatabyteNumber]\n    return payload\n\0"
};
