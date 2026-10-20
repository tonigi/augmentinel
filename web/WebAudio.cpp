#include "stdafx.h"
#include "Audio.h"
EM_JS(void, audioInit, (), {
    const Context=window.AudioContext||window.webkitAudioContext;
    const a=Module.audio={context:Context?new Context():null,bank:new Map(),voices:[],volume:.7};
    a.start=function(v) {
        const s=a.context.createBufferSource(); s.buffer=v.buffer; s.loop=v.type===2; s.playbackRate.value=v.speed;
        s.connect(v.gain); v.source=s; v.started=a.context.currentTime;
        s.onended=()=>{ if(v.source===s) a.voices=a.voices.filter(x=>x!==v); };
        s.start(0,v.offset||0);
    };
    a.resume=()=>{ if(a.context) a.context.resume().catch(console.warn); };
    window.addEventListener('keydown',a.resume);
    document.getElementById('canvas').addEventListener('mousedown',a.resume);
});
EM_JS(int, audioLoad, (const char* name, const unsigned char* bytes, int length), {
    const a=Module.audio; if(!a.context) return 1;
    try {
        const d=new DataView(HEAPU8.buffer,bytes,length);
        const tag=i=>String.fromCharCode(d.getUint8(i),d.getUint8(i+1),d.getUint8(i+2),d.getUint8(i+3));
        if(tag(0)!=='RIFF'||tag(8)!=='WAVE') return 0;
        let fmt=-1,data=-1,size=0;
        for(let i=12;i+8<=length;) {
            const n=d.getUint32(i+4,true); if(i+8+n>length) return 0;
            if(tag(i)==='fmt ') fmt=i+8;
            if(tag(i)==='data') { data=i+8; size=n; }
            i+=8+n+(n&1);
        }
        if(fmt<0||data<0) return 0;
        const format=d.getUint16(fmt,true),channels=d.getUint16(fmt+2,true),rate=d.getUint32(fmt+4,true),bits=d.getUint16(fmt+14,true);
        if(format!==1 || (bits!==8 && bits!==16)) return 0;
        const samples=size/(channels*bits/8),buffer=a.context.createBuffer(channels,samples,rate);
        for(let c=0;c<channels;c++) {
            const out=buffer.getChannelData(c);
            for(let i=0;i<samples;i++) {
                const at=data+(i*channels+c)*(bits/8);
                out[i]=bits===8?(d.getUint8(at)-128)/128:d.getInt16(at,true)/32768;
            }
        }
        a.bank.set(UTF8ToString(name),buffer); return 1;
    } catch(e) { console.error(e); return 0; }
});
EM_JS(int, audioPlaying, (int type), { return Module.audio.voices.some(v=>!type||v.type===type); });
EM_JS(void, audioStop, (int type), {
    const a=Module.audio;
    for(const v of a.voices.filter(v=>!type||v.type===type)) {
        if(v.source) { v.source.onended=null; v.source.stop(); }
        v.gain.disconnect(); if(v.panner) v.panner.disconnect();
    }
    a.voices=a.voices.filter(v=>type&&v.type!==type);
});
EM_JS(int, audioPlay, (const char* name,int type,float x,float y,float z,float speed), {
    const a=Module.audio,buffer=a.bank.get(UTF8ToString(name)); if(!a.context||!buffer) return 0;
    const v={buffer:buffer,type:type,speed:speed,offset:0,gain:a.context.createGain()};
    v.gain.gain.value=type===4?a.volume:1;
    if(x||y||z) {
        v.panner=a.context.createPanner(); v.panner.panningModel='HRTF'; v.panner.refDistance=6;
        v.panner.setPosition(x,y,-z); v.gain.connect(v.panner); v.panner.connect(a.context.destination);
    } else v.gain.connect(a.context.destination);
    a.voices.push(v); if(type!==4) a.start(v); return 1;
});
Audio::Audio() { audioInit(); }
Audio::~Audio() { Stop(); }
bool Audio::Available() const { return EM_ASM_INT({ return !!Module.audio.context; }); }
bool Audio::IsPlaying(AudioType type) const { return audioPlaying(static_cast<int>(type)); }
void Audio::LoadWAV(fs::path path)
{
    auto bytes=FileContents(path.wstring()); auto name=path.filename().string();
    if (!audioLoad(name.c_str(),bytes.data(),bytes.size())) throw std::runtime_error("Unsupported WAV: "+path.string());
}
float Audio::LengthInSeconds(const std::wstring& name)
{
    auto n=to_string(name);
    return EM_ASM_DOUBLE({ const b=Module.audio.bank.get(UTF8ToString($0)); return b?b.duration:0; },n.c_str());
}
bool Audio::Play(const std::wstring& name,AudioType type,XMFLOAT3 pos,float speed)
{
    if(type==AudioType::Tune || type==AudioType::Music) Stop(type);
    auto n=to_string(name); return audioPlay(n.c_str(),static_cast<int>(type),pos.x,pos.y,pos.z,speed);
}
void Audio::Stop(AudioType type) { audioStop(static_cast<int>(type)); }
bool Audio::SetMusicVolume(float volume)
{
    return EM_ASM_INT({ const a=Module.audio; a.volume=$0; for(const v of a.voices) if(v.type===4) v.gain.gain.value=$0; return 1; },volume);
}
bool Audio::SetMusicPlaying(bool play)
{
    return EM_ASM_INT({
        const a=Module.audio; const v=a.voices.find(v=>v.type===4); if(!v) return 0;
        if(!$0 && v.source) {
            v.offset+=a.context.currentTime-v.started;
            v.source.onended=null; v.source.stop(); v.source=null;
        } else if($0 && !v.source) a.start(v);
        return 1;
    },play);
}
void Audio::PositionListener(XMFLOAT3 pos,XMFLOAT3 front,XMFLOAT3 up)
{
    EM_ASM({ const a=Module.audio; if(!a.context) return;
        a.context.listener.setPosition($0,$1,-$2);
        a.context.listener.setOrientation($3,$4,-$5,$6,$7,-$8);
    },pos.x,pos.y,pos.z,front.x,front.y,front.z,up.x,up.y,up.z);
}
