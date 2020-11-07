/* System includes */
#include <alloc.h>
#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SET_MODE 0x00
#define SET_CURSOR 0x02
#define PRINT_CHAR 0x09
#define PRINT_STRING 0x13
#define NUM_COLORS 256
#define VIDEO_INT 0x10
#define TEXT_MODE 0x03
#define VGA_256_COLOR_MODE 0x13
#define VRETRACE_BIT 0x08

/* Height / Width variables */

#define SCREEN_HEIGHT 200
#define SCREEN_WIDTH 320
#define BOTTOM_BANNER 20

#define PALETTE_READ 0x3C7
#define PALETTE_WRITE 0x3C8
#define PALETTE_DATA 0x3C9


#define INPUT_STATUS 0x3DA

/* Game Definitions */
#define MAX_SCORE 5
#define PADDLE_SPEED 8



typedef unsigned char byte;
typedef struct {
  byte color;
  int x, y;
  int dx, dy;
  int width;
  int height;
  int score;
  byte *backup;
} player;

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

void randomEncounter(){
  int e;
  e = randomNumber(50);
  printf("You rolled: %d\n", e);
  switch(e){
    case 1:
    printf("Bumped into a dealer!");

  }
}

void waitForRetrace()
{
  while(inp(INPUT_STATUS)  & VRETRACE_BIT);
  while(!(inp(INPUT_STATUS) & VRETRACE_BIT));
}

void set_mode(byte mode)
{
  union REGS regs;
  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86( VIDEO_INT, &regs, &regs );
}

void print( int x, int y, byte color, const char far *s )
{
  struct REGPACK regs;

#if 1
  int i;
  for( i = 0; i < strlen(s); ++i ) {
    regs.r_ax = SET_CURSOR << 8 | 0x0;
    regs.r_bx = 0x0;
    regs.r_dx = y << 8 | x + i;
    intr( VIDEO_INT, &regs);
    regs.r_ax = PRINT_CHAR << 8 | s[ i ];
    regs.r_bx = 0x0 << 8 | color;
    regs.r_cx = 1;
    intr( VIDEO_INT, &regs);
  }
#else
  regs.r_ax = PRINT_STRING << 8 | 0x0;
  regs.r_bx = 0x0 << 8 | color;
  regs.r_cx = strlen( s ); /* "hello world\0" */
  regs.r_dx = y << 8 | x;
  regs.r_es = FP_SEG( s ); /* Gets far address segment  */
  regs.r_bp = FP_OFF( s ); /* Gets far address offset */
  intr( VIDEO_INT, &regs);
#endif
}

void drawBackground()
{
  int x, y;

  for( y = 0; y < SCREEN_HEIGHT - BOTTOM_BANNER; ++y ) {
    for( x = 0; x < SCREEN_WIDTH; ++x ) {
      SETPIX( x, y, y + 16 );
    }
  }
}

byte *getSkyPalette()
{
  byte *pal;
  int i;
  pal = malloc( NUM_COLORS * 3 ); /* assign memory segment for RGB */

  outp( PALETTE_READ, 0 );
  for( i = 0; i < 16; ++i ) {
    pal[ i*3 + 0 ] = inp( PALETTE_DATA );
    pal[ i*3 + 1 ] = inp( PALETTE_DATA );
    pal[ i*3 + 2 ] = inp( PALETTE_DATA );
  }

  for( i = 16; i < 116; ++i ) {
    pal[ i*3 + 0 ] = MIN( 63, i ); /* RED */
    pal[ i*3 + 1 ] = MIN( 63, i ); /* GREEN */
    pal[ i*3 + 2 ] = 63; /* BLUE */
  }
  for( i = 116; i < 216; ++i ) {
    pal[ i*3 + 0 ] = 5; /* RED */
    pal[ i*3 + 1 ] = (i - 100) / 2; /* GREEN */
    pal[ i*3 + 2 ] = 5; /* BLUE */
  }
  for( i = 217; i < 256; ++i ) {
    pal[ i*3 + 0 ] = 63; /* RED */
    pal[ i*3 + 1 ] = 10; /* GREEN */
    pal[ i*3 + 2 ] = 10; /* BLUE */
  }
  return pal;
}

void setBlackPalette()
{
  int i;

  outp( PALETTE_WRITE, 0 );
  for( i = 0; i < NUM_COLORS * 3; ++i ) {
    outp( PALETTE_DATA, 0 );
  }
}

void setPalette(byte *palette)
{
  int i;

  outp( PALETTE_WRITE, 0 );
  for( i = 0; i < NUM_COLORS * 3; ++i ) {
    outp( PALETTE_DATA, palette[ i ] );
  }
}

void blit2vga( byte far *s, int x, int y, int w, int h )
{
  int i;
  byte far *src = s;
  byte far *dst = VGA + x + y * SCREEN_WIDTH;
  for( i = y; i < y + h; ++i ) {
    movedata(
      FP_SEG( src ),
      FP_OFF( src ),
      FP_SEG( dst ),
      FP_OFF( dst ),
      w
    );
    src += w;
    dst += SCREEN_WIDTH;
  }

}

void blit2mem( byte far *d, int x, int y, int w, int h )
{
  int i;
  byte far *src = VGA + x + y * SCREEN_WIDTH;
  byte far *dst = d;
  for( i = y; i < y + h; ++i ) {
    movedata(
      FP_SEG( src ),
      FP_OFF( src ),
      FP_SEG( dst ),
      FP_OFF( dst ),
      w
    );
    src += SCREEN_WIDTH;
    dst += w;
  }
}

void drawRect( int x, int y, int w, int h, byte c )
{
  int i, j;
  for( j = y; j < y + h; ++j ) {
    for( i = x; i < x + w; ++i ) {
      byte far *dst = VGA + i + j * SCREEN_WIDTH;
      *dst = c;
    }
  }
}

void storePlayer( player *p )
{
  blit2mem( p->backup, p->x, p->y, p->width, p->height );
}

void restorePlayer( player *p )
{
  blit2vga( p->backup, p->x, p->y, p->width, p->height );
}

void drawPlayer( player *p )
{
  drawRect( p->x, p->y, p->width, p->height, p->color );
}


void fadeInPalette( byte *pal )
{
  int i, j;
  byte pal2[ 3 * NUM_COLORS ];

  memset( pal2, 0, 3 * NUM_COLORS );

  for( i = 0; i < 63; ++i ) {
    waitForRetrace();
    outp( PALETTE_WRITE, 0 );
    for( j = 0; j < NUM_COLORS * 3; ++j ) {
      if( pal2[ j ] < pal[ j ] ) {
	pal2[ j ]++;
      }
      outp( PALETTE_DATA, pal2[ j ] );
    }
  }
}

void fadeOutPalette( byte *pal )
{
  int i, j;
  byte pal2[ 3 * NUM_COLORS ];

  memcpy( pal2, pal, 3 * NUM_COLORS );

  for( i = 0; i < 63; ++i ) {
    waitForRetrace();
    outp( PALETTE_WRITE, 0 );
    for( j = 0; j < NUM_COLORS * 3; ++j ) {
      if( pal2[ j ] > 0 ) {
	pal2[ j ]--;
      }
      outp( PALETTE_DATA, pal2[ j ] );
    }
  }
}

int handleGame( int new_game )
{
  int kc = 0;
  /* 0: game still going
   * 1: player has won
   * 3: ESC pressed
   */
  static player p1;
  const char *s = "Player 1 %03d"; 
  const char *p = "Rolled: %d"; 

  static char far *out = NULL;
  static char far *pout = NULL;

  int update_score = 0;

  if( out == NULL ) {
    out = malloc( 256 );
  }
if( pout == NULL ) {
    pout = malloc( 256 );
  }
  if( new_game ) {
    p1.color = 255;
    p1.height = 10;
    p1.width = 10;
    p1.x = 5;
    p1.y = SCREEN_HEIGHT - BOTTOM_BANNER / 2 - p1.height / 2;
    p1.score = randomNumber(2000);
    p1.backup = malloc( p1.height * p1.width );
    memset( p1.backup, 0, p1.height * p1.width );


    sprintf( out, s, p1.score);
    print( 0, 0, 0xf, out );

    sprintf( pout, s, p1.score);
    print( 0, 0, 0xf, pout );


    storePlayer( &p1 );
  }

  /* restore whatever was beneath player sprites */
  restorePlayer( &p1 );

  if(kbhit()) {
    kc = getch();
    if( kc == (char)0 ) {
      kc = getch() << 8;
    } else if( kc == (char)0x1b ) {
      return 3;
    }

    /* special key handling */
    drawBackground();
    
    switch( kc )
    {
    case 0x4800: /* up arrow */
      p1.y -= PADDLE_SPEED;
      if( p1.y < 0 ) {
	    	p1.y = 0;
      }
      randomEncounter();
      break;
    case 0x5000: /* down arrow */
      p1.y += PADDLE_SPEED;
      if( p1.y + p1.height > SCREEN_HEIGHT - BOTTOM_BANNER ) {
    		p1.y = SCREEN_HEIGHT - p1.height;
      }
      randomEncounter();
      break;
	case 0x4d00:
		p1.x += PADDLE_SPEED;
      if( p1.x + p1.height > SCREEN_WIDTH ) {
	    	p1.x = SCREEN_WIDTH - p1.width;
      }
      randomEncounter();
	  break;
	case 0x4b00:
		p1.x -= PADDLE_SPEED;
      if( p1.x < 0 ) {
	    	p1.x = 0;
      }
      randomEncounter();
	  break;
    default: /* other special keys */
      break;
    }
  }


  if( update_score ) {
    sprintf( out, s, p1.score);
    print( 0, 0, 0xf, out );
  }


  storePlayer( &p1 );
  drawPlayer( &p1 );

    return 0;
  
}

void handleGameOver( int winner )
{
  int kc = 0;
  const char end1[] = "Congratulations  Player %d!";
  const char end2[] = "<Press SPACE to EXIT game>";
  static char far *out = NULL;
  const byte col = 11;
  byte i = 0;
  int di = 1;

  if( winner < 3 ) {
    sprintf( out, end1, winner );
    print( 7, 10, col, out );
  }
  sprintf( out, end2 );
  print( 7, 11, col, out );
  while( 1 ) {
    if( kbhit() ) {
      kc = getch();
      if( kc == ' ' ) {
	break;
      }
    }
    waitForRetrace();
    outp( PALETTE_WRITE, col );
    outp( PALETTE_DATA, i );
    outp( PALETTE_DATA, i );
    outp( PALETTE_DATA, i );
    i += di;
    if( i < 1 || i > 62 ) {
      di = -di;
    }
  }
}

int main()
{
  byte *pal;
  int winner = 0;

  set_mode( VGA_256_COLOR_MODE );
  pal = getSkyPalette();
  setBlackPalette();

  clrscr();
  drawBackground();
  fadeInPalette( pal );
  handleGame( 1 );

  while( winner == 0 ) {
    /* breaks if one player won, or ESC pressed */
    winner = handleGame( 0 );
    waitForRetrace();
  }

  handleGameOver( winner );
  fadeOutPalette( pal );

  set_mode( TEXT_MODE );
  

  return 0;
}