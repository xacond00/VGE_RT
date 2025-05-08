/**
 * @file acc_kdtree.h
 * @brief Kdtree acceleration structure header.
 *
 * @author Jozef Bilko
 * @date 2025-05-08
 */
 #pragma once
 #include "accel.h"
 
 class AccelKdTree : public Accel
 {
     struct Node
     {
         AABB box;
         /* range of indices for the node */
         Vec2u range;
 
		/* constructors */
         Node() = default;
         Node(const AABB& inputBox, const Vec2u& inputRange);
         Node(AABB&& inputBox, Vec2u&& inputRange);
     };
 
 public:
     AccelKdTree(const Scene& scene, unsigned inputNodeSize = 8);
 
     size_t nodes_cnt() const override;
     double build_time() const override;
 
     bool intersect(const Ray& r, HitInfo& rec) const override;
     bool ray_test(const Ray& r, float t = InfF) const override;
     void update() override;
     void build() override;
 
 private:
    std::vector<Node> m_kdtree;
    unsigned nodeSize = 8;
    float updateCost = 0;
    float buildCost = 0;
    double buildTime = 0;

     unsigned sortPolygons(const Vec2u& range, unsigned axis, float plane);
     std::pair<float, unsigned> splitPolygons(const Vec2u& range, const AABB& bbox);
     void splitKdtree(unsigned node, float& buildCost);
     void updateKdtree();
 };