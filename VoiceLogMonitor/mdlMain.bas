Attribute VB_Name = "mdlMain"
Option Explicit

Dim frmVoiceDataLog As frmLogMonitor, frmVoiceLog As frmLogMonitor

Public Sub Main()
    'create forms
    Set frmVoiceDataLog = New frmLogMonitor
    Set frmVoiceLog = New frmLogMonitor
    
    'set window size
    frmVoiceDataLog.Width = Screen.TwipsPerPixelX * 1000
    frmVoiceDataLog.Height = Screen.TwipsPerPixelY * 700
    frmVoiceLog.Height = Screen.TwipsPerPixelY * 200
    frmVoiceDataLog.Height = frmVoiceDataLog.Height - frmVoiceLog.Height
    ArrangeWindows
    
    'set filenames
    frmVoiceDataLog.SetLogFileName "VoiceDataLog.txt", &HC0FFC0
    frmVoiceLog.SetLogFileName "VoiceLog.txt", &HC0FFFF
    
    'show forms
    frmVoiceLog.Show
    frmVoiceDataLog.Show
End Sub

Public Sub ArrangeWindows()
    frmVoiceLog.WindowState = frmVoiceDataLog.WindowState
    If frmVoiceDataLog.WindowState = vbNormal And frmVoiceLog.WindowState = vbNormal Then
        frmVoiceLog.Top = frmVoiceDataLog.Top + frmVoiceDataLog.Height
        frmVoiceLog.Left = frmVoiceDataLog.Left
        frmVoiceLog.Width = frmVoiceDataLog.Width
    End If
End Sub

Public Sub OnTimer()
    ArrangeWindows
End Sub

Public Sub OnUnload()
    End
End Sub

Public Sub OnActivate()
    frmVoiceDataLog.SetFocus
End Sub
