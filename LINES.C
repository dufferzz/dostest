/* Released under GPL v2 */
/* Created by Sam Duff 2020 */

/* Application intent is to draw random lines on screen  */
/* Prior to initialization of graphics & drawing, there is a text based */
/* menu screen to select options  */

/* Lines are random colors, lengths & rotation, within screen boundarys */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <alloc.h>
#include <stdlib.h>
#include <dos.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50

#define SCREEN_256_WIDTH 320
#define SCREEN_256_HEIGHT 200

#define SET_MODE 0x00
#define VGA_256_COLOR 0x13
#define SET_CURSOR 0x02
#define TEXT_MODE 0x03
#define VIDEO_INT 0x10
#define VRETRACE_BIT 0x08
#define INPUT_STATUS 0x3DA

#define NUM_COLORS 256

#define PALETTE_READ 0x3C7
#define PALETTE_WRITE 0x3C8
#define PALETTE_DATA 0x3C9


typedef unsigned char byte;

byte far *VGA=(byte far *)0xA0000000L;

#define SETPIX(x, y, c) *(VGA + (x) + (y) * SCREEN_WIDTH)=c

#define GETPIX(x, y) *(VGA + (x) + (y) * SCREEN_WIDTH)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

int randomNumber(int max){
	int num, c;
	randomize();
	num = random(max);
	return num;
}

void setMode(byte mode){

	union REGS regs;
	regs.h.ah = SET_MODE;
	regs.h.al = mode;
	int86(VIDEO_INT, &regs, &regs);
}

int waitForVRetrace(){
	while(inp(INPUT_STATUS) & VRETRACE_BIT);
	while(!(inp(INPUT_STATUS) & VRETRACE_BIT));
}

void drawBackground(){
	int x, y;
	for(y = 0; y < SCREEN_256_HEIGHT; ++y){
		for(x=0; x < SCREEN_256_WIDTH; ++x){
			SETPIX(x, y, y + 16);
		}
	}
}

int playSound(int x){
	int i, j, k, l;

	for(i = 0; i < x; i++){
		sound(i);
		delay(i);
		nosound();
	}

	for(j = 200;j>100; j--){
		sound(j);
		delay(7);
		nosound();
		}
}

int main(){
	byte ch;
	int selected;
	selected = 0;


	clrscr();
	textcolor(WHITE);
	drawBox(1,"Dealer Simulator 2020");
	drawBox(20, "Please select an option");
	drawMenu();
	drawLogo(5,10);
	drawLogo(59,10);
	while(1){
		ch = getch();
		if(ch == 0x1b){  /* Break on ESC */
			printf("Escaping..");
			return 0;
		} else {
			switch(ch){ /* Do action for whatever key press */

				/*Down Arrow*/
				case 0x50:
					selected++;
					/* if(selected > 5){selected=5;} */
					gotoxy(2,8);
					cprintf("Selected: %d", selected);
					break;

				/* Up Arrow */
				case 0x48:
					selected--;
					if(selected == -1){selected = 0;}
					gotoxy(2,8);
					cprintf("Selected: %d", selected);
					break;

				/* Enter Key */
				case 0x0d:
					switch(selected){
						case 0:
							printf("starting lines..");
							setMode(VGA_256_COLOR);
							drawBackground();
							drawLines();
							break;
						case 1:
							printf("Opening Options..");
							break;
						case 2:
							break;
						case 3:
							break;
						case 4:
							break;
						default:
							playSound(selected);
					}
			}
		}
	}
	return 0;
}

int drawLines(){
	while(!kbhit()){
		printf("I am awesome\n");
		delay(250);
	}
}

int drawBox(int y1, char text[]){
	int i;
	/* Draw our first line */
	gotoxy(1,y1);
	cprintf("+");
	for(i=0;i < SCREEN_WIDTH - 2;i++){  /* FOR screen cols - 2 cols */
		cprintf("-");               	/* Print "-" repeatedly */
	}
	gotoxy(SCREEN_WIDTH,y1); 			/* Go to end column of row */
	cprintf("+");

	/* Draw Sidebars */
	gotoxy(1,y1+1); 						/* Go to second row*/
	cprintf("|");					       /* Draw border */
	gotoxy(SCREEN_WIDTH, y1+1);
	cprintf("|");
	gotoxy(1,y1+2);
	cprintf("|");
	gotoxy(SCREEN_WIDTH, y1+2);
	cprintf("|");
	gotoxy(1,y1+3);
	cprintf("|");
	gotoxy(SCREEN_WIDTH, y1+3);
	cprintf("|");
	gotoxy(1,y1+4);
	cprintf("+");
	i = 0;
	for(i=0;i < SCREEN_WIDTH - 2; i++){
		cprintf("-");
	}
	cprintf("+");


	/* Add text */
	i=0;
	gotoxy(SCREEN_WIDTH / 2 - (strlen(text) / 2 - 1), y1+2);
	for(i = 0;i <= strlen(text); ++i){
		cprintf("%c", text[i]);
	}
}

int drawMenu(){
   printMenuItem("1. Start Game", 1, 7);
   printMenuItem("2. Options", 0, 10);
   printMenuItem("3. Credits", 0, 13);
   printMenuItem("4. Quit", 0, 16);
}



int drawLogo(int x, int y){
gotoxy(x,y);
cprintf("     _____");
gotoxy(x,y+1);
cprintf("   .'     `.");
gotoxy(x,y+2);
cprintf("  /  .-=-.  \\   \\ __");
gotoxy(x,y+3);
cprintf("  | (  C\\ \\  \\_.'')");
gotoxy(x,y+4);
cprintf(" _\\  `--' |,'   _/");
gotoxy(x,y+5);
cprintf("/__`.____.'__.-'");
}


int printMenuItem(char text[], int sel, int y ){
	int posX = SCREEN_WIDTH / 2 - (strlen(text) /2 - 1);
   if(sel == 0){
	 int i;
	 textcolor(WHITE);
	 textbackground(BLACK);
	 gotoxy(posX,y);
	 for(i = 0; i <= strlen(text);i++){
	   cprintf("%c", text[i]);
	 }
   } else {
	 int i;
	 textcolor(BLACK);
	 textbackground(WHITE);
	 gotoxy(posX,y);
	 for(i = 0; i < strlen(text); i++){
	   cprintf("%c", text[i]);
	 }

   }


}
