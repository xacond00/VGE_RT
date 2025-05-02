#pragma once
#include "ray.h"
#include "vec.h"
#include "aabb.h"
struct Poly{
    Poly(Vec3f a, Vec3f b, Vec3f c) : q(a), u(b - a), v(c - a) {}
    bool intersect(const Ray& r, HitInfo& rec){

    }
    
    bool ray_test(const Ray& r)const{

    }
    Vec3f center()const{

    }

    AABB bbox()const{
        
    }

    Vec3f q,u,v;
};