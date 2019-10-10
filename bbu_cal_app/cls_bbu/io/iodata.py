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
import subprocess
import random
import re
from smbus import SMBus

#---------------------------------------------------------------------------
# Smart Battery Input and Output Data
# Package Input and Output data.
#--------------------------------------------------------------------------
#input: rawdata: hex string, w/o "0x".

class IOData(object):
    def __init__(self, **kwargs):
        self.name = kwargs.get("name")
        self.units = kwargs.get("units")
        self.data_range = kwargs.get("data_range")
        self.granularity = kwargs.get("granularity")
        self.accuracy = kwargs.get("accuracy")
        self.rawdata = kwargs.get("rawdata")
        self.units_multiplier = kwargs.get("units_multiplier")
        if self.units_multiplier == None:
            self.units_multiplier = 1
        self.value = self.__repr__()

    def __repr__(self):
        return self.rawdata

    def __str__(self):
        ret = "%s : %s %s (%s)" % (self.name.ljust(25), self.value, self.units, self.rawdata)
        return ret

    def dump(self):
        ret = "%s\n" % (self.name)
        ret += "\tUnit           : %s\n" % (self.units)
        ret += "\tRange          : %s\n" % (self.data_range)
        ret += "\tGranularity    : %s\n" % (self.granularity)
        ret += "\tAccuracy       : %s\n" % (self.accuracy)
        ret += "\tValue          : %s\n" % (self.value)
        ret += "\tRawdata        : (%s)\n" % (self.rawdata)
        return ret

class BinaryData(IOData):
    def __init__(self, **kwargs):
        IOData.__init__(self, **kwargs)
        
    def __repr__(self):
        value = int(self.rawdata, 16)
        return value

    def __str__(self):
        ret = "%s : %s (%s)" % (self.name.ljust(25), bin(self.value), self.rawdata)
        return ret

    def dump(self):
        ret = "%s\n" % (self.name)
        ret += "\tValue          : %s\n" % (bin(self.value))
        ret += "\tRawdata        : (%s)\n" % (self.rawdata)
        return ret


class BatteryModeData(IOData):
    def __init__(self, **kwargs):
        IOData.__init__(self, **kwargs)

    def __repr__(self):
        value = int(self.rawdata, 16)
        return value

    def __str__(self):
        ret = "%s : %s (%s)" % ("BatteryMode".ljust(25), bin(self.value), self.rawdata)
        return ret

    def dump(self):
        ret = "%s\n" % (self.name)
        ret += "\tUnit           : %s\n" % (self.units)
        ret += "\tRange          : %s\n" % (self.data_range)
        ret += "\tGranularity    : %s\n" % (self.granularity)
        ret += "\tAccuracy       : %s\n" % (self.accuracy)
        ret += "\tValue          : %s " % (bin(self.value))
        if self.capacity_mode() == 0:
            ret += "(Report in mA or mAh)\n"
        else:
            ret += "(Report in 10mW or 10mWh)\n"
        ret += "\tRawdata        : (%s)\n" % (self.rawdata)
        return ret

    def capacity_mode(self):
        capmode = (self.value >> 15) & 1
        return capmode

class SignedIntData(IOData):
    def __init__(self, **kwargs):
        IOData.__init__(self, **kwargs)

    def __repr__(self):
        value = int(self.rawdata, 16)
        if value > 0x8000:
            value = -(0x10000 - value)
        value = value * self.units_multiplier
        return value

class UnsignedIntData(IOData):
    def __init__(self, **kwargs):
        IOData.__init__(self, **kwargs)

    def __repr__(self):
        value = int(self.rawdata, 16)
        value = value * self.units_multiplier
        return value

class BooleanData(IOData):
    def __init__(self):
        """
        True non-zero
        False zero
        """
        pass

class StringBlockData(IOData):
    def __init__(self, **kwargs):
        IOData.__init__(self, **kwargs)

    def __repr__(self):
        value = ''.join(chr(int(x, 16)) for x in self.rawdata)
        return value
    
    def dump(self):
        ret = "%s\n" % (self.name)
        ret += "\tValue          : %s\n" % (self.value)
        ret += "\tRawdata        : (%s)\n" % (self.rawdata)
        return ret

# subblcok is a tuple list: [("name", lens_of_bytes)]
# sg. [('version', 2), ('name', 1)]
# input: rawdata: hex string list.
# output: __repr__: name,hex_string(no '0x' head) list.
class HexBlockData(IOData):
    def __init__(self, **kwargs):
        self.subblock = kwargs.get("subblock")
        IOData.__init__(self, **kwargs)
        
    def __repr__(self):
        value = {}
        i = 0
        for index in range(len(self.subblock)):
            data_len = self.subblock[index][1]
            value[self.subblock[index][0]] = ''.join(self.rawdata[x] for x in range(i + data_len-1, i-1, -1))
            i += data_len
        return value

    def __str__(self):
        ret = ""
        for (key, value) in self.value.items():
            if ret != "":
                ret += "\n"
            name = self.name + " " + key
            ret += "%s : 0x%s" % (name.ljust(25), value)
        name = self.name + " Rawdata"
        ret += "\n    %s : %s" % (name.ljust(21), self.rawdata)
        return ret

    def dump(self):
        ret = "%s\n" % (self.name)
        for (key, value) in self.value.items():
            if key=="" or key==None:
                key="Value"
            ret += "\t%s_%s: %s\n" % (self.name, key.ljust(15), value)
        ret += "\tRawdata        : (%s)\n" % (self.rawdata)
        return ret

# subblcok is a tuple list: [("name", lens_of_bits)]
# sg. [('version', 2), ('name', 1)]
# input: rawdata: hex string list.
# output: __repr__: bit_name_string,bit_value_string list
class BinaryBlockData(IOData):
    def __init__(self, **kwargs):
        self.subblock = kwargs.get("subblock")
        IOData.__init__(self, **kwargs)
        self.rawdata.reverse()
        self.value = self.__repr__()
        
    def __repr__(self):
        value = {}
        bit_size = len(self.rawdata) * 8
        binstr = bin(int(''.join(self.rawdata), 16))[2:].zfill(bit_size)
#        for (name, offset) in self.subblock.items():
        for index in range(len(self.subblock)):
            name = self.subblock[index][0]
            offset = self.subblock[index][1]
            if offset < bit_size:
                value[name] = binstr[(-1) - offset]
            else:
                value[name] = 'out-of-range'

        return value

    def __str__(self):
        ret = ""
        data = ''.join(x for x in self.rawdata)
        ret += "%s : 0x%s (%s)" % (self.name.ljust(25), data, self.rawdata)
        return ret

    def dump(self):
        ret = "%s\n" % (self.name)
        for (key, data) in self.value.items():
            if key=="" or key==None:
                key="Value"
            ret += "\t%s_%s: %s\n" % (self.name,key.ljust(15),data)
        ret += "\tRawdata        : (%s)\n" % (self.rawdata)
        return ret

