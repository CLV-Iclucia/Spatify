//
// Created by creeper on 6/13/24.
//

#ifndef SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_HASH_H_
#define SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_HASH_H_
#include <Spatify/spatial-hashable.h>
#include <Spatify/arrays.h>
#include <Spatify/mortons.h>
namespace spatify {
template<SpatialHashablePrimitive Primitive>
class SpatialHash {
 public:
  using CoordType = typename Primitive::CoordType;

  template<SpatialHashablePrimitiveAccessor Accessor>
  requires std::convertible_to<typename Accessor::PrimitiveType, Primitive>
  void build(const Accessor &accessor, Real spacing) {
    m_primitive_indices.resize(accessor.size());
    h = spacing;
    auto num_primitives = accessor.size();
    BBox<CoordType, 3> scene_bound{};
    for (int i = 0; i < num_primitives; i++)
      scene_bound = scene_bound.merge(accessor.primitive(i).bbox());
    int x_res = static_cast<int>(std::ceil(scene_bound.extent().x / h));
    int y_res = static_cast<int>(std::ceil(scene_bound.extent().y / h));
    int z_res = static_cast<int>(std::ceil(scene_bound.extent().z / h));
    m_ranges.resize(x_res * y_res * z_res);
    m_resolution = {x_res, y_res, z_res};
    std::vector<std::pair<uint64_t, int>> m_grid_id_pair{};
    for (int i = 0; i < num_primitives; i++) {
      auto bbox = accessor.primitive(i).bbox();
      auto ox = static_cast<int>(bbox.lo.x / h) * h;
      auto oy = static_cast<int>(bbox.lo.y / h) * h;
      auto oz = static_cast<int>(bbox.lo.z / h) * h;
      for (auto it = accessor.primitive(i).rasterize({h, ox, oy, oz}); !it.end(); ++it) {
        uint64_t code = encodeMorton21bit(it.x, it.y, it.z);
        m_grid_id_pair.push_back({code, i});
      }
    }
    std::sort(m_grid_id_pair.begin(), m_grid_id_pair.end());
    Range range{-1, -1};
    std::fill(m_ranges.begin(), m_ranges.end(), range);
    for (int i = 0; i < m_grid_id_pair.size(); i++) {
      const auto &[code, idx] = m_grid_id_pair[i];
      m_primitive_indices[i] = idx;
      auto [x, y, z] = decodeMorton21bit(code);
      if (i == 0) range.begin = i;
      if (i == m_grid_id_pair.size() - 1 || code != m_grid_id_pair[i + 1].first) {
        range.end = i + 1;
        m_ranges[index(x, y, z)] = range;
        range.begin = i + 1;
      }
    }
  }
  template<typename Func>
  void forNeighbouringPrimitives(Real x, Real y, Real z, Real radius, Func &&func) {
    int x_min = static_cast<int>(std::floor((x - radius) / h));
    int x_max = static_cast<int>(std::ceil((x + radius) / h));
    int y_min = static_cast<int>(std::floor((y - radius) / h));
    int y_max = static_cast<int>(std::ceil((y + radius) / h));
    int z_min = static_cast<int>(std::floor((z - radius) / h));
    int z_max = static_cast<int>(std::ceil((z + radius) / h));
    for (int i = x_min; i <= x_max; i++) {
      for (int j = y_min; j <= y_max; j++) {
        for (int k = z_min; k <= z_max; k++) {
          auto opt_range = m_ranges.tryRead(i, j, k);
          if (!opt_range.has_value()) continue;
          const auto &[begin, end] = *opt_range;
          for (int idx = begin; idx <= end; idx++)
            func(m_primitive_indices[idx]);
        }
      }
    }
  }
 private:
  struct Range {
    int begin;
    int end;
  };
  int index(int x, int y, int z) const {
    return x + y * m_resolution.x + z * m_resolution.x * m_resolution.y;
  }
  CoordType h{};
  Vector<CoordType, 3> m_origin{};
  Vec3i m_resolution{};
  std::vector<Range> m_ranges{};
  std::vector<int> m_primitive_indices{};
};

}
#endif //SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_SPATIAL_HASH_H_
