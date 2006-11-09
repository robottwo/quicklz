VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   1110
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6600
   LinkTopic       =   "Form1"
   ScaleHeight     =   1110
   ScaleWidth      =   6600
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command1 
      Caption         =   "Compress"
      Height          =   375
      Left            =   5280
      TabIndex        =   1
      Top             =   360
      Width           =   1215
   End
   Begin VB.TextBox Text1 
      Height          =   375
      Left            =   120
      TabIndex        =   0
      Top             =   360
      Width           =   5055
   End
   Begin VB.Label Label1 
      Caption         =   "Enter a file to compress:"
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
' PLACE QUICK32.DLL IN THE WINDOWS SYSTEM32 DIRECTORY BEFORE RUNNING THIS SAMPLE APPLICATION


Private Sub Command1_Click()
    Dim src() As Byte
    Dim a() As Byte
    Dim b() As Byte
    
    Open Text1 For Binary As #1
    ReDim src(0 To LOF(1) - 1)
    Get #1, , src
    Close

    a = Compress(src)
    MsgBox "Original length: " & GetSize(a) + 1 & vbCrLf & "Compressed length: " & UBound(a) + 1
    b = Decompress(a)
End Sub

Private Sub Form_Load()

End Sub
