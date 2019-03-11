Set UAC = CreateObject("Shell.Application")
Set Shell = CreateObject("WScript.Shell")
If WScript.Arguments.count<1 Then
    WScript.echo "Please use command :  sudo <command> [args]"
ElseIf WScript.Arguments.count=1 Then
    UAC.ShellExecute WScript.arguments(0), "", "", "runas", 1
'    WScript.Sleep 1500
'    Dim ret
'    ret = Shell.Appactivate("用户账户控制")
'    If ret = true Then
'        Shell.sendkeys "%y"        
'    Else
'        WScript.echo "自动获取管理员权限失败，请手动确认。"
'    End If
Else
    Dim ucCount
    Dim args
    args = NULL
    For ucCount=1 To (WScript.Arguments.count-1) Step 1
        args = args & " " & WScript.Arguments(ucCount)
    Next
    UAC.ShellExecute WScript.arguments(0), args, "", "runas", 5
End If
