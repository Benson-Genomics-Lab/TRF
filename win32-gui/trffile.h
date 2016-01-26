/* trffile.h : Handles the file opening and reading routine */

#ifndef TRFFILE_H
#define TRFFILE_H

#include <windows.h>
#include <commdlg.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include "trf.h"
#include "trfcomm.h"
#include "tr30dat.h"

char    seq_fullpath[_MAX_PATH];
char*   seq_buffer;


static long FileLength(FILE *file)
{
    int iCurrentPos, iFileLength;

    iCurrentPos = ftell(file);

    fseek(file, 0, SEEK_END);

    iFileLength = ftell(file);

    fseek(file, iCurrentPos, SEEK_SET);

    return iFileLength;
}

void MakeDirCurrent(HWND hwnd, const char * fullfilename)
{
    char drive[_MAX_DRIVE+1];
    char path[_MAX_PATH];
    char driveandpath[_MAX_DRIVE+1+_MAX_PATH];

    /* Get the drive and path portion out of fullfilename */
    _splitpath( fullfilename, drive, path, NULL, NULL);
    strcpy(driveandpath,drive);
    strcat(driveandpath,path);

    /* Use driveandpath to make current directory */
    _chdir(driveandpath);

    return;
}

int SBaseCount(char* buffer)
{
    int count=0;
    int pos=0;

    /* ignore first line */
    while(buffer[pos]!='\n') pos++;

    for(; buffer[pos]!='\0'; pos++)
    {
        if ( isalpha(buffer[pos])) count++;
    }

    return count;
}
    

BOOL TRFReadFile(HWND hwnd, PSTR pstrFileName)
{
    FILE *file;
    int iLength;

    if (NULL ==( file = fopen(pstrFileName, "rb")))
        return FALSE;

    iLength = FileLength(file);
    seq_buffer = (PSTR) malloc(iLength);

    if(NULL == seq_buffer)
    {
        fclose(file);
        return FALSE;
    }

    fread(seq_buffer, 1, iLength, file);
    fclose(file);
    seq_buffer[iLength-1] = '\0';
    /* Validate for FASTA Format */
    if(seq_buffer[0]!='>')
    {
        MessageBox(hwnd,"File format is not valid !","Tandem Repeats Finder",
                       MB_ICONEXCLAMATION | MB_OK);
        free(seq_buffer);
        return FALSE;
    }

    return TRUE;
}


void AddSeqToMenu(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HMENU MainMenu;
    HMENU PopupMenu;
    int ispresent; /* indicates wether or not item is in history */
    int position;  /* indicates position where item has been found */
    int count;     /* used for looping */
    char strbuffer[_MAX_PATH]; /* to hold string being retrived from menu */

    MainMenu =  GetMenu(hwnd);
    PopupMenu = GetSubMenu(MainMenu,0);

    /* Insert a separator if this is the first sequence to be added */
    if(nsequences==0)
    {
        InsertMenu(PopupMenu,2,MF_BYPOSITION|MF_SEPARATOR,
                    (UINT) 0,(LPCTSTR) NULL);
    }


    /* determine if string is in one of the menus */
    ispresent = 0;
    position  = 0;
    for(count=0;count<nsequences;count++)
    {
        GetMenuString(PopupMenu, IDM_S1+count,strbuffer,_MAX_PATH,MF_BYCOMMAND);
        if(!strcmp(strbuffer,seq_fullpath))
        {
            position = count; /* count is number of items from IDM_S1, zero based */
            ispresent = 1; /* flaged to true */
        }
    }

     /* if string was found move text labels down one space form prior item*/
    if(ispresent)
    {
        /* this code also works if item is found as first */
        /* in which case position=0 */
        for(count=position;count>0;count--)
        {
            GetMenuString(PopupMenu,IDM_S1+count-1,
                            strbuffer,_MAX_PATH,MF_BYCOMMAND);
            ModifyMenu(PopupMenu, IDM_S1+count,
                        MF_BYCOMMAND|MF_STRING|MF_UNCHECKED,
                        IDM_S1+count,(LPCTSTR) strbuffer);
        }

        /* Set IDM_S1 with path string */
        ModifyMenu(PopupMenu, IDM_S1,MF_BYCOMMAND|MF_STRING|MF_UNCHECKED,
                        IDM_S1,(LPCTSTR) seq_fullpath);
    }
    /* if not found add to the beginning of list, moving everithing */
    /* down. if number of sequences < 4 insert a new item at end of list */
    else
    {
        if(nsequences<4)
        {
            InsertMenu(PopupMenu,2+nsequences,MF_ENABLED|MF_BYPOSITION|MF_STRING,
                        IDM_S1+nsequences,(LPCTSTR) "Dummy");
            nsequences++;
        }

        /* move menu strings down one space from last to first */
        for(count=nsequences; count>1 ;count--)
        {
            /* Get string for prior item */
            GetMenuString(PopupMenu,IDM_S1+count-2,
                            strbuffer, _MAX_PATH, MF_BYCOMMAND);

            /* Set string for and uncheck current item */
            ModifyMenu(PopupMenu, IDM_S1+count-1,
                        MF_BYCOMMAND|MF_STRING|MF_UNCHECKED,
                        IDM_S1+count-1,(LPCTSTR) strbuffer);
        }

        /* set the first item's string */
        ModifyMenu(PopupMenu, IDM_S1,MF_BYCOMMAND|MF_STRING|MF_UNCHECKED,
                    IDM_S1,(LPCTSTR) seq_fullpath);

        
    }

    
    return;
}

void OnOpen(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    OPENFILENAME ofn;                     
    HWND    EditWnd;
    HMENU   hMenu;
    char strfullpath[_MAX_PATH];
    char strdirbuffer[_MAX_PATH];
    char    strfiletitle[_MAX_PATH];
    static char szFilter[]  =   "All Files (*.*)\0*.*\0\0";

    /* make directory of last sequence the current */
    if(nsequences>0)
    {
        hMenu   =   GetMenu(hwnd);
        GetMenuString( hMenu,IDM_S1, strfullpath , _MAX_PATH , MF_BYCOMMAND );
        MakeDirCurrent(hwnd,strfullpath);
    }

    /* make string of 0 length */
    strfullpath[0]          =   (char) NULL;

    ofn.lStructSize         =   sizeof(OPENFILENAME);
    ofn.hwndOwner           =   hwnd;
    ofn.hInstance           =   NULL;
    ofn.lpstrFilter         =   szFilter;
    ofn.lpstrCustomFilter   =   NULL;
    ofn.nMaxCustFilter      =   0;
    ofn.nFilterIndex        =   0;
    ofn.nMaxFile            =   _MAX_PATH;
    ofn.nMaxFileTitle       =   0;
    ofn.lpstrInitialDir     =   NULL;
    ofn.lpstrTitle          =   "Open Sequence";
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
        if(!TRFReadFile(hwnd, strfullpath))
        {
            MessageBox(hwnd,"File Could Not Be Opened !","Tandem Repeats Finder",
                       MB_ICONEXCLAMATION | MB_OK);
            return;
        }

        /* update the content of the global string seq_fullpath */
        strcpy(seq_fullpath,strfullpath);

        /* disable view-report menu and toolbar button if they are enabled */
        DisableReport( hwnd, iMsg, wParam, lParam);

        /* Make the directory the current directory */
        MakeDirCurrent(hwnd,seq_fullpath);

        /*  Output to sequence edit window */
        EditWnd =   GetDlgItem(hwnd, IDC_SEQ_EDIT);
        if( strlen(seq_buffer) > MAXEDITLENGTH)
        {
            /* limit text length to value defined in trf.h */
            seq_buffer[MAXEDITLENGTH]='\0'; 
        }
        SetWindowText(EditWnd, seq_buffer);
        free(seq_buffer);   /* free the unused memory */

        /* Set the sequence path on IDC_SEQ_STAT */
        EditWnd =   GetDlgItem(hwnd, IDC_SEQ_STAT);
        SetWindowText(EditWnd, seq_fullpath);


        /* Enable and set the Prefix edit window */
        GetFileTitle(seq_fullpath, strfiletitle,_MAX_PATH);
        EditWnd =   GetDlgItem(hwnd, IDC_PREFIX_EDIT);
        SetWindowText(EditWnd, strfiletitle);
        EnableWindow(EditWnd, TRUE);

        /* Set the output directory edit window text*/
        EditWnd = GetDlgItem(hwnd, IDC_DIR_EDIT);
        if(strcmp(outputpath,"")) SetWindowText(EditWnd, outputpath);
        else
        {
            SetWindowText(EditWnd,_getcwd(strdirbuffer,_MAX_PATH));
        }

        /* Enable the toobar button */
        EditWnd =   GetDlgItem(hwnd, IDC_TOOLBAR);
        SendMessage(EditWnd, TB_SETSTATE, 
                    (WPARAM) IDM_RUN,(LPARAM) MAKELONG(TBSTATE_ENABLED, 0));

        /* Enable menu item for "Start Search" */
        hMenu   =   GetMenu(hwnd);
        EnableMenuItem(hMenu, IDM_RUN, MF_ENABLED);

        /* Add sequence path to history of selections on File Menu */
        AddSeqToMenu( hwnd, iMsg, wParam, lParam);

    }    
    
    return;
}

void OnSequenceSelect(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND    EditWnd;
    HMENU   hmenu;
    char    strbuffer[_MAX_PATH];
    char    strdirbuffer[_MAX_PATH];
    char    strfiletitle[_MAX_PATH];

    /* Get the menu string that has been selected */
    hmenu = GetMenu(hwnd);
    GetMenuString(hmenu,(UINT) LOWORD(wParam),strbuffer,_MAX_PATH,MF_BYCOMMAND);

    /* Read file if valid format */
    if(!TRFReadFile(hwnd, strbuffer)) /* this code is identical to OnOpen */
    {
        MessageBox(hwnd,"File Could Not Be Opened !","Tandem Repeat Finder",
                   MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    /* update the content of the global string seq_fullpath */
    /* only after making sure that selected file is available and well */ 
    strcpy(seq_fullpath,strbuffer);

    /* disable view-report menu and toolbar button if they are enabled */
    DisableReport( hwnd, iMsg, wParam, lParam);

    /* Make the directory the current directory */
    MakeDirCurrent(hwnd,seq_fullpath);

    /*  Output to sequence edit window */
    EditWnd =   GetDlgItem(hwnd, IDC_SEQ_EDIT);

    if(strlen(seq_buffer) > MAXEDITLENGTH) 
    {
        /* limit text length to value defined in trf.h */
        seq_buffer[MAXEDITLENGTH]='\0'; 
    }

    SetWindowText(EditWnd, seq_buffer);
    free(seq_buffer);   /* free the unused memory */

    /* Set the sequence path on IDC_SEQ_STAT */
    EditWnd =   GetDlgItem(hwnd, IDC_SEQ_STAT);
    SetWindowText(EditWnd, seq_fullpath);

    /* Enable and set the Prefix edit window text*/
    GetFileTitle(seq_fullpath, strfiletitle,_MAX_PATH);
    EditWnd =   GetDlgItem(hwnd, IDC_PREFIX_EDIT);
    SetWindowText(EditWnd, strfiletitle);
    EnableWindow(EditWnd, TRUE);

    /* Set the output directory edit window text*/
    EditWnd = GetDlgItem(hwnd, IDC_DIR_EDIT);
    if(strcmp(outputpath,"")) SetWindowText(EditWnd, outputpath);
    else
    {
        SetWindowText(EditWnd,_getcwd(strdirbuffer,_MAX_PATH));
    }


    /* Enable the toobar button for RUN command*/
    EditWnd =   GetDlgItem(hwnd, IDC_TOOLBAR);
    SendMessage(EditWnd, TB_SETSTATE, 
                (WPARAM) IDM_RUN,(LPARAM) MAKELONG(TBSTATE_ENABLED, 0));

    /* Enable menu item for "Start Search" */
    EnableMenuItem(hmenu, IDM_RUN, MF_ENABLED);

    /* Update the location of the sequence file in the file menu history*/
    AddSeqToMenu( hwnd, iMsg, wParam, lParam);

    return;
}

#endif
