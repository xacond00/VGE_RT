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
        if(auto acc = dynamic_cast<Accel *>(m_acc)){
            render_internal(*acc);
        }
        else if(auto acc = dynamic_cast<AccelNone *>(m_acc)){
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
    Scene scene;
    Camera cam;
    Accel* m_acc;
};