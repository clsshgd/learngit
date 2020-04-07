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
# File name: Athena BBU Calibration.py
# Author: Grace Zhuang, Taddeo Tao, Simon Sun
# version history:
# v 1.0.0 base on original code add SES page command to control charge resource.
# v 1.0.1 add version variable to trace this software.
# v 1.0.2 fix the issue about switch charge control
# v 4.1.0 release for Athena+, support the SES driver for Linux 4.19
# ***************************************************************************

"""
import os, sys, imp, argparse, logging, time, datetime,re
from cls_bbu.bbu import bbu_athena
from cls_bbu.sbs import sbs_ti
from cls_bbu.io import smbusslave
from cls_bbu.controller import controller_athena


#logging.basicConfig(stream=sys.stdout, level=logging.INFO,
log_file_name = './log/bbu_learning_' + datetime.datetime.now().strftime("%Y-%m-%d-%H-%M-%S") + ".log"
logging.basicConfig(level=logging.INFO,
        format='%(asctime)s - %(levelname)s: %(message)s',
        filename=log_file_name)
app_version = "4.1.0"

console = logging.StreamHandler()
console.setLevel(logging.WARNING)
formatter = logging.Formatter('%(message)s')
console.setFormatter(formatter)
logging.getLogger('').addHandler(console)
#----------------------------------------------------------------------------
# CLI Options
#----------------------------------------------------------------------------
class Athena_Calibration(object):
    def __init__(self,bus, smbus_slave):
        mysbs = sbs_ti.TISmartBattery(smbus_slave)
        myctrl = controller_athena.Athena_BBU_Controller(bus)
        self.__bbu = bbu_athena.BBU_ATHENA(mysbs, myctrl)
        self.__qmax = 1
        self.__low = controller_athena.LED_REG["LOW"] 
        self.__high = controller_athena.LED_REG["HIGH"] 

    def c1_Enter_Starting_Learning_Condition(self):
        data = self.__bbu.get_data("GaugeStatus1")
        logging.debug("GaugeStatus1:\n%s" % data)
        temperature = int(data.value['T_ambient'],16) * 0.1 - 273.15
        logging.info("Ambient Temperature: %dC" % temperature)
        if temperature <= 40 and temperature >= 10:
            return True
        else:
            logging.error("The Ambient Temperature (%dC) out of range." % temperature)
            return False

    def c2_Charge_BBU(self):
        logging.info("Enable  BBU_EN_L")
        logging.info("Disable BBU_DSG_EN_L")
        logging.info("Disable BBU_TEST_CAL_L")
        self.__bbu.controller.enable(True)
        self.__bbu.controller.discharge(False)
        self.__bbu.controller.calibrate(False)
        #check status
        status = self.__bbu.controller.get_status()
        logging.debug("Check: %s " % status)
        if status['CAL_L'] != self.__high :
            logging.error("Set CAL_L fail, should be %d", self.__high)
            return False
        logging.info("Check RSOC and Pack voltage:")
        logging.info("  Should: (RSOC < 98%) and (Pack voltage < 12000mV)")
        rsoc = self.__bbu.battery.RelativeStateOfCharge().value
        pack_voltage = int(self.__bbu.get_data("DAStatus1").value['PACK Voltage'], 16)
        logging.info("  Get   : (RSOC = %s%%) and (Pack voltage = %dmV)" % (rsoc, pack_voltage))
        if (rsoc >= 98 ) or (pack_voltage >= 12000):
            logging.info("Disable BBU_CHG_EN_L")
            logging.info("Enable  BBU_TEST_CAL_L")
            logging.info("Start discharging ...")
# HERE is a workaround sleep, because of an issue here.
            time.sleep(1)
            self.__bbu.controller.charge(False)
            self.__bbu.controller.calibrate(True)
            time.sleep(1)
            status = self.__bbu.controller.get_status()
            logging.debug("Check: %s " % status)
            if status['CAL_L'] != self.__low or status['CHG_EN_L'] != self.__high :
                logging.error("Set CAL_L or CHG_EN_L fail, CAL_L should be %d, CHG_EN_L should be %d", self.__low, self.__high)
                return False
            while True:
                time.sleep(10)
                rsoc = self.__bbu.battery.RelativeStateOfCharge().value
                pack_voltage = int(self.__bbu.get_data("DAStatus1").value['PACK Voltage'], 16)
                sys.stdout.write("\rNOW: RSOC = %02d%%, Pack voltage = %d mV" % (rsoc, pack_voltage))
                sys.stdout.flush()
                if (rsoc < 98 ) and (pack_voltage < 12000) :
                    sys.stdout.write("\r")
                    logging.info("Stop discharging ...")
                    logging.info("Disable BBU_TEST_CAL_L")
                    self.__bbu.controller.calibrate(False)
                    time.sleep(1)
                    status = self.__bbu.controller.get_status()
                    logging.debug("Check: %s " % status)
                    if status['CAL_L'] != self.__high :
                        logging.error("Set CAL_L fail, should be %d", self.__high)
                        return False
                    break

        logging.info("Start charging to RSOC >= 99% ...")
        logging.info("Enable BBU_CHG_EN_L")
        self.__bbu.controller.charge(True)
        time.sleep(1)
        status = self.__bbu.controller.get_status()
        logging.debug("Check: %s " % status)
        if status['CHG_EN_L'] != self.__low :
            logging.error("Set CHG_EN_L fail, should be %d", self.__low)
            return False
        while True:
            time.sleep(10)
            rsoc = self.__bbu.battery.RelativeStateOfCharge().value
            str = '>'*(rsoc/3)+' '*(43-rsoc/3)
            sys.stdout.write('\r'+str+'[%02s%%]'%(rsoc))
            sys.stdout.flush()
            if rsoc >= 99:
                sys.stdout.write("\n")
                logging.info("Disable BBU_CHG_EN_L")
                logging.info("stop charging ...")
                self.__bbu.controller.charge(False)
                time.sleep(1)
                status = self.__bbu.controller.get_status()
                logging.debug("Check: %s " % status)
                if status['CHG_EN_L'] != self.__high :
                    logging.error("Set CHG_EN_L fail, should be %d", self.__high)
                    return False
                break
        return True

    def c3_IT_Enable(self):
#        security_data = self.__bbu.get_data("SecurityKeys")
#        self.__bbu.set_data("ManufacturerAccess", int(security_data.value['UNSEAL key 1'], 16))
#        self.__bbu.set_data("ManufacturerAccess", int(security_data.value['UNSEAL key 2'], 16))
#        self.__bbu.set_data("ManufacturerAccess", int(security_data.value['FULL ACCESS key 1'], 16))
#        self.__bbu.set_data("ManufacturerAccess", int(security_data.value['FULL ACCESS key 2'], 16))
        logging.info("Unseal BBU:")
        logging.info("Set ManufacturerAccess : 0x0414")
        logging.info("Set ManufacturerAccess : 0x3672")
        logging.info("Set ManufacturerAccess : 0xFFFF")
        logging.info("Set ManufacturerAccess : 0xFFFF")
        self.__bbu.set_data("ManufacturerAccess", 0x0414)
        self.__bbu.set_data("ManufacturerAccess", 0x3672)
        self.__bbu.set_data("ManufacturerAccess", 0xFFFF)
        self.__bbu.set_data("ManufacturerAccess", 0xFFFF)
        
        logging.info("Reset IT enable")
        while True:
            logging.info("Set ManufacturerAccess : 0x%04x" % (sbs_ti.ti_mac_cmd['Gauging']))
            self.__bbu.set_data("ManufacturerAccess", sbs_ti.ti_mac_cmd['Gauging'])
            time.sleep(1)
            data = self.__bbu.get_data("ManufacturingStatus")
            logging.debug("Get ManufacturingStatus:\n%s" % data)
            gauge_en = data.value['GAUGE_EN']
#            if self.__bbu.get_data("ManufacturingStatus").value['GAUGE_EN'] == '1':
            logging.info("Check ManufacturingStatus: Should GAUGE_EN == 1, Get %s" % gauge_en)
            if gauge_en == '1':
                break
            else :
                continue
        logging.info("Seal BBU:")
        logging.info("Set ManufacturerAccess : 0x%04x" % (sbs_ti.ti_mac_cmd['SealDevice']))
        self.__bbu.set_data("ManufacturerAccess", sbs_ti.ti_mac_cmd['SealDevice'])
        return True

    def c4_Rest_Battery_For_BQ_Take_OCV1(self):
        logging.info("Check GaugingStatus Rest, Should REST == 1:")
        start_time = time.time()
        # 9000 seconds = 2.5 hour
        i=1
        while time.time() - start_time < 9000:
            str = '>'*i
            sys.stdout.write('\r'+str)
            sys.stdout.flush()
            if self.__bbu.get_data("GaugingStatus").value['REST'] == '1':
                sys.stdout.write('\n')
                return True
            time.sleep(60)
            i = i + 1
        sys.stdout.write('\n')
        logging.error("Time out")
        return False

    def c5_Discharge_BBU(self):
        self.__bbu.controller.calibrate(True)
        logging.info("Enable BBU_TEST_CAL_L")
        status = self.__bbu.controller.get_status()
        logging.debug("Check: %s " % status)
        if status['CAL_L'] != self.__low :
            logging.error("Set CAL_L fail, should be %d", self.__low)
            return False
        logging.info("start discharging ...")
        logging.info("Check Pack voltage, Should: (Pack voltage < 10800mV)")
        while True:
            pack_voltage = int(self.__bbu.get_data("DAStatus1").value['PACK Voltage'], 16)
            #sys.stdout.write("\rNOW: Pack voltage = %d mV" % (pack_voltage))
            rsoc = self.__bbu.battery.RelativeStateOfCharge().value
            #pack_voltage = int(self.__bbu.get_data("DAStatus1").value['PACK Voltage'], 16)
            logging.info("\rNOW: RSOC = %02d%%, Pack voltage = %d mV" % (rsoc, pack_voltage))
            sys.stdout.write("\rNOW: RSOC = %02d%%, Pack voltage = %d mV" % (rsoc, pack_voltage))
            sys.stdout.flush()
            if (pack_voltage < 10800):
                break
            else:
                time.sleep(60)
        sys.stdout.write("\n")
        logging.info("stop discharging ...")
        logging.info("Disable BBU_TEST_CAL_L")
        self.__bbu.controller.calibrate(False)
        status = self.__bbu.controller.get_status()
        logging.debug("Check: %s " % status)
        if status['CAL_L'] != self.__high :
            logging.error("CAL_L should be %d", self.__high)
            return False
        return True

    def c6_Rest_Battery_For_BQ_Take_OCV2(self):
        data = self.__bbu.get_data("GaugingStatus")
        logging.debug("GaugingStatus:\n %s " % (data))
        self.__qmax = data.value['QMax']
        logging.warning("Check Qmax Bit : qmax_now=%s" % (self.__qmax))
#        self.__qmax = self.__bbu.get_data("GaugingStatus").value['QMax']
        logging.debug("qmax_old = %s " % (self.__qmax))
        if self.__qmax != '1' and self.__qmax != '0':
            logging.error("Get QMax data fail: qmax=%s" % (self.__qmax))
            return False
        logging.info("Check GaugingStatus Rest")
        start_time = time.time()
        # 9000 seconds = 2.5 hour
        i = 1
        while time.time() - start_time < 9000:
            time.sleep(60)
            str = '>' * i
            sys.stdout.write('\r'+str)
            sys.stdout.flush()
            status = self.__bbu.controller.get_status()
            logging.debug("Check: %s " % status)
#            self.__bbu.display_basic()
            if self.__bbu.get_data("GaugingStatus").value['REST'] == '1':
                sys.stdout.write("\n")
                logging.info("GaugingStatus Rest matched, wait 5 minutes")
                time.sleep(300)
                return True
            #time.sleep(60)
            i = i + 1
        sys.stdout.write("\n")
        logging.error("Time out")
        return False

    def c7_Check_Qmax_Bit(self):
        data = self.__bbu.get_data("GaugingStatus")
        logging.debug("GaugingStatus:\n %s " % (data))
        qmax_new = data.value['QMax']
        logging.warning("Check Qmax Bit : qmax_now=%s" % (qmax_new))
#        qmax_new = self.__bbu.get_data("GaugingStatus").value['QMax']
        if qmax_new == self.__qmax:
            logging.error("Check Qmax Bit fail: qmax_old=%s qmax_new=%s" % (self.__qmax, qmax_new))
            time.sleep(300)
            data = self.__bbu.get_data("GaugingStatus")
            logging.debug("GaugingStatus:\n %s " % (data))
            qmax_now_test = data.value['QMax']
            logging.warning("Check Qmax Bit again : qmax_now=%s" % (qmax_now_test))
            return False
        else:
            return True

    def c8_Charge_BBU(self):
        logging.info("Enable BBU_CHG_EN_L")
        self.__bbu.controller.charge(True)
        status = self.__bbu.controller.get_status()
        logging.debug("Check: %s " % status)
        if status['CHG_EN_L'] != self.__low :
            logging.error("CHG_EN_L should be %d", self.__low)
            return False
        else :
            return True

    def calibration(self):
        logging.warning(self.__bbu.battery.Temperature())
        logging.warning(self.__bbu.battery.Current())
        logging.warning(self.__bbu.battery.RelativeStateOfCharge())
        logging.warning("================================================")
        ret = self.__do_step('c1_Enter_Starting_Learning_Condition')
        if True == ret:
            ret = self.__do_step('c2_Charge_BBU')
        if True == ret:
            ret = self.__do_step('c3_IT_Enable')
        if True == ret:
            ret = self.__do_step('c4_Rest_Battery_For_BQ_Take_OCV1')
        if True == ret:
            ret = self.__do_step('c5_Discharge_BBU')
        if True == ret:
            ret = self.__do_step('c6_Rest_Battery_For_BQ_Take_OCV2')
        if True == ret:
            ret = self.__do_step('c7_Check_Qmax_Bit')
        self.__do_step('c8_Charge_BBU')
        if True == ret:
            logging.warning("BBU Learning SUCCESS!")
        else:
            logging.warning("BBU Learning Fail!")
        return ret

    def __do_step(self, cmd):
        logging.warning(cmd)
        func = getattr(self, cmd, None)
        ret = func()
        if ret == True:
            logging.warning("%-39s | \033[32m%s \033[0m|" % (' ', "PASS"))
        else:
            logging.warning("%-39s | \033[31m%s \033[0m|" % (' ', "FAIL"))
        logging.warning("================================================")
        return ret

def query_yes_no():
    valid = {"yes": True, "y": True, "ye": True, "no": False, "n": False}
    prompt = " [Y/N] "
    while True:
        sys.stdout.write("Does OS Control BBU's charging? [Y/N] ")
        choice = raw_input().lower()
        if choice in valid:
            logging.warning("****************************************************************************************")
            logging.warning("* Your input is %s. Means that the OS is now controlling the charging of the BBU. *"%(choice))
            logging.warning("* If you are not sure, confirm it and then run the calibration grogram again!!!        *")
            logging.warning("****************************************************************************************")

            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' (or 'y' or 'n').\n")
def cls_check_ses_tool():
    sg3_utils_path = "cls_bbu/Linux_tool/sg3_utils-master/"
    if os.system("sg_inq --version") != 0 or os.system("sg_ses_microcode -V") != 0:
        logging.info("sg utils not found, try to install it")
        cmd = "export NOWPATH=$PWD;cd %s;chmod a+x configure;./configure >> $NOWPATH/%s 2>&1"%(sg3_utils_path,log_file_name)
        os.system(cmd)
        cmd = "export NOWPATH=$PWD;cd %s;make >> $NOWPATH/%s 2>&1"%(sg3_utils_path,log_file_name)
        os.system(cmd)
        cmd = "export NOWPATH=$PWD;cd %s;make install >> $NOWPATH/%s 2>&1"%(sg3_utils_path,log_file_name)
        os.system(cmd)
        cmd = "ldconfig"
        os.system(cmd)
    if os.system("sg_inq --version") != 0 or os.system("sg_ses_microcode -V") != 0:
        logging.warning("Please install SG_utility tool first!")
        return False
    if  os.system("lsmod|grep -w cls") != 0:
        os.system("uname -r>calibt_log_tmp")
        fp = open("calibt_log_tmp",'r')
        ses_tool_path_tmp = "cls_bbu/Linux_tool/Athena_host_driver_v0.1.3/"
        fp = open("calibt_log_tmp",'r')
        str_uname = fp.read()
        str_ker_num = re.findall(r"\d+\.?\d*",str_uname)
        ker_num = str_ker_num[0]
        ker_num = re.findall(r"\d+",ker_num)
        ker_num_maj = int(ker_num[0])
        ker_num_min = int(ker_num[1])
        driver_name = "cls.ko"
        if ker_num_maj == 3:
            if ker_num_min >= 10:
                print "3.10-4.3"
                ses_tool_path = "%sswitchtec-kernel-backport_3.10_to_4.3/"%(ses_tool_path_tmp)
            else:
                print "unsupported Linux kernel version"
                return False

        elif ker_num_maj == 4:
            if ker_num_min <= 3:
                ses_tool_path = "%sswitchtec-kernel-backport_3.10_to_4.3/"%(ses_tool_path_tmp)

            elif 4 <= ker_num_min <= 7:
                ses_tool_path = "%sswitchtec-kernel-release_4.4_to_4.7/"%(ses_tool_path_tmp)

            elif 8 <= ker_num_min <= 11:
                ses_tool_path = "%sswitchtec-kernel-xlink_4.8_to_4.11/"%(ses_tool_path_tmp)

            elif ker_num_min == 12:
                ses_tool_path = "%sswitchtec-kernel-xlink_4.12/"%(ses_tool_path_tmp)

            elif 13  <= ker_num_min <= 14:
                ses_tool_path = "%sswitchtec-kernel-release_4.13_to_4.14/"%(ses_tool_path_tmp)

            elif ker_num_min == 19:
                if os.system("lsmod|grep -w cls_switchtec") == 0:
                    return True
                ses_tool_path = "%sswitchtec-kernel-release_4.19/"%(ses_tool_path_tmp)
                driver_name = "cls_switchtec.ko"

            else:
                print "unsupported Linux kernel version"
                return False
        cmd = "export NOWPATH=$PWD;cd %s;chmod a+x VERSION-GEN;make >> $NOWPATH/%s 2>&1 "%(ses_tool_path,log_file_name)       
        os.system(cmd)
        cmd = "insmod %s%s"%(ses_tool_path, driver_name)
        if os.system(cmd) != 0:
            logging.warning("Error: insmod %s Fail!"%(driver_name))
            return False
    if os.system("ls /dev/switchtec*") != 0:
        logging.warning("SES driver can't work, please try to power cycle system or contact support!")
        return False
    return True

def set_bbu_charge_en_contorlled(ctlby):
    os.system('sg_ses -p 0x01 /dev/switchtec0|grep -i "relative ES process id">calibt_log_tmp')
    if ctlby == 'CPLD':
        mask1 = 0x07
        mask2 = 0x04
        verify_num = 1
    elif ctlby == 'OS':
        mask1 = 0x0b
        mask2 = 0x08
        verify_num = 2
    else :
        print "set_bbu_charge_en_contorlled() get error options"
        return False
    fp1 = open("calibt_log_tmp",'r')
    str_cani_side = fp1.readline()
    num = str_cani_side.index('id')
    canister_num = int(str_cani_side[num+4])
    if canister_num != 1 and canister_num != 2 :
        print "Warning:identify canister side wrong!"
        return False
    #print "canister side num is",canister_num
    fp1.close()
    os.system('sg_senddiag -P 0x21 /dev/switchtec0 -HHH>calibt_log_tmp')
    fp2 = open("calibt_log_tmp",'r+')
    str_ses_page = fp2.read()
    ses_page = list(str_ses_page)
    if(canister_num == 1):
        comm_num1 = 25
        comm_num2 = 26
        bbu_ctl_bit = 32       
    else:
        comm_num1 = 37
        comm_num2 = 38
        bbu_ctl_bit = 44
    #reset ses page
    ses_page[comm_num1] = '8'
    ses_page[comm_num2] = '0'
    valbbuc = int(ses_page[bbu_ctl_bit])
    valbbuc = valbbuc & mask1
    valbbuc = valbbuc | mask2
    valbbuc = str(valbbuc)
    ses_page[bbu_ctl_bit] = valbbuc
    fp2.seek(0,0)
    for line in ses_page:
        fp2.write(line)
    fp2.close()

    os.system('sg_senddiag --pf --raw=- /dev/switchtec0 <calibt_log_tmp')
    #Check whether the modification is successful
    os.system('sg_senddiag -P 0x21 /dev/switchtec0 -HHH|head -1>calibt_log_tmp')
    fp3 = open("calibt_log_tmp",'r')
    ses_page_ch = fp3.read()
    ses_page_ch = list(ses_page_ch)
    val_check = int(ses_page_ch[bbu_ctl_bit])
    val_check = (val_check >> 2) & 0x03
    if val_check != verify_num :
        logging.warning("Set BBU Charge Enabling Contorlled By",ctlby,"Fail!")
        logging.warning("You should Switch BBU_CHG_EN By SES cmd")
        #print "twi_wr 10 0x0c 0x27 1 0x01"
        return False
    return True

def get_i2c_num_I801() :
    i2c_num = 1
    os.system("i2cdetect -l|grep -i 'i801'|awk '{print $1}'|cut -d '-' -f 2 >i2c_num_I801.log")
    fp = open("i2c_num_I801.log",'r')
    for line in fp:
        i2c_num = int(line)
    fp.close()
    return i2c_num

def main():
    smbus_slave = None
    addr = 0x0b
    result = 0
    
    """
    if cls_check_ses_tool() == False or set_bbu_charge_en_contorlled('OS') == False:
        print "Automatic set BBU Charge Enabling Contorlled By OS Fail!"
        print "Plesse Switch BBU_CHG_EN by OS in SES serial console!!!"
        print "In SES serial console, please enter following command"
        print "twi_wr 10 0x0c 0x27 1 0x01"
        if query_yes_no() == False:
            return 1
    """
    try:
        if cls_check_ses_tool() == False or set_bbu_charge_en_contorlled('OS') == False:
            logging.warning("**************************************************************")
            logging.warning("** Automatic set BBU Charge Enabling Contorlled By OS Fail! **")
            logging.warning("** Please Switch BBU_CHG_EN by OS in SES serial console!!!  **")
            logging.warning("** In SES serial console, please enter following command    **")
            logging.warning("** twi_wr 10 0x0c 0x27 1 0x01                               **")
            logging.warning("**************************************************************")
            logging.warning(" ")
            logging.warning("----------------------------------------------------------------")
            logging.warning("-- If BBU is controlled by CPLD, the calibration may FAIL !!! --")
            logging.warning("----------------------------------------------------------------")

            if query_yes_no() == False:
                return 1
    except:
        logging.warning("Automatic set BBU Charge Enabling Contorlled By OS Fail!")
        logging.warning("Plesse Switch BBU_CHG_EN by OS in SES serial console!!!" )
        logging.warning(" ")
        logging.warning("****************************************************************")
        logging.warning("*- If BBU is controlled by CPLD, the calibration may FAIL !!! -*")
        logging.warning("****************************************************************")
        if query_yes_no() == False:
            return 1
    #install smbus driver and create devnode /dev/i2c-1
    if os.system('modprobe i2c-dev') != 0 or os.system('modprobe i2c_i801') != 0:
        logging.warning("Error: install i2c driver FAIL !!!")
        return 1
    
    bus = get_i2c_num_I801()
    print "bus num is:",bus
    try:
        imp.find_module('smbus')
        smbus_slave = smbusslave.LocalhostSMBusSlave(bus, addr)
    except ImportError:
        sys.exit("No python smbus module")
#        smbus_slave = smbusslave.LocalhostI2CToolSlave(bus, addr)
#    mysbs = sbs.TISmartBattery(smbus_slave)

    print "[log file name]"+log_file_name
    athena = Athena_Calibration(bus,smbus_slave)
    if athena.calibration() == False:
        if set_bbu_charge_en_contorlled('CPLD') == False:
            print "Please Switch BBU_CHG_EN by CPLD By Yourself."
        return 2
    if set_bbu_charge_en_contorlled('CPLD') == False:
        print "Please Switch BBU_CHG_EN by CPLD By Yourself."

if __name__ == "__main__":
    sys.exit(main())

