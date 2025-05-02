#pragma once
#include "aabb.h"
#include "ray.h"
#include "scene.h"
class Accel{
    public:
    Accel(const Scene &scene) : m_scene(scene), m_built(false){
        Uint size = m_scene.poly_cnt();
        m_poly_idx.reserve(size);
        for(Uint i = 0; i < size; i++){
            m_poly_idx.emplace_back(i);
        }
    }
    virtual ~Accel(){}
    virtual bool intersect(const Ray& r, HitInfo& rec)const;
    virtual bool ray_test(const Ray& r)const;
    virtual void update();
    virtual void build();
    bool built(){return m_built;}
    protected:
    std::vector<Uint> m_poly_idx;
    const Scene& m_scene;
    bool m_built;
};