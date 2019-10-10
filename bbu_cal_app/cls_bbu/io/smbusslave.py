#! /usr/bin/python

"""
#*******************************************************************************
# Copyright (C) 2018 Celestica. All Rights Reserved.

# Redistribution and use in source and binary forms, with or without modification,

# are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright notice,

# this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

# 3. All rights, title and interest in and to Celestica's intellectual property rights and the Software, 
#including all technology, associated documentation, data and information are and shall remain the exclusive property of Celestica or its licensors.
#You will not and will not attempt to reverse engineer, decompile or disassemble the Software. 
#You shall not modify or change the Software or make any derivative works based on the Software or permit any third party to do so.
#The name of Celestica may not be used to endorse or promote products derived from this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY CELESTICA "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 

# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.

# IN NO EVENT SHALL CELESTICA BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,

# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 

# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,

# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,

# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Export.  The Software is subject to any and all laws, regulations, 

# orders or other restrictions relative to export,

# re-export or redistribution of the Software that may now or in the future be imposed by the government of the United States or foreign governments.

# You agree to comply with all applicable import and export control laws and regulations.

# You will not export, re-export, divert, transfer or disclose, 

# directly or indirectly the Software and any related documentation or materials without strictly complying with all applicable export control laws and regulations. 

#****************************************************************************
"""
"""
    Smart Battery Interface
"""
import sys
import time
import datetime
import subprocess
import random
import re
from smbus import SMBus

def apply_command(raw_cmd):
#    print "raw Exec Command: %s" % (raw_cmd)
    cmd = tuple(raw_cmd.split(' '))
    cmdexec = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    value = ""
    last_clear = False
    while True:
        line = cmdexec.stdout.readline()
        while line != "":
            value += line
            line = cmdexec.stdout.readline()
        line = cmdexec.stderr.readline()
        while line != "":
            print >> sys.stderr, "%s" % (line)
            line = cmdexec.stderr.readline()
        ret = cmdexec.poll()
        if ret != None:
            if last_clear == True:
                break
            last_clear = True
    if ret != 0:
       raise OSError("cmd: %s\nExitcode: %d" % (raw_cmd, ret))
    return value

def ipmi_value_filter(value):
    ipmi_ret = ""
    lines = value.split('\n')
    for line in lines:
        if line.isspace():
            continue
        mret = re.match("Data length", line)
        if mret != None:
            continue
        sret = re.search("( *[0-9a-fA-F]{2,2} *).", line)
        if sret != None:
            ipmi_ret += line
    return ipmi_ret

#---------------------------------------------------------------------------
# SMBus Slave
#---------------------------------------------------------------------------
class SMBusSlave(object):
    def __init__(self, bus, address):
        """
        bus 8bit integer
        address 7bit integer
        """
        if bus < 0 or bus > 255 or address < 0 or address > 127:
            raise ValueError("invalid bus or address!")
        self.bus = bus
        self.address = address
        self.interval = 0.1

    def __str__(self):
        ret = "SMBus Slave: Bus %d, Address %d" % (self.bus, self.address)
        return ret

    def get_bus(self):
        return self.bus

    def get_address(self):
        return self.address

    def get_byte(self, offset):
        """Return a string hex without prefix '0x'"""
        raise NotImplementedError

    def set_byte(self, offset, data):
        raise NotImplementedError

    def get_word(self, offset):
        """Return a string hex without prefix '0x'"""
        raise NotImplementedError

    def set_word(self, offset, data):
        """Input integer"""
        raise NotImplementedError

    def get_block(self, offset, length):
        """Input integer """
        """Return a list of string without prefix '0x'"""
        raise NotImplementedError

    def set_block(self, offset, data):
        """Input integer """
        raise NotImplementedError

class DebugSlave(SMBusSlave):
    def __init__(self, bus, address):
        SMBusSlave.__init__(self, bus, address)

    def get_byte(self, offset):
        ret = hex(random.randint(0x00, 0xff))
        return data[2:].zfill(2)
    
    def set_byte(self, offset, data):
        pass
    
    def get_word(self, offset):
        ret = "%x%x" % (random.randint(0x00, 0xff), random.randint(0x00, 0xff))
        time.sleep(self.interval)
        return ret
    
    def set_word(self, offset, data):
        pass
    
    def get_block(self, offset, length):
        data = []
        for i in range(length):
            value = hex(random.randint(0x61, 0x7A))
            data.append(value[2:].zfill(2))
        time.sleep(self.interval)
        return data
    
    def set_block(self, bus, address, offset, data):
        pass

class BMCSlaveNet(SMBusSlave):
    def __init__(self, bus, address, host, user, passwd):
        SMBusSlave.__init__(self, bus, address)
        self.bus = bus * 2 + 1
        self.address = address * 2
        self.host = host
        self.user = user
        self.passwd = passwd

    def get_byte(self, offset):
        cmd = "ipmitool -H %s -U %s -P %s raw 0x3A 0x18 %d %d 1 %d" % \
              (self.host, self.user, self.passwd, self.bus, self.address, offset)
        ret = apply_command(cmd)
        ret = ipmi_value_filter(ret)
        return ret
    
    def set_byte(self, offset, data):
        raise NotImplementedError
    
    def get_word(self, offset):
        cmd = "ipmitool -H %s -U %s -P %s raw 0x3A 0x18 %d %d 2 %d" % \
              (self.host, self.user, self.passwd, self.bus, self.address, offset)
        data = apply_command(cmd)
        data = ipmi_value_filter(data)
        data = data.split()
        ret = "%s%s" % (data[1], data[0])
        time.sleep(self.interval)
        return ret
    
    def set_word(self, offset, data):
        cmd = "ipmitool -H %s -U %s -P %s raw 0x3A 0x18 %d %d 0x00 %d %d" % \
              (self.host, self.user, self.passwd, self.bus, self.address, offset, data)
        ret = apply_command(cmd)
    
    def get_block(self, offset, length):
        cmd = "ipmitool -H %s -U %s -P %s raw 0x3A 0x18 %d %d %d %d" % \
              (self.host, self.user, self.passwd, self.bus, self.address, length, offset)
        data = apply_command(cmd)
        data = ipmi_value_filter(data)
        ret = data.split()
        time.sleep(self.interval)
        return ret
    
    def set_block(self, offset, data):
        cmd = "ipmitool -H %s -U %s -P %s raw 0x3A 0x18 %d %d 0x00 %d %d" % \
              (self.host, self.user, self.passwd, self.bus, self.address, offset, data)
        res = apply_command(cmd)
        res = ipmi_value_filter(res)
        ret = res.split()
        return ret

class LocalhostSMBusSlave(SMBusSlave):
    def __init__(self, bus, address):
        SMBusSlave.__init__(self, bus, address)
        self.bus = SMBus(bus)
        self.address = address

    #return: one byte hex string without '0x': '12'
    def get_byte(self, offset):
#        print "%s %s" % (datetime.datetime.utcnow().strftime("%H:%M:%S.%f")[:-4], sys._getframe().f_code.co_name)
        ret = self.bus.read_byte_data(self.address, offset)
        time.sleep(self.interval)
        data = hex(ret)
        return data[2:].zfill(2)
    
    #input: offset:str, data:int
    def set_byte(self, offset, data):
#        print "%s %s" % (datetime.datetime.utcnow().strftime("%H:%M:%S.%f")[:-4], sys._getframe().f_code.co_name)
        self.bus.write_byte_data(self.address, offset, data)
        time.sleep(self.interval)
    
    #return: two byte hex string without '0x': '1234'
    def get_word(self, offset):
#        print "%s %s" % (datetime.datetime.utcnow().strftime("%H:%M:%S.%f")[:-4], sys._getframe().f_code.co_name)
        ret = self.bus.read_word_data(self.address, offset)
        data = hex(ret)
        time.sleep(self.interval)
        return data[2:].zfill(4)

    def set_word(self, offset, data):
#        print "%s %s" % (datetime.datetime.utcnow().strftime("%H:%M:%S.%f")[:-4], sys._getframe().f_code.co_name)
        if sys.byteorder == "big" : 
            data = ((data & 0xff00) >> 8) + ((data & 0x00ff) << 8)
        self.bus.write_word_data(self.address, offset, data)
        time.sleep(self.interval)
    
    #return: hex string list per byte,without '0x' :['12', '0a']
    def get_block(self, offset, length="32"):
#        print "%s %s" % (datetime.datetime.utcnow().strftime("%H:%M:%S.%f")[:-4], sys._getframe().f_code.co_name)
        ret = self.bus.read_block_data(self.address, offset)
        time.sleep(self.interval)
        hex_ret = [hex(i)[2:].zfill(2) for i in ret]
        return hex_ret
    
    def set_block(self, offset, data):
#        print "%s %s" % (datetime.datetime.utcnow().strftime("%H:%M:%S.%f")[:-4], sys._getframe().f_code.co_name)
        data_arr = [(data & 0x00ff), ((data & 0xff00) >> 8)]
        #self.bus.write_i2c_block_data(self.address, offset, data_arr)
        self.bus.write_block_data(self.address, offset, data_arr)
        time.sleep(self.interval)

class LocalhostI2CToolSlave(SMBusSlave):
    def __init__(self, bus, address):
        SMBusSlave.__init__(self, bus, address)
        self.bus = bus
        self.address = address

    def get_byte(self, offset):
        cmd = "i2cget -f -y %d %d %d b" % (bus, address, offset)
        ret = apply_command(cmd)
        return ret[:-1]
    
    def set_byte(self, offset, data):
        raise NotImplementedError
    
    def get_word(self, offset):
        cmd = "i2cget -f -y %d %d %d w" % (bus, address, offset)
        ret = apply_command(cmd)
        time.sleep(self.interval)
        return ret[:-1]

    def set_word(self, offset, data):
        raise NotImplementedError
    
    def get_block(self, offset, length="32"):
        raise NotImplementedError
    
    def set_block(self, offset, data):
        raise NotImplementedError


