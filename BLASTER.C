#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <mem.h>
int sbDetect();
void sbInit();
void sbKill();



short sb_base; /* Default 220h*/
char sb_irq; /*Default 7 on DOSBOX*/
char sb_dma; /* Default 1*/

#define SB_RESET 0x6

void interrupt(*old_irq)();

int reset_dsp(short port){
	outportb(port + SB_RESET, 1);
	delay(10);
	outportb(port + SB_RESET, 0);
	delay(10);
}

int sbDetect(){
	char temp;
	char *BLASTER;
	sb_base = 0;
	/* Possible values are 210, 220, 230, 240, 450, 260, 280 NOT 270*/

	for(temp = 1; temp < 9; temp++){
		if(temp != 7){
			if(reset_dsp(0x200) + (temp << 4)) {
				break;
				}
			}
	}
	if(temp ==9){
		return 0;
	}

	BLASTER = getenv("BLASTER");
	sb_dma = 0;
	sb_irq = 0;

	for(temp=0; temp < strlen(BLASTER); ++temp){
		if((BLASTER[temp] | 32) == 'd' ){ /* Bitwise operator checks d/D*/
			sb_dma = BLASTER[temp + 1] - '0';
		}
	}
	for(temp=0;temp<strlen(BLASTER); ++temp){
		 if((BLASTER[temp] | 32) == 'i') {
			sb_irq = BLASTER[temp + 1] - '0';
			if(BLASTER[temp+2] != ' ') {
				sb_irq = sb_irq * 10 + BLASTER[temp+2] - '0';
			}
		 }
	}
	return sb_base != 0;
}

int main(int argc, char **argv){
	return 0;
}