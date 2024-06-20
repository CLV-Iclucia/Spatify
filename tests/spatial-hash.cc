//
// Created by creeper on 5/23/24.
//
#include <Spatify/arrays.h>
#include <Spatify/spatial-query.h>
#include "Spatify/sparse/sparse-spatial-hash.h"
#include <Spatify/rasterizers.h>
#include <Spatify/bbox.h>
#include <Spatify/lbvh.h>

using namespace spatify;
template<typename T>
struct Point {
  using CoordType = T;
  [[nodiscard]] BBox<T, 3> bbox() const {
    return BBox<T, 3>(pos, pos);
  }
  using RasterizeIterator = PointRasterizer<T>;
  [[nodiscard]] RasterizeIterator rasterize(const RasterizeArgs<T> &args) const {
    const auto &[h, ox, oy, oz] = args;
    return RasterizeIterator(static_cast<int>((pos.x - ox) / h),
                             static_cast<int>((pos.y - oy) / h),
                             static_cast<int>((pos.z - oz) / h));
  }
  Vector<T, 3> pos;
};

template<typename T>
struct PointAccessor {
  using PrimitiveType = Point<T>;
  [[nodiscard]] const Point<T> &primitive(size_t index) const {
    return m_points[index];
  }
  [[nodiscard]] size_t size() const { return m_points.size(); }
  std::vector<Point<T>> m_points;
};
int main() {
  SpatialHash<Point<double>> hash;
}