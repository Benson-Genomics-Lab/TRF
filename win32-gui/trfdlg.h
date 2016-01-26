/**********************************************
*   TRFABOUT.H  :    SUPPORT FOR ABOUT DIALOG  *
***********************************************/

#ifndef CFABOUT_H
#define CFABOUT_H

#include <windows.h>
#include <direct.h> /* for _getcwd() */

extern int ndirectories; /* defined in trfini.h */
extern char outputpath[]; /* defined in trfini.h */

BOOL    CALLBACK   AboutDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam,
                                                      LPARAM lParam)
{
    switch(iMsg)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg,IDOK);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


BOOL    CALLBACK   OutputDirDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam,
                                                      LPARAM lParam)
{
    static char ListBuffer[_MAX_PATH];
    static char HoldBuffer[_MAX_PATH];
    HMENU   MainMenu;
    HWND    MainWindow;

    switch(iMsg)
    {
        case WM_INITDIALOG:
            /* if there are directories added to the menu use the first one
               as the place to start looking for a new one, otherwise start
               at the current directory by passing an empty string */ 
            if(ndirectories>0)
            {
                MainWindow = GetParent(hDlg);
                MainMenu = GetMenu(MainWindow);
                GetMenuString(MainMenu, IDM_D1, ListBuffer,
                              _MAX_PATH, MF_BYCOMMAND);
            }
            else
            {
                ListBuffer[0] = '\0';
            }
            DlgDirList( hDlg, ListBuffer, IDC_OUTLIST, IDC_OUTEDIT,
                        DDL_DIRECTORY|DDL_DRIVES|DDL_EXCLUSIVE);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    /* Copy current path to global variable */
                    _getcwd(HoldBuffer, _MAX_PATH);
                    strcpy(outputpath, HoldBuffer);
                    EndDialog(hDlg,IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg,IDCANCEL);
                    return TRUE;

                case IDC_OUTLIST:
                    switch(HIWORD(wParam))
                    {
                        case LBN_DBLCLK:
                            _getcwd(HoldBuffer, _MAX_PATH);
                            strcat(HoldBuffer,"\\");
                            DlgDirSelectEx(hDlg, ListBuffer, _MAX_PATH,
                                            IDC_OUTLIST);
                            if(ListBuffer[1]==':') HoldBuffer[0]='\0';
                            strcat(HoldBuffer,ListBuffer);
                            DlgDirList( hDlg, HoldBuffer, IDC_OUTLIST,
                                        IDC_OUTEDIT, DDL_DIRECTORY|
                                        DDL_DRIVES|DDL_EXCLUSIVE);
                            break;
                    }
                    return TRUE;
            }
            break;
    }
    return FALSE;
}



#endif
