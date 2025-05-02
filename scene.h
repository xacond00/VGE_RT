#pragma once
#include "mesh.h"
#include "ray.h"
#include "aabb.h"
#include "poly.h"
#include "vec.h"

struct Scene {

    Poly get_poly(Uint i)const{
        auto idx = m_idx[i];
        return Poly(m_vert[idx[0]], m_vert[idx[1]], m_vert[idx[2]]);
    } 
    Uint poly_cnt()const{
        return m_idx.size();
    }
    Uint mesh_cnt()const{
        return m_mesh.size();
    }

    std::vector<Vec3f> m_vert;
    std::vector<Vec3u> m_idx;
    std::vector<Mesh> m_mesh; 
    AABB bbox;
};