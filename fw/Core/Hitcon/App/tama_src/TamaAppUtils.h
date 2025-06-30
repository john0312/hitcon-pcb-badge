#include "screens.h"

/**
 * @brief Decompress and return a component from compressed data
 *
 * @param compressed The const compressed image data to be decompressed
 * @return uint8_t* The pointer to the decompressed data. Must be freed after
 * use.
 */
uint8_t* decompress_component(const CompressedImage* compressed) {
  // new the memory at heap
  uint8_t* decompressed_data =
      new uint8_t[compressed->width * compressed->height];
  // set the memory to 0
  memset(decompressed_data, 0, compressed->width * compressed->height);

  for (int y = 0; y < compressed->height; ++y) {
    for (int x = 0; x < compressed->width; ++x) {
      int byte_index = x;  // each column corresponds to one uint8_t
      int bit_index = y % 8;
      decompressed_data[y * compressed->width + x] =
          (compressed->data[byte_index] & (1 << bit_index)) ? 1 : 0;
    }
  }

  return decompressed_data;
}

/**  The test function for compress and decompress , not in use in runtime */

/**
 * @brief Compress a component into a compressed format
 *
 * @param src
 * @param width
 * @param height
 * @return CompressedImage
 */
CompressedImage compress_component(const uint8_t* src, uint8_t width,
                                   uint8_t height) {
  // calculate compressed size
  uint16_t compressed_size =
      width *
      ((height + 7) /
       8);  // every 8 rows of pixels are packed into one byte, +7 to round up
  uint8_t* compressed_data = (uint8_t*)malloc(compressed_size);
  memset(compressed_data, 0, compressed_size);

  /**
   * The compress method:
   * Example：
   *      original data (8x8):
   *        1 0 1 0 0 1 0 1 row=0
   *        0 1 0 1 1 0 1 0
   *        1 1 1 0 0 1 0 0
   *        0 0 0 1 1 1 1 1
   *        1 0 1 0 1 0 1 0
   *        0 1 0 1 0 1 0 1
   *        1 1 0 0 1 1 0 0
   *        0 0 1 1 0 0 1 1 row=7
   *        col=0         col=7
   *      compressed data:
   *        col=0:  1 0 1 0 1 0 1 0  ->  0b10101010  ->  0xAA
   *        col=1:  0 1 1 0 0 1 1 0  ->  0b01100110  ->  0x66
   *        col=2:  1 0 1 0 1 0 0 1  ->  0b10101001  ->  0xA9
   *        col=3:  0 1 0 1 0 1 0 1  ->  0b01010101  ->  0x55
   *        col=4:  0 1 0 1 1 0 1 0  ->  0b01011010  ->  0x5A
   *        col=5:  1 0 1 1 0 1 1 0  ->  0b10110110  ->  0xB6
   *        col=6:  0 1 0 0 1 0 0 1  ->  0b01001001  ->  0x49
   *        col=7:  1 0 0 1 1 1 1 1  ->  0b10011111  ->  0x9F
   *      method:
   *        1. read row 0 (x=0~7)
   *        2. x=0 save 1 to compressed_data[0],
   *           x=1 save 0 to compressed_data[1]
   *           same as:
   *           col=0: 1
   *           col=1: 0
   *           col=2: 1
   *           ...
   *           col=7: 1
   *        3. y++ (next row), loop x from 0 to 7
   *           same as:
   *           col=0: 1 0 -> keep growing
   *           col=1: 0 1 -> keep growing
   *           col=2: 1 0 -> keep growing
   *           ...
   *           col=7: 1 0
   *        4. repeat until all rows are processed
   */
  // compress the image
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int byte_index = x;  // 每列對應一個 uint8_t
      int bit_index = y % 8;
      if (src[y * width + x]) {
        compressed_data[byte_index] |= (1 << bit_index);
      }
    }
  }

  // return the compressed data
  CompressedImage result = {width, height, compressed_data};
  return result;
}

/**
 * @brief Compress the data and print the compressed information
 *
 * @param src The source data to be compressed
 * @param width The width of the image
 * @param height The height of the image
 */
void compress_data_and_print_info(const uint8_t* src, uint8_t width,
                                  uint8_t height) {
  CompressedImage compressed = compress_component(src, width, height);
  printf("Compressed data:\n");
  printf("Width: %d, Height: %d\n", compressed.width, compressed.height);
  printf("Size after compressed: %d bytes\n", width);
  printf("Data: {");
  for (int i = 0; i < width; ++i) {
    printf("0x%02X", compressed.data[i]);
    if (i < width - 1) {
      printf(", ");
    }
  }
  printf("}\n");

  // free the allocated memory
  free((void*)compressed.data);
}

/**
 * @brief Print the decompressed component data
 *
 * @param decompressed The pointer to the decompressed data
 * @param width The width of the image
 * @param height The height of the image
 */
void print_decompressed_component(const uint8_t* decompressed, uint8_t width,
                                  uint8_t height) {
  printf("Decompressed data:\n");
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      printf("%d ", decompressed[y * width + x]);
    }
    printf("\n");
  }
}

/**
 * @brief Decompress a component and print the decompressed data
 *
 * @param compressed The compressed image data to be decompressed
 */
void decompress_and_print_component(const CompressedImage* compressed) {
  uint8_t* decompressed = decompress_component(compressed);
  print_decompressed_component(decompressed, compressed->width,
                               compressed->height);
  free(decompressed);
}
