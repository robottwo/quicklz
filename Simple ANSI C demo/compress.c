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

      // allocate source buffer and read file
      fseek(ifile, 0, SEEK_END);
      len = ftell(ifile);
      fseek(ifile, 0, SEEK_SET);
      src = (char*) malloc(len);
      fread(src, 1, len, ifile);

      // Allocate destination buffer. It is very important that you
      // allocate exactly len + 36000 bytes or more!
      dst = (char*) malloc(len + 36000);

      // compress and write result
      len = qlz_compress(src, dst, len);
      fwrite(dst, len, 1, ofile);
}



