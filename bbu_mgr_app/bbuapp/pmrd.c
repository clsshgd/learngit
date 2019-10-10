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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PMWR_MAP_LENGTH 4096

int main(int argc, char **argv)
{
	int f;
	void *addr;
	unsigned int base,off,val;

	if(argc < 3) {
		printf("Please input correct arguments:base offset! \n");
		exit(1);
	}

	if(sscanf(argv[1], "%x", &base) < 1) {
		fprintf(stderr, "%s is not a hex number!\n", argv[1]);
		exit(1);
	}

	if(sscanf(argv[2], "%x", &off) < 1) {
		fprintf(stderr, "%s is not a hex number!\n", argv[2]);
		exit(1);
	}

	if(-1 == (f = open("/dev/mem", O_RDONLY))) {
		fprintf(stderr, "/dev/mem open failed:%s\n", strerror(errno));
		exit(1);
	}

	//addr = (void *)base;
	addr =  mmap(0, PMWR_MAP_LENGTH, PROT_READ, MAP_SHARED, f, base);

	if(addr == MAP_FAILED) {
		fprintf(stderr, "mmap %x failed:%s!\n", base, strerror(errno));
		goto pmwr_exit;
	}
	else {
		fprintf(stderr, "mmap %x to %x\n success\n", base, (unsigned int)addr);
	}
		
	val = *(unsigned int *)(addr + off);

	if(*(unsigned int*)(addr + off) != val) {
		fprintf(stderr, "%x != %x\n", *(unsigned int*)(addr+off), val); 
	}
	else {
		fprintf(stderr, "%x == %x\n", *(unsigned int*)(addr + off), val);
	}

	munmap(addr, PMWR_MAP_LENGTH);

pmwr_exit:

	close(f);

	return 0;
}
