#pragma once

#include <math.h>

#define LOOKUP_SIZE static_cast<size_t>(16384)
#define SIN_OFFSET static_cast<size_t>(LOOKUP_SIZE / 4)
#define TAU static_cast<float>(2. * M_PI)
#define ang2ind static_cast<float>(static_cast<float>(LOOKUP_SIZE) / TAU)
#define ind2ang static_cast<float>(TAU / static_cast<float>(LOOKUP_SIZE))

// this could be cut down more by storing just one quadrant at the cost of more complicated lookup ...
float lcos[LOOKUP_SIZE];

void getRad(float rad, float ret[])
{
    if (rad < 0)
    {
        rad = TAU + std::fmod(rad, -TAU);
    }
    else
    {
        rad = std::fmod(rad, TAU);
    }
    int i = ((int)(rad * ang2ind));
    // TODO interpolation
    ret[0] = lcos[i % LOOKUP_SIZE];
    ret[1] = lcos[(i + SIN_OFFSET) % LOOKUP_SIZE];
}

void getRate(float rate, float ret[])
{
    getRad(rate * M_PI, ret);
}

void initCosSinLookup()
{
    for (size_t i = 0; i < LOOKUP_SIZE; i++)
    {
        float angle = ind2ang * (float)i;
        lcos[i] = cos(angle);
    }
}
