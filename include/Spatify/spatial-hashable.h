//
// Created by creeper on 6/13/24.
//

#ifndef SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_HASHABLE_H_
#define SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_HASHABLE_H_
#include <Spatify/bbox.h>
namespace spatify {
template<typename T>
struct RasterizeArgs {
  T h, ox, oy, oz;
};

template<typename T>
concept RasterizeIterator = requires(T t) {
  { t.x } -> std::convertible_to<int>;
  { t.y } -> std::convertible_to<int>;
  { t.z } -> std::convertible_to<int>;
  { t.end() } -> std::convertible_to<bool>;
  { ++t } -> std::same_as<void>;
};

template<typename T>
concept SpatialHashablePrimitive = requires(T t, RasterizeArgs<typename T::CoordType> args) {
  { t.bbox() } -> std::convertible_to<BBox<typename T::CoordType, 3>>;
  { t.rasterize(args) } -> std::convertible_to<typename T::RasterizeIterator>;
};

// this accessor had better be trivially copyable
template<typename T>
concept SpatialHashablePrimitiveAccessor = requires(T t, size_t index) {
  { t.primitive(index) } -> std::same_as<const typename T::PrimitiveType &>;
  { t.size() } -> std::convertible_to<size_t>;
  requires SpatialHashablePrimitive<typename T::PrimitiveType>;
};

}
#endif //SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_HASHABLE_H_
