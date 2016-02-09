# UEmul

#### **U**ser's input **Emul**ator is a CLI generator of mouse and keyboard events based on its own simple macro-command list.

## Usage:

#### Main usage:

```
   UEMUL [/w WinName] | [/c WinClass] /m InputMacro.dat [/l StringLine] [/v]
```

 where

*   WinName        - title of target window to send input events;
*   WinClass       - class of target window to send input events
                    (you can provide both or only one of them);
*   InputMacro.dat - name of file whith user's input macro commands;
*   StringLine     - text data to send as default keyboard input;
*   /v             - verbose mode.

#### Help usage:

```
   UEMUL /h\n\
```
  
 print information about InputMacro.dat file format.

#### Examples:

```
   UEMUL /w \"Login Page - Google Chrome\" /m AutoLogin.dat
```

 find Google Chrome brauser window with opened Login Page tab
 and apply AoutoLogin.dat macro file to it.

```
   UEMUL /c Notepad /m notepad.dat /l "test"
```

 find window with class Notepad and apply notepad.dat macro
 file with default string line "test".

