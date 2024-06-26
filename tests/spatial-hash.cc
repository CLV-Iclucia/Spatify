//
// Created by creeper on 5/23/24.
//
#include <Spatify/spatial-hash.h>
#include <Spatify/sh-pruning.h>
#include <Spatify/bbox.h>
#include <iostream>
using namespace spatify;

struct ShEdge {
  using CoordType = Real;
  Real x0, y0, z0, x1, y1, z1;
  [[nodiscard]] BBox<Real, 3> bbox() const {
    return BBox<Real, 3>{{std::min(x0, x1), std::min(y0, y1), std::min(z0, z1)},
                         {std::max(x0, x1), std::max(y0, y1), std::max(z0, z1)}};
  }
  [[nodiscard]] BBoxIterator<Real> pruningIterator(const SingleCell<Real>& cell) const {
    return BBoxIterator<Real>{bbox(), cell};
  }
};

int main() {
  SpatialHash<ShEdge> sh;
  std::vector<ShEdge> accessor;
  sh.build(accessor, 1.0);
  std::cout << "Spatial hash built successfully!" << std::endl;
}