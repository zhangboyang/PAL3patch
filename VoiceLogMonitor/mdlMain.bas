Attribute VB_Name = "mdlMain"
Option Explicit

Public Declare Function LockWindowUpdate Lib "user32" (ByVal hwndLock As Long) As Long

Public Const prefix = ""

Sub Main()
    frmLog.Show
    frmDataLog.Show
    frmLog.tmrRefresh.Interval = 100
    frmDataLog.tmrRefresh.Interval = 100
    
    frmLog.Top = frmDataLog.Top + frmDataLog.Height
    frmDataLog.Height = frmDataLog.Height - frmLog.Height
    ArrangeForm
End Sub

Sub ArrangeForm()
    frmLog.Top = frmDataLog.Top + frmDataLog.Height
    frmLog.Left = frmDataLog.Left
    frmLog.Width = frmDataLog.Width
    If frmDataLog.WindowState = vbNormal Then
        frmLog.Visible = True
    Else
        frmLog.Visible = False
    End If
    
End Sub

Sub UpdateCaption(frm As Form, str As String)
    If frm.Caption <> str Then
        frm.Caption = str
    End If
End Sub

Sub LoadFile(filename As String, txtbox As TextBox, frm As Form)
    Const n = 100
    Dim s(n) As String, t As String
    Dim m As Integer, i As Integer
    
    On Error GoTo err
    ArrangeForm
    
    Open filename For Input As #1
    m = 1
    Do While Not EOF(1)
        Line Input #1, s((m Mod n) + 1)
        m = m + 1
    Loop
    
    t = "......" & vbCrLf
    For i = (m Mod n) + 1 To n
        t = t & vbCrLf & s(i)
    Next i
    For i = 1 To (m Mod n)
        t = t & vbCrLf & s(i)
    Next i
    t = t & vbCrLf
    
    LockWindowUpdate txtbox.hWnd
    If txtbox.Text <> t Then
        txtbox.Text = t
        txtbox.SelStart = Len(txtbox.Text)
    End If
    LockWindowUpdate 0
    
    Close #1
    UpdateCaption frm, str(Date) & " " & str(Time) & " Log Monitor: " & filename
    Exit Sub
err:
    UpdateCaption frm, str(Date) & " " & str(Time) & " Failed to refresh: " & filename
    Close #1
End Sub
