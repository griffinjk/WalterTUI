#include "tui.h"
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>


int main(){
	initTUI(); 

	struct timeval tv;
	fd_set fds;


	TUIWindow window1 = createWindow(1, 1, 100, 3);
	TUIWindow window2 = createWindow(100, 20, 30, 20);
	changeActiveWindow(window2);
	drawWindow(window1);
	drawWindow(window2);
	wprintf(window2, "This is the second window!", 1, 1);
	wprintf(window1, "This is the first window!", 1, 1);
	flashFrontBuffer();
	while(1){
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		FD_SET(STDOUT_FILENO, &fds);
		int ready = select(STDOUT_FILENO+1, &fds, NULL, NULL, &tv);
		if(ready > 0){
			char c = getchar();
			if (c == 'q'){
				break;
			}
			if (c == ']' || c == '['){
				if(getActiveWindow() == window2){
					changeActiveWindow(window1);
				}else{
					changeActiveWindow(window2);
				}
			}
			if(c == 'd'){
				resizeWindow(getActiveWindow(),
						getWindowSettings(getActiveWindow()).x+1,
						getWindowSettings(getActiveWindow()).y,
						getWindowSettings(getActiveWindow()).width,
						getWindowSettings(getActiveWindow()).height);
			}
			if(c == 'a'){
				resizeWindow(getActiveWindow(),
						getWindowSettings(getActiveWindow()).x-1,
						getWindowSettings(getActiveWindow()).y,
						getWindowSettings(getActiveWindow()).width,
						getWindowSettings(getActiveWindow()).height);
			}
			if(c == 's'){
				resizeWindow(getActiveWindow(),
						getWindowSettings(getActiveWindow()).x,
						getWindowSettings(getActiveWindow()).y+1,
						getWindowSettings(getActiveWindow()).width,
						getWindowSettings(getActiveWindow()).height);
			}
			if(c == 'w'){
				resizeWindow(getActiveWindow(),
						getWindowSettings(getActiveWindow()).x,
						getWindowSettings(getActiveWindow()).y-1,
						getWindowSettings(getActiveWindow()).width,
						getWindowSettings(getActiveWindow()).height);
			}
		}
		eraseBackBuffer();
		drawWindow(window1);
		drawWindow(window2);
		flashFrontBuffer();

		drawFrontBuffer();	
	}
	closeTUI();
	return 0;
}
