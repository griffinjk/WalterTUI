//api for both making windows and for the window manager itself

#include "wwm-window.h"
#include "wwm-wm.h"
#include <stdint.h>

typedef struct{
	uints id;
	uints x;
	uints y;
	uints height;
	uints width;
	Cells* cells;
	char directory;

}WWMWindow;

//defining some of our system calls
print(char* string){
	uintl size;

}


MMWindow *create_window(uints height, uints width){
	WWMWindow window;
	window.x = 0;
	window.y = 0;
	window.height = height;
	window.width = width;
	
	
}


