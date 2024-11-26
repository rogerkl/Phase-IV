#pragma once

#define _USE_MATH_DEFINES
#include <arm_math.h>
#include <complex>
#include <math.h>

class DCBlocker
{
  private:
    float32_t xm1;
    float32_t ym1;

  public:
    void process(const float32_t in[], float32_t out[], size_t frames)
    {
        for (size_t f = 0; f < frames; f++)
        {
            out[f] = in[f] - xm1 + 0.94 * ym1;
            xm1 = in[f];
            ym1 = out[f];
        }
    }
};