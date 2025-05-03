#pragma once
// Created by Ondrej Ac (xacond00)
#include "ray.h"
// Custom transform (hybrid between matrix and normal storage)
struct Transform{
    Transform(){}
    Transform(Vec3f P, Vec3f A, float S = 1.f) : P(P), A(A), S(S){
        update_Tr();
    }
    void update_Tr(){
		Vec3f sin_A = fsin(A);
		Vec3f cos_A = fcos(A);
		Tr[0][0] = cos_A.x() * cos_A.y();
		Tr[0][1] = cos_A.x() * sin_A.y() * sin_A.z() - sin_A.x() * cos_A.z();
		Tr[0][2] = cos_A.x() * sin_A.y() * cos_A.z() + sin_A.x() * sin_A.z();
		Tr[1][0] = sin_A.x() * cos_A.y();
		Tr[1][1] = sin_A.x() * sin_A.y() * sin_A.z() + cos_A.x() * cos_A.z();
		Tr[1][2] = sin_A.x() * sin_A.y() * cos_A.z() - cos_A.x() * sin_A.z();
		Tr[2][0] = -sin_A.y();
		Tr[2][1] = cos_A.y() * sin_A.z();
		Tr[2][2] = cos_A.y() * cos_A.z();
    }

    template <bool scale = false>
    Vec3f vec(Vec3f v)const{
        if constexpr(scale){
            return S * Vec3f(dot(v, Tr[0]), dot(v, Tr[1]), dot(v, Tr[2]));
        }
        return Vec3f(dot(v, Tr[0]), dot(v, Tr[1]), dot(v, Tr[2]));
    }
    template <bool scale = false>
    Vec3f point(Vec3f v)const{
        if constexpr(scale){
            return S * Vec3f(dot(v, Tr[0]), dot(v, Tr[1]), dot(v, Tr[2])) + P;
        }
        return Vec3f(dot(v, Tr[0]), dot(v, Tr[1]), dot(v, Tr[2])) + P;
    }
    Transform &rotate(Vec3f dA){
        A = A + dA;
        update_Tr();
        return *this;
    }
    Transform &translate(Vec3f dP){
        P = P + dP;
        return *this;
    }
    Transform &scale(Float dS){
        S = S * dS;
        return *this;
    }
    Transform compose(Transform other)const{
        other.A = A + other.A;
        other.P = P + other.P;
        other.S = S * other.S;
        other.update_Tr();
        return other;
    }
    // Rotation matrix, position, angle, scale
    Vec3f Tr[3] = {Vec3f(1,0,0),Vec3f(0,1,0),Vec3f(0,0,1)};
    Vec3f P =  Vec3f(0), A = Vec3f(0);
    Float S = Float(0);
};

struct ONB{

};