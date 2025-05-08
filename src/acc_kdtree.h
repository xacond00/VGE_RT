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
        bool isLeaf = false;
        uint32_t leftChild;
        uint32_t rightChild;
 
		/* constructors */
         Node() = default;
         Node(const AABB& b, const Vec2u& r)
             : box(b), isLeaf(true), range(r)
        {
            /* empty */
        }
         Node(const AABB& b, uint32_t left, uint32_t right)
             : box(b), isLeaf(false), leftChild(left), rightChild(right)
        {
            /* empty */
        }
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