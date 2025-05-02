#pragma once
// Created by Ondrej Ac (xacond00)

#include "scene.h"
#include "accel.h"
#include "acc_none.h"
#include "acc_bbox.h"
#include "acc_bvh.h"
#include "camera.h"

class Renderer{
    // TODO: Implement each pointer cast
    // Dynamically dispatches the correct acceleration structure.
    // To avoid virtual call on each ray intersection.
    void render(){
        if(auto acc = dynamic_cast<AccelNone *>(m_acc)){
            render_internal(*acc);
        }
        else if(auto acc = dynamic_cast<AccelBbox *>(m_acc)){
            render_internal(*acc);
        }
        else if(auto acc = dynamic_cast<AccelBvh *>(m_acc)){
            render_internal(*acc);
        }
        else if(auto acc = dynamic_cast<Accel *>(m_acc)){
            render_internal(*acc);
        }
        else{
            std::cout << "Invalid acceleration structure !";
        }
    }
    // Renders the image
    template<class Acc>
    void render_internal(const Acc &acc){
        
    }
    template<class Acc>
    Vec3f sample(const Acc &acc, Ray r)const{
        HitInfo rec;
        Vec3f result(0);
        Vec3f weight(1);
        Uint depth = m_depth;
        while(depth > 0){
            bool hit = acc.intersect(r, rec);
            if(!hit){ // Sky
                float val = std::pow(::pow(fmaxf(r.D.z(), 0), 32),32);
                result = result + weight * lerp(Vec3f(0.5,0.8,1.0), 5.f * Vec3f(1,1,0.2), Vec3f(val)); 
                break;
            }
            else{
                SurfaceInfo si = scene.surface_info(rec);
                r.O = si.P;
                r.D = si.N; // Replace with random reflection
                // emission would go here ... result += weight * emiss
                weight = weight * 0.9f;
            }
            depth--;
        }
        return result;
    }

    Scene scene;
    Camera cam;
    Accel* m_acc;
    Uint m_depth = 10;
};