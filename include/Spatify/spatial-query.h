//
// Created by creeper on 5/23/24.
//
#ifndef SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_QUERY_H_
#define SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_QUERY_H_
#include <Spatify/bbox.h>
namespace spatify {
template <typename T>
concept SpatialQuery = requires(T query, int id) {
  typename T::CoordType;
  { query(BBox<typename T::CoordType, 3>()) } -> std::same_as<bool>;
  { query(id) } -> std::same_as<bool>;
};
//template<typename Derived>
//struct SpatialPairQuery {
//  const Derived &derived() const { return static_cast<const Derived &>(*this); }
//  Derived &derived() { return static_cast<Derived &>(*this); }
//  bool query(int pid_a, int pid_b) { return derived().query(pid_a, pid_b); }
//};
}
#endif //SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_QUERY_H_
