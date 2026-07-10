#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int createWindow(char* name);
int destroyWindow(char* name);

int createWindow(char* name){
	//if the TUIwindows directory doesnt exist yet, create it
	struct stat st = {0};
	if(stat("/tmp/TUIwindows", &st) == -1){
		mkdir("/tmp/TUIwindows", 0755);
	}
	
	//use the name passed to the function to name the window directory
	//is this a terrible terrible way to do it?
	//idk probably, that's what you get when a retard writes code?
	char windowDir[32];
	snprintf(windowDir, sizeof(windowDir), "/tmp/TUIwindows/%s", name);

	//create the window directory
	if(stat(windowDir, &st) == -1){
		mkdir(windowDir, 0755);
	}

	//populate the window directory with files
	char cellsFileDir[48], dataFileDir[48], titleFileDir[48];
	
	sprintf(cellsFileDir, "%s/Cells", windowDir);
	sprintf(dataFileDir, "%s/Data", windowDir);
	sprintf(titleFileDir, "%s/Title", windowDir);

	FILE *cellsFilePtr, *dataFilePtr, *titleFilePtr;

	cellsFilePtr = fopen(cellsFileDir, "w");
	dataFilePtr = fopen(dataFileDir, "w");
	titleFilePtr = fopen(titleFileDir, "w");

	fclose(cellsFilePtr);
	fclose(dataFilePtr);
	fclose(titleFilePtr);
	return 0;
}

int main(){
	createWindow("window2");
}
