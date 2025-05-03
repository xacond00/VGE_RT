#pragma once
// Created by Ondrej Ac (xacond00)
#include "ray.h"
#include "vec.h"
#include "transform.h"
#include "film.h"

struct Camera{
    Camera(Uint w, Uint h, float fov, const Transform &T = Transform()) : film(w,h), T(T), fov(0){}
    Ray sample_ray(Vec2f xy)const{
        
    }
    Film film;
    Transform T;
    float fov;
};