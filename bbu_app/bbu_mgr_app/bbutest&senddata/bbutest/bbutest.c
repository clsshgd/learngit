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

int main(int argc, char* argv[])
{
	u32	ret		= SUCCESS;

	u8	Opcode	= 0;
	int	index	= 0;
	char	*token	= NULL;
	int i2c_num_I801 = 0;
	
	
	
	printf("begin BBU test\n");
	/*step 1: check whether i2c driver is installed*/
	if(cls_bbu_driver_check() == FAIL){
		printf("BBU driver check failure\n");
		return FAIL;
	}
	i2c_num_I801=i2c_num_I801=cls_get_i2c_num_I801();

	/*step 2: check whether BBU is inserted*/
	if(cls_bbu_insert_check(i2c_num_I801) == FAIL){
			printf("BBU not inserted\n");
			return FAIL;
	}
	/*step 3: check last bbu test status*/
	if(cls_bbu_last_status(i2c_num_I801) == FAIL){
		return FAIL;
	}
	
	/*step 4: check whether BBU  is enabled*/
	if(cls_bbu_test_check(i2c_num_I801,BBU_9551) == FAIL){
			printf("BBU test failures\n");
			return FAIL;
	}
	printf("BBU test passed\n");
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
	sprintf(cmd,"modprobe i2c_i801");
	
	if(SYSTEM(cmd) == FAIL){
		printf("bbu command i2c_i801 fail!\n %s\n",cmd);
		return FAIL;
	}

	sprintf(cmd,"modprobe i2c-dev");
	
	if(SYSTEM(cmd) == FAIL){
		printf("bbu command i2c-dev fail!\n %s\n",cmd);
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
		sprintf(cmd,"modprobe i2c_i801");
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
			printf("Please install i2c_i801 driver!\n");
		}
		fclose(fh);
		return SUCCESS;
	}
	fclose(fh);
	return SUCCESS;
}

int cls_bbu_last_status(int i2c_num_I801)
{
	int  ret = SUCCESS;
	u8 cmd[512];
	u8 output[512] = {0};
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
		ret = FAIL;
	}

	fclose(fh);
	return ret;

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
	
//	sprintf(cmd,"modprobe i2c-dev");
	
//	if(SYSTEM(cmd) == FAIL){
//		printf("bbu command fail!\n %s\n",cmd);
//		return FAIL;
//	}

	sprintf(cmd,"i2cdetect -y %d > data.log",i2c_num_I801);

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

int cls_bbu_test_check(int i2c_num_I801,char i2c_dev)
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
	int tmpPinVal = 0; //use to store PCA9551 0x05 PIN2/3 value

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
	//record original value for after test recovery //cn28059704
	a1 = out[2];
	a2 = out[3];
	a1 = asc_to_int(a1);
	a2 = asc_to_int(a2);
	tmpPinVal = a1;
	/*bit0:PRST_L,bit1:PCH_BBU_CHRG_RATE,bit2:DUMP_ARMED_L,bit3:BBU_TEST_L*/
	
	/*if(cls_bbu_last_status(i2c_num_I801) == FAIL){
		return FAIL;
	}*/
	/*bbu test*/
	printf("Begin BBU test\n");
	
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
	//suggest add 1 second delay
	sleep(1);
	
	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
  
	//recovery 0x05 register Test bit and arm bit prepare for next manual test//cn28059704
	/*test successfully write 0x00 to BBU VPD offset 0x70 byte*/
	sprintf(cmd,"i2cset -y %d 0x54 0x70 0x00",i2c_num_I801);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}

	//restore test & arm byte back to PCA9551 register
	a1 = tmpPinVal;
	LED_VAL = a1*16+a2;
	sprintf(cmd,"i2cset -y %d 0x%02x 0x%02x 0x%02x> data.log",i2c_num_I801,i2c_dev,BBU_9551_LED,LED_VAL);

	if(SYSTEM(cmd) == FAIL){
		printf("bbu command fail!\n %s\n",cmd);
		ret = FAIL;
	}
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