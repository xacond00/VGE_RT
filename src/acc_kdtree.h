#pragma once
/*
* author: Jozef Bilko (xbilko03)
*/
#include "accel.h"
#include <algorithm>

class AccelKdTree : public Accel {

    struct Node {
        AABB bbox;
		/* children */
        int left, right;
		/* split 0 = x, 1 = y, 2 = z */
        int split_axis;
		/* IDs for the range of polygons in this node*/
        int start_idx;
        int end_idx;
    };

public:
    AccelKdTree(const Scene &scene) : Accel(scene, Accel_t::KdTree) {}

    //~AccelKdTree() override {}

    bool intersect(const Ray &r, HitInfo &rec) const override {
        return traverse_tree(r, rec, 0);
    }

    bool ray_test(const Ray &r, Float t = InfF) const override {
        return traverse_ray_test(r, t, 0);
    }

    void build() override {
        build_tree(0, m_scene.poly_cnt() - 1, 0);
        m_built = true;
    }

    void update() override {
        m_built = true;
    }

protected:
    /* check KD-Tree for intersection */
    bool traverse_tree(const Ray &r, HitInfo &rec, int node_idx) const {
        if (node_idx == -1) return false;

        const Node &node = m_nodes[node_idx];
        if (!node.bbox.ray_test(r)) return false;

        /* check node for intersection with aabbox */
        if (node.left == -1 && node.right == -1) {
            bool hit = false;
            for (int i = node.start_idx; i <= node.end_idx; ++i) {
                hit |= m_scene.intersect(i, r, rec);
            }
            return hit;
        }

        /* if no hit, traverse for children */
        bool hit_left = traverse_tree(r, rec, node.left);
        bool hit_right = traverse_tree(r, rec, node.right);

        return hit_left || hit_right;
    }

    bool traverse_ray_test(const Ray &r, Float t, int node_idx) const {
        if (node_idx == -1) return false;

        const Node &node = m_nodes[node_idx];

        /* test for intersection */
        if (!node.bbox.ray_test(r, t)) return false;

        /* check node for intersection with aabbox */
        if (node.left == -1 && node.right == -1) {
            for (int i = node.start_idx; i <= node.end_idx; ++i) {
                if (m_scene.ray_test(i, r, t)) return true;
            }
        }

        /* if no hit, traverse for children */
        if (traverse_ray_test(r, t, node.left)) return true;
        if (traverse_ray_test(r, t, node.right)) return true;

        return false;
    }

    void build_tree(int start, int end, int depth) {
        if (start > end) return;

		/* split axis for 3D space */
        int axis = depth % 3;
        
		/* find aabb for polygons */
        AABB node_bbox = m_scene.bbox_in(start, end);
        auto comparator = [axis, this](int i, int j) {
            Vec3f centroid_i = m_scene.get_center(i);
            Vec3f centroid_j = m_scene.get_center(j);
            return centroid_i[axis] < centroid_j[axis];
        };

		/* sort by position */
        std::sort(m_poly_idx.begin() + start, m_poly_idx.begin() + end + 1, comparator);

		/* split space */
        int mid = (start + end) / 2;

        Node node;
        node.bbox = node_bbox;
        node.start_idx = start;
        node.end_idx = end;

        /* build subtrees */
		node.left = m_nodes.size();
        m_nodes.push_back(Node());
        build_tree(start, mid - 1, depth + 1);
        node.right = m_nodes.size();
        m_nodes.push_back(Node());
        build_tree(mid + 1, end, depth + 1);

		/* store current node */
        m_nodes[depth] = node;
    }

private:
	/* nodes in kd-tree */
    std::vector<Node> m_nodes;
    std::vector<int> m_poly_idx;
};
