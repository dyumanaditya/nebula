#include <iostream>
#include <dirent.h>
#include <signal.h>
#include <ncurses.h>
#include "fileManager.h"


int main()
{	
	FileManager f("/home/dyuman");
	f.mainEventLoop();
	return 0;
}