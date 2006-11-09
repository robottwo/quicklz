Attribute VB_Name = "Module1"
Private Declare Function qlz_compress Lib "quick32.dll" (ByVal Source As String, ByVal Destination As String, ByVal Length As Long) As Long
Private Declare Function qlz_size_decompressed Lib "quick32.dll" (ByVal Source As String) As Long
Private Declare Function qlz_decompress Lib "quick32.dll" (ByVal Source As String, ByVal Destination As String) As Long
Private Declare Function qlz_size_source Lib "quick32.dll" (ByVal Source As String) As Long

' If the Visual Basic IDE cannot find quick32.dll even though it's in the system32 directory, try adding a path
' to the quicklz.dll file name in the declarations. This should never be neccessary though.

Public Function GetSize(Source As String) As Long
    GetSize = qlz_size_decompressed(Source)
End Function

Public Function Compress(Source As String) As String
    Dim dst As String
    Dim r As Long
    dst = Space(Len(Source) * 1.2 + 36000)
    r = qlz_compress(Source, dst, Len(Source))
    Compress = Left(dst, r)
End Function

Public Function Decompress(Source As String) As String
    Dim dst As String
    Dim r As Long
    Dim size As Long
    
    size = GetSize(Source)
    If size < 20000000 Then ' Visual Basic can crash if you allocate too long strings
        dst = Space(size)
        r = qlz_decompress(Source, dst)
        Decompress = Left(dst, r)
    End If
End Function


