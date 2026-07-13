#ifndef WALTER_WINDOW_MANAGER_WINDOW
#define WALTER_WINDOW_MANAGER_WINDOW

#include "wwm.h"

//This is the header/api that windows using the Walter Window Manager
//will use

//create a wwm window, use sprocket() systemcall to tell the Walter
//Window Manager. create a directory in /tmp/WWMWindows and make some
//memory mappings to map files to regions in memeory
WWMWindow *create_window();

//manually send a packet to the window manager
int send_packet_to_wm(); //still need to fill in parameters

//print a string to the Cells framebuffer of the window
int wprint(uints x, units y, char* string, int size);


#endif
