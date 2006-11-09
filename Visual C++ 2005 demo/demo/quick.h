unsigned int qlz_size_decompressed(const char *source);
unsigned int qlz_size_source(const char *source);
unsigned int qlz_compress(const char *source, char *destination, unsigned int size);
unsigned int qlz_decompress(const char *source, char *destination);

