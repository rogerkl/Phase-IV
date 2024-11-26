#define BS 1
#define SR 48000

#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "dc_blocker.hpp"
#include "hilbert_iir.hpp"
#include "svf_osc.hpp"
#include <random>
#include <string>

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

#if SR == 8000
const SaiHandle::Config::SampleRate sampleRate = SaiHandle::Config::SampleRate::SAI_8KHZ;
#elif SR == 16000
const SaiHandle::Config::SampleRate sampleRate = SaiHandle::Config::SampleRate::SAI_16KHZ;
#elif SR == 32000
const SaiHandle::Config::SampleRate sampleRate = SaiHandle::Config::SampleRate::SAI_32KHZ;
#elif SR == 48000
const SaiHandle::Config::SampleRate sampleRate = SaiHandle::Config::SampleRate::SAI_48KHZ;
#elif SR == 96000
const SaiHandle::Config::SampleRate sampleRate = SaiHandle::Config::SampleRate::SAI_96KHZ;
#endif

#define MODE_PHASE_MOD_1 0
#define MODE_PHASE_MOD_2 1
#define MODE_PHASE_MOD_3 2
#define MODE_PHASE_MOD_4 3
#define MODE_FREQUENCY_SHIFT 4
#define MODE_PM_MODOUT 5
#define MODE_FS_MODOUT 6
#define NUM_MODES 7

int mode = MODE_PHASE_MOD_4;

DaisyPatchSM patch;
Switch toggleSwitch;
Switch miniButton;
bool m2s;
bool mbWasPressed;

#define CR static_cast<size_t>(SR / BS)

HilbertIir hilbertM2S;

HilbertIir hilbertL1;
HilbertIir hilbertR1;
HilbertIir hilbertL2;
HilbertIir hilbertR2;
HilbertIir hilbertL3;
HilbertIir hilbertR3;
HilbertIir hilbertL4;
HilbertIir hilbertR4;

float frequencyL = 3.;
float frequencyR = 3.;
float depthL = 0.5;
float depthR = 0.5;

SvfOsc<SR, 8> svfOscL;
SvfOsc<SR, 8> svfOscR;
DCBlocker dcBlocker1;
DCBlocker dcBlocker2;

// low pass ingoing
Svf lp1L;
Svf lp1R;
// low pass outgoing
Svf lp2L;
Svf lp2R;

// high pass ingoing
Svf hp1L;
Svf hp1R;
// high pass outgoing
Svf hp2L;
Svf hp2R;

float cv1_norm = 0.5; // freq
float cv2_norm = 0.5; // fine tune
float cv3_norm = 0.;  // depth
float cv4_norm = 0.;  // cv depth

float cv1_alt = 1.;  // volume out
float cv2_alt = 0.5; // mode
float cv3_alt = 0.;  // hp freq
float cv4_alt = 1.;  // lp freq

int cv1_100_norm = 0;
int cv2_100_norm = 0;
int cv3_100_norm = 0;
int cv4_100_norm = 0;

int cv1_100_alt = 0;
int cv2_100_alt = 0;
int cv3_100_alt = 0;
int cv4_100_alt = 0;

#define MAX_FILTER 20000.f
#define MIN_FILTER 20.f
float lpf = MAX_FILTER;
float hpf = MIN_FILTER;

#define volume_out cv1_alt

#include "sin_lookup.hpp"

bool modIn = false;

void ProcessControls();

void setHpFreq(float hpf)
{
    hp1L.SetFreq(hpf);
    hp1R.SetFreq(hpf);
    hp2L.SetFreq(hpf);
    hp2R.SetFreq(hpf);
}

void setLpFreq(float lpf)
{
    lp1L.SetFreq(lpf);
    lp1R.SetFreq(lpf);
    lp2L.SetFreq(lpf);
    lp2R.SetFreq(lpf);
}

static void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    ProcessControls();
    static float32_t tmpL[BS];
    static float32_t tmpR[BS];
    static float32_t tmpHL[2][BS];
    static float32_t tmpHR[2][BS];
    static float32_t oscSin[2][BS];
    static float32_t oscCos[2][BS];
    static float32_t lookupCosSin[2][2];

    if (mode == MODE_PM_MODOUT || mode == MODE_FS_MODOUT)
    {
        for (size_t i = 0; i < size; i++)
        {
            svfOscL.next();
            svfOscR.next();
            if (mode == MODE_FS_MODOUT && !modIn)
            {
                out[0][i] = svfOscL.cos() * volume_out;
                out[1][i] = svfOscL.sin() * volume_out;
            }
            else
            {
                // NB if modulation from input we can't really make sure amplitude is correct
                // so we probably don't get correct frequency shifter balance
                getRate((modIn ? in[1][i] : svfOscL.sin()) * (mode == MODE_FS_MODOUT ? 1. : depthL), lookupCosSin[0]);
                out[0][i] = lookupCosSin[0][0] * volume_out;
                out[1][i] = lookupCosSin[0][1] * volume_out;
            }
        }
    }
    else
    {
        if (m2s || modIn)
        {
            for (size_t i = 0; i < size; i++)
            {
                hp1L.Process(in[0][i]);
                tmpL[i] = hp1L.High();
                lp1L.Process(tmpL[i]);
                tmpL[i] = lp1L.Low();
            }

            hilbertM2S.process(tmpL, tmpHL, size);

            for (size_t i = 0; i < size; i++)
            {
                tmpL[i] = tmpHL[0][i];
                tmpR[i] = tmpHL[1][i];
            }
        }
        else
        {
            for (size_t i = 0; i < size; i++)
            {
                hp1L.Process(in[0][i]);
                hp1R.Process(in[1][i]);
                tmpL[i] = hp1L.High();
                tmpR[i] = hp1R.High();

                lp1L.Process(tmpL[i]);
                lp1R.Process(tmpR[i]);
                tmpL[i] = lp1L.Low();
                tmpR[i] = lp1R.Low();
            }
        }
        hilbertL1.process(tmpL, tmpHL, size);
        hilbertR1.process(tmpR, tmpHR, size);

        for (size_t i = 0; i < size; i++)
        {
            svfOscL.next();
            svfOscR.next();
            if (mode == MODE_FREQUENCY_SHIFT && !modIn)
            {
                oscSin[0][i] = svfOscL.sin();
                oscCos[0][i] = svfOscL.cos();
                oscSin[1][i] = svfOscR.sin();
                oscCos[1][i] = svfOscR.cos();
            }
            else
            {
                getRate((modIn ? in[1][i] : svfOscL.sin()) * (mode == MODE_FREQUENCY_SHIFT ? 1. : depthL),
                        lookupCosSin[0]);
                getRate((modIn ? in[1][i] : svfOscR.sin()) * (mode == MODE_FREQUENCY_SHIFT ? 1. : depthR),
                        lookupCosSin[1]);
                oscSin[0][i] = lookupCosSin[0][1];
                oscCos[0][i] = lookupCosSin[0][0];
                oscSin[1][i] = lookupCosSin[1][1];
                oscCos[1][i] = lookupCosSin[1][0];
            }
            tmpL[i] = oscCos[0][i] * tmpHL[0][i] - oscSin[0][i] * tmpHL[1][i];
            tmpR[i] = oscCos[1][i] * tmpHR[0][i] - oscSin[1][i] * tmpHR[1][i];
        }
        if (mode > MODE_PHASE_MOD_1 && mode < MODE_FREQUENCY_SHIFT)
        {
            hilbertL2.process(tmpL, tmpHL, size);
            hilbertR2.process(tmpR, tmpHR, size);
            for (size_t i = 0; i < size; i++)
            {
                tmpL[i] = oscCos[0][i] * tmpHL[0][i] - oscSin[0][i] * tmpHL[1][i];
                tmpR[i] = oscCos[1][i] * tmpHR[0][i] - oscSin[1][i] * tmpHR[1][i];
            }
            if (mode > MODE_PHASE_MOD_2)
            {
                hilbertL3.process(tmpL, tmpHL, size);
                hilbertR3.process(tmpR, tmpHR, size);
                for (size_t i = 0; i < size; i++)
                {
                    tmpL[i] = oscCos[0][i] * tmpHL[0][i] - oscSin[0][i] * tmpHL[1][i];
                    tmpR[i] = oscCos[1][i] * tmpHR[0][i] - oscSin[1][i] * tmpHR[1][i];
                }
                if (mode > MODE_PHASE_MOD_3)
                {
                    hilbertL4.process(tmpL, tmpHL, size);
                    hilbertR4.process(tmpR, tmpHR, size);
                    for (size_t i = 0; i < size; i++)
                    {
                        tmpL[i] = oscCos[0][i] * tmpHL[0][i] - oscSin[0][i] * tmpHL[1][i];
                        tmpR[i] = oscCos[1][i] * tmpHR[0][i] - oscSin[1][i] * tmpHR[1][i];
                    }
                }
            }
        }
        for (size_t i = 0; i < size; i++)
        {
            hp2L.Process(tmpL[i]);
            hp2R.Process(tmpR[i]);
            tmpL[i] = hp2L.High();
            tmpR[i] = hp2R.High();
            lp2L.Process(tmpL[i]);
            lp2R.Process(tmpR[i]);
            tmpL[i] = lp2L.Low();
            tmpR[i] = lp2R.Low();
        }
        dcBlocker1.process(tmpL, out[0], size);
        dcBlocker2.process(tmpR, out[1], size);
    }
}

void Init(float samplerate)
{
    initCosSinLookup();
    toggleSwitch.Init(patch.B8);
    miniButton.Init(patch.B7);
    lp1L.Init(samplerate);
    lp1R.Init(samplerate);
    lp2L.Init(samplerate);
    lp2R.Init(samplerate);
    setLpFreq(lpf);
    lp1L.SetRes(0.f);
    lp1R.SetRes(0.f);
    lp2L.SetRes(0.f);
    lp2R.SetRes(0.f);
    hp1L.Init(samplerate);
    hp1R.Init(samplerate);
    hp2L.Init(samplerate);
    hp2R.Init(samplerate);
    setHpFreq(hpf);
    hp1L.SetRes(0.f);
    hp1R.SetRes(0.f);
    hp2L.SetRes(0.f);
    hp2R.SetRes(0.f);
}

int main(void)
{
    patch.Init(); // Initialize hardware (daisy seed, and patch)
    patch.SetAudioSampleRate(sampleRate);
    patch.SetAudioBlockSize(BS);

    Init(SR);

    patch.StartAdc();
    patch.StartAudio(AudioCallback);
    while (1)
    {
        //
    }
}

void ProcessControls()
{
    patch.ProcessAnalogControls();
    patch.ProcessDigitalControls();
    miniButton.Debounce();
    bool mbPressed = miniButton.Pressed();

    float cv1 = patch.GetAdcValue(CV_1);
    float cv2 = patch.GetAdcValue(CV_2);
    float cv3 = patch.GetAdcValue(CV_3);
    float cv4 = patch.GetAdcValue(CV_4);

    int cv1_100 = static_cast<int>(cv1 * 100.);
    int cv2_100 = static_cast<int>(cv2 * 100.);
    int cv3_100 = static_cast<int>(cv3 * 100.);
    int cv4_100 = static_cast<int>(cv4 * 100.);

    // when enter alt or norm mode we have to move knob by more than 1/100 of its full value
    // to start register changes
    if (!mbWasPressed && mbPressed)
    {
        cv1_100_alt = cv1_100;
        cv2_100_alt = cv2_100;
        cv3_100_alt = cv3_100;
        cv4_100_alt = cv4_100;
    }
    else if (mbWasPressed && !mbPressed)
    {
        cv1_100_norm = cv1_100;
        cv2_100_norm = cv2_100;
        cv3_100_norm = cv3_100;
        cv4_100_norm = cv4_100;

        m2s = !m2s; // have to press an extra time to undo change between mono 2 stereo change if pressing button to
                    // change alt values, sorry for that
    }
    mbWasPressed = mbPressed;

    // by setting to -1 we unlock the knob so we always trigger change
    if (mbPressed)
    {
        if (cv1_100_alt != cv1_100)
        {
            cv1_alt = cv1;
            cv1_100_alt = -1;
        }
        if (cv2_100_alt != cv2_100)
        {
            cv2_alt = cv2;
            cv2_100_alt = -1;
        }
        if (cv3_100_alt != cv3_100)
        {
            cv3_alt = cv3;
            cv3_100_alt = -1;
        }
        if (cv4_100_alt != cv4_100)
        {
            cv4_alt = cv4;
            cv4_100_alt = -1;
        }
    }
    else
    {
        if (cv1_100_norm != cv1_100)
        {
            cv1_norm = cv1;
            cv1_100_norm = -1;
        }
        if (cv2_100_norm != cv2_100)
        {
            cv2_norm = cv2;
            cv2_100_norm = -1;
        }
        if (cv3_100_norm != cv3_100)
        {
            cv3_norm = cv3;
            cv3_100_norm = -1;
        }
        if (cv4_100_norm != cv4_100)
        {
            cv4_norm = cv4;
            cv4_100_norm = -1;
        }
    }

    toggleSwitch.Debounce();
    modIn = toggleSwitch.Pressed();

    patch.WriteCvOut(CV_OUT_2, m2s ? 5.f : 0.f);

    float freq_knob = cv1_norm;

    bool hifreq;
    if (freq_knob < 0.25)
    {
        hifreq = false;
        freq_knob = freq_knob * 4;
    }
    else
    {
        hifreq = true;
        freq_knob = (freq_knob - 0.25) * 1.33333333;
    }

    float finetune_knob = fmap(cv3_norm, -1.f, 1.f);
    float voct_cv_L = patch.GetAdcValue(CV_5);
    float voct_cv_R = patch.GetAdcValue(m2s ? CV_5 : CV_6);
    float depth_knob = fmap(cv2_norm, 0.f, 1.f);
    float depth_cv_L = fmap(patch.GetAdcValue(CV_7), 0.f, 1.f);
    float depth_cv_R = fmap(patch.GetAdcValue(m2s ? CV_7 : CV_8), 0.f, 1.f);
    float depth_cv_knob = fmap(cv4_norm, 0.f, 1.f);

    if (hifreq)
    {
        float coarse = fmap(freq_knob, 36.f, 96.f);
        float voctL = fmap(voct_cv_L, 0.f, 60.f);
        float voctR = fmap(voct_cv_R, 0.f, 60.f);
        float midi_nn_L = fclamp(coarse + voctL, 0.f, 200.f);
        float midi_nn_R = fclamp(coarse + voctR, 0.f, 200.f);
        frequencyL = mtof(midi_nn_L);
        frequencyR = mtof(midi_nn_R);
    }
    else
    {
        frequencyL = fmap(freq_knob, 0.f, 20.f);
        frequencyR = frequencyL;
    }
    frequencyL += finetune_knob * frequencyL * 0.1;
    frequencyR += finetune_knob * frequencyR * 0.1;

    depthL = fclamp((depth_knob + depth_cv_L * depth_cv_knob), 0.f, 0.95f);
    if (m2s)
        depthR = depthL;
    else
        depthR = fclamp((depth_knob + depth_cv_R * depth_cv_knob), 0.f, 0.95f);

    svfOscL.setFreq(frequencyL);
    if (m2s)
        svfOscR.setFreq(frequencyL);
    else
        svfOscR.setFreq(frequencyR);

    mode = static_cast<int>(std::lround(cv3_alt * MODE_FS_MODOUT));
    hpf = fmap(cv2_alt, MIN_FILTER, MAX_FILTER, daisysp::Mapping::LOG);
    lpf = fmap(cv4_alt, MIN_FILTER, MAX_FILTER, daisysp::Mapping::LOG);
    setLpFreq(lpf);
    setHpFreq(hpf);
}
