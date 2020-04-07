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


#ifndef _BBUAPP_H_
#define _BBUAPP_H_

#include <stdint.h>

/*variable definiation*/
#define SUCCESS             0
#define FAIL               -1
#define FALSE               0
#define TRUE                1

#define BYTE 1
#define WORD 2
#define DWORD 4
#define QWORD 8
#define BYTE_MASK  0x000000ff
#define WORD_MASK  0x0000ffff
#define DWORD_MASK 0xffffffff
#define PART_NUM_START 129
#define PART_NUM_END  14
#define SN_NUM_START	   70
#define SN_NUM_PART1	86
#define SN_NUM_PART2	142
#define SN_NUM_END	89
#define MANUFAC_START 145
#define MANUFAC_END	154
#define VERSION_B1	127
#define VERSION_B2	128
#define VERSION_B3	130
#define VERSION_B4	131

#define SYSTEM(input) system(input)
#define CLS_BBU_I2C_DUMP_SIZE 256
#define CLS_BBU_I2C_GET_SIZE 4
#define CLS_BBU_I2C_SET_SIZE 4
#define BBU_EEPROM 0x54
#define BBU_FUEL_GAUGE 0x0b
#define BBU_9551 0x67
#define BBU_TEST_L 0x00	
#define BBU_DUMP_ARMED_L 0x00
#define BBU_PRST_L  0x01
#define BBU_9551_LED 0x5


typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;


/*BBU commands definiation*/
/*BMC tag*/
#define BMC_GET						0x01
/*BBU inventory*/
#define BBU_PART_NUM				0x20
#define BBU_SN_NUM					0x1c
#define BBU_MANUFAC					0x0
#define BBU_FW_VERSION				0x1a
/*BBU management*/
#define BBU_BATTERY_STATUS			0x16
#define BBU_TMP						0x08
#define BBU_VOLTAGE					0x09
#define BBU_CURRENT					0x0a
#define BBU_REMAIN_CAP				0x0f
#define BBU_FULLCHARGE_CAP		0x10
#define BBU_RUNTIME_EMP			0x11
/*BBU alert*/
#define BBU_OVER_CHARG_ALM		0x8000
#define BBU_TERM_CHARG_ALM		0x4000
#define BBU_OVER_TMP_ALM			0x1000
#define BBU_TERM_DISCHARG_ALM	0x0800
#define BBU_FUN_FAIL					0x30		
#define BBU_REQ_CHARG				0x31		
#define INITIALIZED						0x0080
#define DISCHARGING					0x0040
#define FULLY_CHARGED				0x0020
#define FULLY_DISCHARGED			0x0010

/*BMC data offset define*/
#define BMC_PN_OFF					129
#define BMC_SN_OFF					199
#define BMC_MAN_OFF					274
#define BMC_FWV_OFF					283
#define BMC_STA_OFF					168
#define BMC_TMP_OFF					93
#define BMC_VOL_OFF					98
#define BMC_CUR_OFF					103
#define BMC_REM_OFF					128
#define BMC_FUL_OFF					138
#define BMC_RUN_OFF					143

#define BBU_DATA_LEN					59
#define BBU_VERSION					"v0.1"


/*struct definiation*/
typedef struct Command_s
{
  char commandString[32];
  char commandOpcode;
  char commandDetail[64];
}Command_t;

/*enum definiation*/
enum bbu_i2c_command_type
{
	I2C_BBU_INIT = 1,
	I2C_BBU_DUMP_EEPROM,
	I2C_BBU_DUMP_FUELGAUGE,
	I2C_BBU_SET,
	I2C_BBU_GET
};

/*function definiation*/
int cls_bbu_send_info_to_bmc_command(int i2c_num_I801,u8* bmc_data);
int cls_bbu_send_i2c_get_command(int i2c_num_I801,char i2c_dev,char reg);
int cls_bbu_send_i2c_get_string_command(int i2c_num_I801,char i2c_dev,char reg);
int cls_bbu_output_orgnize(u8 reg,int index, char* output,char* out,u8 fun_tag);
int cls_bbu_driver_check(void);
int cls_bbu_insert_check(int i2c_num_I801);
int cls_bbu_test_check_stillarm(int i2c_num_I801,char i2c_dev);
int cls_bbu_get_data_from_file(char *fname, void *inputBuffer);
int cls_bbu_get_file_length(const char *filename);
int cls_bbu_convert_string_to_lower(char *buffer);
void cls_bbu_print_usage(void);
int cls_bbu_get_info_from_bmc_command(u8 *data);
u32 cls_bbu_hextoint(const char *hex);
int asc_to_int(int i);
void cls_bbu_send_vpdinfo_to_bmc_data_assemble(u8 *input,u8* bmc_data);
int cls_bbu_send_vpdinfo_to_bmc_data_command(int i2c_num_I801,u8* bmc_data);
int cls_get_i2c_num_I801();



#endif

