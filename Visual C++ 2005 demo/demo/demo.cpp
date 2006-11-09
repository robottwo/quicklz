// QuickLZ demo project
// Copyright 2006 by Lasse Mikkel Reinhold

#include <sys/stat.h>
#include <windows.h>
#include "quick.h"
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>

using namespace std;

unsigned int clock, r, u;
char *src, *dst;

void compress(char *source_file, char *destination_file, unsigned int chunksize)
{
	long long N;
	int ifile, ofile;

	ifile = _open(source_file, _O_RDONLY | _O_BINARY);
	N = _lseeki64( ifile, 0u, SEEK_END );

	if (N == 0 || N == -1)
	{
		printf("Error reading source file '%s'.\n", source_file);
		exit(-1);
	}
	try
	{   
		src = new char[chunksize];
		 // remember to allocate exactly N + 36000 bytes or more or qlz_compress will crash
		dst = new char[(int)(chunksize + 36000)];
	}
	catch (...)
	{
		printf("Error allocating memory.\n");
		exit(-1);
	}
	
	_lseeki64( ifile, 0u, SEEK_SET );		
	ofile = _open( destination_file, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);

	if (ofile == -1)
	{
		printf("Error writing destination file '%s'.\n", destination_file);
		exit(-1);
	}

	while (N > 0)
	{
		if (N > chunksize)
			r = chunksize;
		else
			r = (int)N;

		_read( ifile, src, r);
		u = qlz_compress(src, dst, r);
		printf("\r%I64d Mbyte left          ", N >> 20);
		_write(ofile, dst, u);
		N = N - r;
	}

	_close(ofile);
	printf("\rOK                    \n");
}

void decompress(char *source_file, char *destination_file)
{
	long long N;
	int u, ifile, ofile;
	unsigned int *header;
	char tmp[100];

	ifile = _open(source_file, _O_RDONLY | _O_BINARY);
	N = _lseeki64( ifile, 0u, SEEK_END );
	_lseeki64( ifile, 0u, SEEK_SET );	

	if (N == 0 || N == -1)
	{		
		printf("Error reading source file '%s'.\n", source_file);
		exit(-1);
	}

	_read(ifile, tmp, 32);
	_lseeki64( ifile, 0u, SEEK_SET );	

	try
	{
		unsigned int s = qlz_size_decompressed(tmp);
		dst = new char[qlz_size_decompressed(tmp)];
		src = new char[qlz_size_decompressed(tmp) + 36000];
	}
	catch (...)
	{
		printf("Error allocating memory.\n");
		exit(-1);
	}

	ofile = _open(destination_file, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);

	if (ofile == -1)
	{
		printf("Error writing destination file '%s'.\n", destination_file);
		exit(-1);
	}

	while (N > 0)
	{
		_read( ifile, src, 32);
		header = (unsigned int*) src;
		unsigned int hh = qlz_size_source(src);
		r = _read(ifile, src + 32, qlz_size_source(src) - 32);
		if (r != qlz_size_source(src) - 32)
		{
			printf("\nINTERNAL ERROR CAUSED BY CORRUPTED SOURCE FILE OR A BUG IN QUICKLZ.\nPLEASE CONTACT THE DEVELOPER.\n");
			exit(-1);
		}
		N = N - qlz_size_source(src);
		printf("\r%I64d Mbyte left          ", N >> 20);
		u = qlz_decompress(src, dst);

		if (u == 0)
		{
			printf("Source file does not contain QuickLZ data or is version incompatible.\n", source_file);
			exit(-1);
		}		
		_write(ofile, dst, u);
	}

	_close(ofile);
	printf("\rOK                              \n");
}

void inmem(char *source_file)
{
	int u, j, N, y;
	ifstream ifile;
	double mbs, fastest;

	ifile.open(source_file, ios::binary);
	ifile.seekg (0, ios::end);
	N = ifile.tellg();

	if (N == 0 || N == -1)
	{		
		printf("Error reading source file '%s'.\n", source_file);
		exit(-1);
	}
	ifile.seekg (0, ios::beg);

	if (N > 80000000)
	{
		printf("Error: File size is limited to 80 Mbyte for in-memory benchmark.\n");
		exit(-1);
	}

	try
	{   
		src = new char[N];
		// remember to allocate exactly N + 36000 bytes or more or qlz_compress will crash
		dst = new char[N + 36000];
	}
	catch (...)
	{
		printf("Error allocating memory.\n");
		exit(-1);
	}		

	printf("Reading source file...\n");
	ifile.read(src, N);

	printf("Setting HIGH_PRIORITY_CLASS...\n");
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	printf("Benchmarking...\n");

	fastest = 0.0;
	for (j = 0; j < 4; j++)
	{
		y = 0;
		mbs = GetTickCount();
		while(GetTickCount() != mbs);
		mbs = GetTickCount();
		while(GetTickCount() - mbs < 1000)
		{
			u = qlz_compress(src, dst, N);
			y++;
		}
		mbs = (GetTickCount() - mbs);
		mbs = (N*y)/(mbs/1000)/1000000;
		if (fastest	< mbs)
			fastest = mbs;
	}

	printf("\nCompressed at %.1f Mbyte/s.\n", fastest);

	fastest = 0.0;
	for (j = 0; j < 4; j++)
	{
		y = 0;
		mbs = GetTickCount();
		while(GetTickCount() != mbs);
		mbs = GetTickCount();
		while(GetTickCount() - mbs < 1000)
		{
			u = qlz_decompress(dst, src);
			y++;
		}
		mbs = (GetTickCount() - mbs);
		mbs = (N*y)/(mbs/1000)/1000000;
		if (fastest	< mbs)
			fastest = mbs;
	}

	printf("\nDecompressed at %.1f Mbyte/s.\n", fastest);
}


int main(int argc, char* argv[])
{
	//char ** cargv = (char **)argv;

	if (argc == 3 && strcmp(argv[1], "-mem") == 0)
		inmem(argv[2]);
	else if (argc == 4 && strcmp(argv[1], "-d") != 0 && (atoi(argv[3]) > 0 && atoi(argv[3]) < 33))
		compress(argv[1], argv[2], atoi(argv[3]) * 1000000);
	else if (argc == 3 && strcmp(argv[1], "-d"))
		compress(argv[1], argv[2], 4000000);
	else if (argc == 4 && strcmp(argv[1], "-d") == 0)
		decompress(argv[2], argv[3]);
	else
		printf("%s", "QuickLZ 1.0. Copyright 2006 by Lasse Mikkel Reinhold\n\nTo compress a file: quick <source file> <destination file> [chunksize]\nTo decompress a file: quick -d <source file> <destination file>\nTo perform an in-memory benchmark: quick -mem <source file>\n\nChunksize = 1...32 is the disk I/O chunksize in Mbyte (amount of data which\nmust be read and written at a time). Default is 4.\n");
}



