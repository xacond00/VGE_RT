#pragma once
// Created by Ondrej Ac (xacond00)
#include "ray.h"
#include "vec.h"
#include "transform.h"
#include "film.h"

struct Camera{
    Camera(){}
    Camera(Uint w, Uint h, Float fov, const Transform &T = Transform()) : film(w,h), T(T), fov(fov), tfov(tan(0.5f * torad(fov))) {
        update();
    }
    Ray sample_ray(Vec2f xy)const{
        Vec3f lD = norm(sample_film(xy));
        return Ray(T.P, T.vec(lD));
    }

    Vec2u film_size()const{
        return film.dims();
    }
    inline Vec3f sample_film(Vec2f xy) const {
		xy = 2.f * xy * iwh - 1.f;
		xy = xy * scl;
		return Vec3f(xy[0], xy[1], -1);
	}
    void update(){
        wh = film.fdims();
        asp = wh[0] / wh[1];
        fov = fminf(179.9, fmaxf(0.1, fov));
        tfov = tan(0.5f * torad(fov));
        hfov = todeg(2 * atan(tfov * asp));
        iwh = Vec2f(1.0) / wh;
        scl = tfov * Vec2f(asp, -1);
    }
    Film film;
    Transform T;
    Vec2f scl;
    Vec2f wh, iwh;
    Float fov, hfov;
    Float tfov;
    Float asp;
};