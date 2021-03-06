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

from cls_bbu.sbs import sbs
#---------------------------------------------------------------------------
# BBU (Backup Battery Unit)
#---------------------------------------------------------------------------
class BBU(object):
    def __init__(self, battery, controller=None):
        self.battery = battery
        self.controller = controller

    def get_data(self, cmd):
        func = getattr(self.battery, cmd, None)
        data = func()
        return data

    def set_data(self, cmd, data):
        func = getattr(self.battery, cmd, None)
        return func(data)

    def display_basic(self):
        print self.battery.Temperature()
        print self.battery.Voltage()
        print self.battery.Current()
        print self.battery.RelativeStateOfCharge()
        print self.battery.RemainingCapacity()
        print self.battery.FullChargeCapacity()

    def display_v(self):
        print self.battery.BatteryMode()
        print self.battery.Temperature()
        print self.battery.Voltage()
        print self.battery.Current()
        print self.battery.AverageCurrent()
        print self.battery.RelativeStateOfCharge()
        print self.battery.AbsoluteStateOfCharge()
        print self.battery.RemainingCapacity()
        print self.battery.FullChargeCapacity()
        print self.battery.DesignCapacity()
        print self.battery.DesignVoltage()
        print self.battery.SerialNumber()
        print self.battery.ManufacturerName()
        print self.battery.DeviceName()
        print self.battery.DeviceChemistry()

    def display_vv(self):
        for key in sbs.sbs_cmd:
            cmd = getattr(self.battery, key, None)
            output = None
            if cmd != None:
                output = cmd()
            if output != None:
                print output
    
    def display_vvv(self):
        for key in sbs.sbs_cmd:
            cmd = getattr(self.battery, key, None)
            output = None
            if cmd != None:
                output = cmd()
            if output != None:
                print output.dump()
