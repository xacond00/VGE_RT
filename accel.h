#pragma once
// Created by Ondrej Ac (xacond00)
#include "aabb.h"
#include "ray.h"
#include "scene.h"

enum Accel_t{
    None,
    Bbox,
    BVH,
    Octree,
    Blast
};

// Acceleration structure base interface
class Accel{
    public:
    Accel(const Scene &scene, Accel_t type) : m_scene(scene), m_type(type), m_built(false){
        
    }
    virtual ~Accel(){}
    virtual bool intersect(const Ray& r, HitInfo& rec)const;
    virtual bool ray_test(const Ray& r, Float t = InfF)const;
    virtual void update();
    virtual void build();
    bool built(){return m_built;}
    Accel_t type()const{return m_type;}
    protected:
    const Scene& m_scene;
    const Accel_t m_type;
    bool m_built;
};