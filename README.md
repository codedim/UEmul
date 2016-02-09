# UEmul

###### **U**ser's input **Emul**ator is a CLI generator of mouse and keyboard events based on its own simple macro-command list. All events are translating to a specified window.

## Usage:

#### Main usage:

```
   UEMUL [/w WinName] | [/c WinClass] /m InputMacro.dat [/l StringLine] [/v]
```

 where

*   WinName        - title of target window to send input events;
*   WinClass       - class of target window to send input events;

> you can provide both or only one of them

*   InputMacro.dat - name of file whith user's input macro commands;
*   StringLine     - text data to send as default keyboard input;
*   /v             - verbose mode.


#### Help usage:

```
   UEMUL /h
```
  
 print information about InputMacro.dat file format.


#### Examples:

```
   UEMUL /w "Login Page - Google Chrome" /m AutoLogin.dat
```

 find Google Chrome brauser window with opened Login Page tab
 and apply AoutoLogin.dat macro file to it.

```
   UEMUL /c Notepad /m notepad.dat /l "test"
```

 find window with class Notepad and apply notepad.dat macro
 file with default string line "test".



## Macro-command list:

Allowed Commands in **macrofile.dat** file:


```
   DWH: WWW HHH
```
 
>     **D**esctop **W**idth and **H**eight definitions for macro execution control. If DWH is specified and actual desktop resolution has different values the macro will not be executed.

```
   SCW: ChildWinName | (ChildWinClass)
```

>     **S**elect **C**hild **W**indow to send input events.

```
   SMW:
```

>     **S**elect **M**ain **W**indow to send input events.

```
   SLP: NNN
```

>     **SL**ee**P** NNN milliseconds.

```
   SWM:
```

>     **S**et **W**indow **M**aximized. It's usefull, for example, if you are operating with mouse events for best positioning of mouse cursor (when you use the MSP command).

```
   SWF:
```

>     **S**et **W**indow to **F**oreground brings the target window to top.

```
   SWB:
```

>     **S**et **W**indow to **B**ackground makes the target window invizible.

```
   MSP: xPos yPos
```

>     **M**ouse **S**et **P**osition to xPos and yPos desktop point.

```
   MLC:
```

>     **M**ouse **L**eft button **C**lick.

```
   MRC:
```

>     **M**ouse **R**ight button **C**lick.

```
   MMC:
```

>     **M**ouse **M**idle button **C**lick.

```
   MWU:
```

>     **M**ouse **W**heel **U**p (target window must be selected exactly).

```
   MWD:
```

>     **M**ouse **W**heel **D**own (target window must be selected exactly).


```
   KSC: [MOD] KEY
```

>     **K**eyboard, **S**end **C**ommand. Where MOD is a modifier of KEY value from list: CTRL, SHIFT, ALT. You can provide a few modifiers at once separated by '+' character (for example, CTRL+SHIFT). KEY is an optional parameter, that is not to be used whith the single-style modifiers: ENTER, DELETE, BACKSPACE, TAB, ESCAPE, F1,.. F12, LEFTARROW, RIGHTARROW, UPARROW, DOWNARROW, etc.

```
   KSL: StringLine
```

>     **K**eyboard, **S**end **L**ine. The command sends chars from StringLine as keyboard input. If the StringLine parameter is not given it will use '/L' CLI argument as default StringLine. Note: You cannot input the '\"', '<' and '>' chars this way, because these are the special symbols in the Windows Command Line Interface. You can easy use 'shifted' chars in the StringLine parameter, the KSL command will send them with SHIFT modifier. But you cannot use any symbols of international ABCs.


