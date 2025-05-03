#pragma once
// Created by Ondrej Ac (xacond00)
#include "vec.h"

struct Ray{
    Ray(){}
    Ray(Vec3f O, Vec3f D) : O(O), D(D), iD(rcp(D)){}
    Ray(Vec3f O, Vec3f _D, bool norm) : O(O), D(_D), iD(rcp(D)){}
    Vec3f at(Float t){
        return O + Vec3f(t) * D;
    }
    // Origin, direction, inverse direction
    Vec3f O,D,iD;
};

struct HitInfo{
    HitInfo(){}
    HitInfo(Float t, Float u, Float v, Uint idx, bool face) : tuv(t, u, v), idx(idx), face(face) {}
    Float t()const{return tuv[0];}
    Float &t(){return tuv[0];}
    Float u()const{return tuv[1];}
    Float v()const{return tuv[2];}
    Vec3f tuv = Vec3f(InfF, 0, 0);
    Uint idx = -1;
    bool face;
};
