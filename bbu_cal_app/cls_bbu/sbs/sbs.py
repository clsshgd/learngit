#! /usr/bin/python
#
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
from cls_bbu.io.iodata import *

#---------------------------------------------------------------------------
# Smart Battery System Command Set
#---------------------------------------------------------------------------
sbs_cmd = {
  "ManufacturerAccess"     : 0x00 , 
  "RemainingCapacityAlarm" : 0x01 , 
  "RemainingTimeAlarm"     : 0x02 , 
  "BatteryMode"            : 0x03 , 
  "AtRate"                 : 0x04 , 
  "AtRateTimeToFull"       : 0x05 , 
  "AtRateTimeToEmpty"      : 0x06 , 
  "AtRateOK"               : 0x07 , 
  "Temperature"            : 0x08 , 
  "Voltage"                : 0x09 , 
  "Current"                : 0x0a , 
  "AverageCurrent"         : 0x0b , 
  "MaxError"               : 0x0c , 
  "RelativeStateOfCharge"  : 0x0d , 
  "AbsoluteStateOfCharge"  : 0x0e , 
  "RemainingCapacity"      : 0x0f , 
  "FullChargeCapacity"     : 0x10 , 
  "RunTimeToEmpty"         : 0x11 , 
  "AverageTimeToEmpty"     : 0x12 , 
  "AverageTimeToFull"      : 0x13 , 
  "ChargingCurrent"        : 0x14 , 
  "ChargingVoltage"        : 0x15 , 
  "BatteryStatus"          : 0x16 , 
  "CycleCount"             : 0x17 , 
  "DesignCapacity"         : 0x18 , 
  "DesignVoltage"          : 0x19 , 
  "SpecificationInfo"      : 0x1a , 
  "ManufacturerDate"       : 0x1b , 
  "SerialNumber"           : 0x1c , 
  "ManufacturerName"       : 0x20 , 
  "DeviceName"             : 0x21 , 
  "DeviceChemistry"        : 0x22 , 
  "ManufacturerData"       : 0x23 , 
  "OptionalMfgFunction5"   : 0x2f , 
  "OptionalMfgFunction4"   : 0x3c , 
  "OptionalMfgFunction3"   : 0x3d , 
  "OptionalMfgFunction2"   : 0x3e , 
  "OptionalMfgFunction1"   : 0x3f 
}

#---------------------------------------------------------------------------
# Smart Battery System
#---------------------------------------------------------------------------
class SmartBattery(object):
    def __init__(self, slave):
        self.slave = slave
        self.battery_mode = self.BatteryMode()

    def __str__(self):
        return "None"
    
    def ManufacturerAccess(self, cmd=None):
        if cmd == None :
            pass
        else:
            cmd_name = sys._getframe().f_code.co_name
            self.slave.set_word(sbs_cmd[cmd_name], cmd)
    
    def RemainingCapacityAlarm(self):
        pass

    def RemainingTimeAlarm(self):
        pass

    def BatteryMode(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
#        rawdata="0000"
        data = BatteryModeData(name = cmd,
                        units = "Not applicable",
                        data_range = "0...1",
                        granularity = "Not applicable",
                        accuracy = "Not applicable",
                        rawdata = rawdata)
        return data

    def AtRate(self):
        pass

    def AtRateTimeToFull(self):
        pass

    def AtRateTimeToEmpty(self):
        pass

    def AtRateOk(self):
        pass

    def Temperature(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        data = UnsignedIntData(name = cmd,
                                units_multiplier = 0.1,
                                units = "K",
                                data_range = "0 to 6553.5 K",
                                granularity = "0.4K or better",
                                accuracy = "+-3K",
                                rawdata = rawdata)
        return data

    def Voltage(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        data = UnsignedIntData(name = cmd,
                           units = "mV",
                           data_range = "0 to 65535 mV",
                           granularity = None,
                           accuracy = None,
                           rawdata = rawdata)
        return data

    def Current(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        data = SignedIntData(name = cmd,
                              units = "mA",
                              data_range = "-32768 to 32767 mA",
                              granularity = None,
                              accuracy = None,
                              rawdata = rawdata)
        return data

    def AverageCurrent(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        data = SignedIntData(name = cmd,
                              units = "mA",
                              data_range = "-32768 to 32767 mA",
                              granularity = None,
                              accuracy = None,
                              rawdata = rawdata)
        return data

    def MaxError(self):
        pass

    def RelativeStateOfCharge(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        data = UnsignedIntData(name = cmd,
                                units = "%",
                                data_range = "0 to 100%",
                                granularity = "1%",
                                accuracy = None,
                                rawdata = rawdata)
        return data

    def AbsoluteStateOfCharge(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        data = UnsignedIntData(name = cmd,
                                units = "%",
                                data_range = "0 to 100%",
                                granularity = "1%",
                                accuracy = None,
                                rawdata = rawdata)
        return data


    def RemainingCapacity(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        if self.battery_mode.capacity_mode() == 0:
            data = UnsignedIntData(name = cmd,
                                    units = "mAh",
                                    data_range = "0 to 65535 mAh",
                                    granularity = None,
                                    accuracy = None,
                                    rawdata = rawdata)
        else:
            data = UnsignedIntData(name = cmd,
                                    units_multiplier = 10,
                                    units = "mWh",
                                    data_range = "0 to 65535 10mWh",
                                    granularity = None,
                                    accuracy = None,
                                    rawdata = rawdata)
        return data

    def FullChargeCapacity(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        if self.battery_mode.capacity_mode() == 0:
            data = UnsignedIntData(name = cmd,
                                    units = "mAh",
                                    data_range = "0 to 65535 mAh",
                                    granularity = None,
                                    accuracy = None,
                                    rawdata = rawdata)
        else:
            data = UnsignedIntData(name = cmd,
                                    units_multiplier = 10,
                                    units = "mWh",
                                    data_range = "0 to 65535 10mWh",
                                    granularity = None,
                                    accuracy = None,
                                    rawdata = rawdata)
        return data

    def RunTimeToEmpty(self):
        pass

    def AverageTimeToEmpty(self):
        pass

    def AverageTimeToFull(self):
        pass
    
    def ChargingCurrent(self):
        pass

    def ChargingVoltage(self):
        pass

    def BatteryStatus(self):
        pass

    def CycleCount(self):
        pass

    def DesignCapacity(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        if self.battery_mode.capacity_mode() == 0:
            data = UnsignedIntData(name = cmd,
                                    units = "mAh",
                                    data_range = "0 to 65535 mAh",
                                    granularity = None,
                                    accuracy = None,
                                    rawdata = rawdata)
        else:
            data = UnsignedIntData(name = cmd,
                                    units_multiplier = 10,
                                    units = "mWh",
                                    data_range = "0 to 65535 10mWh",
                                    granularity = None,
                                    accuracy = None,
                                    rawdata = rawdata)
        return data

    def DesignVoltage(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        data = UnsignedIntData(name = cmd,
                           units = "mV",
                           data_range = "0 to 65535 mV",
                           granularity = None,
                           accuracy = None,
                           rawdata = rawdata)
        return data

    def SpecificationInfo(self):
        pass

    def ManufacturerDate(self):
        pass

    def SerialNumber(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_word(sbs_cmd[cmd])
        data = UnsignedIntData(name = cmd,
                           units = "",
                           data_range = None,
                           granularity = None,
                           accuracy = None,
                           rawdata = rawdata)
        return data

    def ManufacturerName(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_block(sbs_cmd[cmd])
        data = StringBlockData(name = cmd,
                           units = "",
                           data_range = None,
                           granularity = None,
                           accuracy = None,
                           rawdata = rawdata)
        return data

    def DeviceName(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_block(sbs_cmd[cmd])
        data = StringBlockData(name = cmd,
                           units = "",
                           data_range = None,
                           granularity = None,
                           accuracy = None,
                           rawdata = rawdata)
        return data

    def DeviceChemistry(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.slave.get_block(sbs_cmd[cmd])
        data = StringBlockData(name = cmd,
                           units = "",
                           data_range = None,
                           granularity = None,
                           accuracy = None,
                           rawdata = rawdata)
        return data

    def ManufacturerData(self):
        pass
#        rawdata = self.slave.get_block(sbs_cmd["ManufacturerData"])
#        return rawdata


