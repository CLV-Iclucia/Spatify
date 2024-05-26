#ifndef SPATIFY_INCLUDE_SPATIFY_BBox_H
#define SPATIFY_INCLUDE_SPATIFY_BBox_H
#include <Spatify/types.h>
namespace spatify {
template <typename T, int Dim>
struct BBox {
  Vector<T, Dim> lo{1e9};
  Vector<T, Dim> hi{-1e9};
  BBox() = default;
  BBox(const Vector<T, Dim> &lo, const Vector<T, Dim> &hi) : lo(lo), hi(hi) {
  }
  BBox merge(const BBox &other) const {
    return {cwiseMin<T, Dim>(lo, other.lo), cwiseMax<T, Dim>(hi, other.hi)};
  }
  BBox& expand(const Vector<Real, 3>& p) {
    lo = cwiseMin<T, Dim>(lo, p);
    hi = cwiseMax<T, Dim>(hi, p);
  }
  BBox& expand(const BBox& other) {
    lo = cwiseMin<T, Dim>(lo, other.lo);
    hi = cwiseMax<T, Dim>(hi, other.hi);
  }
  BBox dilate(T factor) const {
    return {lo - factor, hi + factor};
  }
  bool overlap(const BBox &other) const {
    bool overlap_x = lo.x <= other.hi.x && hi.x >= other.lo.x;
    bool overlap_y = lo.y <= other.hi.y && hi.y >= other.lo.y;
    bool overlap_z = lo.z <= other.hi.z && hi.z >= other.lo.z;
    return overlap_x && overlap_y && overlap_z;
  }
  bool inside(const Vector<T, Dim> &point) const {
    return (point.array() >= lo.array()).all() &&
        (point.array() <= hi.array()).all();
  }
  Vector<T, Dim> centre() const {
    return (lo + hi) * static_cast<T>(0.5);
  }
};
}
#endif //COLLISION_INCLUDE_COLLISION_CPU_LBVH_H