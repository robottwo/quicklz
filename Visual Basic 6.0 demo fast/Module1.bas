Attribute VB_Name = "Module1"
Private Declare Function qlz_compress Lib "quick32.dll" (ByRef Source As Byte, ByRef Destination As Byte, ByVal Length As Long) As Long
Private Declare Function qlz_decompress Lib "quick32.dll" (ByRef Source As Byte, ByRef Destination As Byte) As Long
Private Declare Function qlz_size_decompressed Lib "quick32.dll" (ByRef Source As Byte) As Long
Private Declare Function qlz_size_source Lib "quick32.dll" (ByRef Source As Byte) As Long

' If the Visual Basic IDE cannot find quick32.dll even though it's in the system32 directory, try adding a path
' to the quicklz.dll file name in the declarations. This should never be neccessary though.

Function Compress(Source() As Byte) As Byte()
    Dim dst() As Byte
    Dim r As Long
    ReDim dst(0 To UBound(Source) * 1.2 + 36000)
    r = qlz_compress(Source(0), dst(0), UBound(Source) + 1)
    ReDim Preserve dst(0 To r - 1)
    Compress = dst
End Function

Public Function GetSize(Source() As Byte) As Long
    GetSize = qlz_size_decompressed(Source(0))
End Function

Public Function Decompress(Source() As Byte) As Byte()
    Dim dst() As Byte
    Dim r As Long
    Dim size As Long
    size = GetSize(Source)
    If size < 20 * 1000000 Then ' Visual Basic can crash if you allocate too long strings
        ReDim dst(0 To size - 1)
        r = qlz_decompress(Source(0), dst(0))
        ReDim Preserve dst(0 To r - 1)
    End If
End Function





