// DLL wrapper class for the QuickLZ 1.4.x DLL files

using System;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace test
{
    class QuickLZ
    {
        // The C library passes many integers through the C type size_t which is 32 or 64 bits on 32 or 64 bit 
        // systems respectively. The C# type IntPtr has the same property but because IntPtr doesn't allow 
        // arithmetic we cast to and from int on each reference. To pass constants use (IntPrt)1234.
        [DllImport("f:/quicklz140_32_1.dll")] public static extern IntPtr qlz_compress(byte[] source, byte[] destination, IntPtr size, byte[] scratch);
        [DllImport("f:/quicklz140_32_1.dll")] public static extern IntPtr qlz_decompress(byte[] source, byte[] destination, byte[] scratch);
        [DllImport("f:/quicklz140_32_1.dll")] public static extern IntPtr qlz_size_compressed(byte[] source);
        [DllImport("f:/quicklz140_32_1.dll")] public static extern IntPtr qlz_size_decompressed(byte[] source);
        [DllImport("f:/quicklz140_32_1.dll")] public static extern int qlz_get_setting(int setting);

        private byte[] scratch_compress;
        private byte[] scratch_decompress;

        public QuickLZ()
        {
            scratch_compress = new byte[qlz_get_setting(1)];
            if (QLZ_STREAMING_BUFFER == 0)
                scratch_decompress = scratch_compress;
            else
                scratch_decompress = new byte[qlz_get_setting(2)];
        }

        public byte[] Compress(byte[] Source)
        {
            byte[] d = new byte[Source.Length + 400];
            uint s;
            
            s = (uint)qlz_compress(Source, d, (IntPtr)Source.Length, scratch_compress);
            byte[] d2 = new byte[s];
            System.Array.Copy(d, d2, s);
            return d2;
        }

        public byte[] Decompress(byte[] Source)
        {
            byte[] d = new byte[(uint)qlz_size_decompressed(Source)];
            uint s;

            s = (uint)qlz_decompress(Source, d, scratch_decompress);
            return d;
        }

        public uint SizeCompressed(byte[] Source)
        {
            return (uint)qlz_size_compressed(Source);
        }

        public uint SizeDecompressed(byte[] Source)
        {
            return (uint)qlz_size_decompressed(Source);
        }

        public uint QLZ_COMPRESSION_LEVEL
        {
            get
            {
                return (uint)qlz_get_setting(0);
            }
        }

        public uint QLZ_SCRATCH_COMPRESS
        {
            get
            {
                return (uint)qlz_get_setting(1);
            }
        }

        public uint QLZ_SCRATCH_DECOMPRESS
        {
            get
            {
                return (uint)qlz_get_setting(2);
            }
        }

        public uint QLZ_VERSION_MAJOR
        {
            get
            {
                return (uint)qlz_get_setting(7);
            }
        }

        public uint QLZ_VERSION_MINOR
        {
            get
            {
                return (uint)qlz_get_setting(8);
            }
        }


        public int QLZ_VERSION_REVISION
        {
            // negative means beta
            get
            {
                return (int)qlz_get_setting(9);
            }
        }

        public uint QLZ_STREAMING_BUFFER
        {
            get
            {
                return (uint)qlz_get_setting(3);
            }
        }


        public bool QLZ_MEMORY_SAFE
        {
            get
            {
                return qlz_get_setting(6) == 1 ? true : false;
            }
        }



    }
}
