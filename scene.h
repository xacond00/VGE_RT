#pragma once
// Created by Ondrej Ac (xacond00)
#include "mesh.h"
#include "ray.h"
#include "aabb.h"
#include "poly.h"
#include "vec.h"
#include <fstream>
#include <sstream>


// Scene stores data and provides method to construct a triangle
struct Scene {
    // Get i-th polygon from array
    inline Poly get_poly(Uint i)const{
        auto idx = m_poly[i];
        return Poly(m_vert[idx[0]], m_vert[idx[1]], m_vert[idx[2]], i);
    }
    // Intersect i-th polygon from array
    inline bool intersect(Uint i, const Ray& r, HitInfo &rec)const{return get_poly(i).intersect(r, rec);}
    // Ray-test i-th polygon from array
    inline bool ray_test(Uint i, const Ray& r, Float t)const{return get_poly(i).ray_test(r, t);}
    // Finalize surface rec of i-th polygon from array
    SurfaceInfo surface_info(Uint i, const HitInfo &rec) const {
		return get_poly(i).surface_info(rec);
	}
    // Get vertices of i-th polygon
    Vert3 get_vert(Uint i) const{
        auto idx = m_poly[i];
        return {m_vert[idx[0]], m_vert[idx[1]], m_vert[idx[2]]};
    }

    Uint poly_cnt()const{
        return m_poly.size();
    }

    Uint mesh_cnt()const{
        return m_mesh.size();
    }

    AABB bbox_in(Vec2u range){
        return bbox_in(range[0], range[1]);
    }

    AABB bbox_in(Uint beg, Uint end)const{
        Vec3f pmin(InfF);
        Vec3f pmax(-InfF);
        while(beg < end){
            auto V = get_vert(beg);
            for(Uint i = 0; i < 3; i++){
                pmin = min(pmin, V.data[i]);
                pmax = max(pmax, V.data[i]);
            }
            beg++;
        }
        return AABB(pmin, pmax);
    }

    AABB bbox_in(const std::vector<Uint> &poly_idx)const{
        Vec3f pmin(InfF);
        Vec3f pmax(-InfF);
        for(auto i : poly_idx){
            auto V = get_vert(i);
            for(Uint i = 0; i < 3; i++){
                pmin = min(pmin, V.data[i]);
                pmax = max(pmax, V.data[i]);
            }
        }
        return AABB(pmin, pmax);
    }

    void update_bbox(){
        m_bbox = bbox_in(0, poly_cnt());
    }

    void update_mesh(Uint i){
        m_mesh[i].m_bbox = bbox_in(m_mesh[i].range()); 
    }

    bool hit_bbox(const Ray& r, Float max_t = InfF)const{
        return m_bbox.ray_test(r, max_t);
    }

    bool load_obj(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) return false;

        auto trim = [](const std::string& s) -> std::string {
            size_t start = s.find_first_not_of(" \t\r\n");
            size_t end   = s.find_last_not_of(" \t\r\n");
            return (start == std::string::npos)
                ? std::string()
                : s.substr(start, end - start + 1);
        };
        auto parseIndex = [](const std::string& token) -> Uint {
            size_t slash = token.find('/');
            return (std::stoul(
                (slash == std::string::npos) ? token : token.substr(0, slash)
            )) - 1;
        };

        std::string line;
        Mesh current;
        bool in_mesh = false;

        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;

            std::istringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                Vec3f v;
                ss >> v[0] >> v[1] >> v[2];
                m_vert.push_back(v);
                m_bbox.expand(v);
            }
            else if (prefix == "f") {
                if (!in_mesh) {
                    current.m_name = "default";
                    current.m_off = (m_poly.size());
                    current.m_cnt = 0;
                    in_mesh = true;
                }

                std::string v1, v2, v3;
                ss >> v1 >> v2 >> v3;

                Vec3u tri{
                    parseIndex(v1),
                    parseIndex(v2),
                    parseIndex(v3)
                };
                m_poly.push_back(tri);
                current.m_cnt++;
            }
            else if (prefix == "o" || prefix == "g") {
                if (in_mesh) {
                    current.m_bbox = bbox_in(current.range());
                    m_mesh.push_back(current);
                }
                ss >> current.m_name;
                current.m_off = (m_poly.size());
                current.m_cnt = 0;
                current.m_bbox = AABB();
                in_mesh = true;
            }
        }

        if (in_mesh) {
            current.m_bbox = bbox_in(current.range());
            m_mesh.push_back(current);
        }

        return true;
    }

    std::vector<Vec3f> m_vert;
    std::vector<Vec3u> m_poly;
    std::vector<Mesh> m_mesh; 
    AABB m_bbox;
};