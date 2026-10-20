# -*- coding: utf-8 -*-
import json
NOTES=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B']
C=lambda *pairs: list(pairs)
WAVES=C(('saw','Пила'),('raw','Сырая пила'),('square','Квадрат'),('pulse','Импульс 20%'),('tri','Треугольник'),('sine','Синус'),('organ','Орган'))
FT=C(('lowpass','НЧ (low-pass)'),('bandpass','Полосовой'),('highpass','ВЧ (high-pass)'),('notch','Режекторный'))
DM=C(('soft','Мягкий ламповый'),('raw','Сырой асимметричный'),('hard','Жёсткий клип'),('fold','Фолдинг'),('crush','Ступенчатый (крош)'))
GM=C(('track','Как в треке'),('voice','Голосоведение'),('parallel','Весь аккорд параллельно'),('off','Выключен'))
GC=C(('lin','Ровно'),('soft','Быстро, затем плавно'))
CM=C(('off','Выкл: играю аккорды сам'),('d3','Трезвучия лада'),('d7','Септаккорды лада'),('d9','Нонаккорды лада'),('d11','Ундецимы лада'))
VO=C(('close','Тесное'),('open','Открытое'),('drop2','Drop 2'),('spread','Широкое (бас вниз)'))
SC=C(('major','Мажор'),('minor','Натуральный минор'),('dorian','Дорийский'),('harm','Гармонический минор'))
KEYS=[(str(i),n) for i,n in enumerate(NOTES)]
CT=C(('d3','Трезвучия лада'),('d7','Септаккорды лада'),('d9','Нонаккорды лада'),('d11','Ундецимы лада'),('maj','Мажор'),('min','Минор'),('maj7','Большой септ'),('m7','Минорный септ'),('m9','Минорный нона'),('maj9','Мажорный нона'),('sus2','Sus2'),('sus4','Sus4'),('add9','Add9'),('p5','Квинта'),('fifths','Стопка квинт'))
PR=C(('raw','just RAW: аккорды из анализа'),('a','1 – 5 – 6 – 4'),('b','6 – 4 – 1 – 5'),('c','1 – 6 – 4 – 5'),('e','2 – 5 – 1 – 1'),('f','1 – 7 – 6 – 7'),('g','4 – 5 – 3 – 6'))

# id, label, group, type, min, max, def, log, fmt, where
P=[
 ('waveA','Форма A','osc','c',WAVES,None,'raw',0,'', 'both'),
 ('waveB','Форма B','osc','c',WAVES,None,'saw',0,'', 'both'),
 ('waveMix','Смесь A/B','osc','f',0,1,0,0,'pct','both'),
 ('unison','Голоса','osc','i',1,16,7,0,'int','both'),
 ('detune','Детюн','osc','f',0,100,28,0,'ct±','both'),
 ('width','Ширина голосов','osc','f',0,1,.8,0,'pct','both'),
 ('uniBlend','Крайние голоса','osc','f',0,1,1,0,'pct','both'),
 ('phaseRand','Случайная фаза','osc','f',0,1,1,0,'pct','both'),
 ('octUp','Октава вверх','osc','f',0,1,.25,0,'pct','both'),
 ('sub','Саб','osc','f',0,1,.3,0,'pct','both'),
 ('bassLevel','Громкость баса','osc','f',0,2,1,0,'pct','both'),
 ('noise','Шум','osc','f',0,1,.06,0,'pct','both'),
 ('drift','Аналог. дрейф','mod','f',0,30,6,0,'ct','both'),
 ('vibRate','Вибрато, скорость','mod','f',.1,12,4,1,'rate','both'),
 ('vibDepth','Вибрато, глубина','mod','f',0,50,0,0,'ct','both'),
 ('diveAmt','Нырок высоты','mod','f',-24,24,0,0,'st','both'),
 ('diveTime','Время нырка','mod','f',.01,3,.3,1,'sec','both'),
 ('glideMode','Как едут голоса','glide','c',GM,None,'track',0,'','both'),
 ('glideCurve','Кривая глайда','glide','c',GC,None,'lin',0,'','both'),
 ('glide','Время глайда','glide','f',0,4,1.5,0,'sec','both'),
 ('glideSpread','Разброс голосов','glide','f',0,1,.3,0,'pct','both'),
 ('chordMode','Аккорд из одной ноты','chord','c',CM,None,'off',0,'','vst'),
 ('key','Тональность','chord','c',KEYS,None,'9',0,'','both'),
 ('scale','Лад','chord','c',SC,None,'minor',0,'','both'),
 ('ctype','Тип аккорда','chord','c',CT,None,'d9',0,'','web'),
 ('voicing','Расположение','chord','c',VO,None,'open',0,'','both'),
 ('octave','Октава','chord','i',1,5,3,0,'int','web'),
 ('strum','Арпеджио','chord','f',0,250,0,0,'ms','web'),
 ('ftype','Тип фильтра','filt','c',FT,None,'lowpass',0,'','both'),
 ('cutoff','Срез','filt','f',60,18000,2400,1,'hz','both'),
 ('res','Резонанс','filt','f',.1,20,2,0,'x1','both'),
 ('fenv','Огиб. фильтра','filt','f',0,1,.35,0,'pct','both'),
 ('fdecay','Спад фильтра','filt','f',.01,3,.6,1,'sec','both'),
 ('keytrack','Слежение за нотой','filt','f',0,1,0,0,'pct','both'),
 ('lfoRate','LFO, скорость','filt','f',.05,12,.3,1,'rate','both'),
 ('lfoDepth','LFO, глубина','filt','f',0,1,.15,0,'pct','both'),
 ('attack','Атака','env','f',.001,4,.02,1,'sec','both'),
 ('decay','Спад','env','f',.01,4,.8,1,'sec','both'),
 ('sustain','Удержание','env','f',0,1,.7,0,'pct','both'),
 ('release','Затухание','env','f',.01,6,1.2,1,'sec','both'),
 ('dmode','Характер драйва','fx','c',DM,None,'raw',0,'','both'),
 ('drive','Драйв','fx','f',0,1,.25,0,'pct','both'),
 ('chorus','Хорус','fx','f',0,1,.3,0,'pct','both'),
 ('chorusRate','Скорость хоруса','fx','f',.1,4,.7,1,'rate','both'),
 ('delayMix','Дилей','fx','f',0,1,.15,0,'pct','both'),
 ('delayTime','Время дилея','fx','f',.05,1,.375,0,'sec','both'),
 ('delayFb','Повторы','fx','f',0,.9,.35,0,'pct','both'),
 ('revMix','Реверб','fx','f',0,1,.3,0,'pct','both'),
 ('revSize','Размер зала','fx','f',.3,8,3,0,'x1s','both'),
 ('revDamp','Глушение зала','fx','f',0,1,.4,0,'pct','both'),
 ('stereoWidth','Стереобаза','out','f',0,2,1,0,'pct','both'),
 ('lowCut','Срез низа','out','f',20,500,20,1,'hz','both'),
 ('highCut','Срез верха','out','f',1000,20000,20000,1,'hz','both'),
 ('volume','Громкость','out','f',0,1,.7,0,'pct','both'),
 ('bpm','Темп','seq','i',60,180,120,0,'bpm','web'),
 ('prog','Прогрессия','seq','c',PR,None,'b',0,'','web'),
]
GROUPS=[('osc','Осцилляторы','источник'),('mod','Движение','дрейф, вибрато, нырок'),('glide','Глайд','частоты съезжаются и расходятся'),('chord','Аккорд','гармония'),('filt','Фильтр','тембр'),('env','Огибающая','громкость во времени'),('fx','Драйв и эффекты','грязь и воздух'),('out','Выход','стерео и срезы')]
DEF={p[0]:p[6] for p in P}

TR=dict(bassLevel=1.7,waveA='saw',waveB='raw',waveMix=0,unison=7,detune=22,width=1,drift=5,uniBlend=.85,phaseRand=1,octUp=.15,sub=.25,noise=.02,vibRate=4,vibDepth=0,diveAmt=0,diveTime=.3,
 glideMode='track',glide=1.8,glideSpread=.3,glideCurve='lin',ftype='lowpass',cutoff=4200,res=1,fenv=.05,fdecay=1,keytrack=.2,lfoRate=.2,lfoDepth=.05,
 attack=2,decay=2,sustain=.9,release=3,drive=.1,dmode='soft',chorus=.45,chorusRate=.5,delayMix=.1,delayTime=.5,delayFb=.3,revMix=.35,revSize=4,revDamp=.45,
 stereoWidth=1.3,lowCut=25,highCut=9000,volume=.7,key='7',scale='major',ctype='d9',voicing='open',prog='raw',bpm=62)
TRV=[
 ('just RAW: оригинал',{}),
 ('just RAW: шире',dict(detune=32,unison=10,stereoWidth=1.7,chorus=.6)),
 ('just RAW: темнее',dict(cutoff=1800,highCut=5000,res=2)),
 ('just RAW: сырее',dict(waveA='raw',cutoff=7000,drive=.35,dmode='raw',highCut=14000)),
 ('just RAW: медленные глайды',dict(glide=3.2,glideSpread=.6,glideCurve='soft')),
 ('just RAW: быстрые глайды',dict(glide=.6,glideSpread=.15)),
 ('just RAW: голоса врозь',dict(glideMode='voice',glideSpread=1,drift=10)),
 ('just RAW: весь аккорд едет',dict(glideMode='parallel',glide=2.2)),
 ('just RAW: грязный',dict(waveA='raw',drive=.65,dmode='hard',cutoff=6000,noise=.08)),
 ('just RAW: стекло',dict(waveA='organ',waveB='sine',waveMix=.3,octUp=.5,detune=12,chorus=.7,revMix=.55)),
 ('just RAW: под водой',dict(cutoff=800,res=6,lfoDepth=.35,lfoRate=.15,revMix=.6,revSize=7,highCut=4000)),
 ('just RAW: нырок',dict(diveAmt=-12,diveTime=1.2,attack=.4)),
 ('just RAW: плывущий',dict(vibDepth=18,vibRate=3.5,drift=14,glideSpread=.7)),
 ('just RAW: лоуфай',dict(dmode='crush',drive=.45,highCut=5500,lowCut=120,noise=.12,stereoWidth=.9)),
 ('just RAW: 16 голосов',dict(unison=16,detune=30,uniBlend=.6)),
 ('just RAW: мягкая атака',dict(attack=4,release=5,revMix=.5)),
 ('just RAW: импульс',dict(waveA='pulse',waveB='saw',waveMix=.5,cutoff=3000,res=3)),
 ('just RAW: моно-центр',dict(stereoWidth=.6,width=.4,sub=.5)),
]
OTHER=[
 ('Сырой стек',{}),
 ('Стеклянный пэд',dict(waveA='organ',unison=5,detune=12,width=1,drift=4,octUp=.4,sub=.15,noise=.03,cutoff=3600,res=1,fenv=.15,fdecay=1.5,attack=.9,decay=2,sustain=.85,release=3.2,drive=.05,dmode='soft',chorus=.65,delayMix=.2,revMix=.6,revSize=6,ctype='d9',voicing='open',lfoRate=.15,lfoDepth=.25)),
 ('Жёсткий суперсо',dict(waveA='saw',unison=12,detune=55,width=1,drift=10,octUp=.5,sub=.35,noise=.08,cutoff=9000,res=3,fenv=.2,fdecay=.4,attack=.005,decay=.6,sustain=.8,release=.45,drive=.6,dmode='hard',chorus=.2,delayMix=.1,revMix=.22,revSize=2.5,ctype='d7',voicing='spread',lfoDepth=0)),
 ('Импульсный щипок',dict(waveA='pulse',unison=4,detune=15,width=.7,drift=5,octUp=.2,sub=.2,noise=.02,cutoff=700,res=9,fenv=.85,fdecay=.22,attack=.002,decay=.35,sustain=.08,release=.5,drive=.3,dmode='soft',chorus=.25,delayMix=.38,delayTime=.375,delayFb=.45,revMix=.3,ctype='d7',voicing='open',strum=30,glideMode='off')),
 ('Грязная квинта',dict(waveA='raw',unison=9,detune=35,width=.9,drift=12,octUp=.3,sub=.5,noise=.15,cutoff=5000,res=4,fenv=.3,attack=.01,decay=1,sustain=.75,release=.8,drive=.85,dmode='fold',chorus=.15,delayMix=.1,revMix=.25,ctype='fifths',voicing='close')),
 ('Сонный хорал',dict(waveA='tri',unison=8,detune=20,width=1,drift=8,octUp=.6,sub=.1,noise=.1,cutoff=2200,res=1.5,fenv=.25,fdecay=2,attack=1.6,decay=3,sustain=.9,release=5,drive=.15,dmode='crush',chorus=.5,delayMix=.25,delayTime=.6,delayFb=.5,revMix=.7,revSize=8,ctype='d11',voicing='open',lfoRate=.08,lfoDepth=.35)),
 ('Детюн-облако',dict(waveA='saw',unison=16,detune=70,drift=20,width=1,stereoWidth=1.6,cutoff=5000,attack=1.5,release=4,revMix=.5,glide=1.2)),
 ('Квинтовый дрон',dict(ctype='fifths',waveA='raw',unison=9,detune=15,drive=.4,cutoff=2500,attack=1,release=3,voicing='close')),
 ('Космический глайд',dict(glide=3.5,glideMode='voice',glideSpread=.8,vibDepth=10,revMix=.7,revSize=8,delayMix=.35,waveA='tri',waveB='saw',waveMix=.4,attack=.8,release=4)),
 ('Ночной хор',dict(waveA='organ',waveB='tri',waveMix=.5,unison=6,detune=18,cutoff=2000,keytrack=.5,attack=1.2,release=3.5,chorus=.7,revMix=.55,voicing='open',ctype='d11')),
]
PRESETS=[]
for n,o in TRV:
  v=dict(DEF);v.update(TR);v.update(o);PRESETS.append(dict(name=n,group='Из трека just RAW',v=v))
for n,o in OTHER:
  v=dict(DEF);v.update(o);PRESETS.append(dict(name=n,group='Другие',v=v))
print(len(PRESETS),'presets',len(P),'params')

# ---------- web JSON
web_spec=[dict(id=p[0],label=p[1],group=p[2],type=p[3],choices=p[4] if p[3]=='c' else None,min=None if p[3]=='c' else p[4],max=p[5],def_=p[6],log=p[7],fmt=p[8]) for p in P if p[9] in('both','web')]
json.dump(dict(spec=web_spec,groups=GROUPS,presets=PRESETS),open('web.json','w'),ensure_ascii=False)

# ---------- C++
vst=[p for p in P if p[9] in('both','vst')]
def cnum(x): return repr(float(x))+'f'
L=['#pragma once','// Сгенерировано из общей спецификации (та же, что у веб-версии)','#include <vector>','#include "Engine.h"','','struct ParamDef { const char* id; const char* name; const char* group; char type; float min, max, def; bool log; const char* fmt; std::vector<const char*> choices; };','','inline const std::vector<ParamDef>& paramDefs() {','    static const std::vector<ParamDef> defs {']
for p in vst:
  pid,lab,grp,t=p[0],p[1],p[2],p[3]
  if t=='c':
    keys=[k for k,_ in p[4]];d=keys.index(p[6])
    ch=', '.join('"%s"'%l for _,l in p[4])
    L.append('        { "%s", "%s", "%s", \'c\', 0.0f, %d.0f, %d.0f, false, "", { %s } },'%(pid,lab,grp,len(keys)-1,d,ch))
  else:
    L.append('        { "%s", "%s", "%s", \'%s\', %s, %s, %s, %s, "%s", {} },'%(pid,lab,grp,t,cnum(p[4]),cnum(p[5]),cnum(p[6]),'true' if p[7] else 'false',p[8]))
L+=['    };','    return defs;','}','','inline void applyParam(rs::Params& p, int index, float v) {','    switch (index) {']
for i,p in enumerate(vst):
  if p[3] in 'ci': L.append('        case %d: p.%s = (int) (v + 0.5f); break;'%(i,p[0]))
  else: L.append('        case %d: p.%s = v; break;'%(i,p[0]))
L+=['        default: break;','    }','}','']
open('/home/claude/rawstack/Source/Params.h','w').write('\n'.join(L))
# Params struct for engine
S=['struct Params {']
for p in vst:
  if p[3]=='c': S.append('    int %s = %d;'%(p[0],[k for k,_ in p[4]].index(p[6])))
  elif p[3]=='i': S.append('    int %s = %d;'%(p[0],p[6]))
  else: S.append('    float %s = %s;'%(p[0],cnum(p[6])))
S.append('};')
open('params_struct.txt','w').write('\n'.join(S))
# presets
Q=['#pragma once','// Заводские пресеты: те же, что в веб-версии Rawstack','#include <vector>','','struct FactoryPreset { const char* name; std::vector<float> values; };','','// Значения идут в порядке paramDefs()','inline const std::vector<FactoryPreset>& factoryPresets() {','    static const std::vector<FactoryPreset> presets {']
for pr in PRESETS:
  vals=[]
  for p in vst:
    x=pr['v'][p[0]]
    if p[3]=='c':
      x=[k for k,_ in p[4]].index(x)
    vals.append(cnum(x))
  Q.append('        { "%s", { %s } },'%(pr['name'],', '.join(vals)))
Q+=['    };','    return presets;','}','']
open('/home/claude/rawstack/Source/Presets.h','w').write('\n'.join(Q))
