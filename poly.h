#pragma once
// Created by Ondrej Ac (xacond00)
#include "ray.h"
#include "vec.h"
#include "aabb.h"
struct Poly{
    Poly(Vec3f a, Vec3f b, Vec3f c) : Q(a), U(b - a), V(c - a), N(cross(U,V)) {}
    bool intersect(const Ray& r, HitInfo& rec){

    }
    
    bool ray_test(const Ray& r)const{

    }
    Vec3f center()const{

    }

    AABB bbox()const{
        
    }

    Vec3f Q,U,V, N;
};