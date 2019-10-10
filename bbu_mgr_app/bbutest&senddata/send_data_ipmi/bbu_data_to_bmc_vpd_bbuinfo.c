/*******************************************************************************
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
#include <unistd.h>
#include "bbuapp.h"

/*****************************************************************************

  Purpose:
  main program entry point

  Parameters:
  argc argv   - STD ARG input

  Return:
  none

*****************************************************************************/
int main()
{
	u32	ret		= SUCCESS;
	u8	Opcode	= 0;
	int	index	= 0;
	u8 bmc_data[2048] = {0};
	char	*token	= NULL;
	int count_num = 0;
	int i2c_num_I801 = 0;


	/*step 1: check whether i2c driver is installed*/
	if(cls_bbu_driver_check() == FAIL){
		return FAIL;
	}
	i2c_num_I801=i2c_num_I801=cls_get_i2c_num_I801();

	/*step 2: check whether BBU is inserted*/
	if(cls_bbu_insert_check(i2c_num_I801) == FAIL){
			return FAIL;
	}

	/*step 3: check whether BBU  is enabled*/
	/*purely just get Gauge chip data and send to BMC*/
	/*if(cls_bbu_test_check_stillarm(BBU_9551) == FAIL){
		return FAIL;
	}
	*/
	/*
	while(1){
		ret = cls_bbu_send_info_to_bmc_command();
		sleep(8);
	}
	*/
	while(1){
		while(1){
			ret = cls_bbu_send_vpdinfo_to_bmc_data_command(i2c_num_I801,bmc_data);
			if(ret == SUCCESS){
				break;
			}
		}
		while(count_num < 500){
			ret = cls_bbu_send_info_to_bmc_command(i2c_num_I801,bmc_data);
			count_num++;
			sleep(8);
		}
		count_num = count_num-500;	    
	}
	return ret;	
}


int cls_bbu_driver_check(void)
{
	u8 cmd[512];
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	u8	out[64];

	sprintf(cmd,"modprobe i2c_dev");
	
	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		return FAIL;
	}

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
		printf("%s: i2c-i801 driver not insmod\n", __FUNCTION__);
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

int cls_bbu_insert_check(int i2c_num_I801)
{
	
	int  ret = SUCCESS;
	u8 cmd[512];
	u8 output[512] = {0};
	char out[2048];
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	u8   bbu_addr[10] = "0b";

	/*new code*/
	sprintf(cmd,"i2cdetect -y %d|grep -q 0b",i2c_num_I801);

	if(system(cmd) != 0 ){
		if(SYSTEM(cmd) == FAIL){
		    printf("bbu command fail!\n %s\n",cmd);
		    return FAIL;
	    }
	    else
		printf("BBU not inserted, Please insert BBU !!!\n");
		return FAIL;
	}
	/* end*/
	

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
	return ret;
}

int cls_bbu_last_status(int i2c_num_I801)
{
	int  ret = SUCCESS;
	u8 cmd[512];
	char status_out[10];
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	u8   bbu_test_stat[10] = "55";
	
	/*get the value in VPD EEPROM*/
	sprintf(cmd,"i2cget -y %d 0x54 0x70 > data.log",i2c_num_I801);

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
	fw_data_len = fread(status_out, BYTE, fw_img_len, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}

	/*check "0x55"*/
	if(strstr(status_out,bbu_test_stat) != NULL){
		printf("Last BBU test failed, Please check it !!!\n");
		printf("If you want to run this program,set VPD EEPROM (OFFSET :0x70) first,\n");
		printf("for example,#i2cset -y %d 0x54 0x70 0x00\n",i2c_num_I801);
		ret = FAIL;
	}

	fclose(fh);
	return ret;
}

/*
*cls_bbu_test_check
* test BBU
*/

int cls_bbu_test_check_stillarm(int i2c_num_I801,char i2c_dev)
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
	/*PRST_L = a2 & 0x03;*/
	
	/*check bbu insert*/
	/*if (PRST_L != 0)
	{
		printf("BBU not inserted, Please insert BBU !!!\n");
		return FAIL;
	}*/

	if(cls_bbu_last_status(i2c_num_I801) == FAIL){
		return FAIL;
	}
	/*bbu test*/
	sprintf(cmd,"i2cset -y %d 0x54 0x70 0x55",i2c_num_I801);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
	/*set DUMP_ARMED_L*/
	a1 = a1 & 0x0c;
	LED_VAL = a1*16+a2;

	sprintf(cmd,"i2cset -y %d 0x%02x 0x%02x 0x%02x> data.log",i2c_num_I801,i2c_dev,BBU_9551_LED,LED_VAL);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
	
	/*set BBU_TEST_L*/
	a1 = a1 & 0x03;
	LED_VAL = a1*16+a2;
	sprintf(cmd,"i2cset -y %d 0x%02x 0x%02x 0x%02x> data.log",i2c_num_I801,i2c_dev,BBU_9551_LED,LED_VAL);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
	//suggest add 1 second delay
	sleep(1);
	/*test successfully*/
	sprintf(cmd,"i2cset -y %d 0x54 0x70 0x00",i2c_num_I801);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
	/*set TEST back for next test if necessary & BBU still armed*/
	a1 = a1 | 0x04;
	LED_VAL = a1*16+a2;
	sprintf(cmd,"i2cset -y %d 0x%02x 0x%02x 0x%02x> data.log",i2c_num_I801,i2c_dev,BBU_9551_LED,LED_VAL);
	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
	return ret;
}



int cls_bbu_send_data_check(u8* bmc_data,u8* bmc_get_data)
{
	int ret = SUCCESS;
	u8 tmp[2048] = {0};
	u8 tmp1[2048] = {0};
	int i = 0,j = 0;

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

int cls_bbu_send_vpdinfo_to_bmc_data_command(int i2c_num_I801,u8* bmc_data)
{
	int  ret = SUCCESS;
	u8 cmd[1024];
	//u8 bmc_data[2048] = {0};
	u8 out[2048] = {0};
	u8 bmc_get_data[2048] = {0};
	char* tmp;
	FILE *fh;
	u8 *fname = "data.log";
	u32  fw_img_len = 0;
	u32 fw_data_len = 0;
	int i = 0;
	int m = 0,n = 0;
	
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

	fseek(fh, 0, SEEK_SET);
	fw_data_len = fread(out, BYTE, fw_img_len, fh);
	if(fw_data_len < 1){
		printf("%s: ERROR: file data size = %d\n", __FUNCTION__, fw_data_len);
		fclose(fh);
		return FAIL;
	}

	cls_bbu_send_vpdinfo_to_bmc_data_assemble(out,bmc_data);

	fclose(fh);


	return ret;
	
}

void cls_bbu_send_vpdinfo_to_bmc_data_assemble(u8 *input,u8* bmc_data)
{
	u8 out[2048] = {0};
	int m1 = 0,m2 = 0;
	int m3 = 0,m4 = 0;

	memcpy(out,input,sizeof(u8)*512);
	
	/*partnum*/
	strncpy(&bmc_data[0], &out[BMC_PN_OFF], 0x0e);
	/*snnum*/
	strncpy(&bmc_data[0x0e],&out[BMC_SN_OFF], 0x10);
	strncpy(&bmc_data[0x1e],&out[BMC_SN_OFF+72], 0x3);
	/*manufact*/
	strncpy(&bmc_data[0x21],&out[BMC_MAN_OFF], 0x09);
	/*fwversion*/
	m1 = out[256];
	m2 = out[257];
	m3 = out[259];
	m4 = out[260];
	m1 = asc_to_int(m1);
	m2 = asc_to_int(m2);
	m3 = asc_to_int(m3);
	m4 = asc_to_int(m4);
	bmc_data[0x2a]=m1*16+m2;
	bmc_data[0x2b]=m3*16+m4;

	return;

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

void cls_bbu_send_bbuinfo_to_bmc_data_assemble(u8 *input1,u8 *bmc_data)
{
	int i = 0,j = 0;
	u8 out1[2048] = {0};
	u8 tmpbuf[16] = "0x";
	unsigned int value = 0;
	u8 reqtochange = 0;
	
	//memcpy(out,input,sizeof(u8)*512);
	memcpy(out1,input1,sizeof(u8)*512);
	
	/*partnum*/
	//strncpy(&bmc_data[0], &out[BMC_PN_OFF], 0x0e);
	/*snnum*/
	//strncpy(&bmc_data[0x0e],&out[BMC_SN_OFF], 0x10);
	//strncpy(&bmc_data[0x1e],&out[BMC_SN_OFF+72], 0x3);
	/*manufact*/
	//strncpy(&bmc_data[0x21],&out[BMC_MAN_OFF], 0x09);
	/*fwversion*/
	/*m1 = out[256];
	m2 = out[257];
	m3 = out[259];
	m4 = out[260];
	m1 = asc_to_int(m1);
	m2 = asc_to_int(m2);
	m3 = asc_to_int(m3);
	m4 = asc_to_int(m4);
	bmc_data[0x2a]=m1*16+m2;
	bmc_data[0x2b]=m3*16+m4;*/
	
    

	/*fwversion*/
	
	/*batterystat*/
	strncpy(&tmpbuf[2],&out1[BMC_STA_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	bmc_data[0x2c] = (value & 0xff00) >> 8;
	bmc_data[0x2d] = value & 0x00ff;
	/*temperature*/
	strncpy(&tmpbuf[2],&out1[BMC_TMP_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	bmc_data[0x2e] = (value & 0xff00) >> 8 ;
	bmc_data[0x2f] = value & 0x00ff;
	/*voltage*/
	strncpy(&tmpbuf[2],&out1[BMC_VOL_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	bmc_data[0x30] = (value & 0xff00) >> 8 ;
	bmc_data[0x31] = value & 0x00ff;
	/*current*/
	strncpy(&tmpbuf[2],&out1[BMC_CUR_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	bmc_data[0x32] = (value & 0xff00) >> 8 ;
	bmc_data[0x33] = value & 0x00ff;
	/*remaincap*/
	strncpy(&tmpbuf[2],&out1[BMC_REM_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	bmc_data[0x34] = (value & 0xff00) >> 8 ;
	bmc_data[0x35] = value & 0x00ff;
	/*fullchargecap*/
	strncpy(&tmpbuf[2],&out1[BMC_FUL_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	bmc_data[0x36] = (value & 0xff00) >> 8 ;
	bmc_data[0x37] = value & 0x00ff;
	if(value*3 < 3400){
		reqtochange = 1;
	}
	/*runtimetoempty*/
	strncpy(&tmpbuf[2],&out1[BMC_RUN_OFF], 0x4);
	value  = cls_bbu_hextoint(&tmpbuf[0]);
	bmc_data[0x38] = (value & 0xff00) >> 8 ;
	bmc_data[0x39] = value & 0x00ff;
	/*reqtochange*/
	bmc_data[0x3a] = reqtochange;

	
	return;

}

int cls_bbu_send_info_to_bmc_command(int i2c_num_I801,u8* bmc_data)
{
	int  ret = SUCCESS;
	u8 cmd[1024];
	//u8 bmc_data[2048] = {0};
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
	int m = 0,n = 0;
	
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

	cls_bbu_send_bbuinfo_to_bmc_data_assemble(out1,bmc_data);
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