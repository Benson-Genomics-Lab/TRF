/***************************************************************
*   dbgcon.h    :   used for console I/O on a WIN32 GUI
*                   application.
****************************************************************/


#ifndef DBGCON_H
#define DBGCON_H

#include <windows.h>
#include <conio.h>


/* variables used for debuging are defined here */
int DBG1 = 0;
int DBG2 = 0;
int DBG3 = 0;


#define print _cprintf
#define scan  _cscanf


#define CONSOLEWIDTH  80
#define CONSOLEHEIGHT 500
#define CONSOLETITLE    "Debug Window"

void OpenConsole(void)
{
    COORD coord;
    HANDLE handle;

    /* allocate a console for output */
    AllocConsole();

    /* set title, and buffer size */
    SetConsoleTitle(CONSOLETITLE);
    coord.X= CONSOLEWIDTH;
    coord.Y= CONSOLEHEIGHT;
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleScreenBufferSize(handle,coord);

    /* bebug variables are set here */
    cprintf("Enter 3 dbg values to use: ");
    cscanf("%d %d %d",&DBG1,&DBG2,&DBG3);
    cprintf("\nValues entered: %d %d %d",DBG1,DBG2,DBG3);
    cprintf("\n");

    return;
}


void CloseConsole(void)
{
    HANDLE handle;

    /* free the console */
    handle = GetStdHandle(STD_INPUT_HANDLE);

    FreeConsole();

    return;
}




#endif
