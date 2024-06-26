//
// Created by creeper on 6/13/24.
//

#ifndef SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_HASHABLE_H_
#define SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_HASHABLE_H_
#include <Spatify/bbox.h>
namespace spatify {

template <typename T>
struct SingleCell {
  Real ox, oy, oz, h;
};

template<typename T>
concept SHBroadPhasePruningIterator = requires(T t) {
  { t.xOffset() } -> std::convertible_to<int>;
  { t.yOffset() } -> std::convertible_to<int>;
  { t.zOffset() } -> std::convertible_to<int>;
  { t.end() } -> std::convertible_to<bool>;
  { ++t } -> std::same_as<T&>;
};

template<typename T>
concept SpatialHashablePrimitive = requires(T t, SingleCell<typename T::CoordType> cell) {
  { t.bbox() } -> std::convertible_to<BBox<typename T::CoordType, 3>>;
  requires SHBroadPhasePruningIterator<decltype(t.pruningIterator(cell))>;
};

// this accessor had better be trivially copyable
template<typename T>
concept SpatialHashablePrimitiveAccessor = requires(T t, size_t index) {
  { t[index] } -> std::convertible_to<typename T::value_type &>;
  { t.size() } -> std::convertible_to<size_t>;
  requires SpatialHashablePrimitive<typename T::value_type>;
};

}
#endif //SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_HASHABLE_H_
