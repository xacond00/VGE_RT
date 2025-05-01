#pragma once
#include "ray.h"
#include "vec.h"
struct AABB{

    HitInfo intersect(const Ray& r){

    }
    
    bool ray_test(const Ray& r)const{

    }


    Vec3f pmin;
    Vec3f pmax;
};