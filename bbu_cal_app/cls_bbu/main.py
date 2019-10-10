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
import sys
import imp
import argparse
from .bbu import bbu
from .bbu import bbu_athena
from .sbs import sbs_ti
from .io import smbusslave
from .version import VERSION

#BMC_IP = "127.0.0.1"
#BMC_USER = "fmxadmin"
#BMC_PASSWORD = "fmxadmin"
#
#def get_bmc_info():
#    return BMC_IP, BMC_USER, BMC_PASSWORD
#
#def set_bmc_info(ip, user=None, password=None):
#    global BMC_IP
#    global BMC_USER
#    global BMC_PASSWORD
#    BMC_IP = ip
#    if user != None:
#        BMC_USER = user
#    if password != None:
#        BMC_PASSWORD = password
#
#class OXBBUEnableCharge(EnableChargeBehavior):
#    def __init__(self, bbuid):
#        self.bbuid = bbuid
#
#    def __call__(self):
#        ip, user, passwd = get_bmc_info()
#        cmd = " ipmitool -H %s -U %s -P %s raw 0x3a 0x0d %s 0x1" % (ip, user, passwd, hex(self.bbuid))
#        print cmd
#
#class OXBBUDisableCharge(DisableChargeBehavior):
#    def __init__(self, bbuid):
#        self.bbuid = bbuid
#
#    def __call__(self):
#        ip, user, passwd = get_bmc_info()
#        cmd = " ipmitool -H %s -U %s -P %s raw 0x3a 0x0d %s 0x0" % (ip, user, passwd, hex(self.bbuid))
#        print cmd
#
#class OXBBUEnableDischarge(EnableDischargeBehavior):
#    def __init__(self, bbuid):
#        self.bbuid = bbuid
#
#    def __call__(self):
#        ip, user, passwd = get_bmc_info()
#        cmd = " ipmitool -H %s -U %s -P %s raw 0x3a 0x0d %s 0x2" % (ip, user, passwd, hex(self.bbuid))
#        print cmd
#
#class OXBBUDisableDischarge(DisableDischargeBehavior):
#    def __init__(self, bbuid):
#        self.bbuid = bbuid
#
#    def __call__(self):
#        ip, user, passwd = get_bmc_info()
#        cmd = " ipmitool -H %s -U %s -P %s raw 0x3a 0x0d %s 0x0" % (ip, user, passwd, hex(self.bbuid))
#        print cmd
#
#class OXBBUEnableCalibrate(EnableCalibrateBehavior):
#    def __iit__(self, bbuid):
#        self.bbuid = bbuid
#
#    def __call__(self):
#        ip, user, passwd = get_bmc_info()
#        cmd = "ipmitool -H %s -U %s -P %s raw 0x3a 0x0d %s 0x04" % (ip, user, passwd, hex(self.bbuid))
#        apply_command(cmd)
#
#class OXBBUDisableCalibrate(EnableCalibrateBehavior):
#    def __iit__(self, bbuid):
#        self.bbuid = bbuid
#
#    def __call__(self):
#        ip, user, passwd = get_bmc_info()
#        cmd = "ipmitool -H %s -U %s -P %s raw 0x3a 0x0d %s 0x00" % (ip, user, passwd, hex(self.bbuid))
#        apply_command(cmd)
#
#class OXBBUController(Controller):
#    def __init__(self, bbuid):
#        Controller.__init__(self)
#        self.enable_calibrate_behavior = OXBBUEnableCalibrate()
#        self.disable_calibrate_behavior = OXBBUDisableCalibrate()
#
#class OXBBU(BBU):
#    def __init__(self, bbuid):
#        host, user, passwd = get_bmc_info()
#        if bbuid == 0:
#            #slave = DebugSlave("0x09", "0x0b")
#            slave = BMCSlaveNet("0x09", "0x0b", host, user, passwd)
#        elif bbuid == 1:
#            #slave = DebugSlave("0x13", "0x0b")
#            slave = BMCSlaveNet("0x0d", "0x0b", host, user, passwd)
#        else:
#            raise ValueError("Unknonw BBU.")
#        battery = Battery(slave)
#        controller = OXBBUController(bbuid)
#        BBU.__init__(self, battery, controller)
#
#
#    def start_calibrate(self):
#        self.controller.enable_calibrate()
#
#    def stop_calibrate(self):
#        self.controller.disable_calibrate()
#
#class TB5BBU(BBU):
#    def __init__(self, bus, address):
#        if bus == None:
#            bus = "3"
#
#        if address == None:
#            address = "0x0b"
#
#        slave = LocalhostSMBusSlave(bus, address)
#        battery = Battery(slave)
#        BBU.__init__(self, battery)

#----------------------------------------------------------------------------
# CLI Options
#----------------------------------------------------------------------------
def all_integers(x):
    value = int(x,0)
    return value;

def main():
    smbus_slave = None

    parser = argparse.ArgumentParser(description='Display BBU SBS Information.')
    parser.add_argument('bus', type=all_integers, help='BBU SMBus ID')
    parser.add_argument('-a', '--address', type=all_integers, help='BBU SMBus address, default value is 0x0b', default=0x0b)
    parser.add_argument('-B', '--bmc',  action='store_true', help='Access through BMC', default=False)
    parser.add_argument('-H', '--host', help='BMC host', default='127.0.0.1')
    parser.add_argument('-U', '--user', help='BMC user', default='admn')
    parser.add_argument('-P', '--password', help='BMC password', default='admn')
    parser.add_argument('-v', '--verbose', action='count', help='Display verbose information')
    parser.add_argument('-V', '--version', action='version', version='%(prog)s '+VERSION, help='Display version')
    args = parser.parse_args()
    result = 0

    if args.bmc == True:
        smbus_slave = smbusslave.BMCSlaveNet(args.bus, args.address, args.host, args.user, args.password)
    else:
        try:
            imp.find_module('smbus')
            smbus_slave = smbusslave.LocalhostSMBusSlave(args.bus, args.address)
        except ImportError:
            smbus_slave = smbusslave.LocalhostI2CToolSlave(args.bus, args.address)
    mysbs = sbs_ti.TISmartBattery(smbus_slave)
    mybbu = bbu_athena.BBU_ATHENA(mysbs)

    if (args.verbose is None or args.verbose == 0):
        mybbu.display_basic()
    elif (args.verbose == 1):
        mybbu.display_v()
    elif (args.verbose == 2):
        mybbu.display_vv()
    else:
        mybbu.display_vvv()

if __name__ == "__main__":
    main()

