#pragma once
// Created by Ondrej Ac (xacond00)
#include "ray.h"

struct Transform{
    Transform(Vec3f P, Vec3f A){
        cols[3] = P.append(1);
        
    }
    Vec4f cols[4];
};

struct ONB{

};