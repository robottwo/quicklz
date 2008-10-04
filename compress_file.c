// Sample demo for QuickLZ 1.4.0

// Remember to define QLZ_COMPRESSION_LEVEL and QLZ_STREAMING_MODE to the same values for the compressor and decompressor

#include <stdio.h>
#include <stdlib.h>

#include "quicklz.h"

int main(int argc, char* argv[])
{
    FILE *ifile, *ofile;
    char *src, *dst, *scratch;
    size_t len; 
    ifile = fopen(argv[1], "rb");
    ofile = fopen(argv[2], "wb");

    // allocate source buffer and read file
    fseek(ifile, 0, SEEK_END);
    len = ftell(ifile);
    fseek(ifile, 0, SEEK_SET);
    src = (char*) malloc(len);
    fread(src, 1, len, ifile);

    // allocate "uncompressed size" + 400 for the destination buffer
    dst = (char*) malloc(len + 400);

    // QLZ_SCRATCH_COMPRESS is defined in the beginning of the quicklz.h file
    scratch = (char*) malloc(QLZ_SCRATCH_COMPRESS);

    // compress and write result
    len = qlz_compress(src, dst, len, scratch);
    fwrite(dst, len, 1, ofile);
    fclose(ifile);
    fclose(ofile);
    return 0;
}
