// Sample demo for QuickLZ 1.4.x

// Remember to define QLZ_COMPRESSION_LEVEL to the same value for the compressor and decompressor

#include <stdio.h>
#include <stdlib.h>

#include "quicklz.h"

#if QLZ_STREAMING_BUFFER == 0
    #error Define QLZ_STREAMING_BUFFER to a non-zero value for this demo
#endif

int main(int argc, char* argv[])
{
    FILE *ifile, *ofile;
    char *file_data, *compressed, *scratch;
    size_t d, c;

    ifile = fopen(argv[1], "rb");
    ofile = fopen(argv[2], "wb");

    file_data = (char*) malloc(10000);

    // allocate "uncompressed size" + 400 bytes for the destination buffer where 
    // "uncompressed size" = 10000 in worst case in this sample demo
    compressed = (char*) malloc(10000 + 400); 

    // allocate and initially zero out the scratch buffer. After this, make sure it is
    // preserved across calls and never modified manually
    scratch = (char*) malloc(QLZ_SCRATCH_COMPRESS);
	memset(scratch, 0, QLZ_SCRATCH_COMPRESS); 

    // compress the file in random sized packets
    while((d = fread(file_data, 1, rand() % 10000 + 1, ifile)) != 0)
    {
        c = qlz_compress(file_data, compressed, d, scratch);
        printf("%u bytes compressed into %u\n", (unsigned int)d, (unsigned int)c);

        // the buffer "compressed" now contains c bytes which we could have sent directly to a 
        // decompressing site for decompression
        fwrite(compressed, c, 1, ofile);
    }
    fclose(ifile);
    fclose(ofile);
    return 0;
}
