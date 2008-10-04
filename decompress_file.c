// Sample demo for QuickLZ 1.4.0

// Remember to define QLZ_COMPRESSION_LEVEL and QLZ_STREAMING_MODE to the same values for the compressor and decompressor

#include <stdio.h>
#include <stdlib.h>

#include "quicklz.h"

int main(int argc, char* argv[])
{
   FILE *ifile, *ofile;
   char *src, *dst, *scratch;
   unsigned int len;

   ifile = fopen(argv[1], "rb");
   ofile = fopen(argv[2], "wb");

   // allocate source buffer
   fseek(ifile, 0, SEEK_END);
   len = ftell(ifile);
   fseek(ifile, 0, SEEK_SET);
   src = (char*) malloc(len);

   // read file and allocate destination buffer
   fread(src, 1, len, ifile);
   len = qlz_size_decompressed(src);
   dst = (char*) malloc(len);

    // QLZ_SCRATCH_DECOMPRESS is defined in the beginning of the quicklz.h file
    scratch = (char*) malloc(QLZ_SCRATCH_DECOMPRESS);

   // decompress and write result
   len = qlz_decompress(src, dst, scratch);
   fwrite(dst, len, 1, ofile);
   fclose(ifile);
   fclose(ofile);
   return 0;
}

