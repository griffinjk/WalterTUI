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
//Cell is defined in tui.h, make a front and back buffer
static Cell* frontBuffer;
static Cell* backBuffer;

//has info like screen height and width
static TUIInstance* thistui;

//internal window manager
typedef struct{//hidden to the user, internal variable
	uint8_t activeWindow;
	uint8_t howManyWindows;
}windowManager;
static windowManager wManager;


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
