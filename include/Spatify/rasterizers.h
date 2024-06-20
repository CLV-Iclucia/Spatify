//
// Created by creeper on 5/31/24.
//

#ifndef SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_RASTERIZERS_H_
#define SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_RASTERIZERS_H_
// some frequently used rasterizers for point, edge and triangle
namespace spatify {
template<typename T>
struct PointRasterizer {
  int x, y, z;
  bool has_increased{false};
  PointRasterizer(int x, int y, int z) : x(x), y(y), z(z) {}
  [[nodiscard]] bool end() const { return has_increased; }
  void operator++() { has_increased = true; }
};
struct RasterizingPoint {
  int x, y, z;
};
template<typename T>
struct SegmentRasterizer {
 public:
  SegmentRasterizer(RasterizingPoint start, RasterizingPoint end)
      : x(start.x), y(start.y), z(start.z),
        x1(start.x), y1(start.y), z1(start.z),
        x2(end.x), y2(end.y), z2(end.z),
        dx(std::abs(x2 - x1)), dy(std::abs(y2 - y1)), dz(std::abs(z2 - z1)),
        xs((x2 > x1) ? 1 : -1), ys((y2 > y1) ? 1 : -1), zs((z2 > z1) ? 1 : -1) {

    if (dx >= dy && dx >= dz) {
      p1 = 2 * dy - dx;
      p2 = 2 * dz - dx;
      dominant_axis = 0; // X-axis is dominant
    } else if (dy >= dx && dy >= dz) {
      p1 = 2 * dx - dy;
      p2 = 2 * dz - dy;
      dominant_axis = 1; // Y-axis is dominant
    } else {
      p1 = 2 * dy - dz;
      p2 = 2 * dx - dz;
      dominant_axis = 2; // Z-axis is dominant
    }
  }

  SegmentRasterizer &operator++() {
    switch (dominant_axis) {
      case 0: // X-axis is dominant
        if (p1 >= 0) {
          y += ys;
          p1 -= 2 * dx;
        }
        if (p2 >= 0) {
          z += zs;
          p2 -= 2 * dx;
        }
        x += xs;
        p1 += 2 * dy;
        p2 += 2 * dz;
        break;
      case 1: // Y-axis is dominant
        if (p1 >= 0) {
          x += xs;
          p1 -= 2 * dy;
        }
        if (p2 >= 0) {
          z += zs;
          p2 -= 2 * dy;
        }
        y += ys;
        p1 += 2 * dx;
        p2 += 2 * dz;
        break;
      case 2: // Z-axis is dominant
        if (p1 >= 0) {
          y += ys;
          p1 -= 2 * dz;
        }
        if (p2 >= 0) {
          x += xs;
          p2 -= 2 * dz;
        }
        z += zs;
        p1 += 2 * dy;
        p2 += 2 * dx;
        break;
    }
    return *this;
  }

  [[nodiscard]] bool end() const {
    return (x == x2 && y == y2 && z == z2);
  }

  [[nodiscard]] RasterizingPoint current() const {
    return {x, y, z};
  }

 private:
  int x, y, z;
  int x1, y1, z1, x2, y2, z2;
  int dx, dy, dz;
  int xs, ys, zs;
  int p1, p2;
  int dominant_axis;
};

template <typename T>
struct RasterizingTriangle {
  Vector<T, 3> p1, p2, p3;
};

template<typename T>
void project(const Vector<T, 3>& axis, const std::span<Vector<T, 3>> points, float& min, float& max) {
  min = max = axis.dot(points[0]);
  for (const auto& p : points) {
    float val = axis.dot(p);
    if (val < min) min = val;
    if (val > max) max = val;
  }
}

template <typename T>
bool overlap(T minA, T maxA, T minB, T maxB) {
  return !(maxA < minB || maxB < minA);
}

template <typename T>
bool triangleBoxIntersection(const RasterizingTriangle<T>& tri, const BBox<T, 3>& box) {
  std::array<Vector<T, 3>, 3> tri_points = {tri.p1, tri.p2, tri.p3};
  std::array<Vector<T, 3>, 3> box_points = {
      box.lo,
      {box.lo.x, box.lo.y, box.hi.z},
      {box.lo.x, box.hi.y, box.lo.z},
      {box.hi.x, box.lo.y, box.lo.z},
      {box.hi.x, box.hi.y, box.hi.z},
      {box.hi.x, box.hi.y, box.lo.z},
      {box.hi.x, box.lo.y, box.hi.z},
      {box.lo.x, box.hi.y, box.hi.z}
  };

  std::array<Vector<T, 3>, 13> axes = {
      (tri.p2 - tri.p1).cross(tri.p3 - tri.p1),
      {1, 0, 0},
      {0, 1, 0},
      {0, 0, 1}
  };
  int cnt = 4;
  std::array<Vector<T, 3>, 3> tri_edges = {tri.p2 - tri.p1, tri.p3 - tri.p2, tri.p1 - tri.p3};
  std::array<Vector<T, 3>, 3> box_edges = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  for (const auto& tri_edge : tri_edges)
    for (const auto& box_edge : box_edges)
      axes[cnt++] = tri_edge.cross(box_edge);
  for (const auto& axis : axes) {
    T tri_min, tri_max, box_min, box_max;
    project(axis, std::span(tri_points), tri_min, tri_max);
    project(axis, std::span(box_points), box_min, box_max);
    if (!overlap(tri_min, tri_max, box_min, box_max))
      return false;
  }
  return true;
}

}
#endif //SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_RASTERIZERS_H_
