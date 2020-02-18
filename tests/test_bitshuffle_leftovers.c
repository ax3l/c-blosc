/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Unit test for the bitshuffle with blocks that are not aligned.
  See https://github.com/Blosc/python-blosc/issues/220
  Probably related: https://github.com/Blosc/c-blosc/issues/240

  Creation date: 2020-02-18
  Author: Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#include "test_common.h"


int main() {
  /* `size` below is chosen so that it is not divisible by 8
   * (not supported by bitshuffle) and in addition, it is not
   * divisible by 8 (typesize) again.
   */
  int size = 641092;
  int8_t *data = malloc(size);
  int8_t *data_out = malloc(size + BLOSC_MIN_HEADER_LENGTH);
  int8_t *data_dest = malloc(size);

  for (int i = 0; i < size; i++) {
    data[i] = i;
  }

  blosc_init();
  blosc_set_nthreads(1);
  blosc_set_compressor("lz4");
  printf("Blosc version info: %s (%s)\n", BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);

  /* Compress with bitshuffle active  */
  int isize = size;
  int osize = size + BLOSC_MIN_HEADER_LENGTH;
  int csize = blosc_compress(9, BLOSC_NOSHUFFLE, sizeof(int64_t), isize, data, data_out, osize);
  if (csize == 0) {
    printf("Buffer is uncompressible.  Giving up.\n");
    return 1;
  }
  else if (csize < 0) {
    printf("Compression error.  Error code: %d\n", csize);
    return csize;
  }

  printf("Compression: %d -> %d (%.1fx)\n", isize, csize, (1.*isize) / csize);
  FILE *fout = fopen("/tmp/blosc_corrupt_compressed.data", "w");
  fwrite(data_out, csize, 1, fout);
  fclose(fout);

  /* Decompress  */
  int dsize = blosc_decompress(data_out, data_dest, isize);
  if (dsize < 0) {
    printf("Decompression error.  Error code: %d\n", dsize);
    return dsize;
  }

  printf("Decompression succesful!\n");

  int exit_code = memcmp(data, data_dest, size) ? EXIT_FAILURE : EXIT_SUCCESS;

  if (exit_code == EXIT_SUCCESS)
    printf("Succesful roundtrip!\n");
  else
    printf("Decompressed data differs from original!");

  free(data);
  free(data_out);
  free(data_dest);

  return exit_code;
}
