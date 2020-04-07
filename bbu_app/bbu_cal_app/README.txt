#*******************************************************************************
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

#****************************************************************************

********************
Python Environment:
BBU calibration running on python 2.7.5 environment.
Please prepare this version python before run the script.


********************
Run calibration
********************
If you have SES driver and sg3-utils on OS,Just run
python Athena_BBU_calibration.py

********************
If there is no OS SES driver and sg3-utils on OS.
Before Run:Please enter following command in Serial port of SES.
Switch BBU_CHG_EN by OS. By SES cmd:
twi_wr 10 0x0c 0x27 1 0x01
Then Run:
python Athena_BBU_calibration.py

********************
Note:
1. The following third party tools or modules maybe needed during this tool running.
   python-smbus(Ubuntu), impitool, i2c-tools,i2c-tools-python(CentOS)
2. The related driver module need install: smbus(i2c-dev, i2c-i801), ipmi related.
3. Not required but better to have: sg3-utils, SES driver(provided by Celestica)
