#pragma once
// Created by Ondrej Ac (xacond00)
#include "ray.h"
#include "vec.h"
struct Film{
    Film(Uint w, Uint h) : m_data(w*h), m_dims(w,h){}
    void reset(){
        for(auto &val : m_data){
            val = Vec4f(0);
        }
    }
    Vec3f read(Vec2u xy)const{
        auto val = at(xy);
        return val.shrink() / val[3];
    }
    void put(Vec2u xy, Vec3f val, Float weight = 1){
        auto &elem = at(xy);
        elem = elem + val.append(1.f) * weight;
    }

    Vec4f at(Uint x, Uint y)const{ return m_data[x + y * dims()[0]];}
    Vec4f &at(Uint x, Uint y){ return m_data[x + y * dims()[0]];}
    Vec4f at(Vec2u xy)const{ return m_data[xy[0] + xy[1] * dims()[0]];}
    Vec4f &at(Vec2u xy){ return m_data[xy[0] + xy[1] * dims()[0]];}

    Vec2u dims()const{return m_dims;}
    auto data()const{return m_data.data();}
    auto data(){return m_data.data();}

    private:
    std::vector<Vec4f> m_data;
    Vec2u m_dims; 
};