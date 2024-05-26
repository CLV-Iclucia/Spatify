//
// Created by creeper on 5/23/24.
//

#ifndef SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_QUERY_H_
#define SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_QUERY_H_
#include <Spatify/bbox.h>
namespace spatify {

template<typename Derived>
struct SpatialQuery {
  const Derived &derived() const { return static_cast<const Derived &>(*this); }
  Derived &derived() { return static_cast<Derived &>(*this); }
  template <typename T>
  bool query(const BBox<T, 3> &bbox) const { return derived().query(bbox); }
  virtual bool query(int primitive_id) { return derived().query(primitive_id); }
};
template<typename Derived>
struct SpatialPairQuery {
  const Derived &derived() const { return static_cast<const Derived &>(*this); }
  Derived &derived() { return static_cast<Derived &>(*this); }
  bool query(int pid_a, int pid_b) { return derived().query(pid_a, pid_b); }
};
}
#endif //SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_QUERY_H_
