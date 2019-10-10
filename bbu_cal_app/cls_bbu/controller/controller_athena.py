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

from cls_bbu.io import smbusslave
from cls_bbu.controller.controller import BBU_Controller

#PCA9551
LED_REG = {
        "LOW"  : 0,
        "HIGH" : 1,
#        "PWM0" : 2,
#        "PWM1" : 3,
        "LS0"  : "5",
        "LS1"  : "6"
}

PCA9551_LS = [0x05, 0x06] #LS0 = "0x05", LS1 = "0x06"

Athena_9551_LED = {
        "PRSNT_BBU_L"  : 0,
        "CHG_RATE"     : 1,
        "DUMP_ARMED_L" : 2,
        "TEST_L"       : 3,
        "CHG_EN_L"     : 4,
        "CAL_L"        : 6
}

class Athena_BBU_Controller(BBU_Controller):
    def __init__(self,bus):
        self.bus = bus
#        BBU_Controller.__init__(self)
        self.__slave = smbusslave.LocalhostSMBusSlave(bus, 0x67)

    def __get_9551_status(self):
        status = []
        status.append(int(self.__slave.get_byte(PCA9551_LS[0]), 16))
        status.append(int(self.__slave.get_byte(PCA9551_LS[1]), 16))
        return status

    #input: led_id:int, value:int,[0,3]
    def __set_9551_data(self, led_id, value):
        ls = led_id / 4
        shift = (led_id % 4) * 2
        mask = ~(0x03 << shift)
        data = int(self.__slave.get_byte(PCA9551_LS[ls]), 16)
        data &= mask
        data |= (value << shift)
        self.__slave.set_byte(PCA9551_LS[ls], data)

    def get_status(self):
        data = {}
        status = self.__get_9551_status()
        data['PRSNT_BBU_L'] = status[0] & 0x03
        data['CHG_RATE'] = (status[0] >> 2) & 0x03
        data['DUMP_ARMED_L'] = (status[0] >> 4) & 0x03
        data['TEST_L'] = (status[0] >> 6) & 0x03
        data['CHG_EN_L'] = status[1] & 0x03
        data['CAL_L'] = (status[1] >> 4) & 0x03
        return data

    def charge(self, enable):
        if enable:
            self.__set_9551_data(Athena_9551_LED['CHG_EN_L'], LED_REG['LOW'])
        else:
            self.__set_9551_data(Athena_9551_LED['CHG_EN_L'], LED_REG['HIGH'])

    def calibrate(self, enable):
        if enable:
            self.__set_9551_data(Athena_9551_LED['CAL_L'], LED_REG['LOW'])
        else:
            self.__set_9551_data(Athena_9551_LED['CAL_L'], LED_REG['HIGH'])

    def dump_armed(self, enable):
        if enable:
            self.__set_9551_data(Athena_9551_LED['DUMP_ARMED_L'], LED_REG['LOW'])
        else:
            self.__set_9551_data(Athena_9551_LED['DUMP_ARMED_L'], LED_REG['HIGH'])

    def reduce_power(self, enable):
        pass

    def test(self, enable):
        if enable:
            self.__set_9551_data(Athena_9551_LED['TEST_L'], LED_REG['LOW'])
        else:
            self.__set_9551_data(Athena_9551_LED['TEST_L'], LED_REG['HIGH'])

    def charge_rate(self, high):
        pass

    def present(self):
        return True

