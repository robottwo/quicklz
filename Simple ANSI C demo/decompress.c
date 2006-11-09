#include <stdio.h>
#include <stdlib.h>
#include "quick.c"
int main(int argc, char* argv[])
{
     FILE *ifile, *ofile;
     char *src, *dst;
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

     // decompress and write result
     len = qlz_decompress(src, dst);
     fwrite(dst, len, 1, ofile);
}