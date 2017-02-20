VERSION 5.00
Begin VB.Form frmLog 
   BorderStyle     =   3  'Fixed Dialog
   ClientHeight    =   2085
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4350
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2085
   ScaleWidth      =   4350
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrRefresh 
      Left            =   1200
      Top             =   2280
   End
   Begin VB.TextBox txtLog 
      BackColor       =   &H00C0FFFF&
      BeginProperty Font 
         Name            =   "Fixedsys"
         Size            =   12
         Charset         =   134
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1095
      Left            =   240
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   0
      Top             =   360
      Width           =   2295
   End
End
Attribute VB_Name = "frmLog"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Form_Activate()
    frmDataLog.SetFocus
End Sub

Private Sub Form_Resize()
    On Error Resume Next
    txtLog.Top = 0
    txtLog.Left = 0
    txtLog.Width = Me.Width - 100
    txtLog.Height = Me.Height - 400
End Sub

Private Sub Form_Unload(Cancel As Integer)
    End
End Sub

Private Sub tmrRefresh_Timer()
    LoadFile prefix + "VoiceLog.txt", txtLog, Me
End Sub
