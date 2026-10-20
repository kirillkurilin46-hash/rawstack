#pragma once
// Rawstack: звуковой движок (чистый C++17, без JUCE, чтобы его можно было тестировать отдельно)
#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <string>

namespace rs {

constexpr float kPi = 3.14159265358979f;
inline float mtof (float m) { return 440.0f * std::exp2 ((m - 69.0f) / 12.0f); }
inline float clampf (float x, float a, float b) { return x < a ? a : (x > b ? b : x); }

enum { W_SAW, W_RAW, W_SQUARE, W_PULSE, W_TRI, W_SINE, W_ORGAN, W_COUNT };
enum { F_LP, F_BP, F_HP, F_NOTCH };
enum { D_SOFT, D_RAW, D_HARD, D_FOLD, D_CRUSH };
enum { G_TRACK, G_VOICE, G_PARALLEL, G_OFF };
enum { GC_LINEAR, GC_SOFT };

struct Params {
    int waveA = 1;
    int waveB = 0;
    float waveMix = 0.0f;
    int unison = 7;
    float detune = 28.0f;
    float width = 0.8f;
    float uniBlend = 1.0f;
    float phaseRand = 1.0f;
    float octUp = 0.25f;
    float sub = 0.3f;
    float bassLevel = 1.0f;
    float noise = 0.06f;
    float drift = 6.0f;
    float vibRate = 4.0f;
    float vibDepth = 0.0f;
    float diveAmt = 0.0f;
    float diveTime = 0.3f;
    int glideMode = 0;
    int glideCurve = 0;
    float glide = 1.5f;
    float glideSpread = 0.3f;
    int chordMode = 0;
    int key = 9;
    int scale = 1;
    int voicing = 1;
    int ftype = 0;
    float cutoff = 2400.0f;
    float res = 2.0f;
    float fenv = 0.35f;
    float fdecay = 0.6f;
    float keytrack = 0.0f;
    float lfoRate = 0.3f;
    float lfoDepth = 0.15f;
    float attack = 0.02f;
    float decay = 0.8f;
    float sustain = 0.7f;
    float release = 1.2f;
    int dmode = 1;
    float drive = 0.25f;
    float chorus = 0.3f;
    float chorusRate = 0.7f;
    float delayMix = 0.15f;
    float delayTime = 0.375f;
    float delayFb = 0.35f;
    float revMix = 0.3f;
    float revSize = 3.0f;
    float revDamp = 0.4f;
    float stereoWidth = 1.0f;
    float lowCut = 20.0f;
    float highCut = 20000.0f;
    float volume = 0.7f;
};

inline float waveFn (int w, float p)
{
    switch (w)
    {
        case W_SAW:    return 1.0f - 2.0f * p;
        case W_RAW:    return std::tanh (2.5f * ((1.0f - 2.0f * p) + 0.6f * std::sin (6.0f * kPi * p)));
        case W_SQUARE: return p < 0.5f ? 1.0f : -1.0f;
        case W_PULSE:  return p < 0.2f ? 1.0f : -1.0f;
        case W_TRI:    return p < 0.25f ? 4.0f * p : (p < 0.75f ? 2.0f - 4.0f * p : 4.0f * p - 4.0f);
        case W_SINE:   return std::sin (2.0f * kPi * p);
        default:       return (std::sin (2 * kPi * p) + 0.5f * std::sin (4 * kPi * p) + 0.33f * std::sin (6 * kPi * p) + 0.2f * std::sin (10 * kPi * p)) / 1.45f;
    }
}

// Волновые таблицы с ограничением гармоник по октавам (без алиасинга)
class WaveBank
{
public:
    static constexpr int N = 2048, LEVELS = 11;
    static const WaveBank& get() { static WaveBank b; return b; }

    inline float read (int w, int lvl, float ph) const
    {
        const float* t = tab[w][lvl].data();
        const float x = ph * (float) N;
        int i = (int) x;
        const float f = x - (float) i;
        i &= (N - 1);
        return t[i] + (t[i + 1] - t[i]) * f;
    }
    static int levelFor (float hz, float sr)
    {
        const float maxH = sr * 0.45f / std::max (hz, 1.0f);
        int l = 0;
        while (l < LEVELS - 1 && (float) (1024 >> l) > maxH) ++l;
        return l;
    }

private:
    std::vector<float> tab[W_COUNT][LEVELS];
    WaveBank()
    {
        std::vector<double> s (N), c (N), sn (N);
        for (int i = 0; i < N; ++i) { c[i] = std::cos (2.0 * 3.14159265358979323846 * i / N); sn[i] = std::sin (2.0 * 3.14159265358979323846 * i / N); }
        const int H = N / 2 - 1;
        std::vector<double> re (H + 1), im (H + 1);
        for (int w = 0; w < W_COUNT; ++w)
        {
            for (int i = 0; i < N; ++i) s[i] = waveFn (w, (float) i / N);
            for (int h = 1; h <= H; ++h)
            {
                double a = 0, b = 0;
                for (int i = 0; i < N; ++i) { const int k = (int) (((long long) h * i) % N); a += s[i] * c[k]; b += s[i] * sn[k]; }
                re[h] = 2.0 * a / N; im[h] = 2.0 * b / N;
            }
            for (int l = 0; l < LEVELS; ++l)
            {
                const int maxH = std::min (H, std::max (1, 1024 >> l));
                auto& t = tab[w][l];
                t.assign (N + 1, 0.0f);
                for (int i = 0; i < N; ++i)
                {
                    double v = 0;
                    for (int h = 1; h <= maxH; ++h) { const int k = (int) (((long long) h * i) % N); v += re[h] * c[k] + im[h] * sn[k]; }
                    t[i] = (float) v;
                }
                t[N] = t[0];
            }
        }
    }
};

// Фильтр с переменным состоянием (Cytomic/Simper TPT)
struct SVF
{
    float ic1 = 0, ic2 = 0, a1 = 1, a2 = 0, a3 = 0, k = 1; int type = F_LP;
    void set (float fc, float q, float sr, int t)
    {
        const float g = std::tan (kPi * clampf (fc, 20.0f, sr * 0.45f) / sr);
        k = 1.0f / std::max (q, 0.05f);
        a1 = 1.0f / (1.0f + g * (g + k)); a2 = g * a1; a3 = g * a2; type = t;
    }
    inline float proc (float v0)
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1; ic2 = 2.0f * v2 - ic2;
        switch (type) { case F_LP: return v2; case F_BP: return v1; case F_HP: return v0 - k * v1 - v2; default: return v0 - k * v1; }
    }
    void reset() { ic1 = ic2 = 0; }
};

struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void lowpass (float fc, float sr)  { calc (fc, sr, true); }
    void highpass (float fc, float sr) { calc (fc, sr, false); }
    void calc (float fc, float sr, bool lp)
    {
        const float w = 2.0f * kPi * clampf (fc, 10.0f, sr * 0.49f) / sr, cs = std::cos (w), al = std::sin (w) / (2.0f * 0.7071f), a0 = 1.0f + al;
        if (lp) { b0 = (1 - cs) / 2 / a0; b1 = (1 - cs) / a0; b2 = b0; }
        else    { b0 = (1 + cs) / 2 / a0; b1 = -(1 + cs) / a0; b2 = b0; }
        a1 = -2 * cs / a0; a2 = (1 - al) / a0;
    }
    inline float proc (float x) { const float y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
};

struct Env
{
    float v = 0; int stage = 0; // 0 тишина, 1 атака, 2 спад/удержание, 3 затухание
    void gate() { stage = 1; }
    void release() { if (stage != 0) stage = 3; }
    inline float tick (float aInc, float dCoef, float sus, float rCoef)
    {
        switch (stage)
        {
            case 1: v += aInc; if (v >= 1.0f) { v = 1.0f; stage = 2; } break;
            case 2: v = sus + (v - sus) * dCoef; break;
            case 3: v *= rCoef; if (v < 1e-5f) { v = 0; stage = 0; } break;
            default: break;
        }
        return v;
    }
};

struct Osc
{
    float phase = 0, spread = 0, rnd = 0, gain = 0;
    float cur = 60, start = 60, targ = 60, gTime = 0.1f, gPos = 1.0f;
    float inc = 0, gl = 0.7f, gr = 0.7f; int layer = 0, lvl = 0; // layer: 0 основной, 1 октава вверх, 2 саб
};

struct Note
{
    bool on = false, released = false;
    float midi = 60, vel = 0.5f;
    std::vector<Osc> osc;
    Env amp;
    float fenvV = 0; int fstage = 0;
    SVF fl, fr;
    float diveT = 1e9f, noiseAmt = 0, cutoffNow = 1000;
    uint32_t rng = 1;
    long long killAt = -1;
    uint64_t age = 0;
    bool lowest = false;
    inline float noise() { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return (float) (int32_t) rng * (1.0f / 2147483648.0f); }
};

struct DelayLine
{
    std::vector<float> b; int w = 0;
    void init (int n) { b.assign ((size_t) std::max (n, 4), 0.0f); w = 0; }
    inline void push (float x) { b[(size_t) w] = x; if (++w >= (int) b.size()) w = 0; }
    inline float readFrac (float d) const
    {
        const int n = (int) b.size();
        float rp = (float) w - 1.0f - d; while (rp < 0) rp += (float) n;
        int i = (int) rp; const float f = rp - (float) i; const int j = (i + 1) % n;
        return b[(size_t) i] + (b[(size_t) j] - b[(size_t) i]) * f;
    }
};

// Классический Freeverb
class Reverb
{
    struct Comb { std::vector<float> b; int i = 0; float st = 0;
        inline float proc (float x, float fb, float d1, float d2) { const float y = b[(size_t) i]; st = y * d2 + st * d1; b[(size_t) i] = x + st * fb; if (++i >= (int) b.size()) i = 0; return y; } };
    struct AP { std::vector<float> b; int i = 0;
        inline float proc (float x) { const float bo = b[(size_t) i]; const float y = -x + bo; b[(size_t) i] = x + bo * 0.5f; if (++i >= (int) b.size()) i = 0; return y; } };
    Comb cl[8], cr[8]; AP al[4], ar[4];
public:
    void init (float sr)
    {
        static const int ct[8] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 }, at[4] = { 556, 441, 341, 225 };
        const float s = sr / 44100.0f;
        for (int k = 0; k < 8; ++k) { cl[k].b.assign ((size_t) (ct[k] * s), 0.0f); cr[k].b.assign ((size_t) ((ct[k] + 23) * s), 0.0f); cl[k].i = cr[k].i = 0; cl[k].st = cr[k].st = 0; }
        for (int k = 0; k < 4; ++k) { al[k].b.assign ((size_t) (at[k] * s), 0.0f); ar[k].b.assign ((size_t) ((at[k] + 23) * s), 0.0f); al[k].i = ar[k].i = 0; }
    }
    inline void proc (float in, float room, float damp, float& oL, float& oR)
    {
        const float x = in * 0.015f, fb = room * 0.28f + 0.7f, d1 = damp * 0.4f, d2 = 1.0f - d1;
        float l = 0, r = 0;
        for (int k = 0; k < 8; ++k) { l += cl[k].proc (x, fb, d1, d2); r += cr[k].proc (x, fb, d1, d2); }
        for (int k = 0; k < 4; ++k) { l = al[k].proc (l); r = ar[k].proc (r); }
        oL = l; oR = r;
    }
};

// Снимок для визуализации (читается из окна плагина)
struct VizOsc { float semis; float gain; int layer; };
struct VizNote { int slot; float midi; float env; float cutoff; int numOsc; VizOsc osc[48]; };
struct VizState { int numNotes = 0; VizNote notes[24]; int waveA = 0, waveB = 0; float waveMix = 0; std::string chordName; std::string chordNotes; int oscCount = 0; };

inline std::string noteName (int m) { static const char* n[12] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }; return std::string (n[((m % 12) + 12) % 12]) + std::to_string (m / 12 - 1); }

inline std::string chordNameOf (const std::vector<int>& notes)
{
    if (notes.empty()) return "";
    static const char* n[12] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    if (notes.size() == 1) return noteName (notes[0]);
    const int root = notes.front();
    std::vector<int> pcs; for (int m : notes) pcs.push_back (((m - root) % 12 + 12) % 12);
    std::sort (pcs.begin(), pcs.end()); pcs.erase (std::unique (pcs.begin(), pcs.end()), pcs.end());
    std::string key; for (size_t i = 0; i < pcs.size(); ++i) { if (i) key += ","; key += std::to_string (pcs[i]); }
    static const std::pair<const char*, const char*> table[] = {
        {"0,4,7",""},{"0,3,7","m"},{"0,3,6","dim"},{"0,4,8","aug"},{"0,4,7,11","maj7"},{"0,3,7,10","m7"},{"0,4,7,10","7"},{"0,3,6,10","m7b5"},
        {"0,2,4,7,11","maj9"},{"0,2,3,7,10","m9"},{"0,2,4,7,10","9"},{"0,1,3,7,10","m7b9"},{"0,2,4,7,9","6/9"},{"0,2,7,9","6/9 no3"},{"0,3,5,7,10","m7(11)"},
        {"0,2,4,5,7,11","maj11"},{"0,2,3,5,7,10","m11"},{"0,2,7","sus2"},{"0,5,7","sus4"},{"0,2,4,7","add9"},{"0,7","5"},{"0,4,7,9","6"},{"0,2,4,6,7,11","maj9#11"},{"0,4,6,7,11","maj7#11"} };
    for (auto& t : table) if (key == t.first) return std::string (n[((root % 12) + 12) % 12]) + t.second;
    return std::string (n[((root % 12) + 12) % 12]) + "(" + key + ")";
}

class Engine
{
public:
    Params p;
    static constexpr int MAX_NOTES = 24, CHUNK = 16, SCOPE = 16384;

    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        WaveBank::get();
        for (auto& n : notes) { n.on = false; n.osc.reserve (40); }
        chL.init ((int) (0.1f * sr)); chR.init ((int) (0.1f * sr));
        dlL.init ((int) (2.1f * sr)); dlR.init ((int) (2.1f * sr));
        rev.init (sr);
        held.fill (false); dirty = false; countdown = 0; sampleClock = 0;
        smDelay = p.delayTime * sr; smVol = p.volume; limEnv = 0;
        dLp1 = dLp2 = 0; lastLow = -1; lastHigh = -1;
        scopePos.store (0);
    }

    void noteOn (int m, int vel) { if (m < 0 || m > 127) return; held[(size_t) m] = true; lastVel = (float) vel / 127.0f; markDirty(); }
    void noteOff (int m) { if (m < 0 || m > 127) return; held[(size_t) m] = false; markDirty(); }
    void allOff() { held.fill (false); for (auto& n : notes) if (n.on) releaseNote (n); }

    void process (float* outL, float* outR, int n)
    {
        const WaveBank& wb = WaveBank::get();
        updateOutputFilters();
        for (int i = 0; i < n; ++i)
        {
            if (ctrl == 0) { control(); ctrl = CHUNK; }
            --ctrl;

            float bl = 0, br = 0;
            const int wa = p.waveA, wbw = p.waveB; const float mix = clampf (p.waveMix, 0, 1);
            for (auto& nt : notes)
            {
                if (! nt.on) continue;
                float nl = 0, nr = 0;
                for (auto& o : nt.osc)
                {
                    float a;
                    if (o.layer == 2) a = wb.read (W_SINE, o.lvl, o.phase);
                    else
                    {
                        a = wb.read (wa, o.lvl, o.phase);
                        if (mix > 0.001f) a += (wb.read (wbw, o.lvl, o.phase) - a) * mix;
                    }
                    o.phase += o.inc; if (o.phase >= 1.0f) o.phase -= 1.0f;
                    a *= o.gain; nl += a * o.gl; nr += a * o.gr;
                }
                if (nt.noiseAmt > 0) { const float z = nt.noise() * nt.noiseAmt; nl += z; nr += z; }
                const float e = nt.amp.tick (aInc, dCoef, p.sustain, rCoef) * nt.vel;
                nl = nt.fl.proc (nl * e); nr = nt.fr.proc (nr * e);
                bl += nl; br += nr;
                if (nt.amp.stage == 0 && nt.released) nt.on = false;
            }

            // драйв
            const float d = p.drive, pre = 1.0f + d * 12.0f;
            float xl = bl * (1.0f - d) + shape (bl * pre) * 0.55f * d;
            float xr = br * (1.0f - d) + shape (br * pre) * 0.55f * d;
            // хорус
            chPh1 += p.chorusRate * 0.8f / sr; if (chPh1 >= 1) chPh1 -= 1;
            chPh2 += p.chorusRate * 1.2f / sr; if (chPh2 >= 1) chPh2 -= 1;
            chL.push (xl); chR.push (xr);
            const float wl = chL.readFrac ((0.016f + 0.0035f * std::sin (2 * kPi * chPh1)) * sr);
            const float wr = chR.readFrac ((0.016f + 0.0042f * std::sin (2 * kPi * chPh2)) * sr);
            const float cdry = 1.0f - p.chorus * 0.45f, cwet = p.chorus * 0.9f;
            const float cl = xl * cdry + wl * cwet, cr = xr * cdry + wr * cwet;
            // дилей
            smDelay += (p.delayTime * sr - smDelay) * 0.0005f;
            const float dol = dlL.readFrac (smDelay), dor = dlR.readFrac (smDelay);
            const float lpk = 1.0f - std::exp (-2.0f * kPi * 4200.0f / sr);
            dLp1 += (dol - dLp1) * lpk; dLp2 += (dor - dLp2) * lpk;
            dlL.push (cl * p.delayMix + dLp1 * p.delayFb); dlR.push (cr * p.delayMix + dLp2 * p.delayFb);
            // реверб
            float rl, rr;
            const float room = clampf ((p.revSize - 0.3f) / 7.7f, 0.0f, 1.0f);
            rev.proc ((cl + cr) * 0.5f, room, p.revDamp, rl, rr);
            float L = cl + dLp1 + rl * p.revMix * 2.2f;
            float R = cr + dLp2 + rr * p.revMix * 2.2f;
            // стерео и срезы
            const float mid = (L + R) * 0.5f, side = (L - R) * 0.5f * p.stereoWidth;
            L = mid + side; R = mid - side;
            L = hpL.proc (L); R = hpR.proc (R); L = lpL.proc (L); R = lpR.proc (R);
            smVol += (p.volume - smVol) * 0.001f;
            L *= smVol; R *= smVol;
            // лимитер
            const float pk = std::max (std::abs (L), std::abs (R));
            limEnv = pk > limEnv ? pk : limEnv * 0.99985f;
            const float g = limEnv > 0.95f ? 0.95f / limEnv : 1.0f;
            L *= g; R *= g;
            outL[i] += L; outR[i] += R;

            const int sp = scopePos.load (std::memory_order_relaxed);
            scope[(size_t) sp] = (L + R) * 0.5f;
            scopePos.store ((sp + 1) & (SCOPE - 1), std::memory_order_relaxed);
            ++sampleClock;
        }
    }

    // ---- визуализация ----
    std::array<float, SCOPE> scope {};
    std::atomic<int> scopePos { 0 };
    float getSampleRate() const { return sr; }
    void copyViz (VizState& out) { std::lock_guard<std::mutex> lk (vizMutex); out = viz; }

private:
    float sr = 44100;
    std::array<Note, MAX_NOTES> notes;
    std::array<bool, 128> held {};
    bool dirty = false; int countdown = 0; int ctrl = 0;
    float lastVel = 0.8f;
    float aInc = 0, dCoef = 0, rCoef = 0;
    float vibPh = 0, lfoPh = 0, chPh1 = 0, chPh2 = 0.37f;
    long long sampleClock = 0; uint64_t ageCounter = 0;
    uint32_t rnd = 22222;
    DelayLine chL, chR, dlL, dlR; float smDelay = 0, dLp1 = 0, dLp2 = 0;
    Reverb rev; Biquad hpL, hpR, lpL, lpR; float lastLow = -1, lastHigh = -1;
    float smVol = 0.7f, limEnv = 0;
    std::mutex vizMutex; VizState viz; int vizCounter = 0;
    std::vector<int> currentChord;
    std::string cachedName, cachedNotes;
    void cacheChordText()
    {
        cachedName = chordNameOf (currentChord);
        cachedNotes.clear();
        for (int m : currentChord) { if (! cachedNotes.empty()) cachedNotes += "  "; cachedNotes += noteName (m); }
    }

    float rand01() { rnd ^= rnd << 13; rnd ^= rnd >> 17; rnd ^= rnd << 5; return (float) (rnd & 0xFFFFFF) / 16777216.0f; }
    float randBi() { return rand01() * 2.0f - 1.0f; }
    void markDirty() { dirty = true; countdown = (int) (0.02f * sr); }

    inline float shape (float x) const
    {
        x = clampf (x, -1.0f, 1.0f);
        switch (p.dmode)
        {
            case D_SOFT:  return std::tanh (3 * x) / std::tanh (3.0f);
            case D_RAW:   return x > 0 ? std::tanh (4 * x) : std::tanh (1.6f * x) * 0.85f;
            case D_HARD:  return clampf (x * 2.2f, -1, 1) * 0.9f;
            case D_FOLD:  return std::sin (x * kPi * 2.4f) * 0.9f;
            default:      return std::round (std::tanh (2 * x) * 5.0f) / 5.0f;
        }
    }

    void updateOutputFilters()
    {
        if (p.lowCut != lastLow) { hpL.highpass (p.lowCut, sr); hpR.highpass (p.lowCut, sr); lastLow = p.lowCut; }
        if (p.highCut != lastHigh) { lpL.lowpass (p.highCut, sr); lpR.lowpass (p.highCut, sr); lastHigh = p.highCut; }
    }

    // ---------- аккорды и глайд ----------
    std::vector<int> buildTarget()
    {
        std::vector<int> t;
        for (int m = 0; m < 128; ++m) if (held[(size_t) m]) t.push_back (m);
        if (p.chordMode > 0 && ! t.empty())
        {
            static const int scales[4][7] = { {0,2,4,5,7,9,11}, {0,2,3,5,7,8,10}, {0,2,3,5,7,9,10}, {0,2,3,5,7,8,11} };
            const int* sc = scales[std::clamp (p.scale, 0, 3)];
            const int m = t.front();
            const int rel = ((m - p.key) % 12 + 12) % 12;
            int idx = 0; for (int i = 0; i < 7; ++i) if (sc[i] <= rel) idx = i;
            const int rootSemi = sc[idx];
            const int base = m - (rel - rootSemi);
            const int k = p.chordMode == 1 ? 3 : p.chordMode == 2 ? 4 : p.chordMode == 3 ? 5 : 6;
            t.clear();
            for (int j = 0; j < k; ++j) { const int ii = idx + 2 * j; t.push_back (base + sc[ii % 7] + 12 * (ii / 7) - rootSemi); }
            std::sort (t.begin(), t.end());
            if (p.voicing == 1 && t.size() >= 3) for (size_t i = 1; i < t.size(); i += 2) t[i] += 12;
            else if (p.voicing == 2 && t.size() >= 3) t[t.size() - 2] -= 12;
            else if (p.voicing == 3) t[0] -= 12;
            std::sort (t.begin(), t.end());
            for (auto& x : t) x = std::clamp (x, 0, 127);
        }
        return t;
    }

    void resolveChord()
    {
        auto tg = buildTarget();
        currentChord = tg;
        cacheChordText();
        if (tg.empty()) { for (auto& n : notes) if (n.on && ! n.released) releaseNote (n); return; }
        std::vector<Note*> old;
        for (auto& n : notes) if (n.on && ! n.released && n.killAt < 0) old.push_back (&n);
        std::sort (old.begin(), old.end(), [] (Note* a, Note* b) { return a->midi < b->midi; });
        const bool glideOn = p.glideMode != G_OFF && p.glide > 0.005f;
        if (old.empty() || ! glideOn)
        {   // без глайда: держим уже звучащие ноты, гасим лишние, запускаем новые
            std::vector<bool> kept (tg.size(), false);
            for (auto* n : old)
            {
                bool found = false;
                for (size_t i = 0; i < tg.size(); ++i)
                    if (! kept[i] && std::lround (n->midi) == tg[i] && n->osc.size() > 0 && n->osc[0].gPos >= 1.0f) { kept[i] = true; found = true; break; }
                if (! found) releaseNote (*n);
            }
            for (size_t i = 0; i < tg.size(); ++i) if (! kept[i]) startNote ((float) tg[i], (float) tg[i], i == 0, (int) tg.size());
            return;
        }
        if (p.glideMode == G_PARALLEL)
        {
            const float sh = (float) tg[0] - old[0]->midi;
            for (auto* n : old) glideTo (*n, n->midi + sh);
            currentChord.clear(); for (auto* n : old) currentChord.push_back ((int) std::lround (n->midi));
            cacheChordText();
            return;
        }
        const size_t low = p.glideMode == G_TRACK ? std::min<size_t> ({ (size_t) 2, old.size(), tg.size() }) : 0;
        for (size_t i = 0; i < low; ++i) glideTo (*old[i], (float) tg[i]);
        std::vector<Note*> ro (old.begin() + (long) low, old.end());
        std::vector<int> rt (tg.begin() + (long) low, tg.end());
        std::vector<bool> used (ro.size(), false);
        for (int t : rt)
        {
            int best = -1; float bd = 1e9f;
            for (size_t i = 0; i < ro.size(); ++i) if (! used[i]) { const float dd = std::abs (ro[i]->midi - (float) t); if (dd < bd) { bd = dd; best = (int) i; } }
            if (best >= 0) { used[(size_t) best] = true; glideTo (*ro[(size_t) best], (float) t); }
            else
            {   // голос «расходится»: новая нота рождается из ближайшей и уезжает к своей
                float near = old[0]->midi;
                for (auto* n : old) if (std::abs (n->midi - (float) t) < std::abs (near - (float) t)) near = n->midi;
                Note* nn = startNote ((float) t, near, false, (int) tg.size());
                if (nn) glideTo (*nn, (float) t);
            }
        }
        for (size_t i = 0; i < ro.size(); ++i)
        {
            if (used[i]) continue; // лишний голос «сходится» к ближайшей ноте и гаснет
            float t = rt.empty() ? (float) tg.back() : (float) rt[0];
            for (int x : rt) if (std::abs ((float) x - ro[i]->midi) < std::abs (t - ro[i]->midi)) t = (float) x;
            glideTo (*ro[i], t);
            ro[i]->killAt = sampleClock + (long long) (p.glide * sr);
        }
    }

    void glideTo (Note& n, float target)
    {
        for (auto& o : n.osc)
        {
            o.start = o.cur; o.targ = target; o.gPos = 0;
            o.gTime = std::max (0.02f, p.glide * (1.0f + p.glideSpread * randBi() * 0.7f));
        }
        n.midi = target;
    }

    Note* startNote (float target, float startMidi, bool lowest, int total)
    {
        Note* slot = nullptr;
        for (auto& n : notes) if (! n.on) { slot = &n; break; }
        if (! slot) { for (auto& n : notes) if (n.released && (! slot || n.age < slot->age)) slot = &n; }
        if (! slot) { for (auto& n : notes) if (! slot || n.age < slot->age) slot = &n; }
        Note& n = *slot;
        n.on = true; n.released = false; n.killAt = -1; n.age = ++ageCounter; n.lowest = lowest;
        n.midi = target; n.vel = (0.5f / std::sqrt ((float) std::max (1, total))) * (0.55f + 0.45f * lastVel) * (lowest ? p.bassLevel : 1.0f);
        n.osc.clear();
        const int u = std::clamp (p.unison, 1, 16);
        const float vg = 1.0f / std::sqrt ((float) u);
        auto layer = [&] (int lay, float level)
        {
            for (int k = 0; k < u; ++k)
            {
                Osc o; o.layer = lay;
                o.spread = u == 1 ? 0.0f : ((float) k / (float) (u - 1)) * 2.0f - 1.0f;
                o.rnd = randBi();
                o.phase = p.phaseRand * rand01();
                o.gain = vg * level * (1.0f - (1.0f - p.uniBlend) * std::abs (o.spread));
                o.cur = o.start = o.targ = startMidi; o.gPos = 1.0f;
                n.osc.push_back (o);
            }
        };
        layer (0, 1.0f);
        if (p.octUp > 0.01f) layer (1, p.octUp * 0.7f);
        if (lowest && p.sub > 0.01f) { Osc o; o.layer = 2; o.gain = p.sub * 0.9f; o.cur = o.start = o.targ = startMidi; n.osc.push_back (o); }
        n.noiseAmt = p.noise * 0.22f;
        n.rng = 0x9E3779B9u ^ (uint32_t) (ageCounter * 2654435761u);
        n.amp.v = 0; n.amp.gate(); n.fenvV = 0; n.fstage = 1; n.diveT = 0;
        n.fl.reset(); n.fr.reset();
        computeOsc (n, 0.0f);
        return &n;
    }

    void releaseNote (Note& n) { n.released = true; n.amp.release(); }

    void computeOsc (Note& n, float dt)
    {
        const float vib = p.vibDepth / 100.0f * std::sin (2 * kPi * vibPh);
        float dive = 0;
        if (std::abs (p.diveAmt) > 0.01f && n.diveT < p.diveTime) dive = p.diveAmt * (1.0f - n.diveT / std::max (p.diveTime, 0.001f));
        for (auto& o : n.osc)
        {
            if (o.gPos < 1.0f)
            {
                o.gPos = std::min (1.0f, o.gPos + dt / o.gTime);
                float x = o.gPos;
                if (p.glideCurve == GC_SOFT) x = (1.0f - std::exp (-5.0f * x)) / (1.0f - std::exp (-5.0f));
                o.cur = o.start + (o.targ - o.start) * x;
            }
            float m = o.cur + dive;
            if (o.layer == 1) m += 12.0f;
            if (o.layer == 2) m -= 12.0f;
            else m += (o.spread * p.detune + o.rnd * p.drift) / 100.0f + vib;
            const float hz = mtof (m);
            o.inc = hz / sr; o.lvl = WaveBank::levelFor (hz, sr);
            const float pan = o.layer == 2 ? 0.0f : clampf (o.spread * p.width, -1, 1);
            const float x01 = (pan + 1.0f) * 0.5f;
            o.gl = std::cos (x01 * kPi * 0.5f); o.gr = std::sin (x01 * kPi * 0.5f);
        }
    }

    void control()
    {
        const float dt = (float) CHUNK / sr;
        if (dirty) { countdown -= CHUNK; if (countdown <= 0) { dirty = false; resolveChord(); } }
        vibPh += p.vibRate * dt; if (vibPh >= 1) vibPh -= 1;
        lfoPh += p.lfoRate * dt; if (lfoPh >= 1) lfoPh -= 1;
        aInc = 1.0f / std::max (1.0f, p.attack * sr);
        dCoef = std::exp (-1.0f / std::max (1.0f, (p.decay / 3.0f) * sr));
        rCoef = std::exp (-1.0f / std::max (1.0f, (p.release / 4.0f) * sr));
        const float lfo = std::sin (2 * kPi * lfoPh) * p.lfoDepth * p.cutoff * 0.85f;
        const float fa = std::max (0.003f, std::min (p.attack, 0.4f));
        const float fdc = std::exp (-dt / std::max (0.001f, p.fdecay / 3.0f));
        const float qlin = std::max (0.5f, std::pow (10.0f, p.res / 20.0f));
        const float qUse = (p.ftype == F_BP || p.ftype == F_NOTCH) ? std::max (0.1f, p.res) : qlin;
        int oscCount = 0;
        for (auto& n : notes)
        {
            if (! n.on) continue;
            if (n.killAt >= 0 && sampleClock >= n.killAt && ! n.released) releaseNote (n);
            n.diveT += dt;
            if (n.fstage == 1) { n.fenvV += dt / fa; if (n.fenvV >= 1) { n.fenvV = 1; n.fstage = 2; } }
            else n.fenvV *= fdc;
            const float curMidi = n.osc.empty() ? n.midi : n.osc[0].cur;
            float fc = p.cutoff * std::exp2 (p.fenv * 6.0f * n.fenvV) * std::exp2 (p.keytrack * (curMidi - 60.0f) / 12.0f) + lfo;
            fc = clampf (fc, 20.0f, std::min (20000.0f, sr * 0.45f));
            n.cutoffNow = fc;
            n.fl.set (fc, qUse, sr, p.ftype); n.fr.set (fc, qUse, sr, p.ftype);
            computeOsc (n, dt);
            if (! n.released) oscCount += (int) n.osc.size();
        }
        if (++vizCounter >= 32) { vizCounter = 0; publishViz (oscCount); }
    }

    void publishViz (int oscCount)
    {
        std::unique_lock<std::mutex> lk (vizMutex, std::try_to_lock);
        if (! lk.owns_lock()) return;
        viz.numNotes = 0;
        for (int s = 0; s < MAX_NOTES; ++s)
        {
            auto& n = notes[(size_t) s];
            if (! n.on || n.amp.v < 0.003f || viz.numNotes >= 24) continue;
            auto& v = viz.notes[viz.numNotes++];
            v.slot = s; v.midi = n.osc.empty() ? n.midi : n.osc[0].cur; v.env = n.amp.v; v.cutoff = n.cutoffNow;
            v.numOsc = 0;
            for (auto& o : n.osc)
            {
                if (v.numOsc >= 48) break;
                float semis = o.layer == 1 ? 12.0f : o.layer == 2 ? -12.0f : 0.0f;
                if (o.layer != 2) semis += (o.spread * p.detune + o.rnd * p.drift) / 100.0f;
                semis += o.cur - v.midi;
                v.osc[v.numOsc++] = { semis, o.gain, o.layer };
            }
        }
        viz.waveA = p.waveA; viz.waveB = p.waveB; viz.waveMix = p.waveMix; viz.oscCount = oscCount;
        viz.chordName = cachedName;
        viz.chordNotes = cachedNotes;
    }
};

} // namespace rs
