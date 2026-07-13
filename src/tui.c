#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "tui.h"
#include <string.h>

//window manager
typedef struct{//now visible to the user, hopefully no problems
	WindowObject windows[MAX_NUMBER_OF_WINDOWS];
	TUIWindow activeWindow;
	uint8_t howManyWindows;
}windowManager;
static windowManager wManager;

static struct termios originaltty; //get the original terminal settings
				   //to restore them at the end
//Cell is defined in tui.h, make a front and back buffer
static Cell* frontBuffer;
static Cell* backBuffer;

static int* presentationArray;
static uint32_t presentationArraySize;

//has info like screen height and width
static TUIInstance* thistui;

TUIInstance* initTUI(){ //clear the screen, set terminal mode to non-canonical
	//[2J clears screen [H cursor home [?25l hide cursor
	printf("\x1b[2J\x1bH\x1b[?25l"); //clear the screen
	//flush the print buffer to stdout
	fflush(stdout);

	//saves the current terminal settings to a static variable and does some stuff
	struct termios tsettings;
	tcgetattr(STDIN_FILENO, &originaltty);			//save current
	tsettings = originaltty;				//terminal settings
	tsettings.c_lflag &= ~(ICANON | ECHO);			//disable echo and
	tcsetattr(STDIN_FILENO, TCSANOW, &tsettings);  		//canonical mode
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);		//sets STDIN to 
								//nonblocking mode
	//gets the window size with an ioctl call
	struct winsize w;
	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w)==-1){
		perror("ioctl");
	};
	
	//set TUIInstance data
	TUIInstance* tuiinstance = malloc(sizeof(TUIInstance));
	if(!tuiinstance){
		perror("malloc");
		exit(1);
	}
	tuiinstance->screenWidth = w.ws_col;
	tuiinstance->screenHeight = w.ws_row;

	//create presentation array
	presentationArray = malloc(tuiinstance->screenWidth*tuiinstance->screenHeight*sizeof(int));
	if(!presentationArray){
		perror("malloc");
		exit(1);
	}

	//create our front and back buffers
	frontBuffer = malloc(tuiinstance->screenWidth*tuiinstance->screenHeight*sizeof(Cell));
	backBuffer = malloc(tuiinstance->screenWidth*tuiinstance->screenHeight*sizeof(Cell));
	if(!frontBuffer || !backBuffer){
		perror("malloc");
		exit(1);
	}
	//populate our buffers with spaces
	for(int row = 0; row < tuiinstance->screenHeight; row++){
		for(int column = 0; column < tuiinstance->screenWidth; column++){
			frontBuffer[row*tuiinstance->screenWidth+column].ch = ' ';	
			frontBuffer[row*tuiinstance->screenWidth+column].fg = fg_WHITE;
			frontBuffer[row*tuiinstance->screenWidth+column].bg = bg_BLACK;
			
			backBuffer[row*tuiinstance->screenWidth+column].ch = ' ';
			backBuffer[row*tuiinstance->screenWidth+column].fg = fg_WHITE;
			backBuffer[row*tuiinstance->screenWidth+column].bg = bg_BLACK;
		}
	}
	//initialize wManager
	wManager.howManyWindows = 0;

	//sets TUIInstance static variable
	thistui = tuiinstance;
	
	return tuiinstance;
}

//this function is entirely depriciated but im attached to it emotionally so I won't delete it
Cell* createFrameBuffer(TUIInstance* tuiinstance){ //create a static frame buffer that is rows rows and columns columns
						   //so it wont rly support scrolling, sorry!
						   //returns the pointer to the first cell array
	Cell* framebuffer = malloc(tuiinstance->screenWidth*tuiinstance->screenHeight*sizeof(Cell));
	return framebuffer;

}


void closeTUI(){ //please call this at the end of your program twin
	//resets terminal settings to canonical and echo
	tcsetattr(STDIN_FILENO, TCSANOW, &originaltty);
	//reenables STDIN blocking
	fcntl(STDIN_FILENO, F_SETFL, ~O_NONBLOCK);
	//resets colors to default, clears screen, homes cursor, shows cursor
	printf("\x1b[0;39;49m\x1b[2J\x1b[H\x1b[?25h");
	//we still probably (definitely) have a memory leak somewhere
	free(frontBuffer);
	free(backBuffer);
	free(thistui);
	for(int i = 0; i < wManager.howManyWindows; i++){
		free(wManager.windows[i].windowBuffer);
	}
	fflush(stdout);
	return;
}



void drawFrontBuffer(){
	for(int row = 0; row < thistui->screenHeight; row++){
		for(int column = 0; column < thistui->screenWidth; column++){
			printf("\x1b[%d;%dH\x1b[0;%d;%dm%c",
					row,
					column,
					frontBuffer[row*thistui->screenWidth+column].fg,
					frontBuffer[row*thistui->screenWidth+column].bg,
					frontBuffer[row*thistui->screenWidth+column].ch);

		}
	}
	fflush(stdout);
}

void flashFrontBuffer(){
	memcpy(frontBuffer, backBuffer, thistui->screenHeight*thistui->screenWidth*sizeof(Cell));
	return;
}

void eraseBackBuffer(){
	for(int row = 0; row < thistui->screenHeight; row++){
		for(int column = 0; column < thistui->screenWidth; column++){
			backBuffer[row*thistui->screenWidth+column].ch = ' ';
			backBuffer[row*thistui->screenWidth+column].fg = fg_WHITE;
			backBuffer[row*thistui->screenWidth+column].bg = bg_BLACK;
		}
	}
}



wSettings getWindowSettings(TUIWindow window){
	wSettings settings;
	settings.x = wManager.windows[window].x;
	settings.y = wManager.windows[window].y;
	settings.width = wManager.windows[window].width;
	settings.height = wManager.windows[window].height;
	return settings;
}


void drawWindowBuffer(TUIWindow window){
	for(int row = 0; row < thistui->screenHeight; row++){
		for(int column = 0; column < thistui->screenWidth; column++){
			printf("\x1b[%d;%dH\x1b[0;%d;%dm%c",
					row,
					column,
					wManager.windows[window].windowBuffer[row*thistui->screenWidth+column].fg,
					wManager.windows[window].windowBuffer[row*thistui->screenWidth+column].bg,
					wManager.windows[window].windowBuffer[row*thistui->screenWidth+column].ch);

		}
	}
	fflush(stdout);
}

void present(){
	//store the indices of the cells that differ between the back and front buffers
	uint32_t index = 0;
	for(int i = 0; i < thistui->screenHeight*thistui->screenWidth; i++){
		presentationArray[i] = 0;
		if(backBuffer[i].ch != frontBuffer[i].ch || backBuffer[i].fg != frontBuffer[i].fg || backBuffer[i].bg != frontBuffer[i].bg){
			presentationArray[index] = i;
			index++;
		}
	}
	presentationArraySize = index;
}


void drawFrontBufferOptimized(){
	for(uint32_t i = 0; i < presentationArraySize; i++){
		uint32_t index = presentationArray[i];
		printf("\x1b[%d;%dH\x1b[0;%d;%dm%c",
				index/thistui->screenWidth,
				index%thistui->screenWidth,
				frontBuffer[index].fg,
				frontBuffer[index].bg,
				frontBuffer[index].ch);
	}
	fflush(stdout);
}

TUIWindow createWindow(uint16_t x, uint16_t y, uint8_t width, uint8_t height){
	wManager.howManyWindows++;
	wManager.windows[wManager.howManyWindows-1].id=wManager.howManyWindows-1;
	wManager.windows[wManager.howManyWindows-1].x=x;
	wManager.windows[wManager.howManyWindows-1].y=y;
	wManager.windows[wManager.howManyWindows-1].width=width;
	wManager.windows[wManager.howManyWindows-1].height=height;
	//each window has its own frame buffer the size of the entire screen, on a 48x170 screen this is about 73kb
	wManager.windows[wManager.howManyWindows-1].windowBuffer = malloc(thistui->screenWidth*thistui->screenHeight*sizeof(Cell));
	for(int row = 0; row < thistui->screenHeight; row++){
		for(int column = 0; column < thistui->screenWidth; column++){
			wManager.windows[wManager.howManyWindows-1].windowBuffer[row*thistui->screenWidth+column].ch = ' ';	
			wManager.windows[wManager.howManyWindows-1].windowBuffer[row*thistui->screenWidth+column].fg = fg_WHITE;	
			wManager.windows[wManager.howManyWindows-1].windowBuffer[row*thistui->screenWidth+column].bg = bg_BLACK;	
		}
	}
	return wManager.howManyWindows-1;
}

//draw a window to its own windowBuffer, then flash the windowBuffer to backBuffer
//initially uses local coordinates, then global
void drawWindow(TUIWindow window){
	uint8_t localHeight, localWidth;
	localHeight = wManager.windows[window].height;
	localWidth = wManager.windows[window].width;
	//corners
	//top corners
	wManager.windows[window].windowBuffer[0].ch = '#';
	wManager.windows[window].windowBuffer[0+localWidth-1].ch = '#';
	//bottom corners
	wManager.windows[window].windowBuffer[(localHeight-1)*(localWidth)].ch = '#';
	wManager.windows[window].windowBuffer[(localHeight-1)*(localWidth) + localWidth-1].ch = '#';
	//recolor the corners if active
	if(wManager.activeWindow == window){
		wManager.windows[window].windowBuffer[0].fg = fg_BRIGHTWHITE;
		wManager.windows[window].windowBuffer[0+localWidth-1].fg = fg_BRIGHTWHITE;
		//bottom corners
		wManager.windows[window].windowBuffer[(localHeight-1)*localWidth].fg = fg_BRIGHTWHITE;
		wManager.windows[window].windowBuffer[(localHeight-1)*localWidth + localWidth-1].fg = fg_BRIGHTWHITE;
	}else{
		wManager.windows[window].windowBuffer[0].fg = fg_WHITE;
		wManager.windows[window].windowBuffer[0+localWidth-1].fg = fg_WHITE;
		//bottom corners
		wManager.windows[window].windowBuffer[(localHeight-1)*localWidth].fg = fg_WHITE;
		wManager.windows[window].windowBuffer[(localHeight-1)*localWidth + localWidth-1].fg = fg_WHITE;
		
	}

	//sides
	//left and right
	//if active window then color
	if(wManager.activeWindow == window){
		for(uint8_t i = 1; i < localHeight-1; i++){
			wManager.windows[window].windowBuffer[i*localWidth].ch = '|';
			wManager.windows[window].windowBuffer[i*localWidth].fg = fg_BRIGHTWHITE;
			wManager.windows[window].windowBuffer[i*localWidth + localWidth-1].ch = '|';
			wManager.windows[window].windowBuffer[i*localWidth + localWidth-1].fg = fg_BRIGHTWHITE;
		}
		//top and bottom
		for(uint8_t i = 1; i < localWidth-1; i++){
			wManager.windows[window].windowBuffer[i].ch = '-';	
			wManager.windows[window].windowBuffer[i].fg = fg_BRIGHTWHITE;	
			wManager.windows[window].windowBuffer[i+(localHeight-1)*(localWidth)].ch = '-';	
			wManager.windows[window].windowBuffer[i+(localHeight-1)*(localWidth)].fg = fg_BRIGHTWHITE;	
		}
	}else{
		for(uint8_t i = 1; i < localHeight-1; i++){
			wManager.windows[window].windowBuffer[i*localWidth].ch = '|';
			wManager.windows[window].windowBuffer[i*localWidth].fg = fg_WHITE;
			wManager.windows[window].windowBuffer[i*localWidth + localWidth-1].ch = '|';
			wManager.windows[window].windowBuffer[i*localWidth + localWidth-1].fg = fg_WHITE;
		}
		//top and bottom
		for(uint8_t i = 1; i < localWidth-1; i++){
			wManager.windows[window].windowBuffer[i].ch = '-';	
			wManager.windows[window].windowBuffer[i].fg = fg_WHITE;	
			wManager.windows[window].windowBuffer[i+(localHeight-1)*(localWidth)].ch = '-';	
			wManager.windows[window].windowBuffer[i+(localHeight-1)*(localWidth)].fg = fg_WHITE;	
		}
	}	

	flashWindowToBackBuffer(window);
	return;
}

void resizeWindow(TUIWindow window, uint16_t x, uint16_t y, uint8_t width, uint8_t height){
	wManager.windows[window].x = x;
	wManager.windows[window].y = y;
	wManager.windows[window].width = width;
	wManager.windows[window].height = height;
	return;
}

TUIWindow getActiveWindow(){
	return wManager.activeWindow;	
	
}

void flashWindowToBackBuffer(TUIWindow window){
	uint8_t windowHeight, windowWidth, windowX, windowY;
	uint16_t  screenWidth;
	windowHeight = wManager.windows[window].height;
	windowWidth = wManager.windows[window].width;
	windowX = wManager.windows[window].x;
	windowY = wManager.windows[window].y;
	screenWidth = thistui-> screenWidth;

	for(uint8_t y = 0; y < windowHeight; y++){
		for(uint8_t x = 0; x < windowWidth; x++){
			backBuffer[(y+windowY)*screenWidth+x+windowX].ch = wManager.windows[window].windowBuffer[y*(windowWidth)+x].ch;
			backBuffer[(y+windowY)*screenWidth+x+windowX].fg = wManager.windows[window].windowBuffer[y*(windowWidth)+x].fg;
			backBuffer[(y+windowY)*screenWidth+x+windowX].bg = wManager.windows[window].windowBuffer[y*(windowWidth)+x].bg;
		}
	}
	return;
}

//maximum string size of 256 characters
void wprintf(TUIWindow window, char* string, uint8_t x, uint8_t y){
	char charBuffer[256];
	snprintf(charBuffer, sizeof(charBuffer), string);
	
	uint8_t localWidth = wManager.windows[window].width;

	for(int i = 0; i < strlen(string); i++){
		wManager.windows[window].windowBuffer[y*localWidth+x+i].ch = charBuffer[i];
	}
	return;
}


void changeActiveWindow(TUIWindow window){
	wManager.activeWindow = window;

	return;	
}
