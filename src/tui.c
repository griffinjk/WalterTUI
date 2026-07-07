#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "tui.h"
#include <string.h>

static struct termios originaltty; //get the original terminal settings
				   //to restore them at the end

static Cell* frontBuffer;
static Cell* backBuffer;

static TUIInstance* thistui;

typedef struct{//hidden to the user, internal variable
	uint8_t activeWindow;
	uint8_t howManyWindows;
}windowManager;
static windowManager wManager;


TUIInstance* initTUI(){ //clear the screen, set terminal mode to non-canonical
	printf("\x1b[2J\x1bH\x1b[?25l"); //clear the screen
	fflush(stdout);
	struct termios tsettings;
	tcgetattr(STDIN_FILENO, &originaltty);			//save current
	tsettings = originaltty;
					     			//terminal settings
	tsettings.c_lflag &= ~(ICANON | ECHO);			//disable echo and
	tcsetattr(STDIN_FILENO, TCSANOW, &tsettings);  		//canonical mode
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);		//sets STDIN to 
								//nonblocking mode
	struct winsize w;
	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w)==-1){
		perror("ioctl");
	};
	
	TUIInstance* tuiinstance = malloc(sizeof(TUIInstance));
	tuiinstance->screenWidth = w.ws_col;
	tuiinstance->screenHeight = w.ws_row;
	
	//create our front and back buffers
	frontBuffer = malloc(tuiinstance->screenWidth*tuiinstance->screenHeight*sizeof(Cell));
	backBuffer = malloc(tuiinstance->screenWidth*tuiinstance->screenHeight*sizeof(Cell));
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
	wManager.howManyWindows = 0;


	thistui = tuiinstance;
	return tuiinstance;
}

Cell* createFrameBuffer(TUIInstance* tuiinstance){ //create a static frame buffer that is rows rows and columns columns
						   //so it wont rly support scrolling, sorry!
						   //returns the pointer to the first cell array
	Cell* framebuffer = malloc(tuiinstance->screenWidth*tuiinstance->screenHeight*sizeof(Cell));
	return framebuffer;

}

void closeTUI(){
	tcsetattr(STDIN_FILENO, TCSANOW, &originaltty);
	fcntl(STDIN_FILENO, F_SETFL, ~O_NONBLOCK);
	printf("\x1b[0;39;49m\x1b[2J\x1b[H\x1b[25h");
	free(frontBuffer);
	free(backBuffer);
	fflush(stdout);
	return;
}

TUIWindow createWindow(uint16_t x, uint16_t y, uint8_t width, uint8_t height){
	//corners
	backBuffer[y*thistui->screenWidth+x].ch = '#';
	backBuffer[y*thistui->screenWidth+x+width].ch = '#';
	backBuffer[(y+height)*thistui->screenWidth+x].ch = '#';
	backBuffer[(y+height)*thistui->screenWidth+x+width].ch = '#';
	
	//inside
	//top and bottom
	for(int i = x+1; i < width+x; i++){
		backBuffer[y*thistui->screenWidth+i].ch = '-';	
		backBuffer[(y+height)*thistui->screenWidth+i].ch = '-';
	}
	//sides
	for(int i = y+1; i < height+y; i++){
		backBuffer[i*thistui->screenWidth+x].ch = '|';	
		backBuffer[i*thistui->screenWidth+x+width].ch = '|';
	}

	wManager.howManyWindows++;
	return wManager.howManyWindows;
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
			fflush(stdout);
		}
	}
}

void flashFrontBuffer(){
	memcpy(frontBuffer, backBuffer, thistui->screenHeight*thistui->screenWidth*sizeof(Cell));
	return;
}
