#pragma once

#include <math.h>

template <size_t SAMPLERATE, size_t OVERSAMPLING = 1> class SvfOsc
{
  private:
    static const int driftCorr = 10;
    float_t fq = 0; // frequency
    float_t f = 0;  // freq increment
    bool sign = true;
    float_t _sin = 0.;
    float_t _cos = 1.;
    int driftCount = 0;

    void stabilize()
    {
        if (_cos > 1.)
        {
            _cos = 1;
            _sin = 0;
            driftCount = 0;
        }
        else if (_cos < -1.)
        {
            _cos = -1;
            _sin = 0;
            driftCount = 0;
        }
        else if (_sin > 1.)
        {
            _cos = 0;
            _sin = 1;
            driftCount = 0;
        }
        else if (_sin < -1.)
        {
            _cos = 0;
            _sin = -1;
            driftCount = 0;
        }
        else if (driftCount >= driftCorr)
        {
            float_t g = 0.5 * (3 - (_cos * _cos + _sin * _sin));
            _cos *= g;
            _sin *= g;
            driftCount = 0;
        }
    }

    void _next()
    {
        if (sign)
        {
            _sin = _sin + f * _cos;
            _cos = _cos - f * _sin;
        }
        else
        {
            _sin = _sin - f * _cos;
            _cos = _cos + f * _sin;
        }
        stabilize();
    }

    void calcFreq()
    {
        f = std::abs(2. * M_PI * (fq / (float_t)(SAMPLERATE * OVERSAMPLING)));
    }

  public:
    SvfOsc()
    {
        setFreq(1);
    }

    SvfOsc(float_t freq)
    {
        setFreq(freq);
    }

    void next()
    {
        for (size_t i = 0; i < OVERSAMPLING; i++)
        {
            _next();
        }
    }

    void setFreq(float_t freq)
    {
        fq = freq;
        calcFreq();
    }

    float_t sin()
    {
        return _sin;
    }

    float_t cos()
    {
        return _cos;
    }
};
