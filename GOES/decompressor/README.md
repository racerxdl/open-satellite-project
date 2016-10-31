GOES LRIT Decompressor
=========================================

This is a windows version of LRIT decompress. It uses NOAA **LritRice.lib** pre-compiled binary. Until I figure out what it has different from `cfitsio` Rice compression algorithms this will be only way to decompress GOES LRIT data. Also the library for linux is broken, since it was compiled for GCC 2 and the ABI is completely incompatible with the newer GCCs. 
