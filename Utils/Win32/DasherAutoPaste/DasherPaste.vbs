' For the user:
' 1/ Create a shortcut to this script on your startmenu with the shortcut key of your choice.
' 2/ From within your application press your shortcut - any selected text will hopefully appear in Dasher.
' 3/ When you've finished with Dasher, simply close it. The Window last active before Dasher will receive
'    your text.
'
' This script uses the Windows clipboard, so any contents will be lost. If stage 3/ fails, your text will hopefully
' be in the clipboard anyway. Try increasing the number after WScript.Sleep.
'
' Requires Windows Scripting Host 2. This comes with Windows 2000
' If you don't have it, see: http://www.microsoft.com/msdownload/vbscript/scripting.asp

' For the developer:
' This is very hacky. Other approaches:
' - Could send WM_COPY / WM_PASTE messages from Dasher, but doesn't work with Word, IE fields, etc.
' - Could send WM_KEYDOWN, WM_KEYUP events. Very fiddly to get working. This works better.
' - Could use a keyboard journal playback hook to send keys (even more fiddly but still error prone).
' - Could use one of various interfaces to MS Word or application X. Then it would only work with application X.
' - Could make Dasher some sort of proper Windows input device like the Japanese IME. That sounds good.
'   It also sounds like it would take more than six lines of code, so if you have the time, please go ahead...

set ShellObject = WScript.CreateObject("WScript.Shell")
ShellObject.Run "EmptyClipboard.exe", 0, true
ShellObject.SendKeys "^c", true
'ShellObject.Run "dasher.exe --final-clipboard", 1, true
ShellObject.Run "calc", 1, true
WScript.Sleep 500
ShellObject.SendKeys "^v", true
