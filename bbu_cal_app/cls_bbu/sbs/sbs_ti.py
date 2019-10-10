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
from collections import OrderedDict
from cls_bbu.io.iodata import *
from cls_bbu.sbs.sbs import *


#---------------------------------------------------------------------------
# TI ManufacturerAccess Command Set
#---------------------------------------------------------------------------
ti_mac_cmd = {
  "DeviceType"      : 0x0001 ,
  "FirmwareVersion" : 0x0002 ,
  "Gauging"         : 0x0021 ,
  "SealDevice"      : 0x0030 ,
  "ManufacturerBlockAccess"      : 0x0044 ,
  "SafetyStatus"    : 0x0051 ,
  "PFStatus"        : 0x0053 ,
  "OperationStatus" : 0x0054 , 
  "ChargingStatus"  : 0x0055 ,
  "GaugingStatus"   : 0x0056 ,
  "ManufacturingStatus" : 0x0057 ,
  "LifetimeDataBlock2"  : 0x0061 ,
  "DAStatus1"       : 0x0071 ,
  "GaugeStatus1"    : 0x0073 
}
#0x0001
ti_devicetype = OrderedDict([
    ("Device Number"    , 2)
])
#0x0002
ti_firmwareversion = OrderedDict([
    ("Device Number"    , 2),
    ("Version"          , 2),
    ("Build Number"     , 2),
    ("Firmware Type"    , 1),
    ("Impedance Track Version" , 2)
])
#0x0035
ti_securitykeys = OrderedDict([
    ("UNSEAL key 1"     , 2),
    ("UNSEAL key 2"     , 2),
    ("FULL ACESS key 1" , 2),
    ("FULL ACESS key 2" , 2)
])
#0x0051
ti_safetystatus = OrderedDict([
   ("UTD"     , 27),
   ("UTC"     , 26),
   ("PCHGC"   , 25),
   ("CHGV"    , 24),
   ("CHGC"    , 23),
   ("OC"      , 22),
   ("CTO"     , 20),
   ("PTO"     , 18),
   ("OTF"     , 16),
   ("CUVC"    , 14),
   ("OTD"     , 13),
   ("OTC"     , 12),
   ("ASCDL"   , 11),
   ("ASCCL"   , 9),
   ("ASCC"    , 8),
   ("AOLDL"   , 7),
   ("AOLD"    , 6),
   ("OCD2"    , 5),
   ("OCD1"    , 4),
   ("OCC2"    , 3),
   ("OCC1"    , 2),
   ("COV"     , 1),
   ("CUV"     , 0)
])
#0x0054
ti_operationstatus = OrderedDict([
    ("EMSHUT"   , 29),
    ("CB"       , 28),
    ("SLPCC"    , 27),
    ("SLPAD"    , 26),
    ("SMBLCAL"  , 25),
    ("INIT"     , 24),
    ("SLEEPM"   , 23),
    ("XL"       , 22),
    ("CAL_OFFSET"   , 21),
    ("CAL"      , 20),
    ("AUTOCALM" , 19),
    ("AUTH"     , 18),
    ("LED"      , 17),
    ("SDM"      , 16),
    ("SLEEP"    , 15),
    ("XCHG"     , 14),
    ("XDSG"     , 13),
    ("PF"       , 12),
    ("SS"       , 11),
    ("SDV"      , 10),
    ("SEC1"     , 9),
    ("SEC0"     , 8),
    ("BTP_INT"  , 7),
    ("FUSE"     , 5),
    ("PCHG"     , 3),
    ("CHG"      , 2),
    ("DSG"      , 1),
    ("PRES"     , 0),
])

#0x0055
ti_chargingstatus = OrderedDict([
    ("CCC"      , 17),
    ("CVR"      , 16),
    ("CCR"      , 15),
    ("VCT"      , 14),
    ("MCHG"     , 13),
    ("IN"       , 12),
    ("HV"       , 11),
    ("MV"       , 10),
    ("LV"       , 9),
    ("PV"       , 8),
    ("OT"       , 6),
    ("HT"       , 5),
    ("STH"      , 4),
    ("RT"       , 3),
    ("STL"      , 2),
    ("LT"       , 1),
    ("UT"       , 0),
])

#0x0056
ti_gaugingstatus = OrderedDict([
    ("OCVFR"    , 20),
    ("LDMD"     , 19),
    ("RX"       , 18),
    ("QMax"     , 17),
    ("VDQ"      , 16),
    ("NSFM"     , 15),
    ("SLPQMax"  , 13),
    ("QEN"      , 12),
    ("VOK"      , 11),
    ("R_DIS"    , 10),
    ("REST"     , 8),
    ("CF"       , 7),
    ("DSG"      , 6),
    ("EDV"      , 5),
    ("BAL_EN"   , 4),
    ("TC"       , 3),
    ("TD"       , 2),
    ("FC"       , 1),
    ("FD"       , 0),
])
#0x0057
ti_manufacturingstatus = OrderedDict([
   ("CAL_TEST" , 15),
   ("LT_TEST"  , 14),
   ("LED_EN"   , 9),
   ("FUSE_EN"  , 8),
   ("BBR_EN"   , 7),
   ("PF_EN"    , 6),
   ("LF_EN"    , 5),
   ("FET_EN"   , 4),
   ("GAUGE_EN" , 3),
   ("DSG_EN"   , 2),
   ("CHG_EN"   , 1),
   ("PCHG_EN"  , 0)
])
#0x0061
ti_lifetimedatablock2 = OrderedDict([
    ("No. of Shutdowns",        1),
    ("No. of Partial Resets",   1), 
    ("No. of Full Resets",      1), 
    ("No. of WDT resets",       1), 
    ("CB Time Cell 1",          1),
    ("CB Time Cell 2",          1),
    ("CB Time Cell 3",          1),
    ("CB Time Cell 4",          1)
])
#0x0071
ti_dastatus1 = OrderedDict([
    ("Cell Voltage 1", 2),
    ("Cell Voltage 2", 2), 
    ("Cell Voltage 3", 2), 
    ("Cell Voltage 4", 2), 
    ("BAT Voltage"   , 2),
    ("PACK Voltage"  , 2)
])
#0x0073
ti_gaugestatus1 = OrderedDict([
    ("True Rem Q"   , 2), 
    ("True Rem E"   , 2), 
    ("Initial Q"    , 2), 
    ("Initial E"    , 2),
    ("True FCC Q"   , 2), 
    ("True FCC E"   , 2),
    ("T_sim"        , 2), 
    ("T_ambient"    , 2)
])
#---------------------------------------------------------------------------
# Smart Battery System
#---------------------------------------------------------------------------
class TISmartBattery(SmartBattery):
    def __init__(self, slave):
        SmartBattery.__init__(self, slave)

    def get_mac_data(self, cmd, lenth=32):
        self.slave.set_word(sbs_cmd["ManufacturerAccess"], cmd)
        data = self.slave.get_block(sbs_cmd["ManufacturerData"])
#        self.slave.set_block(ti_mac_cmd["ManufacturerBlockAccess"], cmd)
#        data = self.slave.get_block(ti_mac_cmd["ManufacturerBlockAccess"])
#        del data[0:2]
        return data

    def DeviceType(self):        
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_devicetype.items()
#        sub = [("", 2)]
        return HexBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)

    def FirmwareVersion(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_firmwareversion.items()
#        sub = [("Device Number", 2), ("Version", 2), ("Build Number", 2), ("Firmware Type", 1), ("Impedance Track Version", 2)]
        return HexBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)

#    def SecurityKeys(self):
#        cmd = sys._getframe().f_code.co_name
#        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
#        sub = ti_securitykeys.items()
#        return HexBlockData(name = cmd, 
#                           subblock = sub,
#                           rawdata = rawdata)


#    def SafetyAlert(self):        
#        rawdata = self.get_mac_data("0x0050")
#        rawdata = (int(rawdata[3],16)<<24) + (int(rawdata[2],16)<<16) \
#                + (int(rawdata[1],16)<<8) + (int(rawdata[0],16))
#        data_str = hex(rawdata) 
#        data = BinaryData(name="SafetyAlert", rawdata=data_str)
#        return data

    def SafetyStatus(self):        
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_safetystatus.items()
#        sub = [("", 4)]
#        return HexBlockData(name = cmd, 
        return BinaryBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)

#    def PFAlert(self):        
#        rawdata = self.get_mac_data("0x0052")
#        rawdata = (int(rawdata[3],16)<<24) + (int(rawdata[2],16)<<16) \
#                + (int(rawdata[1],16)<<8) + (int(rawdata[0],16))
#        data_str = hex(rawdata) 
#        data = BinaryData(name="PfAlert", rawdata=data_str)
#        return data

    def PFStatus(self):        
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = [("", 4)]
        return HexBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)
    def OperationStatus(self):        
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_operationstatus.items()
        return BinaryBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)

    def ChargingStatus(self):        
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_chargingstatus.items()
        return BinaryBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)

    def GaugingStatus(self):        
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_gaugingstatus.items()
        return BinaryBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)

    def ManufacturingStatus(self):        
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_manufacturingstatus.items()
#        sub = [("", 2)]
        return BinaryBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)
       
    def LifetimeDataBlock2(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_lifetimedatablock2.items()
        return HexBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)


    def DAStatus1(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_dastatus1.items()
        return HexBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)


    def GaugeStatus1(self):
        cmd = sys._getframe().f_code.co_name
        rawdata = self.get_mac_data(ti_mac_cmd[cmd])
        sub = ti_gaugestatus1.items()
#        sub = [("True Rm Q", 2), ("True Rem E", 2), ("Initial Q", 2), ("Initial E", 2), ("True FGG Q", 2), ("True FGG E", 2), ("T_sim", 2), ("T_ambient", 2)]
        return HexBlockData(name = cmd, 
                           subblock = sub,
                           rawdata = rawdata)


