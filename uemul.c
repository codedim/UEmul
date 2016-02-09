/*
**  UEMUL.EXE (User-input Emulator) is a CLI tool that allows 
**  user to send mouse and keyboard events to target window.
**
**  powered by CodeDim 07/2015, modified: 10/2015, 12/2015
*/

#include "uemul.h"

#define PROGNAME  "UEMUL.EXE"
#define PROGVER   "1.2"

/* maximal length of read & write buffers */
#define MAXLEN   100
/* default sleep value */
#define PAUSE    1000

/* function to parse and execute MacroFile commands */
int ExecuteMacroFile(HWND win, char * fn, char * str);
/* to be (1) or not to be (0) verbose to stdout */
int VERBOSE = 0;

/**********************************************************************/

int main(int argc, char * argv[]) {	
	int iArg;
	char * twTitle = NULL;      /* target window Title */
	char * twClass = NULL;      /* target window Class */
	char * uiMacroFile = NULL;  /* user input Macro File name */
	char * uiDefaultStr = NULL; /* user input Default String line */

	/* parsing CLI arguments for help call */
	if (argc == 2 && argv[1][0] == '/' && 
		(argv[1][1] == 'h' || argv[1][1] == 'H')) 
	{
		printf("%s", HELP);
		return 0;
	}	
	/* parsing CLI arguments for main call */
	if (argc < 3) {
		printf("\n%s v%s %s", PROGNAME, PROGVER, USES);
		return 1;
	} 
	for (iArg = 1; iArg < argc; ++iArg) {
		if (strlen(argv[iArg]) != 2 || argv[iArg][0] != '/') {
			printf("\n%s v%s %s", PROGNAME, PROGVER, USES);
			return 1;
		}

		switch (argv[iArg][1]) {
			case 'w':
			case 'W':
				twTitle = argv[iArg+1];
				++iArg;
				break;
			case 'c':
			case 'C':
				twClass = argv[iArg+1];
				++iArg;
				break;
			case 'm':
			case 'M':
				uiMacroFile = argv[iArg+1];
				++iArg;
				break;
			case 'l':
			case 'L':
				uiDefaultStr = argv[iArg+1];
				++iArg;
				break;
			case 'v':
			case 'V':
				VERBOSE = 1;
				break;
			default:
				/* unknown CLI key */
				printf("\n%s v%s %s", PROGNAME, PROGVER, USES);
				return 1;
		}
	}

	/* check for required parameters */
	if (!(twTitle || twClass) || !uiMacroFile) {
		printf("\n%s v%s %s", PROGNAME, PROGVER, USES);
		return 1;
	}


	/* char to oem string transformation is required to be verbose */
	char twOemStr[MAXLEN] = ""; 
	if (VERBOSE) {
		printf("\nExecuting User's Input Macro File \"%s\"\n", uiMacroFile);
		if (twTitle && !twClass) {
			CharToOem(twTitle, twOemStr);
			printf(" for Target Window Name \"%s\"\n", twOemStr);
		} else if (twClass && !twTitle) { 
			printf(" for Target Window Class (%s)\n", twClass);
		} else {
			CharToOem(twTitle, twOemStr);
			printf(" for Target Window Name \"%s\" and Class (%s)\n", 
				twOemStr, twClass);
		}
		if (uiDefaultStr) { 
			printf(" with Default String Line \"%s\"\n", uiDefaultStr);
		}
	}

	/* so lets try to find the target window */
	HWND tw = 0;
	if (twTitle && twClass) 
		tw = FindWindow(twClass, twTitle); 
	else if (twTitle) 
		tw = FindWindow(NULL, twTitle);  
	else 
		tw = FindWindow(twClass, NULL);
	/* check it out */
	if (!tw) {
		if (twTitle) {
			printf("\nError: Cannot find the Target Window \"%s\"\n", 
				twOemStr);
		} else {
			printf("\nError: Cannot find the Target Window (%s)\n", 
				twClass);
		}
		return 1;
	}
	if (VERBOSE) printf("\n Target Window Handle: %d\n\n", tw); 

	/* bring the window to top */
	Sleep(PAUSE);
	SetForegroundWindow(tw);
	Sleep(PAUSE);
	ShowWindow(tw, SW_SHOW);

	/* start emulating User's Input from Macro File */
	return ExecuteMacroFile(tw, uiMacroFile, uiDefaultStr);
}


/**********************************************************************/

/* if there is next parameter the returned value is not NULL */
char * GetNextParam(char * start, char * param) {
	char * endBySpace;
	char * endByTab;
	char * end;
	param[0] = 0x00;

	if (strlen(start)) {
		strcpy(param, start);

		endBySpace = strchr(param, 0x20); 
		endByTab = strchr(param, 0x09);
		if(endBySpace && endByTab) {
			end = (endBySpace < endByTab) ? endBySpace : endByTab;
		} else if (endBySpace) {
			end = endBySpace;
		} else if (endByTab) {
			end = endByTab;
		} else {
			end = NULL;
		}

		if (end) {
			/* determinate param string with \0 symbol */
			*end = 0x00;                    
			/* determine new start position */
			start = start + (end - param);
			/* cut out prefixed whitespaces */
			while (start[0] == 0x20 || start[0] == 0x09) start++;
			/* there is next parameter */
			return start;  
		}
	}

	return NULL;
}

/* the function determines only two parameters */
void GetCommandParams(char * buf, char * fparam, char * sparam) {
	char * start = buf;

	/* cut out command itself, for example "DWH: " substring */
	if (buf[3] == ':' && (buf[4] == 0x20 || buf[4] == 0x09))
		start = &buf[5];
	/* cut out prefixed whitespaces */
	while (start[0] == 0x20 || start[0] == 0x09) ++start;

	/* reset parameter strings */
    fparam[0] = 0x00;  sparam[0] = 0x00; 
	/* determine the first parameter */
	if (start = GetNextParam(start, fparam)) {
		/* determine the second parameter */
		GetNextParam(start, sparam);
	}
}


/**********************************************************************/

/* if the function success the returned value is not zero */
int GetKscModifiers(char * param, WORD mod[], int count) {
	int i;
	char * start = param;
	char buf[MAXLEN] = "";

	for (i = 0; i < count; ++i) mod[i] = 0;
	
	/* checking out for some errors */
	if (start[0] == '[') 
		start++;
	else 
		return 0;
	if (start[strlen(start) - 1] == ']')
		start[strlen(start) - 1] = 0x0;
	else
		return 0;
	if (strchr(start, 0x20) || strchr(start, 0x09)) 
		return 0;

	/* lets getting the modifier by name */
	int res = 0;
	while ( res < count && strlen(start) ) {
		strcpy(buf, start);
		if (start = strchr(buf, '+')) {
			start[0] = 0x00;
			start++;
		} else {
			start = &buf[strlen(buf)];
		}

		if ( !(mod[res] = _GetModifierByName_(buf)) ) 
			return 0;

		res++;
	}

	return res;
}

/* if the function success the returned value is not zero */
int isShiftedChar(char ch) {
	int i;
	char shifted[] = "~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?";
	for (i = 0; i < strlen(shifted); ++i) {
		if (ch == shifted[i]) return 1;
	}
	return 0;
}

/* send keyboard input function */
void SendKbInput(BYTE bVk, DWORD dwFlags) {
	keybd_event(bVk, 0, dwFlags, 0);
	Sleep(30); /* just for case */
}

/* VkKeyScan function doesn't work in OS WinCE (Embedded, etc.) */ 
SHORT VkKeyScanCE(TCHAR ch) {
	/* get VirtualKey Code from predefined table first */
	SHORT res = _VkKeyScan_(ch);
	/* if not found -- get it from the system function */
	if (res == -1) res = VkKeyScan(ch);

	return res;
}


/**********************************************************************/

int ExecuteMacroFile(HWND win, char * fn, char * defStr) {
   	int i;
	FILE * file; 
	char buf[MAXLEN] = "";
	int ln = 0;
	char fparam[MAXLEN]; /* first command parameter */
	char sparam[MAXLEN]; /* second command parameter */

	RECT rect;
	POINT point;
	WPARAM wParam;
	LPARAM lParam;

	HWND tw = win;

	if ( !(file = fopen(fn,"r")) ) {
		printf("\nError: Cannot open file \"%s\" (ErrCode: %d)\r\n", 
			fn, GetLastError());
		return 1;
	}

	while (fgets(buf, sizeof(buf), file)) {
		++ln;
		/* cut out CR-LF symbols in the end of line */
		while (buf[strlen(buf)-1] == 0x0D || 
			buf[strlen(buf)-1] == 0x0A) 
		{
			buf[strlen(buf)-1] = 0x00;
		}

		if (strlen(buf)) {
		    /* outputs comments on stdout */
			if (buf[0] == '#') {
				printf("%s\n", buf);
				continue;
			}

		/**************    parse and process commands    **************/

			/* desctop width and height */
			if (strstr(buf, "DWH:") == buf) {
				GetCommandParams(buf, fparam, sparam);
				if ( !strlen(fparam) || !strlen(sparam) ||
					!atoi(fparam) || !atoi(sparam) )
				{
					printf("\nErr!:%s:%d: Invalid parameter in DWH\
 command.\n", fn, ln);
					fclose(file);
					return 1;
				}
				
				GetWindowRect(GetDesktopWindow(), &rect);
				if ( (atoi(fparam) != rect.right - rect.left) || 
					(atoi(sparam) != rect.bottom - rect.top) )
				{
					printf("\nErr!:%s:%d: Unsupported Desktop\
 Resolution.\n", fn, ln);
					fclose(file);
					return 1;
				}
				if (VERBOSE) printf("OK\n");
			} else 

			/* select child window */
			if (strstr(buf, "SCW:") == buf) {
				char * start = &buf[5];
				while (start[0] == 0x20 || start[0] == 0x09) start++;
				if ( !strlen(start) ) {
					printf("\nErr!:%s:%d: Invalid parameter in SCW\
 command.\n", fn, ln);
					fclose(file);
					return 1;
				}
				
				if ( (start[0] == '(') && 
					(start[strlen(start) - 1] == ')') ) 
				{
					start++;
					start[strlen(start) - 1] = 0x00;
					if ( !(tw = FindWindowEx(tw, 0, start, NULL)) ) {
						printf("\nErr!:%s:%d: Cannot find the specified\
 Child Window with Class (%s).\n", fn, ln, start);
						fclose(file);
						return 1;
					}
				} else {
					if ( (start[0] == '"') && 
						(start[strlen(start) - 1] == '"') ) 
					{
						start++;
						start[strlen(start) - 1] = 0x00;
					}
					if ( !(tw = FindWindowEx(tw, 0, NULL, start)) ) {
						printf("\nErr!:%s:%d: Cannot find the specified\
 Child Window \"%s\".\n", fn, ln, start);
						fclose(file);
						return 1;
					}
				}

				if (VERBOSE) printf("OK\n");
			} else

			/* select main window */
			if (strstr(buf, "SMW:") == buf) {
				tw = win;
				if (VERBOSE) printf("OK\n");
			} else   

            /* sleep */
			if (strstr(buf, "SLP:") == buf) {
				GetCommandParams(buf, fparam, sparam);
				if ( !strlen(fparam) ||	!atoi(fparam) )
				{
					printf("\nErr!:%s:%d: Invalid parameter in SLP\
 command.\n", fn, ln);
					fclose(file);
					return 1;
				}

				Sleep(atoi(fparam));			
				if (VERBOSE) printf("OK\n");
			} else 

			/* TODO: set window maximized */
			if (strstr(buf, "SWM:") == buf) {
				ShowWindow(tw, SW_SHOWMAXIMIZED);
				Sleep(PAUSE);
				if (VERBOSE) printf("OK\n");
			} else

			/* set window to foreground */
			if (strstr(buf, "SWF:") == buf) {
				ShowWindow(tw, SW_SHOW);
				BringWindowToTop(tw);
				SetForegroundWindow(tw);
				SetActiveWindow(tw);
				Sleep(PAUSE);
				if (VERBOSE) printf("OK\n");
			} else

			/* set window to background */
			if (strstr(buf, "SWB:") == buf) {
				ShowWindow(tw, SW_HIDE);
				Sleep(PAUSE);
				if (VERBOSE) printf("OK\n");
			} else

			/* mouse, set position */
			if (strstr(buf, "MSP:") == buf) {
				GetCommandParams(buf, fparam, sparam);
				if ( !strlen(fparam) || !strlen(sparam) ||
					!atoi(fparam) || !atoi(sparam) )
				{
					printf("\nErr!:%s:%d: Invalid parameter in MSP\
 command.\n", fn, ln);
					fclose(file);
					return 1;
				}
				
				point.x = atoi(fparam);
				point.y = atoi(sparam);
				GetWindowRect(GetDesktopWindow(), &rect);
				if ( (point.y < 0 || point.y > rect.right - rect.left) || 
					(point.x < 0 || point.x > rect.bottom - rect.top) )
				{
					printf("\nErr!:%s:%d: Parameter is out of bounds\
 in MSP command.\n", fn, ln);
					fclose(file);
					return 1;
				}

				/* correct point in relation to target window position */
				GetWindowRect(tw, &rect);
                point.x += rect.left;
				point.y += rect.top;
				SetCursorPos(point.x, point.y);
				if (VERBOSE) printf("OK\n");				
			} else 

            /* mouse, left button click */
			if (strstr(buf, "MLC:") == buf) {
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				Sleep(30);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(30);
				if (VERBOSE) printf("OK\n");				
			} else 

			/* mouse, right button click */
			if (strstr(buf, "MRC:") == buf) {
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
				Sleep(30);
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				Sleep(30);
				if (VERBOSE) printf("OK\n");			
			} else 

			/* mouse, midle button click */
			if (strstr(buf, "MMC:") == buf) {
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
				Sleep(30);
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
				Sleep(30);
				if (VERBOSE) printf("OK\n");
			} else 

			/* mouse, wheel up */
			if (strstr(buf, "MWU:") == buf) {
				wParam = (WHEEL_DELTA) << 16;
				GetCursorPos(&point);
				lParam = point.y << 16;
				lParam += point.x;				
				SendMessage(tw, WM_MOUSEWHEEL, wParam, lParam);
				if (VERBOSE) printf("OK\n");				
			} else 

			/* mouse, wheel down */
			if (strstr(buf, "MWD:") == buf) {
				wParam = (-WHEEL_DELTA) << 16;
				GetCursorPos(&point);
				lParam = point.y << 16;
				lParam += point.x;				
				SendMessage(tw, WM_MOUSEWHEEL, wParam, lParam);
				if (VERBOSE) printf("OK\n");
			} else 

			/* keyboard, send command */
			if (strstr(buf, "KSC:") == buf) {
				/* max posible count of key modifiers */
				const int MAX_KEYMOD_COUNT = 3; 
				WORD key; 
				WORD keymod[MAX_KEYMOD_COUNT];    			
				WORD shift_state;

				GetCommandParams(buf, fparam, sparam);
				if ( !strlen(fparam) || strlen(sparam) > 1 || 
					fparam[0] != '[' || fparam[strlen(fparam) - 1] != ']' || 
					!GetKscModifiers(fparam, keymod, MAX_KEYMOD_COUNT) ) 
				{
					printf("\nErr!:%s:%d: Invalid parameter in KSC\
 command.\n", fn, ln);
					fclose(file);
					return 1;
				}
				
				key = (strlen(sparam)) ? VkKeyScanCE(sparam[0]) : 0;


				/* TODO: test multiple modifiers a bit more */
				/* 
				** there is an undocumented featrure in windows if you need 
				** to send a key combination with SHIFT prefix (such as 
				** SHIFT+INS, SHIFT+TAB, SHIFT+END, SHIFT+UP, etc.) the next 
				** key must be EXTENDED 
				*/
				shift_state = 0;
				for(i = 0; i < MAX_KEYMOD_COUNT; ++i) {
					if (keymod[i]) {
						SendKbInput(keymod[i], shift_state | 0);
						if (keymod[i] == VK_SHIFT) shift_state = KEYEVENTF_EXTENDEDKEY;
					}
				}
				if (key) {
					SendKbInput(key, shift_state | 0);
					SendKbInput(key, shift_state | KEYEVENTF_KEYUP);
				}
				for(i = MAX_KEYMOD_COUNT - 1; i >= 0; --i) {
					if (keymod[i]) {
						if (keymod[i] == VK_SHIFT) shift_state = 0;
						SendKbInput(keymod[i], shift_state | KEYEVENTF_KEYUP);
					}
				}
			} else 

			/* keyboard, send line */
			if (strstr(buf, "KSL:") == buf) {
				char oemStr[MAXLEN] = "";
				char * next = &buf[4];

				while (next[0] == 0x20 || next[0] == 0x09) next++;
				if (strlen(next) == 0) next = defStr;
				CharToOem(next, oemStr);
				if (!strstr(next, oemStr)) {
					printf("\nErr!:%s:%d: OEM characters are not allowed\
 in KSL command parameter.\n", fn, ln);
					fclose(file);
					return 1;
				}

				while (strlen(next)) {
					if (isShiftedChar(next[0])) {
						SendKbInput(VK_SHIFT, 0);
					}
					SendKbInput(VkKeyScanCE(next[0]), 0);
					SendKbInput(VkKeyScanCE(next[0]), KEYEVENTF_KEYUP);
					if (isShiftedChar(next[0])) {
						SendKbInput(VK_SHIFT, KEYEVENTF_KEYUP);
					}
					next++;
				}
				if (VERBOSE) printf("OK\n");
			} 

			/* if the line is not a command or comment */
			else {
				printf("\nErr!:%s:%d: Unexpected syntax in Macro\
 File.\n", fn, ln);
				fclose(file);
				return 1;
			}
				
		}
	}

	fclose(file);
	return 0;
}


