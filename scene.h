#pragma once
// Created by Ondrej Ac (xacond00)
#include "camera.h"
#include "mesh.h"
#include "ray.h"
#include "aabb.h"
#include "poly.h"
#include "vec.h"
#include <fstream>
#include <sstream>



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
                    current.name = "default";
                    current.index_start = static_cast<Uint>(m_idx.size());
                    current.index_count = 0;
                    in_mesh = true;
                }

                auto parseIndex = [](const std::string& token) -> Uint {
                    size_t slash = token.find('/');
                    return static_cast<Uint>(std::stoul(
                        (slash == std::string::npos) ? token : token.substr(0, slash)
                    )) - 1;
                };

                std::string v1, v2, v3;
                ss >> v1 >> v2 >> v3;

                Vec3u tri{
                    parseIndex(v1),
                    parseIndex(v2),
                    parseIndex(v3)
                };
                m_idx.push_back(tri);
                current.index_count++;
            }
            else if (prefix == "o" || prefix == "g") {
                if (in_mesh) {
                    // compute mesh bbox
                    for (Uint i = current.index_start;
                         i < current.index_start + current.index_count;
                         ++i) {
                        auto& f = m_idx[i];
                        current.bbox.expand(m_vert[f[0]]);
                        current.bbox.expand(m_vert[f[1]]);
                        current.bbox.expand(m_vert[f[2]]);
                    }
                    m_mesh.push_back(current);
                }
                ss >> current.name;
                current.index_start = static_cast<Uint>(m_idx.size());
                current.index_count = 0;
                current.bbox = AABB();
                in_mesh = true;
            }
        }

        if (in_mesh) {
            for (Uint i = current.index_start;
                 i < current.index_start + current.index_count;
                 ++i) {
                auto& f = m_idx[i];
                current.bbox.expand(m_vert[f[0]]);
                current.bbox.expand(m_vert[f[1]]);
                current.bbox.expand(m_vert[f[2]]);
            }
            m_mesh.push_back(current);
        }

        return true;
    }
    std::vector<Camera> m_cam;
    std::vector<Vec3f> m_vert;
    std::vector<Vec3u> m_idx;
    std::vector<Mesh> m_mesh; 
    AABB m_bbox;
};