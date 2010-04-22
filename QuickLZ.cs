// Fast data compression library
// Copyright (C) 2006-2010 Lasse Mikkel Reinhold
// lar@quicklz.com
//
// Managed safe C# port of QuickLZ. Only a subset of the C library has been
// ported, namely level 1 not in streaming mode. 
//
// QuickLZ can be used for free under the GPL-1, -2 or -3 license (where anything 
// released into public must be open source) or under a commercial license if such 
// has been acquired (see http://www.quicklz.com/order.html). The commercial license 
// does not cover derived or ported versions created by third parties under GPL.
//
// Version 1.4.1 final - april 2010

using System;
using System.Collections.Generic;
using System.Text;

namespace QuickLZSharp
{
    static class QuickLZ
    {
        // Only following settings have been ported:
        public const int QLZ_COMPRESSION_LEVEL = 1;
        public const int QLZ_STREAMING_BUFFER = 0;
        public const int QLZ_MEMORY_SAFE = 0;

        public const int QLZ_VERSION_MAJOR = 1;
        public const int QLZ_VERSION_MINOR = 4;
        public const int QLZ_VERSION_REVISION = 1;

        const int HASH_VALUES = 4096;
        const int MINOFFSET = 2;
        const int UNCONDITIONAL_MATCHLEN = 6;
        const int UNCOMPRESSED_END = 4;
        const int CWORD_LEN = 4;
        const int DEFAULT_HEADERLEN = 9;

        private static int headerLen(byte[] source)
        {
            return ((source[0] & 2) == 2) ? 9 : 3;
        }

        public static int sizeDecompressed(byte[] source)
        {
            if (headerLen(source) == 9)
                return source[5] | (source[6] << 8) | (source[7] << 16) | (source[8] << 24);
            else
                return source[2];
        }

        public static int sizeCompressed(byte[] source)
        {
            if (headerLen(source) == 9)
                return source[1] | (source[2] << 8) | (source[3] << 16) | (source[4] << 24);
            else
                return source[1];
        }

        public static byte[] compress(byte[] source)
        {
            int src = 0;
            int dst = DEFAULT_HEADERLEN + CWORD_LEN;
            uint cword_val = 0x80000000;
            int cword_ptr = DEFAULT_HEADERLEN;
            byte[] destination = new byte[source.Length + 400];
            int[] hashtable = new int[HASH_VALUES];
            int[] cachetable = new int[HASH_VALUES];
            byte[] hash_counter = new byte[HASH_VALUES];
            byte[] d2;
            int fetch = 0;
            int last_matchstart = (source.Length - UNCONDITIONAL_MATCHLEN - UNCOMPRESSED_END - 1);
            int lits = 0;

            if (source.Length == 0)
                return new byte[0];

            if (src <= last_matchstart)
                fetch = source[src] | (source[src + 1] << 8) | (source[src + 2] << 16);

            while (src <= last_matchstart)
            {
                if ((cword_val & 1) == 1)
                {
                    if (src > 3 * (source.Length >> 2) && dst > src - (src >> 5))
                    {
                        d2 = new byte[source.Length + DEFAULT_HEADERLEN];
                        d2[0] = 2 | 0;
                        fastwriteN(d2, 1, source.Length + DEFAULT_HEADERLEN, 4);
                        fastwriteN(d2, 5, source.Length, 4);
                        System.Array.Copy(source, 0, d2, DEFAULT_HEADERLEN, source.Length);
                        return d2;
                    }

                    fastwriteN(destination, cword_ptr, (int)((cword_val >> 1) | 0x80000000), 4);
                    cword_ptr = dst;
                    dst += CWORD_LEN;
                    cword_val = 0x80000000;
                }

                int hash = ((fetch >> 12) ^ fetch) & (HASH_VALUES - 1);
                int o = hashtable[hash];
                int cache = cachetable[hash] ^ fetch;

                cachetable[hash] = fetch;
                hashtable[hash] = src;

                if (cache == 0 && hash_counter[hash] != 0 && (src - o > MINOFFSET || (src == o + 1 && lits >= 3 && src > 3 && source[src] == source[src - 3] && source[src] == source[src - 2] && source[src] == source[src - 1] && source[src] == source[src + 1] && source[src] == source[src + 2])))
                {
                    cword_val = ((cword_val >> 1) | 0x80000000);
                    if (source[o + 3] != source[src + 3])
                    {
                        int f = 3 - 2 | (hash << 4);
                        destination[dst + 0] = (byte)(f >> 0 * 8);
                        destination[dst + 1] = (byte)(f >> 1 * 8);
                        src += 3;
                        dst += 2;
                    }
                    else
                    {
                        int old_src = src;
                        int remaining = ((source.Length - UNCOMPRESSED_END - src + 1 - 1) > 255 ? 255 : (source.Length - UNCOMPRESSED_END - src + 1 - 1));

                        src += 4;
                        if (source[o + src - old_src] == source[src])
                        {
                            src++;
                            if (source[o + src - old_src] == source[src])
                            {
                                src++;
                                while (source[o + (src - old_src)] == source[src] && (src - old_src) < remaining)
                                    src++;
                            }
                        }

                        int matchlen = src - old_src;

                        hash <<= 4;
                        if (matchlen < 18)
                        {
                            int f = (hash | (matchlen - 2));
                            destination[dst + 0] = (byte)(f >> 0 * 8);
                            destination[dst + 1] = (byte)(f >> 1 * 8);
                            dst += 2;
                        }
                        else
                        {
                            fastwriteN(destination, dst, hash | (matchlen << 16), 3);
                            dst += 3;
                        }
                    }
                    fetch = source[src] | (source[src + 1] << 8) | (source[src + 2] << 16);
                    lits = 0;
                }
                else
                {
                    lits++;
                    hash_counter[hash] = 1;
                    destination[dst] = source[src];
                    cword_val = (cword_val >> 1);
                    src++;
                    dst++;
                    fetch = ((fetch >> 8) & 0xffff) | (source[src + 2] << 16);
                }
            }

            while (src <= source.Length - 1)
            {
                if ((cword_val & 1) == 1)
                {
                    fastwriteN(destination, cword_ptr, (int)((cword_val >> 1) | 0x80000000), 4);
                    cword_ptr = dst;
                    dst += CWORD_LEN;
                    cword_val = 0x80000000;
                }

                destination[dst] = source[src];
                src++;
                dst++;
                cword_val = (cword_val >> 1);
            }
            while ((cword_val & 1) != 1)
            {
                cword_val = (cword_val >> 1);
            }
            fastwriteN(destination, cword_ptr, (int)((cword_val >> 1) | 0x80000000), CWORD_LEN);
            destination[0] = 2 | 1;
            fastwriteN(destination, 1, dst, 4);
            fastwriteN(destination, 5, source.Length, 4);
            d2 = new byte[dst];
            System.Array.Copy(destination, d2, dst);
            return d2;
        }


        static void fastwriteN(byte[] a, int i, int N, int n)
        {
            switch (n)
            {
                case 3:
                    a[i] = (byte)N;
                    a[i + 1] = (byte)(N >> 8);
                    a[i + 2] = (byte)(N >> 16);
                    break;
                case 2:
                    a[i] = (byte)N;
                    a[i + 1] = (byte)(N >> 8);
                    break;
                case 4:
                    a[i] = (byte)N;
                    a[i + 1] = (byte)(N >> 8);
                    a[i + 2] = (byte)(N >> 16);
                    a[i + 3] = (byte)(N >> 24);
                    break;
            }
        }

        public static byte[] decompress(byte[] source)
        {
            int size = sizeDecompressed(source);
            int src = headerLen(source);
            int dst = 0;
            uint cword_val = 1;
            byte[] destination = new byte[size];
            int[] hashtable = new int[4096];
            byte[] hash_counter = new byte[4096];
            int last_matchstart = size - UNCONDITIONAL_MATCHLEN - UNCOMPRESSED_END - 1;
            int last_hashed = -1;
            int hash;
            int fetch = 0;

            if ((source[0] & 1) != 1)
            {
                byte[] d2 = new byte[size];
                System.Array.Copy(source, headerLen(source), d2, 0, size);
                return d2;
            }

            for (; ; )
            {
                if (cword_val == 1)
                {
                    cword_val = (uint)(source[src] | (source[src + 1] << 8) | (source[src + 2] << 16) | (source[src + 3] << 24)); // fast_read(src, CWORD_LEN);
                    src += 4;
                    if (dst <= last_matchstart)
                        fetch = source[src] | (source[src + 1] << 8) | (source[src + 2] << 16);
                }

                if ((cword_val & 1) == 1)
                {
                    int matchlen;
                    int offset2;

                    cword_val = cword_val >> 1;
                    hash = (fetch >> 4) & 0xfff;
                    offset2 = hashtable[hash];

                    if ((fetch & 0xf) != 0)
                    {
                        matchlen = (fetch & 0xf) + 2;
                        src += 2;
                    }
                    else
                    {
                        matchlen = source[src + 2];
                        src += 3;
                    }

                    destination[dst + 0] = destination[offset2 + 0];
                    destination[dst + 1] = destination[offset2 + 1];
                    destination[dst + 2] = destination[offset2 + 2];

                    for (int i = 3; i < matchlen; i += 1)
                    {
                        destination[dst + i] = destination[offset2 + i];
                    }
                    dst += matchlen;

                    fetch = destination[last_hashed + 1] | (destination[last_hashed + 2] << 8) | (destination[last_hashed + 3] << 16);
                    while (last_hashed < dst - matchlen)
                    {
                        last_hashed++;
                        hash = ((fetch >> 12) ^ fetch) & (HASH_VALUES - 1);
                        hashtable[hash] = last_hashed;
                        hash_counter[hash] = 1;
                        fetch = fetch >> 8 & 0xffff | destination[last_hashed + 3] << 16;
                    }
                    last_hashed = dst - 1;
                    fetch = source[src] | (source[src + 1] << 8) | (source[src + 2] << 16);
                }
                else
                {
                    if (dst <= last_matchstart)
                    {
                        destination[dst] = source[src];
                        dst += 1;
                        src += 1;
                        cword_val = cword_val >> 1;

                        while (last_hashed < dst - 3)
                        {
                            last_hashed++;
                            int fetch2 = destination[last_hashed] | (destination[last_hashed + 1] << 8) | (destination[last_hashed + 2] << 16);
                            hash = ((fetch2 >> 12) ^ fetch2) & (HASH_VALUES - 1);
                            hashtable[hash] = last_hashed;
                            hash_counter[hash] = 1;
                        }
                        fetch = fetch >> 8 & 0xffff | source[src + 2] << 16;
                    }
                    else
                    {
                        while (dst <= size - 1)
                        {
                            if (cword_val == 1)
                            {
                                src += CWORD_LEN;
                                cword_val = 0x80000000;
                            }

                            destination[dst] = source[src];
                            dst++;
                            src++;
                            cword_val = cword_val >> 1;
                        }
                        return destination;
                    }
                }
            }
        }
    }
}


