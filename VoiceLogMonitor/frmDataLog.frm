VERSION 5.00
Begin VB.Form frmDataLog 
   Caption         =   "Form1"
   ClientHeight    =   9765
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   14265
   LinkTopic       =   "Form1"
   ScaleHeight     =   9765
   ScaleWidth      =   14265
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrRefresh 
      Left            =   0
      Top             =   0
   End
   Begin VB.TextBox txtLog 
      BackColor       =   &H00C0FFC0&
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
      Left            =   600
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   0
      Top             =   480
      Width           =   2295
   End
End
Attribute VB_Name = "frmDataLog"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

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
    LoadFile prefix + "VoiceDataLog.txt", txtLog, Me
End Sub
