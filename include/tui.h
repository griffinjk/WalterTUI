#ifndef TUI_H
#define TUI_H

#include <stdint.h>


#define VLINE "│" 	//u2502
#define HLINE "─"	//u2500
#define TL "┌"		//u250c
#define	TR "┐"		//u2510
#define BL "└"		//u2514
#define BR "┘"		//u2518
#define CROSS "┼"	//u253c
#define T "┬"		//u252c
#define UT "┴"		//u2534
#define RT "├"		//u251c
#define LT "┤"		//u2524

#define BLOCK4 "█" 	//u2588
#define BLOCK3 "▓"	 //unsupported i guess, dont use BLOCK3 but it's u2593
#define BLOCK2 "▒" 	//u2592
#define BLOCK1 "░" 	//u2591
#define BLOCK0 " " 	//just a space gng

#define UP "↑"		//u2191
#define DOWN "↓"	//u2193
#define LEFT "←"	//u2190
#define RIGHT "→"	//u2192

#define fg_RED 31
#define fg_GREEN 32
#define fg_BLUE 34
#define fg_BLACK 30
#define fg_WHITE 37
#define fg_BRIGHTWHITE 97
#define bg_BLACK 40
#define bg_WHITE 47

#define MAX_NUMBER_OF_WINDOWS 5

typedef uint8_t TUIWindow;

typedef struct{
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
}wSettings;

typedef struct{
	char ch;
	uint8_t bg;
	uint8_t fg;
}Cell;

typedef struct{
	uint8_t screenWidth;
	uint8_t screenHeight;
}TUIInstance;

typedef struct{
	Cell* windowBuffer;
	uint8_t id;
	uint8_t width;
	uint8_t height;
	uint16_t x;
	uint16_t y;
}WindowObject;



void closeTUI();
TUIInstance* initTUI();
Cell* createFrameBuffer(TUIInstance* tuiinstance);
TUIWindow createWindow(uint16_t x, uint16_t y, uint8_t width, uint8_t height); //creates a window object, activates it, draws it to the back buffer
void changeActiveWindow(TUIWindow window);
void resizeWindow(TUIWindow window, uint16_t x, uint16_t y, uint8_t width, uint8_t height);
TUIWindow getActiveWindow();
wSettings getWindowSettings(TUIWindow window);
void wprintf(TUIWindow window, char* string, uint8_t x, uint8_t y);
void flashWindowToBackBuffer(TUIWindow window);
void clearWindowBuffer(TUIWindow window);
void present();
void drawFrontBufferOptimized(); //uses presentation array
void drawWindow(TUIWindow window); //draws to back buffer, doesnt rly work
void eraseBackBuffer(); //again, unoptimized, don't use this. doesnt use presentation array
void drawFrontBuffer(); //dont use this, for debug only, not optimized at all
void flashFrontBuffer(); //also dont use this, please. thanks!
void drawWindowBuffer(TUIWindow window); //FOR DEBUG ONLY
#endif
