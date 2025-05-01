#pragma once
#include "vec.h"

struct Ray{
    Ray(){}
    Ray(Vec3f O, Vec3f D) : O(O), D(D), iD(rcp(D)){}
    Ray(Vec3f O, Vec3f D, bool no_inv) : O(O), D(D){}
    Vec3f at(Float t){
        return O + Vec3f(t) * D;
    }
    // Origin, direction, inverse direction
    Vec3f O,D,iD;
};

struct HitInfo{
    Vec3f P, N;
    float t;
    bool face;
    bool valid;
};