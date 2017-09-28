#!/usr/bin/python
# Simple script to record the battery voltage info and shutdown the pi zero
# if we receive instruction to from the battery monitor.
#
# Must be run as root for the shutdown command to work
SERIALPORT='/dev/ttyAMA0'  # TTY serial port on rPi Zero

import os
import sys
import time

import serial

def InitPort(port=SERIALPORT):
  try:
    ser = serial.Serial(
        port=port,
        baudrate=57600,
        parity=serial.PARITY_NONE,
        bytesize=serial.EIGHTBITS,
        stopbits=serial.STOPBITS_ONE,
    )
    ser.isOpen()
  except serial.SerialException, e:
    print "Error: %s" % e
    sys.exit(1)
  return ser

def ReadSer(ser, timeout=3):
  out = ''
  start = time.time()
  while time.time() - start < timeout:
    if ser.inWaiting() > 0:
      out += ser.read(1)
    if out[-1] == '\n':
      break;
  return out


ser = InitPort()
run = ''
reading = ''
while True:
  if ser.inWaiting():
    x = ReadSer(ser)
    if x[:5] == 'Vbat:':
      reading = x[6:].strip()
      now = time.strftime('%Y-%m-%d %H:%M:%S')
    if x[:4] == 'run:':
      run = x[5]
    if run == '0':
      print('LOW BATT: SHUTDOWN')
      with open('battmon.log', 'a') as f:
        f.write('%s,shutdown\n' % time.strftime('%Y-%m-%d %H:%M:%S'))
      os.system("/sbin/shutdown -h now")
    if reading and run:
      line = '%s,%s,%s' % (now, reading, run)
      with open('battmon.log', 'a') as f:
        f.write(line + '\n')
      reading = ''
      run = ''
  time.sleep(.1)
