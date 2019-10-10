
*******************************************************************************
# Copyright © 2018 Celestica. All Rights Reserved.

# Redistribution and use in source and binary forms, with or without modification,

# are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright notice,

# this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

# 3. All rights, title and interest in and to Celestica’s intellectual property rights and the Software, 
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
****************************************************************************

********************
Usage:
#chmod a+x bbuapp
#./bbuapp --get help menu
bbuapp Help Menu:
        bbuapp partnum get
        bbuapp snnum get
        bbuapp manufact get
        bbuapp fwversion get
        bbuapp batterystat get
        bbuapp temperature get
        bbuapp voltage get
        bbuapp current get
        bbuapp remaincap get
        bbuapp fullchargcap get
        bbuapp runtimetoempty get
        bbuapp reqtochange get
        bbuapp bmc send
        bbuapp bmc get
*****************************************************************************
# ./bbuapp snnum get
BBU sn number :  B0977LS201807190025

********************
When use bmc relative command ipmitool need be installed on OS.
ipmitool mc info to get BMC information.

# ipmitool mc info
Device ID                 : 32
Device Revision           : 1
Firmware Revision         : 2.12
IPMI Version              : 2.0
Manufacturer ID           : 12290
Manufacturer Name         : Unknown (0x3002)
Product ID                : 977 (0x03d1)
Product Name              : Unknown (0x3D1)
Device Available          : yes
Provides Device SDRs      : no
Additional Device Support :
    Sensor Device
    SDR Repository Device
    SEL Device
    FRU Inventory Device
    IPMB Event Receiver
    IPMB Event Generator
    Chassis Device
Aux Firmware Rev Info     :
    0x00
    0x00
    0x00
    0x00
    
#And relative driver should be installed, check by ls /dev/ipmi*
# ls /dev/ipmi*
/dev/ipmi0

#Check module by
# lsmod|grep -i ipmi
ipmi_devintf           20480  0
ipmi_ssif              24576  0
ipmi_si                57344  0
ipmi_msghandler        49152  3 ipmi_ssif,ipmi_devintf,ipmi_si

*****************************************************************************
Note:
1. The following third party tools or modules maybe needed during this tool running.
   tools: ipmiutils ipmitool
2. The related driver module need install: ipmi_ssif,ipmi_devintf,ipmi_si,ipmi_msghandler

