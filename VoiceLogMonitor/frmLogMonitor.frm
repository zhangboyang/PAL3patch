VERSION 5.00
Begin VB.Form frmLogMonitor 
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "frmLogMonitor"
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrRefresh 
      Interval        =   100
      Left            =   2880
      Top             =   600
   End
   Begin VB.TextBox txtLogMonitor 
      BeginProperty Font 
         Name            =   "Fixedsys"
         Size            =   12
         Charset         =   134
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1575
      Left            =   0
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   0
      Top             =   0
      Width           =   1935
   End
End
Attribute VB_Name = "frmLogMonitor"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private Declare Function SendMessage Lib "user32" Alias "SendMessageA" (ByVal hwnd As Long, ByVal wMsg As Long, ByVal wParam As Long, lParam As Any) As Long
Private Const WM_VSCROLL = &H115
Private Const SB_BOTTOM = 7

Private Const MaxLine As Long = 100
Dim strLogFileName As String

Public Sub SetLogFileName(strFileName As String, colBgColor As Long)
    strLogFileName = strFileName
    txtLogMonitor.BackColor = colBgColor
End Sub

Private Sub Form_Activate()
    OnActivate
End Sub

Private Sub Form_Resize()
    txtLogMonitor.Width = Me.ScaleWidth
    txtLogMonitor.Height = Me.ScaleHeight
End Sub

Private Sub UpdateTitle(strTitle As String)
    If Me.Caption <> strTitle Then Me.Caption = strTitle
End Sub

Private Sub Form_Unload(Cancel As Integer)
    OnUnload
End Sub

Private Sub tmrRefresh_Timer()
    OnTimer
    
    Dim s(MaxLine) As String, t As String
    Dim m As Long, i As Long
    
    On Error GoTo Err
    
    Open strLogFileName For Input As #1
    m = 1
    Do While Not EOF(1)
        Line Input #1, s((m Mod MaxLine) + 1)
        m = m + 1
    Loop
    
    t = "... more lines omited ..." & vbCrLf
    For i = (m Mod MaxLine) + 1 To MaxLine
        t = t & vbCrLf & s(i)
    Next i
    For i = 1 To (m Mod MaxLine)
        t = t & vbCrLf & s(i)
    Next i
    t = t & vbCrLf
    
    If txtLogMonitor.Text <> t Then
        Me.BackColor = txtLogMonitor.BackColor
        txtLogMonitor.Visible = False
        txtLogMonitor.Text = t
        txtLogMonitor.SelStart = Len(txtLogMonitor.Text)
        SendMessage txtLogMonitor.hwnd, WM_VSCROLL, SB_BOTTOM, ByVal 0
        txtLogMonitor.Visible = True
    End If
    
    Close #1
    UpdateTitle Str(Date) & " " & Str(Time) & " | Log Monitor: " & strLogFileName
    Exit Sub
Err:
    UpdateTitle Str(Date) & " " & Str(Time) & " | Refresh failed: " & strLogFileName
    Close #1
End Sub
