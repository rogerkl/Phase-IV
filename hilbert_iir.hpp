#pragma once

#define _USE_MATH_DEFINES
#include <arm_math.h>
#include <complex>
#include <math.h>

// NB SR (samplerate) and BS (blocksize) must be defined prior to inclusion of this file
// TODO check if it possible to use templates

// pre calculated coefficients 2*4 coeffs - bandwith 20hz - (SR/2-20hz)

/*
./HilbertDesign 8 8000
Phase reference path c coefficients:
0.32823234422649566477,0.74474035919882020096,0.92584116731343335527,0.98963406226209460481,
+90 deg path c coefficients:
0.09911943454055793801,0.56580797760554835119,0.85880325902190268739,0.96478877491639269959,

./HilbertDesign 8 16000
Phase reference path c coefficients:
0.39075029998604082770,0.80830222823261088649,0.95326097992242819767,0.99412806118429197788,
+90 deg path c coefficients:
0.12324566451434042225,0.64085754401172823069,0.90331544865377699782,0.97923935976614651100,

./HilbertDesign 8 22050
Phase reference path c coefficients:
0.41930867829400075841,0.83274163605261197763,0.96238521986186009727,0.99550632642863068344,
+90 deg path c coefficients:
0.13504962149638541358,0.67230881543061549177,0.91914544737832959420,0.98380830943492758589,

./HilbertDesign 8 32000
Phase reference path c coefficients:
0.45193438094176713316,0.85759393063732336504,0.97083155043884861168,0.99671622833513739970,
+90 deg path c coefficients:
0.14921497977618233821,0.70617154919183322548,0.93444577489711666551,0.98789885674035038399,

./HilbertDesign 8 44100
Phase reference path c coefficients:
0.47944111608296202665,0.87624358989504858020,0.97660296916871658368,0.99749940412203375040,
+90 deg path c coefficients:
0.16177741706363166219,0.73306690130335572242,0.94536301966806279840,0.99060051416704042460,

./HilbertDesign 8 48000
Phase reference path c coefficients:
0.48660436861367767358,0.88077943527246449484,0.97793125561632343601,0.99767386185073303473,
+90 deg path c coefficients:
0.16514909355907719801,0.73982901254452670958,0.94794090632917971107,0.99120971270525837227,

./HilbertDesign 8 88200
Phase reference path c coefficients:
0.53657167370137603957,0.90905654392691770393,0.98552189956921232294,0.99862097908663194357,
+90 deg path c coefficients:
0.18994027213428349432,0.78430572131109466127,0.96327784869510013621,0.99458100007596472736,

./HilbertDesign 8 96000
Phase reference path c coefficients:
0.54331142587629943641,0.91245076641443212395,0.98635050978014437995,0.99871842272710276145,
+90 deg path c coefficients:
0.19346824887735480925,0.78995292705630404395,0.96502852355058721390,0.99493573378216093595,
*/

// NB Coeffs are initialized squared as this is the way they are used

#if SR == 8000
constexpr float32_t coeffs[2][4] = {{0.32823234422649566477,  //
                                     0.74474035919882020096,  //
                                     0.92584116731343335527,  //
                                     0.98963406226209460481}, //
                                    {0.09911943454055793801,  //
                                     0.56580797760554835119,  //
                                     0.85880325902190268739,  //
                                     0.96478877491639269959}};
#elif SR == 16000
constexpr float32_t coeffs[2][4] = {{0.39075029998604082770,  //
                                     0.80830222823261088649,  //
                                     0.95326097992242819767,  //
                                     0.99412806118429197788}, //
                                    {0.12324566451434042225,  //
                                     0.64085754401172823069,  //
                                     0.90331544865377699782,  //
                                     0.97923935976614651100}};
#elif SR == 22050
constexpr float32_t coeffs[2][4] = {{0.41930867829400075841,  //
                                     0.83274163605261197763,  //
                                     0.96238521986186009727,  //
                                     0.99550632642863068344}, //
                                    {0.13504962149638541358,  //
                                     0.67230881543061549177,  //
                                     0.91914544737832959420,  //
                                     0.98380830943492758589}};
#elif SR == 32000
constexpr float32_t coeffs[2][4] = {{0.45193438094176713316,  //
                                     0.85759393063732336504,  //
                                     0.97083155043884861168,  //
                                     0.99671622833513739970}, //
                                    {0.14921497977618233821,  //
                                     0.70617154919183322548,  //
                                     0.93444577489711666551,  //
                                     0.98789885674035038399}};
#elif SR == 44100
constexpr float32_t coeffs[2][4] = {{0.47944111608296202665,  //
                                     0.87624358989504858020,  //
                                     0.97660296916871658368,  //
                                     0.99749940412203375040}, //
                                    {0.16177741706363166219,  //
                                     0.73306690130335572242,  //
                                     0.94536301966806279840,  //
                                     0.99060051416704042460}};
#elif SR == 48000
constexpr float32_t coeffs[2][4] = {{0.48660436861367767358,  //
                                     0.88077943527246449484,  //
                                     0.97793125561632343601,  //
                                     0.99767386185073303473}, //
                                    {0.16514909355907719801,  //
                                     0.73982901254452670958,  //
                                     0.94794090632917971107,  //
                                     0.99120971270525837227}};
#elif SR == 88200
constexpr float32_t coeffs[2][4] = {{0.53657167370137603957,  //
                                     0.90905654392691770393,  //
                                     0.98552189956921232294,  //
                                     0.99862097908663194357}, //
                                    {0.18994027213428349432,  //
                                     0.78430572131109466127,  //
                                     0.96327784869510013621,  //
                                     0.99458100007596472736}};
#elif SR == 96000
constexpr float32_t coeffs[2][4] = {{0.54331142587629943641,  //
                                     0.91245076641443212395,  //
                                     0.98635050978014437995,  //
                                     0.99871842272710276145}, //
                                    {0.19346824887735480925,  //
                                     0.78995292705630404395,  //
                                     0.96502852355058721390,  //
                                     0.99493573378216093595}};
#endif

/*
     ................ filter 1 .................
   +--> allpass --> allpass --> allpass --> allpass --> delay --> out1
   |
  in
   |    ................ filter 2 .................
   +--> allpass --> allpass --> allpass --> allpass ------------> out2 (+90 deg)

   allpass = out(t) = a^2*(in(t) + out(t-2)) - in(t-2)
*/

class HilbertIir
{
  private:
    float32_t inBuff[2][4][2];
    float32_t outBuff[2][4][2];
    float32_t out1delay1 = 0;
    float32_t tmp[BS];

    void processAllPass(const float32_t in[BS], float32_t out[BS], int filter, int number, size_t frames)
    {
        for (size_t i = 0; i < frames; i++)
        {
            // out(t) = a^2*(in(t) + out(t-2)) - in(t-2)
            out[i] = coeffs[filter][number] * (in[i] + outBuff[filter][number][1]) - inBuff[filter][number][1];
            outBuff[filter][number][1] = outBuff[filter][number][0];
            outBuff[filter][number][0] = out[i];
            inBuff[filter][number][1] = inBuff[filter][number][0];
            inBuff[filter][number][0] = in[i];
        }
    }

  public:
    void process(const float32_t in[BS], float32_t out[2][BS], size_t frames)
    {
        // TODO It wouldn't surprise me if this could be optimized more ;-) ......
        // filter1
        processAllPass(in, tmp, 0, 0, frames);
        processAllPass(tmp, out[0], 0, 1, frames);
        processAllPass(out[0], tmp, 0, 2, frames);
        processAllPass(tmp, out[0], 0, 3, frames);
        // delay
        std::copy(out[0], out[0] + (frames - 1), tmp);
        tmp[0] = out1delay1;
        out1delay1 = out[0][frames - 1];
        std::copy(tmp, tmp + frames, out[0]);
        // filter2
        processAllPass(in, tmp, 1, 0, frames);
        processAllPass(tmp, out[1], 1, 1, frames);
        processAllPass(out[1], tmp, 1, 2, frames);
        processAllPass(tmp, out[1], 1, 3, frames);
    }
};