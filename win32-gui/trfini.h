/********************************************************
* TRFINI.h : contains initialization files for program  *
*               routines are called from trf.c          *
********************************************************/

#ifndef TRFINI_H
#define TRFINI_H

#include <windows.h>

/* menu management globals */
int nsequences;     /* number of sequences in menu */
int nparameters;    /* number of parameter sets in menu */
int ndirectories;   /* number of directories in menu */
char inipath[_MAX_PATH];   /* fully qualified path of ini file */
char helppath[_MAX_PATH]; /* fully qualified path of help file */
char outputpath[_MAX_PATH]; /*path of output directory*/


/* Reads all initialization information when program starts */
void TRFReadINI(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	FILE* pfile;
	char string_buffer[_MAX_PATH];
	char key_buffer[3]={' ',' ','\0'}; /* a two character string used for the keys*/
	HMENU hMenuMain;    /* stores a handle to the main menu */
	HMENU hMenuPopup;   /* stores a handle to a popup menu */
	int char_one='1';   /* integer value of character 1 */
	RECT wndrect; /* hold the size and position of the main window */

	/* make sure file is created if it doesn't exist */
	pfile = fopen("trf.ini","a");
	fclose(pfile);

	/* get full path of ini file and store in global inipath */
	_fullpath(inipath,"trf.ini",_MAX_PATH);

	/***********************************
	* Parameter Initialization
	***********************************/

	nparameters = 0; /* initialize global */

	/* if there are no parameters store the default */
	GetPrivateProfileString("PARAMETERS","P1","nothing",string_buffer,
							 _MAX_PATH,inipath);
	if(!strcmp("nothing",string_buffer))
	{
		WritePrivateProfileString("PARAMETERS","SEL","1",inipath);
        WritePrivateProfileString("PARAMETERS","P1","2,7,7,50,500",inipath);
	}

	/* go thru stored parameters to update menu */

	hMenuMain = GetMenu(hwnd);
	hMenuPopup= GetSubMenu(hMenuMain,1);

	/* append a separator to the menu */
	AppendMenu(hMenuPopup,MF_SEPARATOR,(UINT) 0,(LPCTSTR) NULL);

	key_buffer[0]='P';
	for(nparameters=0;nparameters<4;nparameters++)
	{

		/* synthesize key_buffer string to be P1, P2, P3, etc. */
		key_buffer[1]=(char)(nparameters+char_one); /* set to '1' when nparameters=0 */

		/* get the menu text for the corresponding key */
		GetPrivateProfileString("PARAMETERS",key_buffer,"nothing",
								string_buffer,_MAX_PATH,inipath);

		/*if nothing is found break from loop */
		if(!strcmp("nothing",string_buffer)) break;

		/* append a menu item for each key found */
		AppendMenu(hMenuPopup,MF_ENABLED|MF_STRING,IDM_P1+nparameters,
					(LPCTSTR)string_buffer);

		
	}

	/* Check Menu Item that corresponds to value of SEL */
	GetPrivateProfileString("PARAMETERS","SEL","nothing",
							string_buffer,_MAX_PATH,inipath);
	CheckMenuItem(hMenuPopup,IDM_P1+atoi(string_buffer)-1,
				  MF_BYCOMMAND|MF_CHECKED);



	/***********************************
	* Sequence Initialization
	***********************************/

	nsequences=0; /* initialize global */

	/* only execute if there are sequences stored */
	GetPrivateProfileString("SEQUENCES","S1","nothing",string_buffer,
							 _MAX_PATH,inipath);
	if(strcmp("nothing",string_buffer))
	{
	

		/* go thru stored sequences to update menu */

		hMenuMain = GetMenu(hwnd);
		hMenuPopup= GetSubMenu(hMenuMain,0);

		/* Insert a separator separator in the menu */
		InsertMenu(hMenuPopup,IDM_EXIT,MF_BYCOMMAND|MF_SEPARATOR,
					(UINT) 0,(LPCTSTR) NULL);

		key_buffer[0]='S';
		for(nsequences=0;nsequences<4;nsequences++)
		{

			/* synthesize key_buffer string to be S1, S2, S3, etc. */
			key_buffer[1]=(char)(nsequences+char_one); /* set to '1' when nsequences=0 */

			/* get the string text for the corresponding key */
			GetPrivateProfileString("SEQUENCES",key_buffer,"nothing",
								string_buffer,_MAX_PATH,inipath);

			/*if nothing is found break from loop */
			if(!strcmp("nothing",string_buffer)) break;

			/* append a menu item for each key found */
			InsertMenu(hMenuPopup,2+nsequences,MF_ENABLED|MF_BYPOSITION|MF_STRING,
						IDM_S1+nsequences,(LPCTSTR)string_buffer);

		}

	}

	/***********************************
    * Directories Initialization
	***********************************/

    ndirectories=0; /* global variable */
    outputpath[0]='\0'; /* initialize to empty string. Other parts of */
                        /* the program use this as an indication to */
                        /* use the input directory as output */

    hMenuMain = GetMenu(hwnd);
    hMenuPopup= GetSubMenu(hMenuMain,2);

    /* Only execute if there are directories stored */
    GetPrivateProfileString("DIRECTORIES","D1","nothing",string_buffer,
							 _MAX_PATH,inipath);
    if(strcmp("nothing",string_buffer))
	{
        key_buffer[0]='D';
        for(ndirectories=0;ndirectories<4;ndirectories++)
        {

            /* synthesize key_buffer string to be D1, D2, D3, etc. */
            key_buffer[1]=(char)(ndirectories+char_one); /* set to '1' when nparameters=0 */

            /* get the menu text for the corresponding key */
            GetPrivateProfileString("DIRECTORIES",key_buffer,"nothing",
								string_buffer,_MAX_PATH,inipath);

            /*if nothing is found break from loop */
            if(!strcmp("nothing",string_buffer)) break;

            /* append a menu item for each key found */
            AppendMenu(hMenuPopup,MF_ENABLED|MF_STRING,IDM_D1+ndirectories,
					(LPCTSTR)string_buffer);
        }

    }

    /* Check Menu Item that corresponds to value of SEL */
    GetPrivateProfileString("DIRECTORIES","SEL","nothing",
                             string_buffer,_MAX_PATH,inipath);
    /* If SEL is not present use SEL=0 which means output to input directory*/
    if(!strcmp("nothing", string_buffer)) strcpy(string_buffer,"0");
    CheckMenuItem(hMenuPopup,IDM_OUTTOINPUT+atoi(string_buffer),
                      MF_BYCOMMAND|MF_CHECKED);

    /* Initialize outputpath to text of selected item*/
    /* leave it as '\0' ( set above) if SEL=0 */
    if(strcmp("0", string_buffer))
    {
        GetMenuString(hMenuPopup,IDM_OUTTOINPUT+atoi(string_buffer),
                        (LPTSTR) outputpath,_MAX_PATH, MF_BYCOMMAND);
        /* copy also to content od IDC_DIR_EDIT */
        SetDlgItemText(hwnd, IDC_DIR_EDIT, outputpath);
    }

	/***********************************
    * Options Menu Initialization
	***********************************/
    GetPrivateProfileString("OPTIONS","DATA","nothing",
								string_buffer,_MAX_PATH,inipath);
    if(!strcmp("1",string_buffer))
        CheckMenuItem( hMenuMain, IDM_DATA, MF_BYCOMMAND|MF_CHECKED);

    GetPrivateProfileString("OPTIONS","MASKED","nothing",
								string_buffer,_MAX_PATH,inipath);
    if(!strcmp("1",string_buffer))
        CheckMenuItem( hMenuMain, IDM_MASKED, MF_BYCOMMAND|MF_CHECKED);

    GetPrivateProfileString("OPTIONS","FLANKING","nothing",
								string_buffer,_MAX_PATH,inipath);
    if(!strcmp("1",string_buffer))
        CheckMenuItem( hMenuMain, IDM_FLANKING, MF_BYCOMMAND|MF_CHECKED);


	/***********************************
	* Size and Position Initialization
	***********************************/

	/* make sure tha size has been stored previously */
	GetPrivateProfileString("WINDOW","LEFT","nothing",
								string_buffer,_MAX_PATH,inipath);
	if(strcmp("nothing",string_buffer))
	{

        wndrect.left = atoi(string_buffer);

        GetPrivateProfileString("WINDOW","TOP","nothing",
                                    string_buffer,_MAX_PATH,inipath);
        wndrect.top = atoi(string_buffer);

        GetPrivateProfileString("WINDOW","RIGHT","nothing",
                                    string_buffer,_MAX_PATH,inipath);
        wndrect.right = atoi(string_buffer);

        GetPrivateProfileString("WINDOW","BOTTOM","nothing",
                                    string_buffer,_MAX_PATH,inipath);
        wndrect.bottom = atoi(string_buffer);

        MoveWindow( hwnd,
                    wndrect.left,
                    wndrect.top,
                    wndrect.right - wndrect.left,
                    wndrect.bottom - wndrect.top,
                    TRUE);

        GetPrivateProfileString("WINDOW","MAXIMIZED","nothing",
                                    string_buffer,_MAX_PATH,inipath);
        if(!strcmp("1",string_buffer))
        {
            PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }
        
	}

	return;
}

void TRFWriteINI(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU   hmenu;
	char    strbuffer[_MAX_PATH];
	int     count;
	char    keybuffer[3] = {' ',' ','\0'}; /* two character string for INI section keys */
	char    charone='1';
	RECT    wndrect; /*holds information about size an pos. of main window*/

	hmenu = GetMenu(hwnd);

	/**************************************
	*       write sequences to ini file   *
	***************************************/

	keybuffer[0]='S';
	for(count=0;count<nsequences;count++)
	{
		
		GetMenuString(hmenu,IDM_S1+count,(LPTSTR) strbuffer,
					  _MAX_PATH, MF_BYCOMMAND);
		keybuffer[1]=charone+(char)count; /* set character to 1,2,3,etc. */

		WritePrivateProfileString("SEQUENCES",keybuffer,strbuffer,inipath);
	}

	/***************************************
	*       write parameters to ini file   *
	****************************************/

	/* find parameter set that is checked in menu*/
	for(count=0; count<nparameters ; count++)
	{
		/* loop until the checked parameter set is found */
		if( MF_CHECKED & GetMenuState(hmenu,IDM_P1+count,MF_BYCOMMAND))
				 break;

	}

	/* output selection to inifile */
	_itoa(count+1,strbuffer,10); /* get string for SEL key */
	WritePrivateProfileString( "PARAMETERS", "SEL", strbuffer,inipath);

	/* output all sets of parameters into ini file*/
	keybuffer[0]='P';
	for(count=0;count<nparameters;count++)
	{
		
		GetMenuString(hmenu,IDM_P1+count,(LPTSTR) strbuffer,
					  _MAX_PATH, MF_BYCOMMAND);
		keybuffer[1]=charone+(char)count; /* set character to 1,2,3,etc. */

		WritePrivateProfileString("PARAMETERS",keybuffer,strbuffer,inipath);
	}

	/*************************************
    * write Output Directories to INI file
	**************************************/

    /* find option that is checked in menu*/
    for(count=0; count<ndirectories ; count++)
	{
		/* loop until the checked parameter set is found */
        if( MF_CHECKED & GetMenuState(hmenu,IDM_OUTTOINPUT+count,MF_BYCOMMAND))
				 break;

	}

	/* output selection to inifile */
    _itoa(count,strbuffer,10); /* get string for SEL key */
    WritePrivateProfileString( "DIRECTORIES", "SEL", strbuffer,inipath);

    /* output all directories to ini file*/
    keybuffer[0]='D';
    for(count=0;count<ndirectories;count++)
	{
		
        GetMenuString(hmenu,IDM_D1+count,(LPTSTR) strbuffer,
					  _MAX_PATH, MF_BYCOMMAND);
		keybuffer[1]=charone+(char)count; /* set character to 1,2,3,etc. */

        WritePrivateProfileString("DIRECTORIES",keybuffer,strbuffer,inipath);
	}

	/***************************************
    * write options menu state to ini file *
	****************************************/

    if( MF_CHECKED & GetMenuState(hmenu,IDM_DATA,MF_BYCOMMAND))
        WritePrivateProfileString("OPTIONS","DATA","1",inipath);
    else
        WritePrivateProfileString("OPTIONS","DATA","0",inipath);

    if( MF_CHECKED & GetMenuState(hmenu,IDM_MASKED,MF_BYCOMMAND))
        WritePrivateProfileString("OPTIONS","MASKED","1",inipath);
    else
        WritePrivateProfileString("OPTIONS","MASKED","0",inipath);

    if( MF_CHECKED & GetMenuState(hmenu,IDM_FLANKING,MF_BYCOMMAND))
        WritePrivateProfileString("OPTIONS","FLANKING","1",inipath);
    else
        WritePrivateProfileString("OPTIONS","FLANKING","0",inipath);


	/*************************************
	* write Size and Position to INI file
	**************************************/

    if( !(IsIconic(hwnd)||IsZoomed(hwnd)) ) /* if window is min or max
                                             don't save size and pos. */
    {
        GetWindowRect(hwnd,&wndrect);

        _itoa(wndrect.left,strbuffer,10);
        WritePrivateProfileString("WINDOW","LEFT",strbuffer,inipath);

        _itoa(wndrect.top,strbuffer,10);
        WritePrivateProfileString("WINDOW","TOP",strbuffer,inipath);

        _itoa(wndrect.right,strbuffer,10);
        WritePrivateProfileString("WINDOW","RIGHT",strbuffer,inipath);

        _itoa(wndrect.bottom,strbuffer,10);
        WritePrivateProfileString("WINDOW","BOTTOM",strbuffer,inipath);

    }
    if(IsZoomed(hwnd))
    {
        WritePrivateProfileString("WINDOW","MAXIMIZED","1",inipath);
    }
    else WritePrivateProfileString("WINDOW","MAXIMIZED","0",inipath);


	
	return;
}

void GetHelpFilePath(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    if(!_fullpath(helppath,"trf.hlp", _MAX_PATH))
    {
        MessageBox(hwnd, "Help files not found!",
                    "Tandem Repeats Finder",MB_ICONINFORMATION|MB_OK);
        strcpy(helppath, "trf.hlp");
    }
    
    return;
}


#endif
