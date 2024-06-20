//
// Created by creeper on 5/23/24.
//

#ifndef SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_LBVH_H_
#define SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_LBVH_H_
#include <array>
#include <vector>
#include <Spatify/platform.h>
#include <Spatify/spatial-query.h>
#include <Spatify/parallel.h>
#include <concepts>
#include <atomic>
namespace spatify {
inline unsigned int expandBits(unsigned int v) {
  v = (v * 0x00010001u) & 0xFF0000FFu;
  v = (v * 0x00000101u) & 0x0F00F00Fu;
  v = (v * 0x00000011u) & 0xC30C30C3u;
  v = (v * 0x00000005u) & 0x49249249u;
  return v;
}

inline int lcp(uint64_t a, uint64_t b) {
  return countLeadingZeros64Bit(a ^ b);
}

template <typename T>
concept BvhPrimitive = requires(T t) {
  { t.bbox() } -> std::convertible_to<BBox<typename T::CoordType, 3>>;
};

template <typename T>
concept BvhPrimitiveAccessor = requires(T t, int idx) {
  requires BvhPrimitive<typename T::PrimitiveType>;
  { t(idx) } -> std::convertible_to<typename T::PrimitiveType>;
  { t.size() } -> std::convertible_to<int>;
};

template<typename T>
class LBVH {
 public:
  LBVH() {}
  ~LBVH() = default;
  template<SpatialQuery Query>
  void runSpatialQuery(Query&& Q) const {
    std::array<int, 64> stack{};
    int top{};
    int nodeIdx = 0;
    if (!Q.query(bbox[nodeIdx]))
      return;
    bool hit = false;
    while (true) {
      if (isLeaf(nodeIdx)) {
        int pr_id = idx[nodeIdx - nPrs + 1];
        bool flag = Q.query(pr_id);
        hit |= flag;
        if (!top) return;
        nodeIdx = stack[--top];
        continue;
      }
      int lc = lch[nodeIdx];
      int rc = rch[nodeIdx];
      bool intl = Q.query(bbox[lc]);
      bool intr = Q.query(bbox[rc]);
      if (!intl && !intr) {
        if (!top) return;
        nodeIdx = stack[--top];
        continue;
      }
      if (intl && !intr) nodeIdx = lc;
      else if (!intl) nodeIdx = rc;
      else {
        nodeIdx = lc;
        stack[top++] = rc;
      }
    }
  }
  [[nodiscard]] bool isLeaf(int index) const {
    return index >= nPrs - 1;
  }
  [[nodiscard]] int findSplit(int l, int r) const {
    if (mortons[l] == mortons[r])
      return (l + r) >> 1;
    int commonPrefix = lcp(mortons[l], mortons[r]);
    int search = l;
    int step = r - l;
    do {
      step = (step + 1) >> 1;
      if (int newSearch = search + step; newSearch < r) {
        uint64_t splitCode = mortons[newSearch];
        if (lcp(mortons[l], splitCode) > commonPrefix)
          search = newSearch;
      }
    } while (step > 1);
    return search;
  }
  [[nodiscard]] int delta(int i, int j) const {
    if (j < 0 || j > nPrs - 1) return -1;
    return lcp(mortons[i], mortons[j]);
  }

  uint32_t mortonCode(const Vector<T, 3> &p) const {
    unsigned int x = std::max((p(0) - scene_bound.lo.x) / (scene_bound.hi.x - scene_bound.lo.x) * 1024, 0.0);
    unsigned int y = std::max((p(1) - scene_bound.lo.y) / (scene_bound.hi.y - scene_bound.lo.y) * 1024, 0.0);
    unsigned int z = std::max((p(2) - scene_bound.lo.z) / (scene_bound.hi.z - scene_bound.lo.z) * 1024, 0.0);
    return (expandBits(x) << 2) | (expandBits(y) << 1) | expandBits(z);
  }

  uint64_t encodeBBox(const BBox<T, 3> &aabb, int index) const {
    uint32_t morton = mortonCode(aabb.centroid());
    return (static_cast<uint64_t>(morton) << 32) | index;
  }
  int nPrs;
  std::vector<BBox<Real, 3>> bbox{};
  template <BvhPrimitiveAccessor Accessor>
  void update(Accessor accessor) {
    nPrs = accessor.size();
    mortons.resize(2 * nPrs - 1);
    mortons_copy.resize(2 * nPrs - 1);
    bbox.resize(2 * nPrs - 1);
    fa.resize(2 * nPrs - 1);
    lch.resize(nPrs - 1);
    rch.resize(nPrs - 1);
    idx.resize(nPrs);
    // first compute the bounding box of the scene in parallel
    for (int i = 0; i < nPrs; i++) {
      bbox[i] = accessor(i).bbox();
      scene_bound.expand(bbox[i]);
    }
    parallel_for(0,
                 nPrs,
                 [this](int i) {
                   mortons[i] = encodeBBox(bbox[i], i);
                   idx[i] = i;
                 });
    mortons_copy = mortons;
    parallel_sort(idx.begin(),
                  idx.end(),
                  [this](int a, int b) {
                    return mortons[a] < mortons[b];
                  });
    parallel_for(0,
                 nPrs,
                 [this](int i) {
                   mortons[i] = mortons_copy[idx[i]];
                 });
    fa[0] = -1;
    parallel_for(0,
                 nPrs - 1,
                 [this](int i) {
                   int dir = delta(i, i + 1) > delta(i, i - 1) ? 1 : -1;
                   int min_delta = delta(i, i - dir);
                   int lmax = 2;
                   while (delta(i, i + lmax * dir) > min_delta) lmax <<= 1;
                   int len = 0;
                   for (int t = lmax >> 1; t; t >>= 1) {
                     if (delta(i, i + (len | t) * dir) > min_delta)
                       len |= t;
                   }
                   int l = std::min(i, i + len * dir);
                   int r = std::max(i, i + len * dir);
                   int split = findSplit(l, r);
                   if (l == split)
                     lch[i] = nPrs - 1 + split;
                   else lch[i] = split;
                   if (r == split + 1)
                     rch[i] = nPrs + split;
                   else rch[i] = split + 1;
                   fa[rch[i]] = fa[lch[i]] = i;
                 });
    parallel_for(0,
                 nPrs,
                 [this, &accessor](int i) {
                   int node_idx = nPrs + i - 1;
                   bbox[node_idx] = accessor(idx[i]).bbox();
                 });
    std::vector<std::atomic<bool>> processed(nPrs - 1);
    parallel_for(0,
                 nPrs,
                 [this, &processed](int i) {
                   int node_idx = nPrs + i - 1;
                   while (fa[node_idx] != -1) {
                     int parent = fa[node_idx];
                     // use atomic flag to let the second thread enter the critical section
                     if (!processed[parent].exchange(true)) return;
                     bbox[parent] = bbox[lch[parent]];
                     bbox[parent].expand(bbox[rch[parent]]);
                     node_idx = parent;
                   }
                 });
  }
//  template<typename Derived>
//  void runSelfSpatialQuery(const SpatialPairQuery<Derived> &Q) {
//    std::array<uint64_t, 64> stack{};
//    int top{};
//    uint64_t pair = 0;
//    int a = 0, b = 0;
//    while (true) {
//      if (isLeaf(a) && isLeaf(b)) {
//        Q.query(idx[a], idx[b]);
//        if (!top) return;
//        pair = stack[--top];
//        a = static_cast<int>(pair >> 32);
//        b = static_cast<int>(pair & 0xFFFFFFFF);
//        continue;
//      }
//      bool go_left = true, go_right = true;
//      if (!isLeaf(a)) {
//        if (!bbox[lch[a]].overlap(bbox[b]))
//          go_left = false;
//        if (!bbox[rch[a]].overlap(bbox[b]))
//          go_right = false;
//        if (go_left && go_right) {
//          stack[top++] = (static_cast<uint64_t>(rch[a]) << 32) | b;
//          a = lch[a];
//        } else if (go_left) a = lch[a];
//        else if (go_right) a = rch[a];
//        else {
//          if (!top) return;
//          pair = stack[--top];
//          a = static_cast<int>(pair >> 32);
//          b = static_cast<int>(pair & 0xFFFFFFFF);
//        }
//      } else if (!isLeaf(b)) {
//        if (!bbox[a].overlap(bbox[lch[b]]))
//          go_left = false;
//        if (!bbox[a].overlap(bbox[rch[b]]))
//          go_right = false;
//        if (go_left && go_right) {
//          stack[top++] = (static_cast<uint64_t>(a) << 32) | rch[b];
//          b = lch[b];
//        } else if (go_left) b = lch[b];
//        else if (go_right) b = rch[b];
//        else {
//          if (!top) return;
//          pair = stack[--top];
//          a = static_cast<int>(pair >> 32);
//          b = static_cast<int>(pair & 0xFFFFFFFF);
//        }
//      }
//    }
//  }
 private:
  std::vector<uint64_t> mortons, mortons_copy;
  std::vector<int> fa;
  std::vector<int> lch;
  std::vector<int> rch;
  std::vector<int> idx;
  BBox<Real, 3> scene_bound{};
};
}
#endif //SIMCRAFT_SPATIFY_INCLUDE_SPATIFY_LBVH_H_