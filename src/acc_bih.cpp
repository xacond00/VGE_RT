/**
 * @file acc_bih.cpp
 * @brief Bounding Interval Hierarchy (BIH) acceleration structure implementation.
 * @author Marek Konečný
 * @date 2025-05-07
 */
#include "acc_bih.h"
#include <algorithm>

bool AccelBih::intersect(const Ray &ray_query, HitInfo &hit_info) const
{
    struct stack_item { Uint node_index; Float entry_t; };
    constexpr Uint STACK_CAP = 64;
    stack_item stack[STACK_CAP];
    Uint sp = 0;

    // Root-box test
    if (!nodes[0].bounds.ray_test(ray_query, hit_info.t()))
        return false;
    stack[sp++] = {0, hit_info.t()};

    bool any_hit = false;
    while (sp) {
        auto [node_idx, t_cur] = stack[--sp];
        if (t_cur > hit_info.t()) continue;

        const BihNode &node = nodes[node_idx];
        if (node.is_leaf()) {
            // test each polygon
            for (Uint i = node.index_range[0]; i < node.index_range[1]; ++i)
                if (poly(i).intersect(ray_query, hit_info))
                    any_hit = true;
            continue;
        }

        // compute child intervals
        bool hit_left, hit_right;
        Float enter_left, enter_right;
        compute_child_hit_intervals(node, ray_query, hit_info.t(),
                                   hit_left, hit_right,
                                   enter_left, enter_right);

        Uint left_child  = node.index_range[1];
        Uint right_child = node.index_range[0];
        // push near-first
        if (hit_left && hit_right) {
            if (enter_left < enter_right) {
                stack[sp++] = { right_child, enter_right };
                stack[sp++] = { left_child,  enter_left  };
            } else {
                stack[sp++] = { left_child,  enter_left  };
                stack[sp++] = { right_child, enter_right };
            }
        } else if (hit_left) {
            stack[sp++] = { left_child, enter_left };
        } else if (hit_right) {
            stack[sp++] = { right_child, enter_right };
        }
    }
    return any_hit;
}


bool AccelBih::ray_test(const Ray &ray_query, Float t_max) const
{
    constexpr Uint STACK_CAP = 64;
    Uint stack[STACK_CAP];
    Uint sp = 0;

    if (!nodes[0].bounds.ray_test(ray_query, t_max))
        return false;
    stack[sp++] = 0;

    while (sp) {
        const BihNode &node = nodes[stack[--sp]];
        if (node.is_leaf()) {
            for (Uint i = node.index_range[0]; i < node.index_range[1]; ++i)
                if (poly(i).ray_test(ray_query, t_max))
                    return true;
            continue;
        }

        bool hit_left, hit_right;
        Float enter_left, enter_right;
        compute_child_hit_intervals(node, ray_query, t_max, hit_left, hit_right, enter_left, enter_right);

        Uint left_child  = node.index_range[1];
        Uint right_child = node.index_range[0];
        if (hit_left)  stack[sp++] = left_child;
        if (hit_right) stack[sp++] = right_child;
    }
    return false;
}


void AccelBih::build()
{
    double build_timer = timer();

    nodes.clear();
    nodes.reserve(m_scene.poly_cnt());
    // root node contains all polygons
    nodes.emplace_back(m_scene.m_bbox, Vec2u(0, m_scene.poly_cnt()));

    Float total_cost = 0.0f;
    build_node_recursive(0, total_cost);
    last_build_cost  = total_cost;
    last_update_cost = total_cost;
    m_built          = true;

    last_build_time = timer(build_timer);
}


// SAH splitting & binning
std::pair<Float,Uint> AccelBih::find_split_sah(const Vec2u &index_range, const AABB  &bounds, uint8_t &out_axis, Float &out_plane)
{
    struct BinLayer   { AABB bound; Uint count = 0; };
    struct SplitLayer { Float area = 0; Uint count = 0; };

    constexpr Uint BINS = 32;
    Float best_cost = InfF;
    out_axis  = 0;
    out_plane = (bounds.pmin[0] + bounds.pmax[0]) * 0.5f;

    for (Uint axis = 0; axis < 3; ++axis) {
        BinLayer bins[BINS];
        Float extent = bounds.pmax[axis] - bounds.pmin[axis];
        if (extent < 1e-6f) continue;
        Float inv_scale = BINS / extent;

        // bin by centroid
        for (Uint i = index_range[0]; i < index_range[1]; ++i) {
            auto tri_bb = vert(i).bbox();
            Int bidx = Int((tri_bb.center()[axis] - bounds.pmin[axis]) * inv_scale);
            bidx = std::clamp(bidx, 0, Int(BINS - 1));
            bins[bidx].count++;
            bins[bidx].bound.expand(tri_bb);
        }

        // prefix/suffix sweeps
        SplitLayer left_sl[BINS-1], right_sl[BINS-1];
        BinLayer acc_l, acc_r;
        for (Uint i = 0; i < BINS-1; ++i) {
            acc_l.bound.expand(bins[i].bound);
            acc_l.count += bins[i].count;
            left_sl[i]  = { acc_l.bound.area(), acc_l.count };

            acc_r.bound.expand(bins[BINS-1-i].bound);
            acc_r.count += bins[BINS-1-i].count;
            right_sl[BINS-2-i] = { acc_r.bound.area(), acc_r.count };
        }

        // evaluate plane positions
        Float step = extent / BINS;
        Float plane_pos = bounds.pmin[axis] + step;
        for (Uint i = 0; i < BINS-1; ++i, plane_pos += step) {
            if (left_sl[i].count == 0 || right_sl[i].count == 0) continue;
            Float cost = left_sl[i].area * left_sl[i].count + right_sl[i].area * right_sl[i].count;

            if (cost < best_cost) {
                best_cost = cost;
                out_axis  = uint8_t(axis);
                out_plane = plane_pos;
            }
        }
    }

    // partition and fallback if degenerate
    Uint split_idx = partition_polygons(index_range, out_axis, out_plane);
    if (split_idx == index_range[0] || split_idx == index_range[1]) {
        split_idx = (index_range[0] + index_range[1]) >> 1;
        std::nth_element(
            m_poly.begin() + index_range[0],
            m_poly.begin() + split_idx,
            m_poly.begin() + index_range[1],
            [axis=out_axis,this](Uint a, Uint b){
                return vert(a).bbox().center()[axis] < vert(b).bbox().center()[axis];
            }
        );
        best_cost = 0.0f;  // force subdivision
    }
    return { best_cost, split_idx };
}


Uint AccelBih::partition_polygons(const Vec2u &index_range, uint8_t axis, Float plane_split)
{
    Int i = index_range[0];
    Int j = index_range[1] - 1;
    while (i <= j) {
        if (vert(i).bbox().center()[axis] < plane_split)
            ++i;
        else
            std::swap(m_poly[i], m_poly[j--]);
    }
    return i;
}


// Recursive build
void AccelBih::build_node_recursive(Uint node_index, Float &cost_accum)
{
    BihNode &node = nodes[node_index];
    const AABB &bb = node.bounds;
    Uint start = node.index_range[0];
    Uint end   = node.index_range[1];
    Uint cnt   = end - start;
    Float leaf_cost = bb.area() * cnt;

    if (cnt <= max_leaf_polygons) {
        cost_accum += leaf_cost;
        return;
    }

    uint8_t axis;
    Float plane;
    auto [sah_cost, mid] = find_split_sah(node.index_range, bb, axis, plane);
    if (mid == start || mid == end || sah_cost >= leaf_cost) {
        cost_accum += leaf_cost;
        return;
    }

    // compute split-interval bounds
    Float lmax = -InfF, rmin = InfF;
    for (Uint i = start; i < mid; ++i)
        lmax = std::max(lmax, vert(i).bbox().pmax[axis]);
    for (Uint i = mid; i < end;   ++i)
        rmin = std::min(rmin, vert(i).bbox().pmin[axis]);

    node.split_axis = axis;
    node.left_max   = lmax;
    node.right_min  = rmin;

    // create children
    Vec2u left_range(start, mid), right_range(mid, end);
    Uint left_idx  = nodes.size();
    nodes.emplace_back(bbox_in(left_range),  left_range);
    Uint right_idx = nodes.size();
    nodes.emplace_back(bbox_in(right_range), right_range);

    node.index_range = Vec2u(right_idx, left_idx);

    build_node_recursive(left_idx,  cost_accum);
    build_node_recursive(right_idx, cost_accum);
}


// Helper to compute child interval bounds
inline void AccelBih::compute_child_hit_intervals(
    const BihNode &node,
    const Ray     &r,
    Float          t_max,
    bool          &hit_left,
    bool          &hit_right,
    Float         &enter_left,
    Float         &enter_right) const
{
    const auto &o   = r.O;
    const auto &inv = r.iD;
    uint8_t axis    = node.split_axis;
    uint8_t a1      = (axis + 1) % 3;
    uint8_t a2      = (axis + 2) % 3;
    const auto &pmin = node.bounds.pmin;
    const auto &pmax = node.bounds.pmax;

    // left interval
    Float t0 = (pmin[axis]   - o[axis]) * inv[axis];
    Float t1 = (node.left_max - o[axis]) * inv[axis];
    if (t0 > t1)
        std::swap(t0, t1);
    hit_left   = !(t0 > t_max || t1 < 0.0f);
    enter_left = t0;

    if (hit_left) {
        Float tmin = t0, tmax = t1;
        // slab a1
        Float u0 = (pmin[a1] - o[a1]) * inv[a1];
        Float u1 = (pmax[a1] - o[a1]) * inv[a1];
        if (u0 > u1) std::swap(u0, u1);
        tmin = std::max(tmin, u0);
        tmax = std::min(tmax, u1);

        if (tmin > tmax) {
            hit_left = false;
        } else {
            // slab a2
            Float v0 = (pmin[a2] - o[a2]) * inv[a2];
            Float v1 = (pmax[a2] - o[a2]) * inv[a2];
            if (v0 > v1) std::swap(v0, v1);
            tmin = std::max(tmin, v0);
            tmax = std::min(tmax, v1);
            hit_left = (tmin <= tmax);
            if (hit_left)
                enter_left = tmin;
        }
    }

    // right interval
    Float r0 = (node.right_min - o[axis]) * inv[axis];
    Float r1 = (pmax[axis]   - o[axis]) * inv[axis];
    if (r0 > r1)
        std::swap(r0, r1);
    hit_right   = !(r0 > t_max || r1 < 0.0f);
    enter_right = r0;

    if (hit_right) {
        Float tmin = r0, tmax = r1;
        Float u0 = (pmin[a1] - o[a1]) * inv[a1];
        Float u1 = (pmax[a1] - o[a1]) * inv[a1];
        if (u0 > u1) std::swap(u0, u1);
        tmin = std::max(tmin, u0);
        tmax = std::min(tmax, u1);

        if (tmin > tmax) {
            hit_right = false;
        } else {
            Float v0 = (pmin[a2] - o[a2]) * inv[a2];
            Float v1 = (pmax[a2] - o[a2]) * inv[a2];
            if (v0 > v1) std::swap(v0, v1);
            tmin = std::max(tmin, v0);
            tmax = std::min(tmax, v1);
            hit_right = (tmin <= tmax);
            if (hit_right)
                enter_right = tmin;
        }
    }
}


// Refit the BIH for dynamic scene
void AccelBih::update()
{
    Float refit_cost = 0.0f;
    // walk backwards so parents can use updated child bounds
    for (size_t idx = nodes.size(); idx-- > 0; ) {
        BihNode &node = nodes[idx];
        if (node.is_leaf()) {
            // recompute leaf bounds
            node.bounds = bbox_in(node.index_range);
            refit_cost += node.bounds.area() * Float(node.index_range[1] - node.index_range[0]);
        
        } else if (node.is_internal()) {
            const BihNode &l = nodes[node.index_range[0]];
            const BihNode &r = nodes[node.index_range[1]];
            node.bounds     = l.bounds + r.bounds;
            node.left_max   = l.bounds.pmax[node.split_axis];
            node.right_min  = r.bounds.pmin[node.split_axis];
        }
    }

    // rebuild if refit is too expensive
    if (refit_cost > 1.2f * last_build_cost)
        build();
    else
        last_update_cost = refit_cost;

    m_built = true;
}