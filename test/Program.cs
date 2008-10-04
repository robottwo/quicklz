// C# demo project for QuickLZ 1.40

using System;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace test
{
    class Program
    {
        static void Main(string[] args)
        {
            QuickLZ qlz = new QuickLZ();
            
            // Show how the .dll was compiled (can be modified through the flags in the beginning of the quicklz.c
            // file). The pre-compiled .dll in the DLL directory is furthermore compiled for 32-bit Win32 and using 
            // __stdcall.
            System.Console.Write("QLZ_COMPRESSION_LEVEL  = " + qlz.QLZ_COMPRESSION_LEVEL + "\n");
            System.Console.Write("QLZ_STREAMING_BUFFER   = " + qlz.QLZ_STREAMING_BUFFER + "\n");
            System.Console.Write("QLZ_MEMORY_SAFE        = " + qlz.QLZ_MEMORY_SAFE + "\n");
            System.Console.Write("QLZ_VERSION_MAJOR      = " + qlz.QLZ_VERSION_MAJOR + "\n");
            System.Console.Write("QLZ_VERSION_MINOR      = " + qlz.QLZ_VERSION_MINOR + "\n");
            System.Console.Write("QLZ_VERSION_REVISION   = " + qlz.QLZ_VERSION_REVISION + "\n\n");

            byte[] original = System.IO.File.ReadAllBytes("d:/bench/proteins.txt");

            // Note that if in streaming mode packets must be decompressed in the same order as they were compressed.
            byte[] c = qlz.Compress(original);
            System.Console.Write("Compressed " + original.Length + " bytes into " + c.Length + " bytes.\n\n");

            byte[] d = qlz.Decompress(c);
            System.Console.Write ("Decompressed back into " + d.Length + " bytes.");

            System.Console.ReadKey(true);
        }
    }
}
