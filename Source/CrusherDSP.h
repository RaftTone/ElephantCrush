#pragma once
#include <cmath>
#include <algorithm>

// =============================================================================
//  ElephantCrush DSP — original, from-scratch implementation.
//  The coefficients/curves below were derived by black-box measurement
//  (input/output null-testing) of a reference effect and re-implemented
//  independently. No third-party source code is used.
//  Chain: DistTube -> DistMech -> MMFilter (LP, 1 biquad/ch) -> Compressor -> master
// =============================================================================
namespace cc
{
    inline float sgn (float v) noexcept { return v < 0.0f ? -1.0f : 1.0f; }
    inline float clampf (float v, float lo, float hi) noexcept { return v<lo?lo:(v>hi?hi:v); }

    // Master volume law (0..1 -> gain). Measured curve: gain = v^2.737.
    inline float masterVolGain (float v01) noexcept
    {
        v01 = clampf (v01,0.f,1.f);
        return std::pow (v01, 2.737f);
    }

    // --- Tube distortion: soft parabolic clip, drive = 1 + p*4, threshold 0.6 --
    struct DistTube
    {
        float drive = 1.0f;
        void setAmount (float p01) noexcept { drive = 1.0f + clampf(p01,0,1) * 4.0f; }
        inline float processSample (float x) const noexcept
        {
            const float s = x * drive, a = std::fabs (s);
            if (a <= 0.6f) return s;
            const float D = drive - 0.6f;
            const float t = 0.70710678f * (1.0f - (a - 0.6f) / D);
            return sgn (s) * (0.6f + 0.8f * (0.5f - t * t));
        }
    };

    // --- Mech distortion: harder clip, drive = 1 + p*10, threshold 0.75 --------
    struct DistMech
    {
        float drive = 1.0f;
        void setAmount (float p01) noexcept { drive = 1.0f + clampf(p01,0,1) * 10.0f; }
        inline float processSample (float x) const noexcept
        {
            const float s = x * drive, a = std::fabs (s);
            if (a <= 0.75f) return s;
            return sgn (s) * (0.75f + 0.25f * (a - 0.75f) / (drive - 0.75f));
        }
    };

    // --- RBJ biquad low-pass (Audio EQ Cookbook), Direct Form I --------------
    //  y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2  (float state, matters at high Q)
    struct Biquad
    {
        float b0=1,b1=0,b2=0,a1=0,a2=0;
        float x1=0,x2=0,y1=0,y2=0;
        void reset() noexcept { x1=x2=y1=y2=0; }
        void makeLowPass (double fs, double fc, double Q) noexcept
        {
            fc = std::clamp (fc, 20.0, fs * 0.49);
            const double w0 = 2.0 * M_PI * fc / fs;
            const double cw = std::cos (w0), sw = std::sin (w0);
            const double alpha = sw / (2.0 * std::max (0.0001, Q));
            const double a0 = 1.0 + alpha;
            b0 = float ((1.0 - cw) * 0.5 / a0);
            b1 = float ((1.0 - cw) / a0);
            b2 = b0;
            a1 = float (-2.0 * cw / a0);
            a2 = float ((1.0 - alpha) / a0);
        }
        inline float processSample (float x) noexcept
        {
            const float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1; x1 = x;
            y2 = y1; y1 = y;
            return y;
        }
    };

    // --- Multi-mode low-pass filter. Cutoff 80 Hz .. 16000 Hz -----------------
    //  Measured mapping: t = -0.25*log10(1-cf); fc = 80 + 15920*t;  Q = 1 + 6*res.
    //
    //  Resonance limiter (measured behaviour): the filter output is peak-limited so
    //  the resonance can't exceed ~1.0 — a frequency-flat gain (scales the whole
    //  output), instant attack, very slow release (~7 s). Below threshold the gain
    //  is exactly 1, so quiet / low-resonance material is untouched.
    struct MMFilter
    {
        Biquad bq; double fs = 44100.0;
        float  g = 1.0f, atkCoef = 0.0f, relCoef = 0.0f;
        static constexpr float kThresh = 1.0f;     // resonance ceiling (~unity)
        void prepare (double sampleRate) noexcept
        {
            fs = sampleRate; bq.reset(); g = 1.0f;
            atkCoef = std::exp (-1.0f / float (0.002 * sampleRate));  // ~2 ms attack
            relCoef = std::exp (-1.0f / float (7.0   * sampleRate));  // ~7 s  release
        }
        void setCutoffRes (float cutoff01, float res01) noexcept
        {
            const double cf = clampf (cutoff01, 0.f, 0.99999f);
            double t = -0.25 * std::log10 (1.0 - cf);
            t = std::clamp (t, 0.0, 1.0);
            double freq = 80.0 + 15920.0 * t;                   // 80 .. 16000 Hz
            freq = std::clamp (freq, 20.0, fs * 0.49);
            const double Q = 1.0 + 6.0 * clampf (res01,0,1);    // res*6 + 1
            bq.makeLowPass (fs, freq, Q);
        }
        inline float processSample (float x) noexcept
        {
            const float y = bq.processSample (x);
            const float a = std::fabs (y);
            const float gTarget = a > kThresh ? kThresh / a : 1.0f;   // limit resonance to ~1
            g = gTarget < g ? atkCoef * g + (1.0f - atkCoef) * gTarget
                            : relCoef * g + (1.0f - relCoef) * gTarget;
            return y * g;                                             // frequency-flat gain
        }
    };

    // --- Compressor: pre-gain G = 1/(1-amount), stereo-linked ------------------
    //  detector = max(|L|,|R|)*G ; above 0.31 -> parabolic-knee target:
    //    target = 0.31 + 1.38*(0.5 - (0.7071*(1 - (det-0.31)/(G-0.31)))^2)
    //    gain   = target/det ; instant attack, linear gain release.
    //  PHAT = instant peak detector; NORMAL = slew-limited detector.
    struct Compressor
    {
        float G = 1.0f, gain = 1.0f, relInc = 0.0f, relSamples = 11.0f;
        float detStep = 0.0f, detOff = 0.0f;   // normal-mode slew detector
        float detEnv = 0.0f;
        bool phat = false;
        void prepare (double fs) noexcept
        {
            relSamples = float (0.00025 * fs);        // gain release (~11 smp @44.1k)
            detStep    = float (1.0 / (0.002 * fs));   // normal detector attack (~2 ms)
            detOff     = float (1.0 / (0.02  * fs));   // normal detector release (~20 ms)
            gain = 1.0f; relInc = 0.0f; detEnv = 0.0f;
        }
        void setAmount (float amt01, bool mode) noexcept
        {
            G = 1.0f / (1.0f - clampf (amt01, 0.f, 0.99f));   // amount clamped to 0.99
            phat = mode;
        }
        static inline float knee (float detG, float G) noexcept
        {
            if (detG <= 0.31f) return 1.0f;              // measured knee constants
            const float over = (detG - 0.31f) / (G - 0.31f);
            const float u = 0.707107008f * (1.0f - over);
            const float target = 0.31f + 1.38f * (0.5f - u * u);
            return target / detG;
        }
        inline void processStereo (float& L, float& R) noexcept
        {
            const float rect = std::max (std::fabs (L), std::fabs (R));
            float det = rect;                            // PHAT: instant peak
            if (! phat)                                  // NORMAL: slew-limited detector
            {
                const float t1 = detEnv - detOff;
                detEnv = rect > t1 ? std::max (0.0f, t1 + std::min (detStep, rect - t1))
                                   : std::max (0.0f, t1);
                det = detEnv;
            }
            const float gi = knee (det * G, G);
            if (gi < gain) { gain = gi; relInc = (1.0f - gi) / relSamples; }   // instant attack
            L = L * G * gain; R = R * G * gain;
            if (gain < 1.0f) { gain += relInc; if (gain > 1.0f) gain = 1.0f; } // fast release
        }
    };
}
