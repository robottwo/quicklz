VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   1110
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6495
   LinkTopic       =   "Form1"
   ScaleHeight     =   1110
   ScaleWidth      =   6495
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox Text1 
      Height          =   375
      Left            =   120
      TabIndex        =   1
      Top             =   360
      Width           =   4935
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Compress"
      Height          =   375
      Left            =   5160
      TabIndex        =   0
      Top             =   360
      Width           =   1215
   End
   Begin VB.Label Label1 
      Caption         =   "Enter a file to compress (max 20 Mbyte):"
      Height          =   255
      Left            =   120
      TabIndex        =   2
      Top             =   120
      Width           =   3855
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
' For Visual Basic 6.0
'
' Example of using the QuickLZ compression library
' Copyright Lasse Mikkel Reinhold, 2006
'
' This demo is slow (around 1/3 of the full speed of QuickLZ - still beats many compressor libraries out there,
' though) because it operates on string variables. String variables are stored as unicode in Visual Basic, so
' Visual Basic must convert between Unicode and ASCII at each call to the DLL. This is also why arguments are
' passed ByVal.
'
' For better performance (around 4/5 of the full speed), use the other Visual Basic demo which operates on
' Byte() arrays.
'
' PLACE QUICK32.DLL IN THE WINDOWS SYSTEM32 DIRECTORY BEFORE RUNNING THIS SAMPLE APPLICATION

Private Sub Command1_Click()
    Dim src As String
    Dim a As String
    Dim b As String
    
    Open Text1 For Binary As #1
    src = Space(LOF(1)) ' Get and Put are multiple times faster than Input and Print
    Get #1, , src
    Close
    
    a = Compress(src)
    MsgBox "Original length: " & Len(src) & vbCrLf & "Compressed length: " & Len(a)
    b = Decompress(a)
    MsgBox b = src
End Sub


