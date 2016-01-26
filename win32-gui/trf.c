/**********************************************************
TRF.C -- TANDEM REPEATS FINDER FOR WINDOWS NT/9x
	   (C) GARY BENSON, 1998
***********************************************************/




#include <windows.h>
#include "TRF.h"
#include "trfdlg.h"
#include "trffile.h"
#include "trfcomm.h"
#include "trfini.h"


LRESULT CALLBACK        WndProc(HWND, UINT, WPARAM, LPARAM);


static char    szAppName[] =   "Tandem Repeats Finder";
HINSTANCE    hInst;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   PSTR szCmdLine, int iCmdShow)
{
	
	HWND            hwnd;
	MSG             msg;
	WNDCLASSEX      wndclass;

	hInst   =   hInstance; //global var. initialization.

	wndclass.cbSize         = sizeof(wndclass);
	wndclass.style          = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc    = WndProc;
	wndclass.cbClsExtra     = 0;
	wndclass.cbWndExtra     = 0;
	wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon(hInstance, "trf");
    wndclass.hCursor        = LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground  = (HBRUSH) /* GetStockObject(WHITE_BRUSH); */ (COLOR_BTNFACE + 1);
    wndclass.lpszMenuName   = "trf";
	wndclass.lpszClassName  = szAppName;
    wndclass.hIconSm        = LoadIcon (hInstance, "trf,1");

	RegisterClassEx (&wndclass) ;

	hwnd = CreateWindow(szAppName,
			"Tandem Repeats Finder",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			hInstance,
			NULL);

	ShowWindow (hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage (&msg,NULL,0,0))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

    char buffer[_MAX_PATH]; /* to ask winhelp to display specified window*/
	

	switch (iMsg)
	{       
		case WM_CREATE:
            OnCreate(hwnd, iMsg, wParam, lParam);
			TRFReadINI(hwnd, iMsg, wParam, lParam); /* defined in trfini.h ,
													needs to be called after
													all childs are created*/

            UpdateParameters(hwnd, iMsg, wParam, lParam);  /* initialize selections */
            GetHelpFilePath(hwnd, iMsg, wParam, lParam); /* defined in trfini.h */


			return 0;

		case WM_SIZE:
			
            OnSize(hwnd, iMsg, wParam, lParam); /* defined in trfcomm.h */

			return 0;
        case WM_TIMER:
            SetProgressBar();
            return 0;

		case WM_COMMAND :
            if(paramset.running==1) return 0; /* prevent reentrance */
			switch(LOWORD(wParam))
			{
			case IDM_OPEN:
				 OnOpen(hwnd, iMsg, wParam, lParam); /* calls funtion
                                                     in trffile.h */
				 return 0;
            case IDM_S1:
            case IDM_S2:
            case IDM_S3:
            case IDM_S4:
                 OnSequenceSelect(hwnd, iMsg, wParam, lParam); /* calls
                                                                 funtion in
                                                                 trffile.h */
                 return 0;

			case IDM_REPORT:
				 OnReport(hwnd, iMsg, wParam, lParam); /* calls funtion
                                                         in trfcomm.h */
				 return 0;

            case IDM_PREVIOUS:
                 OnPrevious(hwnd, iMsg, wParam, lParam); /* defined in
                                                            trfcomm.h */
                 return 0;

            case IDM_DEFAULT:
                 DisableReport(hwnd, iMsg, wParam, lParam); /* in trfcomm.h */
                 OnDefault(hwnd, iMsg, wParam, lParam); /* calls function
                                                            in trfcomm.h */
                 return 0;

            case IDM_SAVE:
				 OnSave(hwnd, iMsg, wParam, lParam); /* calls funtion
                                                         in trfcomm.h */
				 return 0;

            case IDM_P1:
            case IDM_P2:
            case IDM_P3:
            case IDM_P4:
                 DisableReport(hwnd, iMsg, wParam, lParam); /* in trfcomm.h */
                 OnParameterSelect(hwnd, iMsg, wParam, lParam); /* calls
                                                                 funtion in
                                                                 trfcomm.h */
                 return 0;
			case IDM_RUN:
                 OnRun(hwnd, iMsg, wParam, lParam);  /* on trfcomm.h */
				 return 0;


			case IDM_EXIT:
				 SendMessage(hwnd, WM_CLOSE, 0, 0);
				 return 0;

            case IDM_OUTPUTDIR:
                 OnOutputdir(hwnd, iMsg, wParam, lParam); /* in trfcomm.h */
				 return 0;

            case IDM_D1:
            case IDM_D2:
            case IDM_D3:
            case IDM_D4:
            case IDM_OUTTOINPUT:
                 OnDirChoice(hwnd, iMsg, wParam, lParam); /* in trfcomm.h */
                 return 0;

            case IDM_DATA:
            case IDM_MASKED:
            case IDM_FLANKING:
                 OnOptions(hwnd, iMsg, wParam, lParam); /* in trfcomm.h */
                 return 0;

            case IDM_HELP:
                 WinHelp(hwnd,helppath,HELP_FINDER,0);
                 return 0;

            case IDM_QUICKSTART:
                 strcpy(buffer, helppath);
                 strcat(buffer,">topic");
                 WinHelp(hwnd,buffer,HELP_CONTEXT,(UINT)1);
                 return 0;


			case IDM_ABOUT :
				 DialogBox(hInst, "AboutDialog", hwnd, AboutDlgProc);
				 return 0;

            case IDM_CITATION :
                 DialogBox(hInst, "CitationDlg", hwnd, AboutDlgProc);
				 return 0;

            case IDM_FEEDBACK :
                 DialogBox(hInst, "FeedbackDlg", hwnd, AboutDlgProc);
				 return 0;


            case IDC_MPS_COMBO:
            case IDC_AP_COMBO:
            case IDC_MAS_COMBO:
                if(HIWORD(wParam) == CBN_SELCHANGE)
                    DisableReport(hwnd, iMsg, wParam, lParam); /* in trfcomm.h */
                return 0;

            case IDC_PREFIX_EDIT:
                if(HIWORD(wParam) == EN_CHANGE)
                    DisableReport(hwnd, iMsg, wParam, lParam); /* in trfcomm.h */
                return 0;


			}
			break;

        case WM_NOTIFY:
             switch(( (LPNMHDR) lParam)->code)
             {
                case TTN_NEEDTEXT:
                    {
                        LPTOOLTIPTEXT lpttt;
                        lpttt = (LPTOOLTIPTEXT) lParam;

                        switch(lpttt->hdr.idFrom)
                        {
                            case IDM_OPEN:
                                lpttt->lpszText = "Open Sequence";
                                break;

                            case IDM_RUN:
                                lpttt->lpszText = "Start Search";
                                break;

                            case IDM_REPORT:
                                lpttt->lpszText = "View Report";
                                break;

                            case IDM_SAVE:
                                lpttt->lpszText = "Save Current Parameters";
                                break;

                            case IDM_OUTPUTDIR:
                                lpttt->lpszText = "Select Output Directory";
                                break;

                            case IDM_OUTTOINPUT:
                                lpttt->lpszText = "Output to Input Directory";
                                break;

                            case IDM_QUICKSTART:
                                lpttt->lpszText = "Quick Start";
                                break;

                            default:
                                lpttt->lpszText = "Unknown";
                                break;
                        }
                        break;
                    }
                    break;
             }

             break;

        case WM_CLOSE:
             /* if search is running ask for verification */
             if(paramset.running)
             {
                int i;
                i= MessageBox(hwnd,"TRF is currently scanning your sequence.\n"
                              "Are you sure you want to terminate the program?",
                              "TRF", MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2);
                if(i==IDNO) return 0;
             }
             DestroyWindow(hwnd);
             return 0;
	
		case WM_DESTROY:
             WinHelp(hwnd,helppath, HELP_QUIT, 0);
			 TRFWriteINI(hwnd,iMsg,wParam,lParam); /* defined in trfini.h */
			 PostQuitMessage(0);
			 return 0;
	}

	return DefWindowProc(hwnd,iMsg,wParam,lParam);

}
