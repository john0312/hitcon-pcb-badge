#ifndef TAMA_APP_UTILS_H
#define TAMA_APP_UTILS_H

#include <cstring>

#include "screens.h"

/**
 * @brief Decompress and return a component from compressed data
 *
 * @param compressed The const compressed image data to be decompressed
 * @return uint8_t* The pointer to the decompressed data. Must be freed after
 * use.
 */
void decompress_component(const CompressedImage* compressed,
                          uint8_t* decompressed_buffer);

#ifdef SIMU
/**
 * @brief Compress a component into a compressed format
 *
 * @param src The source data to be compressed
 * @param width The width of the image
 * @param height The height of the image
 * @return CompressedImage The compressed image data
 */
CompressedImage compress_component(const uint8_t* src, uint8_t width,
                                   uint8_t height);

/**
 * @brief Compress the data and print the compressed information
 *
 * @param src The source data to be compressed
 * @param width The width of the image
 * @param height The height of the image
 */
void compress_data_and_print_info(const uint8_t* src, uint8_t width,
                                  uint8_t height);

/**
 * @brief Print the decompressed component data
 *
 * @param decompressed The pointer to the decompressed data
 * @param width The width of the image
 * @param height The height of the image
 */
void print_decompressed_component(const uint8_t* decompressed, uint8_t width,
                                  uint8_t height);

/**
 * @brief Decompress a component and print the decompressed data
 *
 * @param compressed The compressed image data to be decompressed
 */
void decompress_and_print_component(const CompressedImage* compressed);
#endif

#endif  // TAMA_APP_UTILS_H