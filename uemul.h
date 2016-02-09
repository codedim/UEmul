/*
**  UEMUL.EXE (User-input Emulator) is a CLI tool that allows 
**  user to send mouse and keyboard events to target window.
**
**  powered by CodeDim 07/2015, modified 10/2015, 12/2015
*/

#define WINVER 0x0500

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* program CLI uses information */
#define USES "is a command line User-input Emulator tool.\n\
\nMain uses:\n\n\
   UEMUL [/w WinName] | [/c WinClass] /m InputMacro.dat [/l StringLine] [/v]\n\
\n where\n\
   WinName        - title of target window to send input events;\n\
   WinClass       - class of target window to send input events;\n\
                    (you can provide both or only one of them);\n\
   InputMacro.dat - name of file whith user's input macro commands;\n\
   StringLine     - text data to send as default keyboard input;\n\
   /v             - verbose mode.\n\
\nHelp uses:\n\n\
   UEMUL /h\n\
\n print help information about InputMacro.dat file format.\n\
\nExamples:\n\n\
   UEMUL /w \"Login Page - Google Chrome\" /m AutoLogin.dat\n\
\n find Google Chrome brauser window with opened Login Page tab\n\
 and apply AoutoLogin.dat macro file to it.\n\n\
   UEMUL /c Notepad /m notepad.dat /l \"test\"\n\
\n find window with class Notepad and apply notepad.dat macro\n\
 file with default string line \"test\".\n"

/* program CLI help information */
#define HELP "\nAllowed Commands in InputMacro.dat file:\n\n\
   DWH: WWW HHH\n\
     Desctop Width and Height definitions for macro execution\n\
     control. If DWH is specified and actual desktop resolution\n\
     has different values the macro will not be executed.\n\
   SCW: ChildWinName | (ChildWinClass)\n\
     Select Child Window to send input events.\n\
   SMW:\n\
     Select Main Window to send input events.\n\
   SLP: NNN\n\
     SLeeP NNN milliseconds.\n\
   SWM:\n\
     Set Window Maximized. It's usefull, for example, if you are\n\
     operating with mouse events for best positioning of mouse\n\
     cursor (when you use the MSP command).\n\
   SWF:\n\
     Set Window to Foreground brings the target window to top.\n\
   SWB:\n\
     Set Window to Background makes the target window invizible.\n\
   MSP: xPos yPos\n\
     Mouse Set Position to xPos and yPos desktop point.\n\
   MLC:\n\
     Mouse Left button Click.\n\
   MRC:\n\
     Mouse Right button Click.\n\
   MMC:\n\
     Mouse Midle button Click.\n\
   MWU:\n\
     Mouse Wheel Up (target window must be selected exactly).\n\
   MWD:\n\
     Mouse Wheel Down (target window must be selected exactly).\n\
   KSC: [MOD] KEY\n\
     Keyboard, Send Command.\n\
     Where MOD is a modifier of KEY value from list: CTRL, SHIFT,\n\
     ALT. You can provide a few modifiers at once separated by '+'\n\
     character (for example, CTRL+SHIFT).\n\
     KEY is an optional parameter, that is not to be used whith\n\
     the single-style modifiers: ENTER, DELETE, BACKSPACE, TAB,\n\
     ESCAPE, F1,.. F12, LEFTARROW, RIGHTARROW, UPARROW, DOWNARROW,\n\
     etc.\n\
   KSL: StringLine\n\
     Keyboard, Send Line.\n\
     The command sends chars from StringLine as keyboard input.\n\
     If the StringLine parameter is not given it will use '/L'\n\
     CLI argument as default StringLine. Note: You cannot input\n\
     the '\"', '<' and '>' chars this way, because these are the\n\
     special symbols in the Windows Command Line Interface.\n\
     You can easy use 'shifted' chars in the StringLine parameter,\n\
     the KSL command will send them with SHIFT modifier.\n\
     But you cannot use any symbols of international ABCs.\n"
                  

/* Macro File Commands - just for information here   */
#define  DWH_  "DWH:"	/* desctop width and height  */
#define  SCW_  "SCW:"	/* select child window       */
#define  SMW_  "SMW:"	/* select main window        */
#define  SLP_  "SLP:"	/* sleep                     */
#define  SWM_  "SWM:"	/* set window maximized      */
#define  SWF_  "SWF:"	/* set window to foreground  */
#define  SWB_  "SWB:"	/* set window to background  */
#define  MSP_  "MSP:"	/* mouse, set position       */
#define  MLC_  "MLC:"	/* mouse, left button click  */
#define  MRC_  "MRC:"	/* mouse, right button click */
#define  MMC_  "MMC:"	/* mouse, midle button click */
#define  MWU_  "MWU:"	/* mouse, wheel up           */
#define  MWD_  "MWD:"	/* mouse, wheel down         */
#define  KSC_  "KSC:"	/* keyboard, send command    */
#define  KSL_  "KSL:"	/* keyboard, send line       */

/* if the function success the returned value is not zero */
WORD _GetModifierByName_(char * ModName)
{
  /* common modifiers */
  if (!strcmp(ModName, "SHIFT"))           return VK_SHIFT;
  if (!strcmp(ModName, "CTRL"))            return VK_CONTROL;
  if (!strcmp(ModName, "CONTROL"))         return VK_CONTROL;
  if (!strcmp(ModName, "ALT"))             return VK_MENU;
  /* single-style modifiers */
  if (!strcmp(ModName, "BKSP"))            return VK_BACK;
  if (!strcmp(ModName, "BS"))              return VK_BACK;
  if (!strcmp(ModName, "BACKSPACE"))       return VK_BACK;
  if (!strcmp(ModName, "BREAK"))           return VK_CANCEL;
  if (!strcmp(ModName, "CAPSLK"))          return VK_CAPITAL;
  if (!strcmp(ModName, "CAPSLOCK"))        return VK_CAPITAL;
  if (!strcmp(ModName, "CLEAR"))           return VK_CLEAR;
  if (!strcmp(ModName, "DEL"))             return VK_DELETE;
  if (!strcmp(ModName, "DELETE"))          return VK_DELETE;
  if (!strcmp(ModName, "DOWN"))            return VK_DOWN;
  if (!strcmp(ModName, "DOWNARROW"))       return VK_DOWN;
  if (!strcmp(ModName, "END"))             return VK_END;
  if (!strcmp(ModName, "ENTER"))           return VK_RETURN;
  if (!strcmp(ModName, "ESC"))             return VK_ESCAPE;
  if (!strcmp(ModName, "ESCAPE"))          return VK_ESCAPE;
  if (!strcmp(ModName, "F1"))              return VK_F1;
  if (!strcmp(ModName, "F2"))              return VK_F2;
  if (!strcmp(ModName, "F3"))              return VK_F3;
  if (!strcmp(ModName, "F4"))              return VK_F4;
  if (!strcmp(ModName, "F5"))              return VK_F5;
  if (!strcmp(ModName, "F6"))              return VK_F6;
  if (!strcmp(ModName, "F7"))              return VK_F7;
  if (!strcmp(ModName, "F8"))              return VK_F8;
  if (!strcmp(ModName, "F9"))              return VK_F9;
  if (!strcmp(ModName, "F10"))             return VK_F10;
  if (!strcmp(ModName, "F11"))             return VK_F11;
  if (!strcmp(ModName, "F12"))             return VK_F12;
  if (!strcmp(ModName, "HELP"))            return VK_HELP;
  if (!strcmp(ModName, "HOME"))            return VK_HOME;
  if (!strcmp(ModName, "INS"))             return VK_INSERT;
  if (!strcmp(ModName, "INSERT"))          return VK_INSERT;
  if (!strcmp(ModName, "LEFT"))            return VK_LEFT;
  if (!strcmp(ModName, "LEFTARROW"))       return VK_LEFT;
  if (!strcmp(ModName, "NUMLOCK"))         return VK_NUMLOCK;
  if (!strcmp(ModName, "PGDN"))            return VK_NEXT;
  if (!strcmp(ModName, "PGDOWN"))          return VK_NEXT;
  if (!strcmp(ModName, "PAGEDOWN"))        return VK_NEXT;
  if (!strcmp(ModName, "PGUP"))            return VK_PRIOR;
  if (!strcmp(ModName, "PAGEUP"))          return VK_PRIOR;
  if (!strcmp(ModName, "PRTSC"))           return VK_PRINT;
  if (!strcmp(ModName, "PRTSCRN"))         return VK_PRINT;
  if (!strcmp(ModName, "PRINTSCREEN"))     return VK_PRINT;
  if (!strcmp(ModName, "RIGHT"))           return VK_RIGHT;
  if (!strcmp(ModName, "RIGHTARROW"))      return VK_RIGHT;
  if (!strcmp(ModName, "SCROLLLOCK"))      return VK_SCROLL;
  if (!strcmp(ModName, "TAB"))             return VK_TAB;
  if (!strcmp(ModName, "UP"))              return VK_UP;
  if (!strcmp(ModName, "UPARROW"))         return VK_UP;
	
  return 0;
}

/* if the function success the returned value is not -1 */
SHORT _VkKeyScan_(TCHAR ch) {
  switch (ch) {
	/* common symbols */
	case '`': return 0xC0;
	case '1': return 0x31;
	case '2': return 0x32;
	case '3': return 0x33;
	case '4': return 0x34;
	case '5': return 0x35;
	case '6': return 0x36;
	case '7': return 0x37;
	case '8': return 0x38;
	case '9': return 0x39;
	case '0': return 0x30;
	case '-': return 0xBD;
	case '=': return 0xBB;
	case 'q': return 0x51;
	case 'w': return 0x57;
	case 'e': return 0x45;
	case 'r': return 0x52;
	case 't': return 0x54;
	case 'y': return 0x59;
	case 'u': return 0x55;
	case 'i': return 0x49;
	case 'o': return 0x4F;
	case 'p': return 0x50;
	case '[': return 0xDB;
	case ']': return 0xDD;
	case '\\': return 0xDC;
	case 'a': return 0x41;
	case 's': return 0x53;
	case 'd': return 0x44;
	case 'f': return 0x46;
	case 'g': return 0x47;
	case 'h': return 0x48;
	case 'j': return 0x4A;
	case 'k': return 0x4B;
	case 'l': return 0x4C;
	case ';': return 0xBA;
	case '\'': return 0xDE;
	case 'z': return 0x5A;
	case 'x': return 0x58;
	case 'c': return 0x43;
	case 'v': return 0x56;
	case 'b': return 0x42;
	case 'n': return 0x4E;
	case 'm': return 0x4D;
	case ',': return 0xBC;
	case '.': return 0xBE;
	case '/': return 0xBF;
	/* shifted symbols */
	case '~': return 0x1C0;
	case '!': return 0x131;
	case '@': return 0x132;
	case '#': return 0x133;
	case '$': return 0x134;
	case '%': return 0x135;
	case '^': return 0x136;
	case '&': return 0x137;
	case '*': return 0x138;
	case '(': return 0x139;
	case ')': return 0x130;
	case '_': return 0x1BD;
	case '+': return 0x1BB;
	case 'Q': return 0x151;
	case 'W': return 0x157;
	case 'E': return 0x145;
	case 'R': return 0x152;
	case 'T': return 0x154;
	case 'Y': return 0x159;
	case 'U': return 0x155;
	case 'I': return 0x149;
	case 'O': return 0x14F;
	case 'P': return 0x150;
	case '{': return 0x1DB;
	case '}': return 0x1DD;
	case '|': return 0x1DC;
	case 'A': return 0x141;
	case 'S': return 0x153;
	case 'D': return 0x144;
	case 'F': return 0x146;
	case 'G': return 0x147;
	case 'H': return 0x148;
	case 'J': return 0x14A;
	case 'K': return 0x14B;
	case 'L': return 0x14C;
	case ':': return 0x1BA;
	case '"': return 0x1DE;
	case 'Z': return 0x15A;
	case 'X': return 0x158;
	case 'C': return 0x143;
	case 'V': return 0x156;
	case 'B': return 0x142;
	case 'N': return 0x14E;
	case 'M': return 0x14D;
	case '<': return 0x1BC;
	case '>': return 0x1BE;
	case '?': return 0x1BF;
	/* not determined symbol */
	default: return -1;
  }
}

