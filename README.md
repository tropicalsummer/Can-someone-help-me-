//This is the error that i have encountered.
Traceback (most recent call last):
  File "C:\Users\USER\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\3.0.2/tools/upload.py", line 66, in <module>
    esptool.main(cmdline)
  File "C:/Users/USER/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/tools/esptool\esptool.py", line 3552, in main
    esp.connect(args.before, args.connect_attempts)
  File "C:/Users/USER/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/tools/esptool\esptool.py", line 519, in connect
    last_error = self._connect_attempt(mode=mode, esp32r0_delay=False)
  File "C:/Users/USER/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/tools/esptool\esptool.py", line 499, in _connect_attempt
    self.sync()
  File "C:/Users/USER/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/tools/esptool\esptool.py", line 438, in sync
    timeout=SYNC_TIMEOUT)
  File "C:/Users/USER/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/tools/esptool\esptool.py", line 376, in command
    self.write(pkt)
  File "C:/Users/USER/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/tools/esptool\esptool.py", line 339, in write
    self._port.write(buf)
  File "C:/Users/USER/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/tools/pyserial\serial\serialwin32.py", line 325, in write
    raise SerialTimeoutException('Write timeout')
serial.serialutil.SerialTimeoutException: Write timeout
Failed uploading: uploading error: exit status 1
