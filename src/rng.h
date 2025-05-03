#include <defines.h>
#include <vec.h>
struct RNG{
    RNG(){}
    RNG(Uint seed){
        x = seed;
    }
    inline uint32_t xorshift32() {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return x;
    }
    
    inline Float rafl() {
        uint32_t x = 0x3f800000 | (xorshift32() & 0x007FFFFF);
        return *(float *)&x - 1.f;
    }

    inline Vec3f sample_cos_distribution() {
        Float r0 = rafl();
        Float r1 = rafl();
        const Float phi = Pi2F * r0;
        Vec2f r_fact = sqrt(Vec2f(r1, Float(1) - r1));
        Vec2f d = r_fact[0] * fcossin(phi);
        return d.append(r_fact[1]);
    }
    uint32_t x;
};