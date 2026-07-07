#include "tui.h"
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
	TUIInstance* instance = initTUI(); 

	struct timeval tv;
	fd_set fds;

	createWindow(15, 20, 100, 10);

	flashFrontBuffer();
	while(1){
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		FD_SET(STDOUT_FILENO, &fds);
		int ready = select(STDOUT_FILENO+1, &fds, NULL, NULL, &tv);
		if(ready > 0){
			char c = getchar();
			if (c == 'q'){
				break;
			}
		}
		drawFrontBuffer();	
	}
	free(instance);
	closeTUI();
	return 0;
}
