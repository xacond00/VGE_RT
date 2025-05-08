/**
 * @file acc_kdtree.c
 * @brief Kdtree acceleration structure implementation.
 *
 * @author Jozef Bilko
 * @date 2025-05-08
 */
 #include "acc_kdtree.h"
 #include <algorithm>
 
 AccelKdTree::AccelKdTree(const Scene& scene, unsigned inputNodeSize)
     : Accel(scene, Accel_t::KdTree), nodeSize(inputNodeSize)
 {
     build();
 }
 
 /* override */
 size_t AccelKdTree::nodes_cnt() const
 {
     return m_kdtree.size();
 }
 
 /* override */
 double AccelKdTree::build_time() const
 {
     return buildTime;
 }
 
 /* override */
 void AccelKdTree::update()
 {
     updateKdtree();
     m_built = true;
 }
 
 /* override */
 void AccelKdTree::build()
 {
     double elapsedTime = timer();
 
     m_kdtree.clear();
     m_kdtree.reserve(m_scene.poly_cnt());
     m_kdtree.emplace_back(m_scene.m_bbox, Vec2u(0, m_poly.size()));
 
     float cost = 0;
     splitKdtree(0, cost);
     buildCost = cost;
     updateCost = cost;
     m_built = true;
 
     buildTime = timer(elapsedTime);
 }
 
 /* override */
 bool AccelKdTree::intersect(const Ray& r, HitInfo& rec) const
 {
     unsigned stack[64];
     unsigned sptr = 0;
     stack[sptr++] = 0;
 
     /* traverse tree */
     while (sptr)
     {
         const Node& node = m_kdtree[stack[--sptr]];
 
         /* check for ray x aabb collision */
         if (node.box.ray_test(r, rec.t()))
         {
             if (node.isLeaf)
             {
                 for (unsigned i = node.range[0]; i < node.range[1]; i++)
                 {
                     poly(i).intersect(r, rec);
                 }
             }
             else
             {
                 stack[sptr++] = node.leftChild;
                 stack[sptr++] = node.rightChild;
             }
         }
     }
     return rec.idx != -1;
 }
 
 /* override */
 bool AccelKdTree::ray_test(const Ray& r, float t) const
 {
     unsigned stack[1024];
     unsigned sptr = 0;
     stack[sptr++] = 0;
 
     /* traverse tree */
     while (sptr)
     {
         const Node& node = m_kdtree[stack[--sptr]];
 
         /* check for ray x aabb collision */
         if (node.box.ray_test(r, t))
         {
             if (node.isLeaf)
             {
                 for (unsigned i = node.range[0]; i < node.range[1]; i++)
                 {
                     if (poly(i).ray_test(r, t))
                     {
                         return true;
                     }
                 }
             }
             else
             {
                 stack[sptr++] = node.leftChild;
                 stack[sptr++] = node.rightChild;
             }
         }
     }
     return false;
 }
 
 std::pair<float, unsigned> AccelKdTree::splitPolygons(const Vec2u& range, const AABB& bbox)
 {
     constexpr unsigned binCount = 32;
     struct binStruct
     {
         AABB bounds;
         unsigned count = 0;
     };
 
     float bestCost = std::numeric_limits<float>::max();
     unsigned bestAxis = 0;
     float bestPlane = bbox.center()[0];
 
    /* find best split axis (X, Y, or Z) */
     for (unsigned axis = 0; axis < 3; ++axis)
     {
         binStruct bins[binCount];
         float binSize = bbox.pmax[axis] - bbox.pmin[axis];

         /* ignore axis with zero length */ 
         if (binSize < 1e-5f)
         {
             continue;
         }
 
         float scale = binCount / binSize;
 
        /* bin polygons */
         for (unsigned i = range[0]; i < range[1]; ++i)
         {
             const AABB& box = vert(i).bbox();
             int binIdx = int((box.center()[axis] - bbox.pmin[axis]) * scale);
             binIdx = std::clamp(binIdx, 0, int(binCount - 1));
             bins[binIdx].count++;
             bins[binIdx].bounds.expand(box);
         }
 
        /* prefix sum */
         AABB leftBox;
         unsigned leftCount = 0;
 
         AABB rightBoxes[binCount];
         unsigned rightCounts[binCount] = {};
 
         for (int i = binCount - 1; i >= 0; --i)
         {
             if (i < binCount - 1)
             {
                 rightBoxes[i] = rightBoxes[i + 1];
                 rightCounts[i] = rightCounts[i + 1];
             }
             rightBoxes[i].expand(bins[i].bounds);
             rightCounts[i] += bins[i].count;
         }
 
        /* find best split position */
         for (unsigned i = 1; i < binCount; ++i)
         {
             leftBox.expand(bins[i - 1].bounds);
             leftCount += bins[i - 1].count;
 
             float cost = leftBox.area() * leftCount + rightBoxes[i].area() * rightCounts[i];
             if (cost < bestCost)
             {
                 bestCost = cost;
                 bestAxis = axis;
                 bestPlane = bbox.pmin[axis] + (binSize * i) / binCount;
             }
         }
     }
 
    /* split polygons based on chosen axis and plane */
     unsigned splitIdx = sortPolygons(range, bestAxis, bestPlane);
     return {bestCost, splitIdx};
 }
 
 void AccelKdTree::splitKdtree(unsigned nodeIdx, float& buildCost)
 {
     Node& node = m_kdtree[nodeIdx];
     const AABB& bbox = node.box;
     unsigned start = node.range[0];
     unsigned end = node.range[1];
     unsigned size = end - start;
     float currentNodeCost = bbox.area() * size;
 
     if (size > nodeSize) {
         auto [cost, mi] = splitPolygons(node.range, bbox);
 
         if (mi > start && mi < end && cost < currentNodeCost)
         {
             Vec2u leftRange(start, mi);
             Vec2u rightRange(mi, end);
 
             unsigned leftIdx = m_kdtree.size();
             m_kdtree.emplace_back(bbox_in(leftRange), leftRange);
 
             unsigned rightIdx = m_kdtree.size();
             m_kdtree.emplace_back(bbox_in(rightRange), rightRange);
 
             node = Node(bbox, leftIdx, rightIdx);
 
             splitKdtree(leftIdx, buildCost);
             splitKdtree(rightIdx, buildCost);
         }
         else
         {
             buildCost += currentNodeCost;
         }
     }
     else
     {
         buildCost += currentNodeCost;
     }
 }
 
 unsigned AccelKdTree::sortPolygons(const Vec2u& range, unsigned axis, float plane)
 {
     auto isPolygonInLeftTree = [&](int idx)
     {
         return vert(idx).bbox().center()[axis] < plane;
     };
 
     int left = range[0];
     int right = range[1];
 
     while (left < right)
     {
        /* if polygon is on left - move forward */
         if (isPolygonInLeftTree(left))
         {
             left++;
         }
         else
         /* else move polygon right */
         {
             right--;
             std::swap(m_poly[left], m_poly[right]);
         }
     }
     return left;
 }
 
 void AccelKdTree::updateKdtree()
 {
     double t_start = timer();
     float totalCost = 0.0f;
 
     for (int i = static_cast<int>(m_kdtree.size()) - 1; i >= 0; i--)
     {
         Node& node = m_kdtree[i];
 
         if (node.isLeaf)
         {
             node.box = bbox_in(node.range);
             totalCost += node.box.area() * (node.range[1] - node.range[0]);
         }
         else
         {
             const AABB& leftBox  = m_kdtree[node.leftChild].box;
             const AABB& rightBox = m_kdtree[node.rightChild].box;
             node.box = leftBox + rightBox;
         }
     }
 
     if (totalCost > 1.2f * buildCost)
     {
         build();
     }
     else
     {
         updateCost = totalCost;
     }
 }