/**
 * @file acc_bih.h
 * @brief Bounding Interval Hierarchy (BIH) acceleration structure header.
 *
 * Based on the Bounding Interval Hierarchy (BIH) paper:
 * https://citeseerx.ist.psu.edu/document?repid=rep1&type=pdf&doi=e69d95ce461af2b88c7d3822b7be52eea8dd3af7
 *
 * @author Marek Konečný
 * @date 2025-05-07
 */
#pragma once

#include "accel.h"
#include <vector>

/**
 * @brief BIH acceleration structure.
 *
 * Builds a two-plane per-node hierarchy over scene's polygons.
 */
class AccelBih : public Accel {
public:
    /**
     * @brief Runs build() on construct.
     * @param m_scene
     * @param max_leaf_polygons Threshold for leaf node size before splitting.
     */
    explicit AccelBih(const Scene &m_scene, Uint max_leaf_polygons = 4)
        : Accel(m_scene, Accel_t::BIH), max_leaf_polygons(max_leaf_polygons)
    {
        build();
    }

    /**
     * @brief Intersect a ray with all polygons, recording the closest hit.
     * @param ray_query Query ray.
     * @param hit_info Hit information.
     * @return True if any polygon is hit.
     */
    bool intersect(const Ray &ray_query, HitInfo &hit_info) const override;

    /**
     * @brief Test whether ray hits any polygon.
     * @param ray_query Query ray.
     * @param t_max Maximum ray parameter (distance).
     * @return True if ray hits something before t_max.
     */
    bool ray_test(const Ray &ray_query, Float t_max = InfF) const override;

    /**
     * @brief Refits the BIH for dynamic scene; rebuilds if too costly.
     * @warning Not testetd.
     */
    void update() override;

    /**
     * @brief Full BIH rebuild.
     */
    void build() override;

private:
    /**
     * @brief BIH node, storing either a polygon indices range or child indices.
     * @note  41B (aligned to 44B).
     */
    struct BihNode {
        AABB    bounds;        // Bounding box clipped by split planes
        uint8_t split_axis;    // Split axis index (0=x,1=y,2=z)
        Float   left_max;      // Upper bound of left child interval
        Float   right_min;     // Lower bound of right child interval
        Vec2u   index_range;   // If leaf: [start,end) in poly_index; if internal: [right,left) child-node indices

        BihNode() = default;
        BihNode(AABB &&bounds, Vec2u &&index_range)
            : bounds(std::move(bounds)), index_range(std::move(index_range)) {}
        BihNode(const AABB &bounds, const Vec2u &index_range)
            : bounds(bounds), index_range(index_range) {}

        inline bool is_leaf()     const { return index_range[0] < index_range[1]; }
        inline bool is_internal() const { return index_range[0] > index_range[1]; }
        inline bool is_empty()    const { return index_range[0] == index_range[1]; }
    };


    /**
     * @brief Finds best split hyperplane using SAH heuristic.
     * @param index_range polygon index range [start,end) in poly_index.
     * @param bounds      Bounding box of those polygons.
     * @param out_axis    Returned best axis (0,1,2).
     * @param out_plane   Returned split coordinate on that axis.
     * @return Pair(best_cost, split_index) where split_index partitions poly_index.
     */
    std::pair<Float,Uint> find_split_sah(const Vec2u &index_range, const AABB  &bounds, uint8_t &out_axis, Float &out_plane);

    /**
     * @brief Partition polygons by side of the split plane.
     * @param index_range [start,end) to partition.
     * @param axis        Axis to compare.
     * @param plane_split Coordinate of splitting plane.
     * @return First index in the right-hand partition.
     */
    Uint partition_polygons(const Vec2u &index_range, uint8_t axis, Float plane_split);

    /**
     * @brief Recursively builds hiearchy of nodes using SAH splits.
     * @param node_index Index in nodes of the current node.
     * @param cost_accum Accumulator of SAH leaf costs.
     */
    void build_node_recursive(Uint node_index, Float &cost_accum);

    /**
     * @brief Utility method for computing entry/exit t-intervals for left/right child nodes.
     * @param node        Traversed node.
     * @param ray_query   Query ray.
     * @param t_max       Current maximum t (closest hit so far).
     * @param hit_left    True if ray intersects left interval.
     * @param hit_right   True if ray intersects right interval.
     * @param enter_left  Ray entry t on left side.
     * @param enter_right Ray entry t on right side.
     */
    inline void compute_child_hit_intervals(
        const BihNode &node,
        const Ray     &ray_query,
        Float          t_max,
        bool          &hit_left,
        bool          &hit_right,
        Float         &enter_left,
        Float         &enter_right
    ) const;


    size_t      nodes_cnt() const override { return nodes.size(); }
    const char* type_name() const override { return "BIH"; }

    std::vector<BihNode> nodes;                // All BIH nodes in flat array
    Uint                 max_leaf_polygons;    // Leaf split threshold
    Float                last_build_cost  = 0; // SAH cost from last build
    Float                last_update_cost = 0; // SAH cost from last update
};
