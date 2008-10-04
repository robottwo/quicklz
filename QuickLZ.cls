VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "QuickLZ"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Private Declare Function qlz_compress Lib "quicklz140_32_1.dll" (ByRef Source As Byte, ByRef Destination As Byte, ByVal Length As Long, ByRef Destination As Byte) As Long
Private Declare Function qlz_decompress Lib "quicklz140_32_1.dll" (ByRef Source As Byte, ByRef Destination As Byte, ByRef Scratch As Byte) As Long
Private Declare Function qlz_size_compressed Lib "quicklz140_32_1.dll" (ByRef Source As Byte) As Long
Private Declare Function qlz_size_decompressed Lib "quicklz140_32_1.dll" (ByRef Source As Byte) As Long
Private Declare Function qlz_get_setting Lib "quicklz140_32_1.dll" (ByVal Length As Long) As Long

Dim scratch_decompress() As Byte
Dim scratch_compress() As Byte
    
Function Compress(Source() As Byte) As Byte()
    Dim dst() As Byte
    Dim r As Long
        
    ReDim dst(0 To UBound(Source) + 400)
    r = qlz_compress(Source(0), dst(0), UBound(Source) + 1, scratch_compress(0))
    ReDim Preserve dst(0 To r - 1)
    Compress = dst
End Function

Public Function Decompress(Source() As Byte) As Byte()
    Dim dst() As Byte
    Dim r As Long
    Dim size As Long
    
    size = SizeDecompressed(Source)
    ReDim dst(0 To size - 1)
    r = qlz_decompress(Source(0), dst(0), scratch_decompress(0))
    ReDim Preserve dst(0 To r - 1)
    Decompress = dst
End Function


Public Function SizeCompressed(Source() As Byte) As Long
    SizeCompressed = qlz_size_compressed(Source(0))
End Function

Public Function SizeDecompressed(Source() As Byte) As Long
    SizeDecompressed = qlz_size_decompressed(Source(0))
End Function

Property Get QLZ_SCRATCH_COMPRESS() As Long
    QLZ_SCRATCH_COMPRESS = qlz_get_setting(1)
End Property

Property Get QLZ_SCRATCH_DECOMPRESS() As Long
    QLZ_SCRATCH_DECOMPRESS = qlz_get_setting(2)
End Property

Property Get QLZ_VERSION_MAJOR() As Long
    QLZ_VERSION_MAJOR = qlz_get_setting(7)
End Property

Property Get QLZ_VERSION_MINOR() As Long
    QLZ_VERSION_MINOR = qlz_get_setting(8)
End Property

Property Get QLZ_VERSION_REVISION() As Long
    QLZ_VERSION_REVISION = qlz_get_setting(9)
End Property

Property Get QLZ_MEMORY_SAFE() As Boolean
    QLZ_MEMORY_SAFE = IIf(qlz_get_setting(6) = 1, True, False)
End Property

Property Get QLZ_COMPRESSION_LEVEL() As Long
    QLZ_COMPRESSION_LEVEL = qlz_get_setting(0)
End Property

Property Get QLZ_STREAMING_BUFFER() As Long
    QLZ_STREAMING_BUFFER = qlz_get_setting(3)
End Property


Private Sub Class_Initialize()
    ReDim scratch_decompress(QLZ_SCRATCH_DECOMPRESS)
    ReDim scratch_compress(QLZ_SCRATCH_COMPRESS)
End Sub



