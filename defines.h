#pragma once
// Created by Ondrej Ac (xacond00)
#include <string>
#include <iostream>
#include <vector>
#include <array>
using Float = float;
using Uint = unsigned;
constexpr Float InfF = 1e4;
constexpr Float EpsF = 1e-4;
constexpr Float Eps6F = 1e-6;
constexpr Float PiF = 3.14159265358979323846;

inline bool within(Float t, Float min, Float max){
    return (t >= min) && (t <= max); 
}
inline bool inside(Float t, Float min, Float max){
    return (t > min) && (t < max); 
}
