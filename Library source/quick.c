/*                      QuickLZ 1.0 data compression library
                    Copyright (C) 2006 by Lasse Mikkel Reinhold

QuickLZ can be used for free under the GPL license (where anything released into public
must be open source) or under a commercial license if such has been acquired (order at 
http://quicklz.earthstorm.com/order).

Caution! This source is only compatible with the x64 and x86 architecturs. It assumes 
little endian, sizeof(unsigned int) = 4 and uses unaligned memory access. Examples of 
supported processors are 80386, 80486, Pentium 4 EMT, Pentium 4, Xeon, Athlon, Athlon 64,
Opteron, Sempron, Celeron. The Itanium processor is IA64 architecture and is not 
supported.

Caution! qlz_compress expects the size of the destination buffer to be exactly "size + 
36000" bytes or more and will crash otherwise because these 36000 bytes are used as
scratch storage */

typedef unsigned int ui32;

ui32 headerlen = 8*sizeof(ui32);
const ui32 QCLZ_ASCII = 0x5a4c4351;
enum HEADERFIELDS {QCLZ = 0, VERSION = 1, COMPSIZE = 2, UNCOMPSIZE = 3, COMPRESSIBLE = 4, RESERVED1 = 5, RESERVED2 = 6, RESERVED3 = 7};

void memcpy_sloppy(char *dst, const char *src, unsigned int n)
{
	char *end = dst + n;
	while(dst < end)
	{
		*((ui32*)dst) = *((ui32*)src);
		dst = dst + sizeof(ui32);
		src = src + sizeof(ui32);
	}
}

unsigned int qlz_size_decompressed(const char *source)
{
	ui32 *header;
	header = (ui32*)source;
	if (header[QCLZ] != QCLZ_ASCII)
		return 0;
	else
		return header[UNCOMPSIZE];
}

unsigned int qlz_size_source(const char *source)
{
	ui32 *header;
	header = (ui32*)source;
	if (header[QCLZ] != QCLZ_ASCII)
		return 0;
	else
		return header[COMPSIZE];
}



unsigned int qlz_compress(const char *source, char *destination, unsigned int size)
{
	const char *last_byte = source + size - 1;
	const char *src = source + 1;
	const char **hashtable = (const char**)(destination + size + 36000 - sizeof(char*) * 4096);
	const ui32 SEQLEN = 2 + (1 << 11);
	char *cword_ptr = destination + headerlen;
	char *dst = destination + headerlen + sizeof(ui32) + 1;
	char *prev_dst = dst;
	const char *prev_src = src;
	ui32 cword_val = 0;
	ui32 cword_counter = sizeof(ui32)*8 - 1;
	ui32 hash;
	ui32 *header = (ui32*)destination;
	const char *guarantee_uncompressed = last_byte - 4*sizeof(ui32);

	header[VERSION] = 3;
	header[UNCOMPSIZE] = size;
	header[QCLZ] = QCLZ_ASCII;

	for(hash = 0; hash < 4096; hash++)
	{
		// If this line crashes, remember to allocate size+36000 bytes for the destination
		hashtable[hash] = source;
	}

	// save first byte uncompressed
	*(destination + headerlen + sizeof(ui32)) = *source;

	while(src < guarantee_uncompressed - SEQLEN)
	{

		ui32 fetch;
		cword_counter--;

		if (cword_counter == 0)
		{
			// check if destination pointer could exceed destination buffer
			if (dst + SEQLEN + 128 > destination + size + SEQLEN + 256)
			{
				memcpy_sloppy(destination + headerlen, source, size-4);
				*((ui32*)(destination + headerlen + size - 4)) = *((ui32*)(source + size - 4));
				header[COMPRESSIBLE] = 0;
				header[COMPSIZE] = size + headerlen + sizeof(ui32);
				*((ui32*)(destination + header[COMPSIZE]) - 1) = QCLZ_ASCII;
				return header[COMPSIZE];
			}

			// store control word
			*((ui32*)cword_ptr) = (cword_val >> 1) | (1 << (sizeof(ui32)*8 - 1));
			cword_counter = sizeof(ui32)*8 - 1;
			cword_ptr = dst;
			dst = dst + sizeof(ui32);

			// check if source chunk is compressible
			if (dst - prev_dst > src - prev_src && src + 2*SEQLEN < guarantee_uncompressed)
			{
				while(src < prev_src + SEQLEN - sizeof(ui32)*8)
				{
					*((ui32*)dst - 1) = (1 << (sizeof(ui32)*8 - 1));
					memcpy_sloppy(dst , src, sizeof(ui32)*8 - 1);
					dst = dst + sizeof(ui32)*8 - 1 + sizeof(ui32);
					src = src + sizeof(ui32)*8 - 1;
				}
				prev_src = src;
				prev_dst = dst;
				cword_ptr = dst - sizeof(ui32);
			}
		}

		// check for rle sequence
		if (*(ui32 *)src == *((ui32 *)(src + 1)))
		{
			const char *orig_src; 
			fetch = *(ui32 *)src;
			orig_src = src;
			src = src + sizeof(ui32) + 1;
			while (fetch == *(((ui32 *)src)) && src < orig_src + SEQLEN - sizeof(ui32))
				src = src + sizeof(ui32);
			*((ui32*)dst) = ((fetch & 0xff) << 16) | ((src - orig_src) << 4) | 15;
			dst = dst + 3;
			cword_val = (cword_val >> 1) | (1 << (sizeof(ui32)*8 - 1));
		}
		else
		{
			const char *o;

			// fetch source data and update hash table
			fetch = *(ui32*)src;
			hash = ((fetch >> 12) ^ fetch) & 0x0fff;
			o = hashtable[hash];
			hashtable[hash] = src;

			if (src - o <= 131071 && src - o > 3 && (((*(unsigned int*)o) ^ (*(const unsigned int*)src)) & 0xffffff) == 0)
			{
				ui32 offset;
				ui32 matchlen;

				offset = (ui32)(src - o);
				if ((*(unsigned int*)o) != (*(unsigned int*)src))
				{
					if(offset <= 63)
					{
						// encode lz match
						*dst = (offset << 2);
						dst++;
						cword_val = (cword_val >> 1) | (1 << (sizeof(ui32)*8 - 1));
						src = src + 3;
					}
					else if (offset <= 16383)
					{
						// encode lz match
						*((ui32*)dst) = (offset << 2) | 1;
						dst = dst + 2;
						cword_val = (cword_val >> 1) | (1 << (sizeof(ui32)*8 - 1));
						src = src + 3;
					}
					else
					{
						// encode literal
						*dst = *src;
						src++;
						dst++;
						cword_val = (cword_val >> 1);
					}

				}
				else
				{
					// encode lz match
					cword_val = (cword_val >> 1) | (1 << (sizeof(ui32)*8 - 1));
					matchlen = 3;

					while(*(o + matchlen) == *(src + matchlen) && matchlen < SEQLEN)
					{
						matchlen++;
					}
					src = src + matchlen;
					
					if (matchlen <= 18 && offset <= 1023)
					{
						*((ui32*)dst) = ((matchlen - 3) << 2) | (offset << 6) | 2;
						dst = dst + 2;
					}

					else if (matchlen <= 34 && offset <= 65535)
					{
						*((ui32*)dst) = ((matchlen - 3) << 3) | (offset << 8) | 3;
						dst = dst + 3;
					}
					else if (matchlen >= 3)
					{
						*((ui32*)dst) = ((matchlen - 3) << 4) | (offset << 15) | 7;
						dst = dst + 4;
					}
				}
			}
			else
			{
				// encode literal
				*dst = *src;
				src++;
				dst++;
				cword_val = (cword_val >> 1);
			}
		}
	}

	// save last source bytes as literals
	while (src <= last_byte)
	{
		cword_counter--;
		if (cword_counter == 0)
		{
			*((ui32*)cword_ptr) = (cword_val >> 1) | (1 << (sizeof(ui32)*8 - 1));
			cword_counter = sizeof(ui32)*8 - 1;
			cword_ptr = dst;
			dst = dst + sizeof(ui32);
		}
		*dst = *src;
		src++;
		dst++;
		cword_val = (cword_val >> 1);
	}

	cword_val = cword_val >> (cword_counter - 1);
	*((ui32*)cword_ptr) = (cword_val >> 1) | (1 << (sizeof(ui32)*8 - 1));
	dst = dst + 4;

	header[COMPRESSIBLE] = 1;
	header[COMPSIZE] = (ui32)(dst - destination - 1 + sizeof(ui32));
	*((ui32*)(destination + header[COMPSIZE]) - 1) = QCLZ_ASCII;
	return header[COMPSIZE];
}

unsigned int qlz_decompress(const char *source, char *destination)
{
	const char *src = source + headerlen;
	char *dst = destination;
	const ui32 *header = (ui32*)source;
	const char* last_byte = destination + header[UNCOMPSIZE];
	ui32 cword_val;
	ui32 cword_counter = 1;
	const ui32 bitlut[16] = {4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
	const char *guaranteed_uncompressed = last_byte - sizeof(ui32);

	if (header[QCLZ] != QCLZ_ASCII || *((ui32*)(source + header[COMPSIZE]) - 1) != QCLZ_ASCII)
		return 0;

	if (header[COMPRESSIBLE] != 1)
	{
		memcpy_sloppy(destination, source + headerlen, header[UNCOMPSIZE] - sizeof(ui32));
		*((ui32*)(destination + header[UNCOMPSIZE] - sizeof(ui32))) = *((ui32*)(source + headerlen + header[UNCOMPSIZE] - sizeof(ui32)));
		return header[UNCOMPSIZE];
	}

	// prevent spurious memory read on a source with size < 4
	if (dst >= guaranteed_uncompressed)
	{
		src = src + sizeof(ui32);
		while(dst < last_byte)
		{
			*dst = *src;
			dst++;
			src++;
		}
		return (unsigned int)(dst - destination);
	}


	while(1)
	{
		cword_counter--;
		if (cword_counter == 0)
		{
			// fetch control word
			cword_val = *((ui32*)src);
			src = src + sizeof(ui32);
			cword_counter = sizeof(ui32)*8 - 1;
		}

		// check if we must decode lz match
		if ((cword_val & 1) == 1)
		{
			ui32 offset;
			ui32 matchlen;

			cword_val = cword_val >> 1;
			if (((*src) & 3) == 0)
			{
				offset = (*(ui32*)src & 0xff) >> 2;
				*(ui32*)dst = *(ui32*)(dst - offset);
				dst = dst + 3;
				src++;
			}
			else if (((*src) & 2) == 0)
			{
				offset = (*(ui32*)src & 0xffff) >> 2;
				*(ui32*)dst = *(ui32*)(dst - offset);
				dst = dst + 3;
				src = src + 2;
			}
			else if (((*src) & 1) == 0)
			{
				offset = (*(ui32*)src & 0xffff) >> 6;
				matchlen = ((*(ui32*)src >> 2) & 15) + 3;
				memcpy_sloppy(dst, dst - offset, matchlen);
				src = src + 2;
				dst = dst + matchlen;
			}
			else if (((*src) & 4) == 0)
			{
				offset = (*(ui32*)src & 0xffffff) >> 8;
				matchlen = ((*(ui32*)src >> 3) & (sizeof(ui32)*8 -1)) + 3;
				memcpy_sloppy(dst, dst - offset, matchlen);
				src = src + 3;
				dst = dst + matchlen;
			}
			else if (((*src) & 8) == 0)
			{
				offset = (*(ui32*)src >> 15);
				matchlen = ((*(ui32*)src >> 4) & 2047) + 3;
				memcpy_sloppy(dst, dst - offset, matchlen);
				src = src + 4;
				dst = dst + matchlen;
			}
			else
			{
				// decode rle sequence
				unsigned char rle_char;
				char *beyond;

				rle_char = (unsigned char)(*(ui32*)src >> 16);
				matchlen = ((ui32)(*(ui32*)src >> 4) & 0xfff);
				beyond = dst + matchlen;
				while(dst < beyond)
				{
					*((ui32*)dst) = rle_char | (rle_char << 8) | (rle_char << 16) | (rle_char << 24);
					dst = dst + sizeof(ui32);
				}
				src = src + 3;
				dst = beyond;
			}
		}
		else
		{
			// decode literal
			*(ui32*)dst = *(ui32*)src;
			dst = dst + bitlut[cword_val & 0xf];
			src = src + bitlut[cword_val & 0xf];
			cword_counter = cword_counter - bitlut[cword_val & 0xf] + 1;
			cword_val = cword_val >> (bitlut[cword_val & 0xf]);

			if (dst >= guaranteed_uncompressed)
			{
				// decode last literals and exit
				while(dst < last_byte)
				{
					cword_counter--;
					if (cword_counter == 0)
					{
						src = src + sizeof(ui32);
						cword_counter = sizeof(ui32)*8 - 1;
					}
					*dst = *src;
					dst++;
					src++;
				}
				return (unsigned int)(dst - destination);
			}
		}
	}
}
