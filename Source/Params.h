#pragma once
// Сгенерировано из общей спецификации (та же, что у веб-версии)
#include <vector>
#include "Engine.h"

struct ParamDef { const char* id; const char* name; const char* group; char type; float min, max, def; bool log; const char* fmt; std::vector<const char*> choices; };

inline const std::vector<ParamDef>& paramDefs() {
    static const std::vector<ParamDef> defs {
        { "waveA", "Форма A", "osc", 'c', 0.0f, 6.0f, 1.0f, false, "", { "Пила", "Сырая пила", "Квадрат", "Импульс 20%", "Треугольник", "Синус", "Орган" } },
        { "waveB", "Форма B", "osc", 'c', 0.0f, 6.0f, 0.0f, false, "", { "Пила", "Сырая пила", "Квадрат", "Импульс 20%", "Треугольник", "Синус", "Орган" } },
        { "waveMix", "Смесь A/B", "osc", 'f', 0.0f, 1.0f, 0.0f, false, "pct", {} },
        { "unison", "Голоса", "osc", 'i', 1.0f, 16.0f, 7.0f, false, "int", {} },
        { "detune", "Детюн", "osc", 'f', 0.0f, 100.0f, 28.0f, false, "ct±", {} },
        { "width", "Ширина голосов", "osc", 'f', 0.0f, 1.0f, 0.8f, false, "pct", {} },
        { "uniBlend", "Крайние голоса", "osc", 'f', 0.0f, 1.0f, 1.0f, false, "pct", {} },
        { "phaseRand", "Случайная фаза", "osc", 'f', 0.0f, 1.0f, 1.0f, false, "pct", {} },
        { "octUp", "Октава вверх", "osc", 'f', 0.0f, 1.0f, 0.25f, false, "pct", {} },
        { "sub", "Саб", "osc", 'f', 0.0f, 1.0f, 0.3f, false, "pct", {} },
        { "bassLevel", "Громкость баса", "osc", 'f', 0.0f, 2.0f, 1.0f, false, "pct", {} },
        { "noise", "Шум", "osc", 'f', 0.0f, 1.0f, 0.06f, false, "pct", {} },
        { "drift", "Аналог. дрейф", "mod", 'f', 0.0f, 30.0f, 6.0f, false, "ct", {} },
        { "vibRate", "Вибрато, скорость", "mod", 'f', 0.1f, 12.0f, 4.0f, true, "rate", {} },
        { "vibDepth", "Вибрато, глубина", "mod", 'f', 0.0f, 50.0f, 0.0f, false, "ct", {} },
        { "diveAmt", "Нырок высоты", "mod", 'f', -24.0f, 24.0f, 0.0f, false, "st", {} },
        { "diveTime", "Время нырка", "mod", 'f', 0.01f, 3.0f, 0.3f, true, "sec", {} },
        { "glideMode", "Как едут голоса", "glide", 'c', 0.0f, 3.0f, 0.0f, false, "", { "Как в треке", "Голосоведение", "Весь аккорд параллельно", "Выключен" } },
        { "glideCurve", "Кривая глайда", "glide", 'c', 0.0f, 1.0f, 0.0f, false, "", { "Ровно", "Быстро, затем плавно" } },
        { "glide", "Время глайда", "glide", 'f', 0.0f, 4.0f, 1.5f, false, "sec", {} },
        { "glideSpread", "Разброс голосов", "glide", 'f', 0.0f, 1.0f, 0.3f, false, "pct", {} },
        { "chordMode", "Аккорд из одной ноты", "chord", 'c', 0.0f, 4.0f, 0.0f, false, "", { "Выкл: играю аккорды сам", "Трезвучия лада", "Септаккорды лада", "Нонаккорды лада", "Ундецимы лада" } },
        { "key", "Тональность", "chord", 'c', 0.0f, 11.0f, 9.0f, false, "", { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" } },
        { "scale", "Лад", "chord", 'c', 0.0f, 3.0f, 1.0f, false, "", { "Мажор", "Натуральный минор", "Дорийский", "Гармонический минор" } },
        { "voicing", "Расположение", "chord", 'c', 0.0f, 3.0f, 1.0f, false, "", { "Тесное", "Открытое", "Drop 2", "Широкое (бас вниз)" } },
        { "ftype", "Тип фильтра", "filt", 'c', 0.0f, 3.0f, 0.0f, false, "", { "НЧ (low-pass)", "Полосовой", "ВЧ (high-pass)", "Режекторный" } },
        { "cutoff", "Срез", "filt", 'f', 60.0f, 18000.0f, 2400.0f, true, "hz", {} },
        { "res", "Резонанс", "filt", 'f', 0.1f, 20.0f, 2.0f, false, "x1", {} },
        { "fenv", "Огиб. фильтра", "filt", 'f', 0.0f, 1.0f, 0.35f, false, "pct", {} },
        { "fdecay", "Спад фильтра", "filt", 'f', 0.01f, 3.0f, 0.6f, true, "sec", {} },
        { "keytrack", "Слежение за нотой", "filt", 'f', 0.0f, 1.0f, 0.0f, false, "pct", {} },
        { "lfoRate", "LFO, скорость", "filt", 'f', 0.05f, 12.0f, 0.3f, true, "rate", {} },
        { "lfoDepth", "LFO, глубина", "filt", 'f', 0.0f, 1.0f, 0.15f, false, "pct", {} },
        { "attack", "Атака", "env", 'f', 0.001f, 4.0f, 0.02f, true, "sec", {} },
        { "decay", "Спад", "env", 'f', 0.01f, 4.0f, 0.8f, true, "sec", {} },
        { "sustain", "Удержание", "env", 'f', 0.0f, 1.0f, 0.7f, false, "pct", {} },
        { "release", "Затухание", "env", 'f', 0.01f, 6.0f, 1.2f, true, "sec", {} },
        { "dmode", "Характер драйва", "fx", 'c', 0.0f, 4.0f, 1.0f, false, "", { "Мягкий ламповый", "Сырой асимметричный", "Жёсткий клип", "Фолдинг", "Ступенчатый (крош)" } },
        { "drive", "Драйв", "fx", 'f', 0.0f, 1.0f, 0.25f, false, "pct", {} },
        { "chorus", "Хорус", "fx", 'f', 0.0f, 1.0f, 0.3f, false, "pct", {} },
        { "chorusRate", "Скорость хоруса", "fx", 'f', 0.1f, 4.0f, 0.7f, true, "rate", {} },
        { "delayMix", "Дилей", "fx", 'f', 0.0f, 1.0f, 0.15f, false, "pct", {} },
        { "delayTime", "Время дилея", "fx", 'f', 0.05f, 1.0f, 0.375f, false, "sec", {} },
        { "delayFb", "Повторы", "fx", 'f', 0.0f, 0.9f, 0.35f, false, "pct", {} },
        { "revMix", "Реверб", "fx", 'f', 0.0f, 1.0f, 0.3f, false, "pct", {} },
        { "revSize", "Размер зала", "fx", 'f', 0.3f, 8.0f, 3.0f, false, "x1s", {} },
        { "revDamp", "Глушение зала", "fx", 'f', 0.0f, 1.0f, 0.4f, false, "pct", {} },
        { "stereoWidth", "Стереобаза", "out", 'f', 0.0f, 2.0f, 1.0f, false, "pct", {} },
        { "lowCut", "Срез низа", "out", 'f', 20.0f, 500.0f, 20.0f, true, "hz", {} },
        { "highCut", "Срез верха", "out", 'f', 1000.0f, 20000.0f, 20000.0f, true, "hz", {} },
        { "volume", "Громкость", "out", 'f', 0.0f, 1.0f, 0.7f, false, "pct", {} },
    };
    return defs;
}

inline void applyParam(rs::Params& p, int index, float v) {
    switch (index) {
        case 0: p.waveA = (int) (v + 0.5f); break;
        case 1: p.waveB = (int) (v + 0.5f); break;
        case 2: p.waveMix = v; break;
        case 3: p.unison = (int) (v + 0.5f); break;
        case 4: p.detune = v; break;
        case 5: p.width = v; break;
        case 6: p.uniBlend = v; break;
        case 7: p.phaseRand = v; break;
        case 8: p.octUp = v; break;
        case 9: p.sub = v; break;
        case 10: p.bassLevel = v; break;
        case 11: p.noise = v; break;
        case 12: p.drift = v; break;
        case 13: p.vibRate = v; break;
        case 14: p.vibDepth = v; break;
        case 15: p.diveAmt = v; break;
        case 16: p.diveTime = v; break;
        case 17: p.glideMode = (int) (v + 0.5f); break;
        case 18: p.glideCurve = (int) (v + 0.5f); break;
        case 19: p.glide = v; break;
        case 20: p.glideSpread = v; break;
        case 21: p.chordMode = (int) (v + 0.5f); break;
        case 22: p.key = (int) (v + 0.5f); break;
        case 23: p.scale = (int) (v + 0.5f); break;
        case 24: p.voicing = (int) (v + 0.5f); break;
        case 25: p.ftype = (int) (v + 0.5f); break;
        case 26: p.cutoff = v; break;
        case 27: p.res = v; break;
        case 28: p.fenv = v; break;
        case 29: p.fdecay = v; break;
        case 30: p.keytrack = v; break;
        case 31: p.lfoRate = v; break;
        case 32: p.lfoDepth = v; break;
        case 33: p.attack = v; break;
        case 34: p.decay = v; break;
        case 35: p.sustain = v; break;
        case 36: p.release = v; break;
        case 37: p.dmode = (int) (v + 0.5f); break;
        case 38: p.drive = v; break;
        case 39: p.chorus = v; break;
        case 40: p.chorusRate = v; break;
        case 41: p.delayMix = v; break;
        case 42: p.delayTime = v; break;
        case 43: p.delayFb = v; break;
        case 44: p.revMix = v; break;
        case 45: p.revSize = v; break;
        case 46: p.revDamp = v; break;
        case 47: p.stereoWidth = v; break;
        case 48: p.lowCut = v; break;
        case 49: p.highCut = v; break;
        case 50: p.volume = v; break;
        default: break;
    }
}
