#include "../Source/Params.h"
#include "../Source/Presets.h"
#include <cstdio>
#include <chrono>
#include <string>
static void wav(const char* fn,const std::vector<float>& L,const std::vector<float>& R,int sr){
  FILE* f=fopen(fn,"wb");int n=(int)L.size();int data=n*4;auto w32=[&](int v){fwrite(&v,4,1,f);};auto w16=[&](short v){fwrite(&v,2,1,f);};
  fwrite("RIFF",1,4,f);w32(36+data);fwrite("WAVEfmt ",1,8,f);w32(16);w16(1);w16(2);w32(sr);w32(sr*4);w16(4);w16(16);fwrite("data",1,4,f);w32(data);
  for(int i=0;i<n;i++){short a=(short)std::max(-32767.f,std::min(32767.f,L[i]*32767));short b=(short)std::max(-32767.f,std::min(32767.f,R[i]*32767));w16(a);w16(b);} fclose(f);}
int main(int argc,char**argv){
  int presetIdx=argc>1?atoi(argv[1]):0; const char* out=argc>2?argv[2]:"out.wav";
  const int sr=44100; rs::Engine e; 
  auto& pr=factoryPresets()[presetIdx]; for(size_t i=0;i<pr.values.size();++i) applyParam(e.p,(int)i,pr.values[i]);
  e.prepare(sr);
  std::vector<std::vector<int>> ch={{43,50,59,66,69,74},{31,38,59,66,69,74},{35,42,62,66,69,74},{36,43,59,62,64,67},{38,45,62,64,69,71},{40,47,62,64,67,69},{36,43,59,62,64,67},{43,50,59,62,66,74}};
  double bar=4*60.0/62; int total=(int)((bar*ch.size()+5)*sr);
  std::vector<float> L(total,0),R(total,0); int pos=0; auto t0=std::chrono::steady_clock::now();
  std::vector<int> cur;
  for(size_t c=0;c<=ch.size();++c){
    int at=(int)(c*bar*sr); if(at>pos){e.process(L.data()+pos,R.data()+pos,at-pos);pos=at;}
    for(int m:cur) e.noteOff(m);
    if(c<ch.size()){cur=ch[c];for(int m:cur)e.noteOn(m,100);} else cur.clear();
  }
  e.process(L.data()+pos,R.data()+pos,total-pos);
  double secs=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count();
  float pk=0;double rms=0;bool nan=false;for(int i=0;i<total;i++){pk=std::max(pk,std::abs(L[i]));rms+=L[i]*L[i];if(!std::isfinite(L[i]))nan=true;}
  printf("%s: render %.2fs for %.1fs audio (x%.1f realtime), peak %.3f rms %.1f dB nan=%d\n",pr.name,secs,total/(double)sr,total/(double)sr/secs,pk,20*log10(sqrt(rms/total)+1e-9),nan);
  rs::VizState v; e.copyViz(v);
  wav(out,L,R,sr);
}
