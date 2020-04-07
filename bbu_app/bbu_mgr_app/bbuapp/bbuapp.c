/******************************************************************************** 
# Copyright © 2018 Celestica. All Rights Reserved.

# Redistribution and use in source and binary forms, with or without modification,

# are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright notice,

# this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

# 3. All rights, title and interest in and to Celestica’s intellectual property rights and the Software, 
including all technology, associated documentation, data and information are and shall remain the exclusive property of Celestica or its licensors.
You will not and will not attempt to reverse engineer, decompile or disassemble the Software. 
You shall not modify or change the Software or make any derivative works based on the Software or permit any third party to do so.
The name of Celestica may not be used to endorse or promote products derived from this software without specific prior written permission.

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
 * --------------------------------------------------------------------------
 * --------------------------------------------------------------------------
 *
 * Description:
 *      This file implements the commands issued for bbu management
 *
 * File:
 * $Id: bbuapp.c
 *
 * $Log:
 ****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include "bbuapp.h"



/* Globals */
Command_t CommandTable[] =
{
  {"partnum",        	BBU_PART_NUM,	"BBU part number : "},
  {"snnum",        	BBU_SN_NUM,		"BBU sn number : "},
  {"manufact",     	BBU_MANUFAC,	"BBU Manufacture : "},
  {"fwversion",     	BBU_FW_VERSION,	"BBU FW version : "},
  {"batterystat",     	BBU_BATTERY_STATUS,	"BBU battery status : "},
  {"temperature",   	BBU_TMP,			"BBU Temperature : "},
  {"voltage",          BBU_VOLTAGE,		"BBU Voltage : "},
  {"current",         	BBU_CURRENT,		"BBU Current : "},
  {"remaincap",	BBU_REMAIN_CAP,	"BBU Remaining Capacity : "},
  {"fullchargcap",	BBU_FULLCHARGE_CAP,	"BBU FullCharge Capacity : "},
  {"runtimetoempty",      BBU_RUNTIME_EMP,	"BBU RunTime to Empty : "},
  {"reqtochange",    	BBU_REQ_CHARG,	"BBU Required to Change : "},
  {"functionfail",    	BBU_FUN_FAIL,		"BBU Function Fail : "}, 
  {"bmc",    	BMC_GET,	"BBU Info get from BMC : "},
  {"all",    	ALL_INFO_GET,	"All BBU Info List : "},
  {"test",    	BBU_TEST_CMD,	"Clear BBU test result in VPD : "},
  {"bbu",    	BBU_SETTING,	"BBU arm: "},
};


/*****************************************************************************

  Purpose:
  main program entry point

  Parameters:
  argc argv   - STD ARG input

  Return:
  none

*****************************************************************************/
int main(int argc, char* argv[])
{
	u32	ret		= SUCCESS;
	u8	Opcode	= 0;
	int	index	= 0;
	int i2c_num_I801 = 0;
	char	*token	= NULL;
	/*i2cdetect -l ,I801 */
	

	if (argc > 2) {
		token = argv[1];

		if (token != NULL){
			//get the command opcode
			cls_bbu_convert_string_to_lower(token);
			for (index = 0; index < (sizeof (CommandTable)/sizeof(CommandTable[0])); index++){
				if (!strcmp(CommandTable[index].commandString, token)){
					Opcode = CommandTable[index].commandOpcode;
					break;
				}
			}
		}
		if(index == (sizeof (CommandTable)/sizeof(CommandTable[0]))){
			cls_bbu_print_usage();
			return FAIL;
		}
		
		//seems step 1 & 2 just use i2cdetect -l command can solve,sometimes it still need modprobe  cn28059704
		/*step 1: check whether i2c driver is installed*/
		if(cls_bbu_driver_check() == FAIL){
			return FAIL;
		}
		i2c_num_I801=cls_get_i2c_num_I801();

		/*step 2: check whether BBU is inserted*/
		if(cls_bbu_insert_check(i2c_num_I801,BBU_9551) == FAIL){
			return FAIL;
		}
		
		//this test function will trig test part of bbu, we don't need this here 
		/*step 3: check whether BBU dump mode is enabled*/
		//if(cls_bbu_test_check(BBU_9551) == FAIL){
		//	return FAIL;
		//}

		if(!strcmp("get",argv[2])){
			
			if(!strcmp("all",argv[1])){ //do all info get when all get

				for (index = 0; index < 12; index++)
				{
					Opcode = CommandTable[index].commandOpcode;
					cls_bbu_send_i2c_get_command(i2c_num_I801,BBU_FUEL_GAUGE,Opcode);
				}

			}else{//get one by one or special commands get
				ret = cls_bbu_send_i2c_get_command(i2c_num_I801,BBU_FUEL_GAUGE,Opcode);
			}
		}else if(!strcmp("send",argv[2])){
			ret = cls_bbu_send_info_to_bmc_command(i2c_num_I801);
		}else if(!strcmp("clear",argv[2])&&(!strcmp("test",argv[1]))){//clear bbu failure record in VPD
			ret = clear_BBU_TestBit(i2c_num_I801);
		}else if(!strcmp("arm",argv[2])&&(!strcmp("bbu",argv[1]))){//bbu arm command
			ret = arm_bbu(i2c_num_I801,BBU_9551);
		}else if(!strcmp("disarm",argv[2])&&(!strcmp("bbu",argv[1]))){//bbu disarm command
			ret = disarm_bbu(i2c_num_I801,BBU_9551);
		}else if(!strcmp("armstat",argv[2])&&(!strcmp("bbu",argv[1]))){//bbu status check command
			ret = check_bbu_arm_status(i2c_num_I801,BBU_9551);
		}
		else{
			cls_bbu_print_usage();
		}
	}
	else{
		cls_bbu_print_usage();
}
	return ret;
	
}

/*cls_bbu_print_usage
* print usage of command
* return none.
*/
void cls_bbu_print_usage(void)
{
	printf( "****************************************************************"
	      "*************\n" );
	printf("bbuapp version: %s\n",BBU_VERSION);
	printf("bbuapp Help Menu:\n");
	printf("\tbbuapp partnum get               ----Get BBU Part Number\n");
	printf("\tbbuapp snnum get                 ----Get BBU Serial Number\n");
	printf("\tbbuapp manufact get              ----Get BBU Manufacture information\n");
	printf("\tbbuapp fwversion get             ----Get BBU FW Version\n");
	printf("\tbbuapp batterystat get           ----Get BBU Battery Status\n");
	printf("\tbbuapp temperature get           ----Get BBU Temperature\n");
	printf("\tbbuapp voltage get               ----Get BBU Voltage\n");
	printf("\tbbuapp current get               ----Get BBU Current\n");
	printf("\tbbuapp remaincap get             ----Get BBU Remain Capacity\n");
	printf("\tbbuapp fullchargcap get          ----Get BBU Full Charge Capacity\n");
	printf("\tbbuapp runtimetoempty get        ----Get BBU Run time to empty\n");
	printf("\tbbuapp reqtochange get           ----Suggest Change BBU\n");
	printf("\tbbuapp bmc send                  ----Send VPD&etc.data to BMC\n");
	printf("\tbbuapp bmc get                   ----Get VPD&etc.data from BMC\n");
	printf("\tbbuapp all get                   ----Get All BBU Information\n");
	printf("\tbbuapp test clear                ----clear BBU test result in ROM \n");
	printf("\tbbuapp bbu arm                   ----arm BBU for AC lost\n");
	printf("\tbbuapp bbu disarm                ----disarm BBU\n");
	printf("\tbbuapp bbu armstat               ----Get BBU Arm Status\n");
	printf( "****************************************************************"
	      "*************\n" );
}

int cls_bbu_output_orgnize(u8 reg, int index,char* output,char* out,u8 fun_tag)
{
	u32 result;
	
	switch (reg){
		case BBU_TMP:
			result = cls_bbu_hextoint(out);
			//sprintf(&output[0],"%s %d �K",CommandTable[index].commandDetail,(result/10)); //modify it to Cel
			sprintf(&output[0],"%s %d C",CommandTable[index].commandDetail,(result/10)-273); //modify it to Cel
			printf("%s\n",&output[0]);
			break;
		case BBU_VOLTAGE:
			result = cls_bbu_hextoint(out);
			sprintf(&output[0],"%s %d mV",CommandTable[index].commandDetail,result);
			printf("%s\n",&output[0]);
			break;
		case BBU_CURRENT:
			result = cls_bbu_hextoint(out);
			if(result>0x8000){	//the current has  signed process
				result = -(0x10000-result);
			}
			sprintf(&output[0],"%s %d mA",CommandTable[index].commandDetail,result*3);
			printf("%s\n",&output[0]);
			break;
		case BBU_REMAIN_CAP:
			result = cls_bbu_hextoint(out);
			sprintf(&output[0],"%s %d mA/mW",CommandTable[index].commandDetail,result*3);
			printf("%s\n",&output[0]);
			break;
		case BBU_FULLCHARGE_CAP:
			result = cls_bbu_hextoint(out);
			if(fun_tag){
				if(result*3 < 3400){
					printf("BBU Required to Change!!! CAP smaller than 3400mAh\n");
				}else{
					printf("BBU No Required to Change !\n");
				}
				break;
			}
			sprintf(&output[0],"%s %d mA/mW",CommandTable[index].commandDetail,result*3);
			printf("%s\n",&output[0]);
			break;
		case BBU_RUNTIME_EMP:
			result = cls_bbu_hextoint(out);
			sprintf(&output[0],"%s %d minutes",CommandTable[index].commandDetail,result);
			printf("%s\n",&output[0]);
			break;
		case BBU_BATTERY_STATUS:
			result = cls_bbu_hextoint(out);
			sprintf(&output[0],"%s",CommandTable[index].commandDetail);
			printf("%s\n",&output[0]);
			printf("OVER_CHARGED_ALARM\t\t%s\n",((result&BBU_OVER_CHARG_ALM) ? "yes":"no"));
			printf("TERMINATE_CHARGE_ALARM\t\t%s\n",((result&BBU_TERM_CHARG_ALM) ? "yes":"no"));
			printf("OVER_TEMPERATURE_ALARM\t\t%s\n",((result&BBU_OVER_TMP_ALM) ? "yes":"no"));
			printf("TERMINATE_DISCHARGE_ALARM\t%s\n",((result&BBU_TERM_DISCHARG_ALM) ? "yes":"no"));
			printf("INITIALIZED\t\t\t%s\n",((result&INITIALIZED) ? "yes":"no"));
			printf("DISCHARGING\t\t\t%s\n",((result&DISCHARGING) ? "yes":"no"));
			printf("FULLY_CHARGED\t\t\t%s\n",((result&FULLY_CHARGED) ? "yes":"no"));
			printf("FULLY_DISCHARGED\t\t%s\n",((result&FULLY_DISCHARGED) ? "yes":"no"));
			break;
		default:
			break;
	}


}

int cls_bbu_driver_check(void)
{
	u8 cmd[512];
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	u8	out[64];

	sprintf(cmd,"lsmod | grep i2c_i801 > data.log");

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		return FAIL;
	}

	fh = fopen(fname, "rb+");

	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		//printf("%s: i2c-i801 driver not insmod\n", __FUNCTION__);
		sprintf(cmd,"modprobe i2c-i801");
		if(SYSTEM(cmd) == FAIL){
			printf("bbu command fail!\n %s\n",cmd);
			fclose(fh);
			return FAIL;
		}
		//check again
		sprintf(cmd,"lsmod | grep i2c_i801 > data.log");
		if(SYSTEM(cmd) == FAIL){
			printf("bbu command fail!\n %s\n",cmd);
			fclose(fh);
			return FAIL;
		}
		fh = fopen(fname, "rb+");
		if(!fh){
			printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
			return FAIL;
		}
		fw_img_len = cls_bbu_get_file_length(fname);
		if(fw_img_len < 1){
			printf("Please install i2c-i801 driver!\n");
		}
		fclose(fh);
		return SUCCESS;
	}
	fclose(fh);
	return SUCCESS;
}

//this test function is removed and move to test app part
int cls_bbu_test_check(int i2c_num_I801,char i2c_dev)
{
	int  ret = SUCCESS;
#if 0
	u8  cmd[512];

	sprintf(cmd,"i2cset -y %d 0x%x 0x%x 0x%x",i2c_num_I801,i2c_dev,BBU_9551_LED,BBU_DUMP_ARMED_L);

	if(SYSTEM(cmd) == FAIL){
		printf("cls_bbu_test_command fail!\n");
		ret = FAIL;
	}
	
	sprintf(cmd,"i2cset -y %d 0x%x 0x%x 0x%x",i2c_num_I801,i2c_dev,BBU_9551_LED,BBU_TEST_L);
	
	if(SYSTEM(cmd) == FAIL){
		printf("cls_bbu_test_command fail!\n");
		ret = FAIL;
	}
#endif
	return ret;
}

/*
*cls_bbu_insert_check
* para: i2c device address
* try to get BBU insert status
* test device existence
* test presence value if equal 0
*/
int cls_bbu_insert_check(int i2c_num_I801,char i2c_dev)
{
	
	int  ret = SUCCESS;
	u8 cmd[512];
	u8 output[512] = {0};
	char out[2048];
	int a1 = 0;//a1,a2 temp val
	int a2 = 0;
	int LED_VAL = 0;  //pin value
	int PRST_L = 0;  //tmp presence val
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	u8   bbu_addr[10] = "0b";
	
	sprintf(cmd,"modprobe i2c-dev");
	
	if(SYSTEM(cmd) == FAIL){
		printf("bbu command i2c-dev fail!\n %s\n",cmd);
		fclose(fh);
		return FAIL;
	}
	sprintf(cmd,"modprobe i2c_i801");
	
	if(SYSTEM(cmd) == FAIL){
		printf("bbu command i2c_i801 fail!\n %s\n",cmd);
		fclose(fh);
		return FAIL;
	}
	
	sprintf(cmd,"i2cdetect -y %d > data.log",i2c_num_I801);

	if(SYSTEM(cmd) == FAIL){
		printf("cls_bbu_test_command fail!\n");
		return FAIL;
	}

	fh = fopen(fname, "rb+");

	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len);
		printf("Please make sure insert BBU and modprobe i2c-i801 driver!!!\n");
		fclose(fh);
		return FAIL;
	 }

	fseek(fh, 0, SEEK_SET);
	fw_data_len = fread(out, BYTE, fw_img_len, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}

	if(strstr(out,bbu_addr) == NULL){
		printf("BBU not inserted, Please insert BBU !!!\n");
		ret = FAIL;
	}

	fclose(fh);
	
	
	/*get PCA9551 input LED bit0~3 value*/
	sprintf(cmd,"i2cget -y %d 0x%02x 0x%02x > data.log",i2c_num_I801,i2c_dev,BBU_9551_INPUT);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		return FAIL;
	}

	fh = fopen(fname, "rb+");
	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len);
		fclose(fh);
		return FAIL;
	}

	fseek(fh, 0, SEEK_SET);
	fw_data_len = fread(out, BYTE, fw_img_len, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}

	fclose(fh);
	a1 = out[2];
	a2 = out[3];
	a1 = asc_to_int(a1);
	a2 = asc_to_int(a2);
	/*bit0:PRST_L,bit1:PCH_BBU_CHRG_RATE,bit2:DUMP_ARMED_L,bit3:BBU_TEST_L*/
	PRST_L = a2 & 0x01;
	
	/*check bbu insert*/
	if (PRST_L != 0)
	{
		printf("BBU not inserted, Please insert BBU !!!\n");
		return FAIL;
	}

	
	return ret;
}

int cls_bbu_send_data_check(u8* bmc_data,u8* bmc_get_data)
{
	int ret = SUCCESS;
	u8 tmp[2048] = {0};
	u8 tmp1[2048] = {0};
	int i = 0;
	int j = 0;

	sprintf(&tmp[0],"01 ");
	for(i=0;i<BBU_DATA_LEN;i++){
		sprintf(&tmp[3+i*3],"%02x ",bmc_data[i]);
		
	}

	for(i=0,j=1;i<(BBU_DATA_LEN*3+10),j<(BBU_DATA_LEN*3+10);i++,j++){
		if((bmc_get_data[j]=='\n' )){
			j=j+1;
		}
		tmp1[i] = bmc_get_data[j];
	}

	for(i=0;i<(BBU_DATA_LEN*3 + 1);i++){
		if(tmp[i] != tmp1[i]){
			return FAIL;
		}
	}

	return ret;
}

int cls_bbu_send_info_to_bmc(u8* bmc_data)
{
	int ret = SUCCESS;
	u8 cmd[512] = {0};

	sprintf(cmd,"ipmitool raw 0x3a 0x3b \
		0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \
		0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \
		0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \
		0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \
		0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \
		0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \
		0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \
		0x%02x 0x%02x 0x%02x\
		",bmc_data[0],bmc_data[1],bmc_data[2],bmc_data[3],bmc_data[4],bmc_data[5],
		bmc_data[6],bmc_data[7],bmc_data[8],bmc_data[9],bmc_data[10],bmc_data[11],
		bmc_data[12],bmc_data[13],bmc_data[14],bmc_data[15],bmc_data[16],bmc_data[17],
		bmc_data[18],bmc_data[19],bmc_data[20],bmc_data[21],bmc_data[22],bmc_data[23],
		bmc_data[24],bmc_data[25],bmc_data[26],bmc_data[27],bmc_data[28],bmc_data[29],
		bmc_data[30],bmc_data[31],bmc_data[32],bmc_data[33],bmc_data[34],bmc_data[35],
		bmc_data[36],bmc_data[37],bmc_data[38],bmc_data[39],bmc_data[40],bmc_data[41],
		bmc_data[42],bmc_data[43],bmc_data[44],bmc_data[45],bmc_data[46],bmc_data[47],
		bmc_data[48],bmc_data[49],bmc_data[50],bmc_data[51],bmc_data[52],bmc_data[53],
		bmc_data[54],bmc_data[55],bmc_data[56],bmc_data[57],bmc_data[58]);

	if(SYSTEM(cmd) == FAIL){
		printf("ERROR: ipmitool raw 0x3a 0x3b fail!\n");
		ret = FAIL;
	}

	return ret;

}

void cls_bbu_send_info_to_bmc_data_assemble(u8 *input, u8 *input1,u8 *bmc_data)
{
	int i = 0;
	int j = 0;
	u8 output[2048] = {0};
	u8 out[2048] = {0};
	u8 out1[2048] = {0};
	u8 tmpbuf[16] = "0x";
	unsigned int value;
	u8 reqtochange = 0;
	int m1 = 0,m2 = 0;
	int m3 = 0,m4 = 0;
	int n1 = 0,n2 = 0;

	memcpy(out,input,sizeof(u8)*512);
	memcpy(out1,input1,sizeof(u8)*512);
	
	/*partnum*/
	strncpy(&output[0], &out[BMC_PN_OFF], 0x0e);
	/*snnum*/
	strncpy(&output[0x0e],&out[BMC_SN_OFF], 0x10);
	strncpy(&output[0x1e],&out[BMC_SN_OFF+72], 0x3);
	/*manufact*/
	strncpy(&output[0x21],&out[BMC_MAN_OFF], 0x09);
	/*fwversion*/
	m1 = out[256];
	m2 = out[257];
	m3 = out[259];
	m4 = out[260];
	m1 = asc_to_int(m1);
	m2 = asc_to_int(m2);
	m3 = asc_to_int(m3);
	m4 = asc_to_int(m4);
	output[0x2a]=m1*16+m2;
	output[0x2b]=m3*16+m4;
	
    //printf("fw version test\t%x %x,%x %x  output  %02x %02x \n",m1,m2,m3,m4,output[0x2a],output[0x2b]);

	/*fwversion*/
	//strncpy(&output[0x2a],&out[BMC_FWV_OFF], 0x2);
	/*batterystat*/
	strncpy(&tmpbuf[2],&out1[BMC_STA_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	output[0x2c] = (value & 0xff00) >> 8;
	output[0x2d] = value & 0x00ff;
	/*temperature*/
	strncpy(&tmpbuf[2],&out1[BMC_TMP_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	output[0x2e] = (value & 0xff00) >> 8 ;
	output[0x2f] = value & 0x00ff;
	/*voltage*/
	strncpy(&tmpbuf[2],&out1[BMC_VOL_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	output[0x30] = (value & 0xff00) >> 8 ;
	output[0x31] = value & 0x00ff;
	/*current*/
	strncpy(&tmpbuf[2],&out1[BMC_CUR_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	output[0x32] = (value & 0xff00) >> 8 ;
	output[0x33] = value & 0x00ff;
	/*remaincap*/
	strncpy(&tmpbuf[2],&out1[BMC_REM_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	output[0x34] = (value & 0xff00) >> 8 ;
	output[0x35] = value & 0x00ff;
	/*fullchargecap*/
	strncpy(&tmpbuf[2],&out1[BMC_FUL_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	output[0x36] = (value & 0xff00) >> 8 ;
	output[0x37] = value & 0x00ff;
	if(value*3 < 3400){
		reqtochange = 1;
	}
	/*runtimetoempty*/
	strncpy(&tmpbuf[2],&out1[BMC_RUN_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	output[0x38] = (value & 0xff00) >> 8 ;
	output[0x39] = value & 0x00ff;
	/*reqtochange*/
	output[0x3a] = reqtochange;

	memcpy(bmc_data,output,512);

	return;

}

int cls_bbu_send_info_to_bmc_command(int i2c_num_I801)
{
	int  ret = SUCCESS;
	u8 cmd[1024];
	u8 bmc_data[2048] = {0};
	u8 out[2048] = {0};
	u8 out1[2048] = {0};
	u8 bmc_get_data[2048] = {0};
	char* tmp;
	FILE *fh,*fh1;
	u8 *fname = "data.log";
	u8 *fname1 = "data1.log";
	u32  fw_img_len = 0;
	u32  fw_img_len1 = 0;
	u32 fw_data_len = 0;
	int i = 0;
	int m = 0;
	int n = 0;
	
	sprintf(cmd,"i2cdump -y %d 0x54 >data.log b",i2c_num_I801);
	if(SYSTEM(cmd) == FAIL){
		printf("cls_bbu_send_i2c_get_command fail!\n");
		ret = FAIL;
	}
	
	sprintf(cmd,"i2cdump -y %d 0x0b w >data1.log",i2c_num_I801);
	if(SYSTEM(cmd) == FAIL){
		printf("cls_bbu_send_i2c_get_command fail!\n");
		ret = FAIL;
	}

	fh = fopen(fname, "rb+");
	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fh1 = fopen(fname1, "rb+");
	if(!fh1){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		fclose(fh);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len);
		printf("Please make sure insert BBU and modprobe i2c-i801 driver!!!\n");
		fclose(fh);
		return FAIL;
	 }

	fw_img_len1 = cls_bbu_get_file_length(fname1);
	if(fw_img_len1 < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len1);
		printf("Please make sure insert BBU and modprobe i2c-i801 driver!!!\n");
		fclose(fh1);
		fclose(fh);
		return FAIL;
	 }

	fseek(fh, 0, SEEK_SET);
	fw_data_len = fread(out, BYTE, fw_img_len, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}
	//printf("out origin is\n%s",out);	
	fseek(fh1, 0, SEEK_SET);
	fw_data_len = fread(out1, BYTE, fw_img_len1, fh1);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		fclose(fh1);
		return FAIL;
	}

	cls_bbu_send_info_to_bmc_data_assemble(out,out1,bmc_data);
	ret = cls_bbu_send_info_to_bmc(bmc_data);
	if(ret == FAIL){
		fclose(fh);
		fclose(fh1);
		return FAIL;
	}

	ret = cls_bbu_get_info_from_bmc_command(&bmc_get_data[0]);
	if(ret == FAIL){
		fclose(fh);
		fclose(fh1);
		return FAIL;
	}

	ret = cls_bbu_send_data_check(bmc_data,bmc_get_data);
	if(ret == FAIL){
		printf("ERROR: bmc send fail!send&receive data different!\n");
		fclose(fh);
		fclose(fh1);
		return FAIL;
	}
	fclose(fh);
	fclose(fh1);

	return ret;
	
}

/*
* cls_bbu_get_info_from_bmc_command
* dump BMC BBU information by ipmi cmds
*/
int cls_bbu_get_info_from_bmc_command(u8 *data)
{
	int ret = SUCCESS;
	u8 cmd[512] = {0};
	u8 out[2048] = {0};
	FILE *fh;
	u8 *fname = "data2.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	
	sprintf(cmd,"ipmitool raw 0x3a 0x3a >data2.log");

	if(SYSTEM(cmd) == FAIL){
		printf("cls_bbu_get_info_from_bmc_command fail!\n");
		ret = FAIL;
	}

	fh = fopen(fname, "rb+");

	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len);
		printf("Please make sure insert BBU and modprobe i2c-i801 driver!!!\n");
		fclose(fh);
		return FAIL;
	 }

	fseek(fh, 0, SEEK_SET);
	fw_data_len = fread(out, fw_img_len, BYTE, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}

	memcpy(data,out,fw_img_len);
	fclose(fh);

	return ret;
}

/*
* cls_bbu_send_i2c_get_command
* Get data
*/
int cls_bbu_send_i2c_get_command(int i2c_num_I801,char i2c_dev,char reg)
{
	int  ret = SUCCESS;
	u8 cmd[512];
	u8 output[2048] = {0};
	char out[64];
	int i = 0;
	int index = 0;
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	u8   fun_tag = 0;

	if(reg == BMC_GET){
		ret = cls_bbu_get_info_from_bmc_command(&output[0]);
		printf("%s\n",output);
		return ret;
	}
	if(reg == BBU_PART_NUM ||reg == BBU_SN_NUM ||
	   reg == BBU_MANUFAC || reg == BBU_FW_VERSION){
		ret = cls_bbu_send_i2c_get_string_command(i2c_num_I801,BBU_EEPROM,reg);
		return ret;
	}
	if(reg == BBU_REQ_CHARG){
		fun_tag = 1;
		reg = BBU_FULLCHARGE_CAP;
	}
	//BBU_REQ_CHARG/BBU_TMP run here
	sprintf(cmd,"i2cget -y %d 0x%x 0x%x w >data.log",i2c_num_I801,i2c_dev,reg);

	if(SYSTEM(cmd) == FAIL){
		printf("cls_bbu_send_i2c_get_command fail!\n");
		ret = FAIL;
	}

	fh = fopen(fname, "rb+");

	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len);
		printf("Please make sure insert BBU and modprobe i2c-i801 driver!!!\n");
		fclose(fh);
		return FAIL;
	 }

	fseek(fh, 0, SEEK_SET);
	fw_data_len = fread(out, fw_img_len, CLS_BBU_I2C_GET_SIZE, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}
	for (index = 0; index < (sizeof (CommandTable)/sizeof(CommandTable[0])); index++){
		if (CommandTable[index].commandOpcode == reg){
			cls_bbu_output_orgnize(reg,index,output,out,fun_tag);
			break;
		}
       }
	if(index == (sizeof (CommandTable)/sizeof(CommandTable[0]))){
		sprintf(&output[0],"command opcode nomatch %x %x",CommandTable[index].commandOpcode,reg);
	}
	
	fclose(fh);
	return ret;
}

int cls_bbu_send_i2c_get_string_command(int i2c_num_I801,char i2c_dev,char reg)
{
	int  ret = SUCCESS;
	u8 cmd[512];
	u8 output[2048] = {0};
	u8 out[2048] = {0};
	char* tmp;
	int i = 0;
	int index =0;
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;

	sprintf(cmd,"i2cdump -y %d 0x54 >data.log b",i2c_num_I801);

	if(SYSTEM(cmd) == FAIL){
		printf("cls_bbu_send_i2c_get_command fail!\n");
		ret = FAIL;
	}

	fh = fopen(fname, "rb+");

	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len);
		printf("Please make sure insert BBU and modprobe i2c-i801 driver!!!\n");
		fclose(fh);
		return FAIL;
	 }

	fseek(fh, PART_NUM_START, SEEK_SET);
	fw_data_len = fread(out, BYTE, fw_img_len, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}

	switch(reg){
		case BBU_PART_NUM:
			out[PART_NUM_END] = '\0';
			for (index = 0; index < (sizeof (CommandTable)/sizeof(CommandTable[0])); index++){
				if (CommandTable[index].commandOpcode == reg){
					sprintf(&output[0],"%s %s\n",CommandTable[index].commandDetail,out);
					printf("%s",output);
					break;
				}
      	 		}
		break;
		case BBU_SN_NUM:
			strcpy(&out[SN_NUM_PART1],&out[SN_NUM_PART2]);
			out[SN_NUM_END] = '\0';
			for (index = 0; index < (sizeof (CommandTable)/sizeof(CommandTable[0])); index++){
				if (CommandTable[index].commandOpcode == reg){
					sprintf(&output[0],"%s %s\n",CommandTable[index].commandDetail,&out[SN_NUM_START]);
					printf("%s",output);
					break;
				}
      	 		}
		break;
		case BBU_MANUFAC:
			out[MANUFAC_END] = '\0';
			for (index = 0; index < (sizeof (CommandTable)/sizeof(CommandTable[0])); index++){
				if (CommandTable[index].commandOpcode == reg){
					sprintf(&output[0],"%s %s\n",CommandTable[index].commandDetail,&out[MANUFAC_START]);
					printf("%s",output);
					break;
				}
      	 		}
		break;
		case BBU_FW_VERSION:
			for (index = 0; index < (sizeof (CommandTable)/sizeof(CommandTable[0])); index++){
				if (CommandTable[index].commandOpcode == reg){
					sprintf(&output[0],"%s %c%c%c%c\n",CommandTable[index].commandDetail,
						out[VERSION_B1],out[VERSION_B2],out[VERSION_B3],out[VERSION_B4]);
					printf("%s",output);
					break;
				}
      	 		}
		break;
		default:
		break;
	}
	
	fclose(fh);
	return ret;
}

int cls_bbu_get_file_length(const char *filename)
{
	int length = -1;
	struct stat file_stat;
	
	if(0 == stat(filename, &file_stat)){
		length = file_stat.st_size;
	}
	
	return length;
}

int cls_bbu_convert_string_to_lower(char *buffer)
{
	char *s;
	
	/*convert to lower case*/
	for (s=buffer; *s; s++)
	*s=(char)tolower(*s);
	
	return 0;
}

unsigned int cls_bbu_hextoint(const char *hex)
 {
	unsigned int result = 0;
	char tmp[32] = {0};
	int i = 0;
	
	memcpy(tmp,hex+2,4);

	while (tmp[i])
	{
		if (tmp[i] > 47 && tmp[i] < 58)
			result += (tmp[i] - 48);
		else if (tmp[i]> 64 && tmp[i] < 71)
			result += (tmp[i] - 55);
		else if (tmp[i] > 96 && tmp[i] < 103)
			result += (tmp[i] - 87);

		if (tmp[++i])
			result <<= 4;
	}
	
	return result;
}

int asc_to_int(int i)
{
	if (i >= 48 && i <=57)
	i = i - 48;
	else if (i >= 65 && i <= 70)
	i = i-55;
	else if (i >= 97 && i <= 102)
	i = i -87;
	else
	return -1;
	return i;
}

/*
* clear_BBU_TestBit
* no parameter
* Action: Clear VPD in BBU offset 0x70 byte to zero
* clean test result if BBU test failure
* return: 0 SUCCESS, -1 FAIL
*/
int clear_BBU_TestBit(int i2c_num_I801)
{
	int  ret = SUCCESS;
	u8 cmd[512];
	/*test successfully write 0x00 to BBU VPD offset 0x70 byte*/
	sprintf(cmd,"i2cset -y %d 0x54 0x70 0x00",i2c_num_I801);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
	return ret;
}

/*
* arm_bbu
* arm_bbu(BBU_9551)
* set ARM pin to 0 of PCA9551
* para: i2c_dev 
*/
int arm_bbu(int i2c_num_I801,char i2c_dev)
{
	int  ret = SUCCESS;
	u8  cmd[512];
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	u8	out[64];
	int a1 = 0,a2 = 0;
	int LED_VAL = 0,PRST_L = 0;

	/*get LED bit0~3 value*/
	sprintf(cmd,"i2cget -y %d 0x%02x 0x%02x > data.log",i2c_num_I801,i2c_dev,BBU_9551_LED);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}

	fh = fopen(fname, "rb+");
	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len);
		fclose(fh);
		return FAIL;
	}

	fseek(fh, 0, SEEK_SET);
	fw_data_len = fread(out, BYTE, fw_img_len, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}

	fclose(fh);
	a1 = out[2];
	a2 = out[3];
	a1 = asc_to_int(a1);
	a2 = asc_to_int(a2);
	/*bit0:PRST_L,bit1:PCH_BBU_CHRG_RATE,bit2:DUMP_ARMED_L,bit3:BBU_TEST_L*/

	/*set DUMP_ARMED_L*/
	a1 = a1 & 0x0c;
	LED_VAL = a1*16+a2;

	sprintf(cmd,"i2cset -y %d 0x%02x 0x%02x 0x%02x> data.log",i2c_num_I801,i2c_dev,BBU_9551_LED,LED_VAL);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
	return ret;
}
	
	
	/*
* disarm_bbu
* disarm_bbu(BBU_9551)
* set ARM pin to 1 of PCA9551
* para: i2c_dev 
*/
int disarm_bbu(int i2c_num_I801,char i2c_dev)
{
	int  ret = SUCCESS;
	u8  cmd[512];
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	u8	out[64];
	int a1 = 0,a2 = 0;
	int LED_VAL = 0,PRST_L = 0;

	/*get PCA9551 input LED bit0~3 value*/
	sprintf(cmd,"i2cget -y %d 0x%02x 0x%02x > data.log",i2c_num_I801,i2c_dev,BBU_9551_LED);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		return FAIL;
	}

	fh = fopen(fname, "rb+");
	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len);
		fclose(fh);
		return FAIL;
	}

	fseek(fh, 0, SEEK_SET);
	fw_data_len = fread(out, BYTE, fw_img_len, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}

	fclose(fh);
	a1 = out[2];
	a2 = out[3];
	a1 = asc_to_int(a1);
	a2 = asc_to_int(a2);
	/*disarm bbu*/
	/*assert DUMP_ARMED_L signal high*/	
	a1 = a1 & 0x0c;
	a1 = a1 | 0x01;
	LED_VAL = a1*16+a2;

	sprintf(cmd,"i2cset -y %d 0x%02x 0x%02x 0x%02x> data.log",i2c_num_I801,i2c_dev,BBU_9551_LED,LED_VAL);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
  return ret;

	
}

/*
* check_bbu_arm_status
* check_bbu_arm_status(BBU_9551)
* check ARM pin of PCA9551
* para: i2c_dev 
*/
int check_bbu_arm_status(int i2c_num_I801,char i2c_dev)
{
	int  ret = SUCCESS;
	u8  cmd[512];
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	u8	out[64];
	int a1 = 0; //tmp val

	/*get PCA9551 input LED bit0~3 value*/
	sprintf(cmd,"i2cget -y %d 0x%02x 0x%02x > data.log",i2c_num_I801,i2c_dev,BBU_9551_LED);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		return FAIL;
	}

	fh = fopen(fname, "rb+");
	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}

	fw_img_len = cls_bbu_get_file_length(fname);
	if(fw_img_len < 1){
		printf("%s: ERROR: Image file size = %d\n", __FUNCTION__, fw_img_len);
		fclose(fh);
		return FAIL;
	}

	fseek(fh, 0, SEEK_SET);
	fw_data_len = fread(out, BYTE, fw_img_len, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}

	fclose(fh);
	a1 = out[2];
	a1 = asc_to_int(a1);

	/*disarm bbu*/
	/*assert DUMP_ARMED_L signal high*/	
	a1 = a1 & 0x03;
	if(a1 == BBU_ARM){
		printf("BBU ARMED!\n");
	}else if (a1==BBU_DISARM) 
	{
		printf("BBU DISARMED!\n");
	}else{
			printf("BBU ARM ABNORMAL STATUS!!!\n");
	}



  return ret;

	
}
int cls_get_i2c_num_I801(){

  char i2c_num_I801_c[10];
  int i2c_num_I801 = 0;
  u8 cmd[512];
	FILE *fh;
  u8 *fname = "i2c_num_I801.log";
  sprintf(cmd,"i2cdetect -l|grep -i 'i801'|awk '{print $1}'|cut -d '-' -f 2 >i2c_num_I801.log");
  if(SYSTEM(cmd) == FAIL ){
    printf("get i2c num fail!\n");
    return FAIL;
  }
  fh = fopen(fname, "r");

	if(!fh){
		printf("%s: ERROR: could not open file %s\n", __FUNCTION__,fname);
		return FAIL;
	}
  fgets(i2c_num_I801_c,10,fh);
  fclose(fh);
  i2c_num_I801=atoi(i2c_num_I801_c);
  return i2c_num_I801;

}

