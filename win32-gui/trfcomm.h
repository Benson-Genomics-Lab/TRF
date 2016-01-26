/* trfcomm.h : trf's command handling routines */

#ifndef TRFCOMM_H
#define TRFCOMM_H


#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <mbstring.h>
#include <direct.h> /* for _getcwd() */
#include <process.h>
#include "trf.h"
#include "trfini.h"
#include "trfrun.h"
#include "dirdlg.h"

extern HINSTANCE hInst; /* defined in trf.c */
extern void MakeDirCurrent(HWND, const char*); /* defined in trffile.h */
extern char outputpath[]; /* defined in trfini.h */
extern char seq_fullpath[]; /* defined in trffile.h */


/* Initialize matrix of strings for control IDC_AP_COMBO */
char *AP[]={"2,7,7","2,5,7","2,5,5","2,3,5"};
int   AP_count = 4;  /* number of items in matrix */
int   AP_default = 0; /* zero based */

/* Initialize matrix of strings for control IDC_MAS_COMBO */
char *MAS[]={"20","30","40","50","60","70","80","90","100","150"};
int   MAS_count = 10; /* number of items in matrix */
int   MAS_default = 3; /* zero based */

/* Initialize matrix of strings for control IDC_MPS_COMBO */
char *MPS[] = { "1","2","3","4","5","10","20","30","40","50","100",
                "200","300","400","500","1000","2000"};
int   MPS_count = 17; /* number of items in matrix */
int   MPS_default= 14; /* zero based */


TBBUTTON tbbuttons[] = {    {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0},
							{0,IDM_OPEN,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},

                            {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0},
                            {4,IDM_SAVE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},

                            {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0},
                            {2,IDM_OUTPUTDIR,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},

                            {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0},
                            {5,IDM_OUTTOINPUT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},

                            {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0},
							{1,IDM_RUN,TBSTATE_INDETERMINATE,TBSTYLE_BUTTON,0,0},

                            {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0},
                            {3,IDM_REPORT,TBSTATE_INDETERMINATE,TBSTYLE_BUTTON,0,0}

                            };


void OnCreate(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static WNDCLASSEX      seqwndclass; /* window class for the edit window */
	static char EditClass[]="EditClass";
    HWND        ChildWnd; /* to temporarily hold the window handles */
	TEXTMETRIC tm;
	HDC         hdc;
	RECT        rect; /* client rect */
	int xchar, ychar;
	int counter;

    int nparts = 3;   /* number of panes in status bar */
    int awidths[3] = {200, 400, -1}; /* panes right borders */

	hdc  =  GetDC(hwnd);
	GetTextMetrics(hdc,&tm);
	xchar = tm.tmAveCharWidth;
	ychar = tm.tmHeight+tm.tmExternalLeading;
	ReleaseDC(hwnd,hdc);
	GetClientRect(hwnd,&rect);
	

	InitCommonControls();
	
    ChildWnd   =   CreateToolbarEx(hwnd,
                                    CCS_TOP|WS_CLIPSIBLINGS|
                                    WS_CHILD|WS_VISIBLE|TBSTYLE_TOOLTIPS,
									IDC_TOOLBAR,
                                    6,hInst,IDBITMAP_TOOLBAR,
									tbbuttons,
                                    12,16,15,16,15,sizeof(TBBUTTON));

    if (ChildWnd==NULL)    MessageBox(hwnd,"Toolbar Creation Failed!","TRF",
										MB_ICONEXCLAMATION|MB_OK);
	

	CreateWindow(   "static","Alignment Parameters (match, mismatch, indel)",
                    WS_CHILD|WS_VISIBLE|SS_SIMPLE, 0, 0, 0, 0,
                    hwnd, (HMENU) IDC_AP_STAT, hInst,0);
	
    ChildWnd = CreateWindow(   "combobox",NULL,
                    WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, 0, 0, 0, 0,
                    hwnd,(HMENU)IDC_AP_COMBO, hInst,0);

	for(counter=0;counter<AP_count;counter++)
	{
        SendMessage(ChildWnd, CB_ADDSTRING, 0,  (LPARAM) AP[counter]);
	}
	
	CreateWindow(   "static","Minimum Alignment Score To Report Repeat",
                    WS_CHILD|WS_VISIBLE|SS_SIMPLE, 0, 0, 0, 0,
                    hwnd, (HMENU) IDC_MAS_STAT, hInst,0);
	
    ChildWnd = CreateWindow(   "combobox",NULL,
                    WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, 0, 0, 0, 0,
                    hwnd, (HMENU)IDC_MAS_COMBO, hInst, 0);

	for(counter=0;counter<MAS_count;counter++)
	{
        SendMessage(ChildWnd, CB_ADDSTRING, 0,  (LPARAM) MAS[counter]);
	}

    CreateWindow(   "static","Maximun Period Size",
                    WS_CHILD|WS_VISIBLE|SS_SIMPLE, 0, 0, 0, 0,
                    hwnd, (HMENU) IDC_MPS_STAT, hInst,0);

    ChildWnd = CreateWindow( "combobox",NULL,
                    WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, 0, 0, 0, 0,
                    hwnd, (HMENU) IDC_MPS_COMBO, hInst,0);

	for(counter=0;counter<MPS_count;counter++)
	{
        SendMessage(ChildWnd, CB_ADDSTRING, 0,  (LPARAM) MPS[counter]);
	}


    CreateWindow(   "button","Parameters", WS_CHILD|WS_VISIBLE|BS_GROUPBOX,
                    0, 0, 0, 0, hwnd,(HMENU) IDC_PARAM_GRP, hInst, 0);

    CreateWindow(   "button","Output Destination",
                    WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 0, 0, 0, 0,
                    hwnd, (HMENU) IDC_PREFIX_GRP, hInst, 0);

    CreateWindow(   "static","Directory",
                    WS_CHILD|WS_VISIBLE|SS_SIMPLE, 0, 0, 0, 0,
                    hwnd, (HMENU) IDC_DIR_STAT, hInst,0);

    ChildWnd = CreateWindowEx( WS_EX_STATICEDGE, "edit","",
                    WS_CHILD|WS_VISIBLE|ES_LEFT|ES_AUTOHSCROLL|ES_READONLY,
                    0, 0, 0, 0, hwnd,(HMENU)IDC_DIR_EDIT, hInst,0);

    SendMessage(ChildWnd,EM_SETMARGINS,(WPARAM)EC_LEFTMARGIN ,(LPARAM)10);


    CreateWindow(   "static","File Prefix",
                    WS_CHILD|WS_VISIBLE|SS_SIMPLE, 0, 0, 0, 0,
                    hwnd, (HMENU) IDC_PREFIX_STAT, hInst,0);

    ChildWnd = CreateWindowEx( WS_EX_STATICEDGE, "edit","",
                    WS_CHILD|WS_VISIBLE|ES_LEFT|ES_AUTOHSCROLL|WS_DISABLED,
                    0, 0, 0, 0, hwnd,(HMENU)IDC_PREFIX_EDIT, hInst,0);

    SendMessage(ChildWnd,EM_SETMARGINS,(WPARAM)EC_LEFTMARGIN ,(LPARAM)10);

	CreateWindow(   "button","Sequence",
                    WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 0, 0, 0, 0,
                    hwnd, (HMENU) IDC_SEQ_GRP, hInst,0);

    CreateWindow(   "static","",
                    WS_CHILD|WS_VISIBLE|SS_LEFTNOWORDWRAP, 0, 0, 0, 0,
                    hwnd, (HMENU) IDC_SEQ_STAT, hInst,0);

	
    ChildWnd = CreateWindowEx(WS_EX_STATICEDGE,"edit","",
            WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOHSCROLL|
            ES_AUTOVSCROLL|ES_READONLY, 0, 0, 0, 0,
            hwnd,(HMENU)IDC_SEQ_EDIT,hInst,0);

    ChildWnd = CreateStatusWindow( WS_CHILD|WS_VISIBLE, "Ready", hwnd, IDC_STATUSBAR);
    SendMessage(ChildWnd, SB_SETPARTS,(WPARAM) nparts,
                (LPARAM)(LPINT)awidths);

    paramset.percent=-1;

    return;
}



void OnSize(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND            ChildWnd;
    TEXTMETRIC      tm;
    HDC             hdc;
    RECT            rect; /* client rectangle of main window */
    int             xchar, ychar;

    /* find out the text metrics */
	hdc  =  GetDC(hwnd);
	GetTextMetrics(hdc,&tm);
	xchar = tm.tmAveCharWidth;
	ychar = tm.tmHeight+tm.tmExternalLeading;
	ReleaseDC(hwnd,hdc);

    /* find out the size of client area */
	GetClientRect(hwnd,&rect);

    /* Resize the toolbar */
    ChildWnd =  GetDlgItem(hwnd, IDC_TOOLBAR);
    SendMessage(ChildWnd,TB_AUTOSIZE,0,0);
			
    /* Resize the status bar */
    ChildWnd =  GetDlgItem(hwnd, IDC_STATUSBAR);
    SendMessage(ChildWnd, WM_SIZE,0,0);

    /* rezize IDC_PARAM_GRP */
    ChildWnd =  GetDlgItem(hwnd, IDC_PARAM_GRP);
    MoveWindow(  ChildWnd, 0, 2*ychar, rect.right, 8*ychar, TRUE);

    /* resize IDC_AP_STAT */
    ChildWnd =  GetDlgItem(hwnd, IDC_AP_STAT);
    MoveWindow( ChildWnd, 3*xchar,(int)(3.5*ychar),
                50*xchar,2*ychar, TRUE);

    /* resize IDC_AP_COMBO */
    ChildWnd =  GetDlgItem(hwnd, IDC_AP_COMBO);
    MoveWindow( ChildWnd, 53*xchar, (int)(3.5*ychar),
                10*xchar,200*ychar, TRUE);

    /* resize IDC_MAS_STAT */
    ChildWnd =  GetDlgItem(hwnd, IDC_MAS_STAT);
    MoveWindow( ChildWnd, 3*xchar, (int)(5.5*ychar),
                50*xchar, 2*ychar, TRUE);

    /* resize IDC_MAS_COMBO */
    ChildWnd =  GetDlgItem(hwnd, IDC_MAS_COMBO);
    MoveWindow( ChildWnd, 53*xchar,(int)(5.5*ychar),
                10*xchar, 200*ychar, TRUE);

    /* resize IDC_MPS_STAT */
    ChildWnd =  GetDlgItem(hwnd, IDC_MPS_STAT);
    MoveWindow( ChildWnd, 3*xchar,(int)(7.5*ychar),
                22*xchar,2*ychar, TRUE);

    /* resize IDC_MPS_COMBO */
    ChildWnd =  GetDlgItem(hwnd, IDC_MPS_COMBO);
    MoveWindow( ChildWnd, 30*xchar,(int)(7.5*ychar),
                8*xchar,200*ychar, TRUE);

    /* resize IDC_PREFIX_GRP */
    ChildWnd =  GetDlgItem(hwnd, IDC_PREFIX_GRP);
    MoveWindow( ChildWnd, 0, (int)(10.5*ychar),
                rect.right, 6*ychar, TRUE);

    /* resize IDC_DIR_START */
    ChildWnd =  GetDlgItem(hwnd, IDC_DIR_STAT);
    MoveWindow( ChildWnd, 3*xchar, (int)(12*ychar),
                18*xchar,(int)(1.5*ychar), TRUE);

    /* resize IDC_DIR_EDIT */
    ChildWnd =  GetDlgItem(hwnd, IDC_DIR_EDIT);
    MoveWindow( ChildWnd, 18*xchar, (int)(12*ychar),
                rect.right-21*xchar,(int)(1.5*ychar), TRUE);

    /* resize IDC_PREFIX_START */
    ChildWnd =  GetDlgItem(hwnd, IDC_PREFIX_STAT);
    MoveWindow( ChildWnd, 3*xchar, (int)(14*ychar),
                20*xchar,(int)(1.5*ychar), TRUE);

    /* resize IDC_PREFIX_EDIT */
    ChildWnd =  GetDlgItem(hwnd, IDC_PREFIX_EDIT);
    MoveWindow( ChildWnd, 18*xchar, (int)(14*ychar),
                rect.right-21*xchar,(int)(1.5*ychar), TRUE);

    /* resize IDC_SEQ_GRP */
    ChildWnd =  GetDlgItem(hwnd, IDC_SEQ_GRP);
    MoveWindow( ChildWnd, 0, 17*ychar, rect.right,
                rect.bottom-(int)(18.5*ychar), TRUE);

    /* resize IDC_SEQ_STAT */
    ChildWnd = GetDlgItem(hwnd, IDC_SEQ_STAT);
    MoveWindow( ChildWnd, 3*xchar, (int)(18.5*ychar),
                rect.right-4*xchar, (int)(1.5*ychar), TRUE);

    /* resize IDC_SEQ_EDIT */
    ChildWnd =  GetDlgItem(hwnd, IDC_SEQ_EDIT);
    MoveWindow( ChildWnd, 3*xchar, 20*ychar,
                rect.right-6*xchar,rect.bottom-22*ychar, TRUE);
    
    return;
}


void OnReport(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    char    fullhtmlfilename[_MAX_PATH];

    /* make full file name using the content of the output directory*/
    GetDlgItemText(hwnd, IDC_DIR_EDIT, fullhtmlfilename, _MAX_PATH);
    strcat(fullhtmlfilename, "\\");
    strcat(fullhtmlfilename, paramset.outputfilename);


	/* Do the call to the shell */
	ShellExecute(hwnd,(LPCTSTR) "open", (LPCTSTR) fullhtmlfilename,
					NULL,NULL,0);

	return;
}

void OnSave(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	char bigbuffer[20];
	char smallbuffer[10];
	char loopbuffer[20];
	HWND ChildWnd;
	int  count; /* counter for menu string substitution */
	HMENU MainMenu;
	HMENU PopupMenu;


	MainMenu = GetMenu(hwnd);
	PopupMenu= GetSubMenu(MainMenu,1);

	bigbuffer[0]='\0';

	/* insert control's text */
	ChildWnd =  GetDlgItem(hwnd, IDC_AP_COMBO);
	SendMessage(ChildWnd, WM_GETTEXT, (WPARAM) 10,(LPARAM) smallbuffer);
	strcat(bigbuffer,smallbuffer);
	/* insert comma */
	strcat(bigbuffer,",");

	/* insert control's text */
	ChildWnd =  GetDlgItem(hwnd, IDC_MAS_COMBO);
	SendMessage(ChildWnd, WM_GETTEXT, (WPARAM) 10,(LPARAM) smallbuffer);
	strcat(bigbuffer,smallbuffer);
	/* insert comma */
	strcat(bigbuffer,",");

	/* insert control's text */
	ChildWnd =  GetDlgItem(hwnd, IDC_MPS_COMBO);
	SendMessage(ChildWnd, WM_GETTEXT, (WPARAM) 10,(LPARAM) smallbuffer);
	strcat(bigbuffer,smallbuffer);


	/* menu strings have to be moved down to make room for new item*/
	/* if nparameter less than 4 a new new item has to be appended */
	if(nparameters<4)
	{
		AppendMenu(PopupMenu,MF_ENABLED|MF_STRING,IDM_P1+(UINT)nparameters," ");
		nparameters++;
	}
	/* move menu strings down one space from last to first */
	for(count=0;count<(nparameters-1);count++)
	{
		/* Get string for prior item */
		GetMenuString(PopupMenu,IDM_P1+nparameters-1-count-1,
						loopbuffer, 20, MF_BYCOMMAND);

		/* Set string for and uncheck current item */
		ModifyMenu(PopupMenu, IDM_P1+nparameters-1-count,
					MF_BYCOMMAND|MF_STRING|MF_UNCHECKED,
					IDM_P1+nparameters-1-count,(LPCTSTR) loopbuffer);
	}

	/* set the first item's string and check mark */
	ModifyMenu(PopupMenu, IDM_P1,MF_BYCOMMAND|MF_STRING|MF_CHECKED,
				IDM_P1,(LPCTSTR) bigbuffer);

	/* done */

	return;
}

void UpdateParameters(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU   hMenu;
	char    strbuffer[20];
	int     counter;
	HWND    hwndchild;

	/* find menu item that is checked */
	hMenu = GetMenu(hwnd);
	for(counter=0; counter<nparameters ; counter++)
	{
		/* loop until the checked parameter set is found */
		if( MF_CHECKED & GetMenuState(hMenu,IDM_P1+counter,MF_BYCOMMAND))
				 break;

	}

	/* Copy string from menu to strbuffer*/
	GetMenuString(hMenu, IDM_P1+counter,strbuffer,20,MF_BYCOMMAND);


	/* break strbuffer into 3 separate strings */
    strbuffer[5]='\0';
    for(counter=0;strbuffer[6+counter]!=',';counter++);
    strbuffer[6+counter]='\0';

    /* set first string into IDC_AP_COMBO */
	hwndchild = GetDlgItem(hwnd,IDC_AP_COMBO);
	SendMessage(hwndchild,CB_SELECTSTRING,(WPARAM) -1,
                (LPARAM)(LPCSTR) strbuffer );
	
    /* set second string into IDC_MAS_COMBO */
	hwndchild = GetDlgItem(hwnd,IDC_MAS_COMBO);
	SendMessage(hwndchild,CB_SELECTSTRING,(WPARAM) -1,
                (LPARAM)(LPCSTR) &(strbuffer[6]) );

    /* set last string into IDC_MPS_COMBO */
	hwndchild = GetDlgItem(hwnd,IDC_MPS_COMBO);
	SendMessage(hwndchild,CB_SELECTSTRING,(WPARAM) -1,
                (LPARAM)(LPCSTR) &(strbuffer[counter+7]) );

	/*done */

	return;

}

void OnParameterSelect(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU hmenu;
	int count;

	hmenu = GetMenu(hwnd);

	/* uncheck all items from IDM_P1 to IDM_P1+parameters */
	for (count=0; count<nparameters; count++)
	{
		CheckMenuItem(hmenu, IDM_P1+count,MF_BYCOMMAND|MF_UNCHECKED);
	}

	/* check menu item that was clicked or selected */
	CheckMenuItem(hmenu,(UINT) LOWORD(wParam),MF_BYCOMMAND|MF_CHECKED);

	/* call fuction to update values on controls */
	UpdateParameters( hwnd, iMsg, wParam, lParam);

	return;
}

void OnPrevious(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME ofn;                     
	char strfullpath[_MAX_PATH];
    HMENU   hMenu;

	static char szFilter[]  =   "HTML Files (*.html)\0*.html\0"\
								"All Files (*.*)\0*.*\0\0";

    /* if there is at least one output directory setup, make that
    directory the current directory. Otherwise ckeck if there is a
    sequence in the file menu and chage to that directory. If neither
    of these two works just stay in current directory */

    hMenu = GetMenu(hwnd);
    if(ndirectories)
    {
        GetMenuString( hMenu,IDM_D1, strfullpath , _MAX_PATH , MF_BYCOMMAND );
        _chdir(strfullpath);
    }
    else
    {
        if(nsequences)
        {
            GetMenuString( hMenu,IDM_S1, strfullpath , _MAX_PATH ,
                           MF_BYCOMMAND );
            MakeDirCurrent(hwnd, strfullpath);
            /* this function is used because IDM_S1 contains a file name*/
        }
    }
    
	strfullpath[0]          = (char) NULL;

	ofn.lStructSize         =   sizeof(OPENFILENAME);
	ofn.hwndOwner           =   hwnd;
	ofn.hInstance           =   NULL;
	ofn.lpstrFilter         =   szFilter;
	ofn.lpstrCustomFilter   =   NULL;
	ofn.nMaxCustFilter      =   0;
	ofn.nFilterIndex        =   1;
	ofn.nMaxFile            =   _MAX_PATH;
	ofn.nMaxFileTitle       =   0;
	ofn.lpstrInitialDir     =   NULL;
    ofn.lpstrTitle          =   "Open Previous Reports";
	ofn.nFileOffset         =   0;
	ofn.nFileExtension      =   0;
	ofn.lpstrDefExt         =   NULL;
	ofn.lCustData           =   0L;
	ofn.lpfnHook            =   NULL;
	ofn.lpTemplateName      =   NULL;
	ofn.lpstrFile           =   strfullpath;
	ofn.lpstrFileTitle      =   NULL;
	ofn.Flags               =   OFN_HIDEREADONLY | OFN_CREATEPROMPT;

	if(GetOpenFileName(&ofn))
	{
		/* make system call to shell */
		ShellExecute(hwnd,(LPCTSTR) "open", (LPCTSTR) strfullpath,
					NULL,NULL,0);
	}

	return;
}

void DisableReport(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND    Child;
	HMENU   hMenu;

	/* Disable the toobar button */
    Child =   GetDlgItem(hwnd, IDC_TOOLBAR);
    SendMessage(Child, TB_SETSTATE, (WPARAM) IDM_REPORT,
                (LPARAM) MAKELONG(TBSTATE_INDETERMINATE, 0));

	/* Enable menu item for "Start Search" */
	hMenu   =   GetMenu(hwnd);
	EnableMenuItem(hMenu, IDM_REPORT, MF_GRAYED);

    /* Change the status bar text to "Ready" */
    Child   =   GetDlgItem(hwnd, IDC_STATUSBAR);
    SendMessage(Child, SB_SETTEXT, (WPARAM) 0, 
                    (LPARAM) (LPSTR) "Ready.");
    UpdateWindow(Child);

                                               
	
	return;
}
void OnDefault(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    char strbuffer[20] = "2,7,7,50,500";
    char loopbuffer[20];
	int  count; /* counter for menu string substitution */
	HMENU MainMenu;
	HMENU PopupMenu;


	MainMenu = GetMenu(hwnd);
	PopupMenu= GetSubMenu(MainMenu,1);


	/* menu strings have to be moved down to make room for new item*/
	/* if nparameter less than 4 a new new item has to be appended */
	if(nparameters<4)
	{
		AppendMenu(PopupMenu,MF_ENABLED|MF_STRING,IDM_P1+(UINT)nparameters," ");
		nparameters++;
	}
	/* move menu strings down one space from last to first */
	for(count=0;count<(nparameters-1);count++)
	{
		/* Get string for prior item */
		GetMenuString(PopupMenu,IDM_P1+nparameters-1-count-1,
						loopbuffer, 20, MF_BYCOMMAND);

		/* Set string for and uncheck current item */
		ModifyMenu(PopupMenu, IDM_P1+nparameters-1-count,
					MF_BYCOMMAND|MF_STRING|MF_UNCHECKED,
					IDM_P1+nparameters-1-count,(LPCTSTR) loopbuffer);
	}

	/* set the first item's string and check mark */
	ModifyMenu(PopupMenu, IDM_P1,MF_BYCOMMAND|MF_STRING|MF_CHECKED,
                IDM_P1,(LPCTSTR) strbuffer);

	/* call fuction to update values on controls */
	UpdateParameters( hwnd, iMsg, wParam, lParam);
    
    return;
}

void OnOutputdir(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    UINT choice;
	HMENU MainMenu;
	HMENU PopupMenu;
    char    strbuffer[_MAX_PATH];
    int count;
    int position;
    int found=0;
    UINT menustate;
    DIRSEARCHNAME dsn;

    /* use doug's dialog to get directory */
    dsn.hwndOwner   = hwnd;
    dsn.hInstance   = hInst;
    dsn.lpstrDir    = outputpath;
    dsn.nMaxDir     = _MAX_PATH;
    dsn.lpstrInitialDir = NULL;
    dsn.lpstrTitle  = "TRF Output Directory";

    choice = GetDirSearchName( &dsn);

    /* only execute if dismised with OK button */
    if(choice)
    {

        MainMenu = GetMenu(hwnd);
        PopupMenu= GetSubMenu(MainMenu,2);

        /**********************************************************/
        /*if outputpath is already in menu list move it to top and*/
        /*mark as selected                                        */
        /**********************************************************/

        /* find position where it is located in menu*/
        for(count=0; count<ndirectories; count++)
        {
            GetMenuString( PopupMenu, IDM_D1+count, strbuffer,
                           _MAX_PATH, MF_BYCOMMAND);
            /* if menu string is equal to outputpath break and set found*/
            if(!strcmp(strbuffer,outputpath))
            {
                position = count; /* zero based from IDM_D1 */
                found = 1; /* used a couple of sections down */
                break;
            }
        }

        /* If selected item is found and position is checked then
        don't do anything else since the box was dismised with OK
        but no new directory was selected. This also controls that
        report viewing doesn't have to be disabled */
        if(found)
        {
            menustate = GetMenuState(PopupMenu,IDM_D1+position,MF_BYCOMMAND);
            if(MF_CHECKED & menustate)
                return;
        }

        /*Disable current report since output directory has changed*/
        DisableReport(hwnd, iMsg, wParam, lParam);
            

        /* uncheck all the items since new one will be selected */
        for( count=0;count<=ndirectories;count++)
        {
             CheckMenuItem(PopupMenu, IDM_OUTTOINPUT+count,
                           MF_BYCOMMAND|MF_UNCHECKED);
        }

        if(found) /* move item to the beginning of list */
        {
            /* this code also works if item is found as first */
            /* in which case position=0 */
            for(count=position;count>0;count--)
            {
                GetMenuString(PopupMenu,IDM_D1+count-1,
                                strbuffer,_MAX_PATH,MF_BYCOMMAND);
                ModifyMenu(PopupMenu, IDM_D1+count,
                            MF_BYCOMMAND|MF_STRING|MF_UNCHECKED,
                            IDM_D1+count,(LPCTSTR) strbuffer);
            }

            /* Set IDM_D1 with outputpath string and check it*/
            ModifyMenu(PopupMenu, IDM_D1,MF_BYCOMMAND|MF_STRING|MF_CHECKED,
                            IDM_D1,(LPCTSTR) outputpath);
        }            

        /************************************************************/
        /*if item is not on list move items down and insert new item*/
        /*at the top and mark as selecte                            */
        /************************************************************/
        else
        {
            if(ndirectories<4)
            {
                AppendMenu(PopupMenu,MF_ENABLED|MF_STRING,
                           IDM_D1+(UINT)ndirectories," ");
                ndirectories++;
            }

            /* move menu strings down one space from last to first */
            for(count=ndirectories; count>1 ;count--)
            {
                /* Get string for prior item */
                GetMenuString(PopupMenu,IDM_D1+count-2,
                                strbuffer, _MAX_PATH, MF_BYCOMMAND);

                /* Set string for and uncheck current item */
                ModifyMenu(PopupMenu, IDM_D1+count-1,
                            MF_BYCOMMAND|MF_STRING|MF_UNCHECKED,
                            IDM_D1+count-1,(LPCTSTR) strbuffer);
            }

            /* set the first item's string */
            ModifyMenu(PopupMenu, IDM_D1,MF_BYCOMMAND|MF_STRING|MF_CHECKED,
                        IDM_D1,(LPCTSTR) outputpath);


        }
        /* make item the text of IDC_DIR_EDIT */
        SetDlgItemText(hwnd, IDC_DIR_EDIT, outputpath);

    }        
    return;
}

void OnDirChoice(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU MainMenu;
	HMENU PopupMenu;
    char    strbuffer[_MAX_PATH];
    int count;
    int position;
    UINT menustate;

    /* Get handles to menu and submenu */
    MainMenu = GetMenu(hwnd);
    PopupMenu= GetSubMenu(MainMenu,2);

    /* find position of item cliked (zero based from IDM_OUTTOINPUT) */
    position = LOWORD(wParam)-IDM_OUTTOINPUT;


    /* If selected item is already checked then don't do anything else
    since the item that was clicked was the one that was active. This
    also controls that report viewing doesn't have to be disabled */

    menustate = GetMenuState(PopupMenu,IDM_OUTTOINPUT+position,MF_BYCOMMAND);
    if(MF_CHECKED & menustate) return;

    /*Disable current report since output directory has changed*/
    DisableReport(hwnd, iMsg, wParam, lParam);

    /* uncheck all the items */
    for(count=0;count<=ndirectories;count++)
    {
         CheckMenuItem(PopupMenu, IDM_OUTTOINPUT+count,MF_BYCOMMAND|MF_UNCHECKED);
    }

    if(position==0) /* means IDM_OUTTOINPUT was cliked */
    {

        /* check IDM_OUTTOINPUT */
        CheckMenuItem(PopupMenu, IDM_OUTTOINPUT,MF_BYCOMMAND|MF_CHECKED);

        /* set outputpath to '\0' */
        outputpath[0]='\0';

        /* If sequence is open set content of IDC_DIR_EDIT to path of
           open sequence, otherwise clear it*/
        GetDlgItemText(hwnd, IDC_PREFIX_EDIT, strbuffer, _MAX_PATH);
        if(strcmp(strbuffer,""))
        {
            MakeDirCurrent(hwnd, seq_fullpath);
            _getcwd(strbuffer, _MAX_PATH);
            SetDlgItemText(hwnd,IDC_DIR_EDIT, strbuffer);
        }
        else SetDlgItemText(hwnd, IDC_DIR_EDIT, "");

    }
    else /* means one of the other items was cliked */ 
    {
        /* get string of the item that was clicked and put it in outputpath*/
        GetMenuString(PopupMenu,IDM_OUTTOINPUT+position,
                                outputpath, _MAX_PATH, MF_BYCOMMAND);

        /* move item to start of list */
        /* this code also works if item is found as first */
        /* in which case position=1 */
        for(count=position;count>1;count--)
        {
            /* Get Menu String of item on previous position of count */
            GetMenuString(PopupMenu,IDM_OUTTOINPUT+count-1, strbuffer,
                          _MAX_PATH,MF_BYCOMMAND);
            ModifyMenu(PopupMenu,IDM_OUTTOINPUT+count,
                       MF_BYCOMMAND|MF_STRING|MF_UNCHECKED,
                       IDM_OUTTOINPUT+count,(LPCTSTR) strbuffer);
        }

        /* Set IDM_D1 with outputpath string and check it*/
        ModifyMenu(PopupMenu, IDM_D1,MF_BYCOMMAND|MF_STRING|MF_CHECKED,
                   IDM_D1,(LPCTSTR) outputpath);

        /* Output the string to IDC_DIR_EDIT */
        SetDlgItemText(hwnd, IDC_DIR_EDIT, outputpath);

    }

    return;
}

void OnOptions(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    UINT item;
    int  checked;
    HMENU MainMenu;

    MainMenu = GetMenu(hwnd);

    /* Find out which item was clicked */
    item = (UINT) LOWORD(wParam);
    /*Find out if item is checked*/
    checked = MF_CHECKED & GetMenuState(MainMenu,item, MF_BYCOMMAND);

    /* if checked then unchek and viceversa */
    if(checked)
        CheckMenuItem(MainMenu,item,MF_BYCOMMAND|MF_UNCHECKED);
    else
        CheckMenuItem(MainMenu,item,MF_BYCOMMAND|MF_CHECKED);

    return;
}


void WorkThread(void* dummy)
{
    HWND hwndchild;


    /* change the cursor to hourglass */
    SetCursor(LoadCursor(NULL,IDC_WAIT));
    paramset.running = 1;

    /* set a timer for updating the progress bar */
    SetTimer((HWND)paramset.guihandle,1,1000,NULL);

    /* call tandem repeats finder routine */
    TRFControlRoutine();

    /* if successful then enable menu items */
    if(paramset.endstatus==CTRL_SUCCESS)
    {
        /* Enable the toobar button to view report */
        hwndchild =   GetDlgItem((HWND)paramset.guihandle, IDC_TOOLBAR);
        SendDlgItemMessage((HWND)paramset.guihandle, IDC_TOOLBAR,
                           TB_SETSTATE,(WPARAM) IDM_REPORT,
                           (LPARAM) MAKELONG(TBSTATE_ENABLED, 0));

        /* Enable menu item for "View | Current Report" */
        EnableMenuItem(GetMenu((HWND)paramset.guihandle), IDM_REPORT,
                       MF_ENABLED);
    }

    paramset.running = 0;

    /* change cursor to arrow */
    SetCursor(LoadCursor(NULL,IDC_ARROW));

    /* kill the timer for updating the progress bar */
    KillTimer((HWND)paramset.guihandle,1);


    return;
}

void OnRun(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndchild;
    char buffer[500];



    /*****************************************
    *   set parameters from interface into
    *   globals found in trfrun.h
    ******************************************/

    /* get match, mismatch, and indel from interface */
    hwndchild = GetDlgItem(hwnd, IDC_AP_COMBO);
    SendMessage(hwndchild,WM_GETTEXT,(WPARAM) 500, (LPARAM) buffer);
    buffer[1] = '\0';
    buffer[3] = '\0';
    buffer[5] = '\0';
    paramset.match   = atoi(buffer);
    paramset.mismatch= atoi(&buffer[2]);
    paramset.indel   = atoi(&buffer[4]);

    /* get minscore */
    hwndchild = GetDlgItem(hwnd, IDC_MAS_COMBO);
    SendMessage(hwndchild,WM_GETTEXT,(WPARAM) 500, (LPARAM) buffer);
    paramset.minscore = atoi(buffer);

    /* get maxperiod */
    hwndchild = GetDlgItem(hwnd, IDC_MPS_COMBO);
    SendMessage(hwndchild,WM_GETTEXT,(WPARAM) 500, (LPARAM) buffer);
    paramset.maxperiod = atoi(buffer);

    /* PM and PI are constant in this version */
    paramset.PM = 80;
    paramset.PI = 10;

    /* from the menu state set the flags */
    paramset.datafile = (MF_CHECKED & GetMenuState(GetMenu(hwnd),
                         IDM_DATA,MF_BYCOMMAND))>0;
    paramset.maskedfile = (MF_CHECKED & GetMenuState(GetMenu(hwnd),
                           IDM_MASKED, MF_BYCOMMAND))>0;
    paramset.flankingsequence = (MF_CHECKED & GetMenuState(GetMenu(hwnd),
                                 IDM_FLANKING, MF_BYCOMMAND))>0;
    paramset.flankinglength = 500;

    /* get the name of the input file */
    strcpy(paramset.inputfilename,seq_fullpath);

    /* get the prefix for the output files */
    GetDlgItemText(hwnd, IDC_PREFIX_EDIT, paramset.outputprefix, _MAX_PATH);

    /* get the output directory and change to it */
    GetDlgItemText(hwnd, IDC_DIR_EDIT, paramset.outputdirectory, _MAX_PATH);
    _chdir(paramset.outputdirectory);

    /* for set the gui handle to handle of main window */
    paramset.guihandle = (int) hwnd;


#ifdef  PCDEBUG
    sprintf(buffer, "match: %d\nmismatch: %d\nindel: %d\n"
                    "minscore: %d\nmaxperiod: %d\n"
                    "PM: %d\nPI: %d\ndatafile: %d\n"
                    "maskedfile: %d\nflankingsequence: %d\n"
                    "inputfile: %s\noutputprefix: %s\n"
                    "outputdirectory: %s\nguihandle: %d",
                    
            paramset.match,paramset.mismatch,paramset.indel,
            paramset.minscore,paramset.maxperiod,
            paramset.PM,paramset.PI,paramset.datafile,
            paramset.maskedfile,paramset.flankingsequence,
            paramset.inputfilename,paramset.outputprefix,
            paramset.outputdirectory,paramset.guihandle);
    MessageBox(NULL,buffer,"TRF",MB_OK);
#endif


    /* run program on a different thread and return */
    _beginthread(WorkThread,0,NULL);

    return;
}

#endif
