//
// Created by creeper on 5/31/24.
//

#ifndef SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_MORTONS_H_
#define SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_MORTONS_H_
namespace spatify {
inline uint32_t morton10BitEncode(uint32_t x) {
  x = (x | (x << 16)) & 0x030000FF;
  x = (x | (x << 8)) & 0x0300F00F;
  x = (x | (x << 4)) & 0x030C30C3;
  x = (x | (x << 2)) & 0x09249249;
  return x;
}

inline uint32_t morton10BitDecode(uint32_t x) {
  x = x & 0x09249249;
  x = (x | (x >> 2)) & 0x030C30C3;
  x = (x | (x >> 4)) & 0x0300F00F;
  x = (x | (x >> 8)) & 0x030000FF;
  x = (x | (x >> 16)) & 0x000003FF;
  return x;
}

inline std::tuple<uint32_t, uint32_t, uint32_t> decodeMorton10bit(uint32_t code) {
  uint32_t x = morton10BitDecode(code);
  uint32_t y = morton10BitDecode(code >> 1);
  uint32_t z = morton10BitDecode(code >> 2);
  return {x, y, z};
}

inline uint32_t encodeMorton10bit(int x, int y, int z) {
  return (morton10BitEncode(x) << 2) | (morton10BitEncode(y) << 1) | morton10BitEncode(z);
}

inline uint32_t morton21BitEncode(uint32_t x) {
  x = (x | (x << 16)) & 0x001F0000;
  x = (x | (x << 8)) & 0x001F00F0;
  x = (x | (x << 4)) & 0x001F0C30;
  x = (x | (x << 2)) & 0x00192492;
  return x;
}
inline uint32_t morton21BitDecode(uint32_t x) {
  x = x & 0x00192492;
  x = (x | (x >> 2)) & 0x001F0C30;
  x = (x | (x >> 4)) & 0x001F00F0;
  x = (x | (x >> 8)) & 0x001F0000;
  x = (x | (x >> 16)) & 0x0001FFFF;
  return x;
}
inline std::tuple<uint32_t, uint32_t, uint32_t> decodeMorton21bit(uint32_t code) {
  uint32_t x = morton21BitDecode(code);
  uint32_t y = morton21BitDecode(code >> 1);
  uint32_t z = morton21BitDecode(code >> 2);
  return {x, y, z};
}
inline uint32_t encodeMorton21bit(int x, int y, int z) {
  return (morton21BitEncode(x) << 2) | (morton21BitEncode(y) << 1) | morton21BitEncode(z);
}
}
#endif //SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_MORTONS_H_
