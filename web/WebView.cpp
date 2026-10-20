#include "stdafx.h"
#include "View.h"
#include <emscripten/html5.h>

namespace {
GLuint shader(GLenum type, const char* source)
{
    GLuint id=glCreateShader(type);
    glShaderSource(id,1,&source,nullptr); glCompileShader(id);
    GLint ok; glGetShaderiv(id,GL_COMPILE_STATUS,&ok);
    if (!ok) { char log[4096]; glGetShaderInfoLog(id,sizeof(log),nullptr,log); throw std::runtime_error(log); }
    return id;
}
GLuint program(const char* vs, const char* fs)
{
    GLuint v=shader(GL_VERTEX_SHADER,vs), f=shader(GL_FRAGMENT_SHADER,fs), p=glCreateProgram();
    glAttachShader(p,v); glAttachShader(p,f); glLinkProgram(p); glDeleteShader(v); glDeleteShader(f);
    GLint ok; glGetProgramiv(p,GL_LINK_STATUS,&ok);
    if (!ok) { char log[4096]; glGetProgramInfoLog(p,sizeof(log),nullptr,log); throw std::runtime_error(log); }
    return p;
}
const char* vertex=R"(#version 300 es
precision highp float;
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in uint colour;
layout(location=3) in vec2 uv;
uniform mat4 wvp, world;
uniform vec4 palette[20];
uniform vec3 eye;
uniform float zFade, fog;
uniform int fogColour, lighting;
out vec4 vColour;
out vec2 vUV;
void main() {
    vec4 clip=wvp*vec4(position,1.0);
    gl_Position=clip;
    gl_Position.z=2.0*clip.z-clip.w;
    vec3 wp=(world*vec4(position,1.0)).xyz;
    vec3 n=mat3(world)*normal;
    float light=1.0;
    if (lighting!=0) {
        if (dot(wp-eye,n)<0.0)
            light=0.35+max(dot(n,normalize(vec3(1.,.5,.5))),0.)*.8+max(dot(n,normalize(vec3(-1.,.5,.5))),0.)*.2;
        else light=0.175;
    }
    vColour=mix(palette[fogColour],clamp(light,0.,1.)*palette[int(colour)],exp(-length(clip.xyz)*fog));
    vColour*=exp(-clamp(wp.z,0.,32.)*zFade);
    vUV=uv;
})";
const char* fragment=R"(#version 300 es
precision highp float;
in vec4 vColour;
in vec2 vUV;
uniform float dissolved, noise;
out vec4 outputColour;
float rnd(vec2 p) { return fract(sin(dot(p,vec2(12.9898,78.233)))*43758.5453); }
void main() {
    if (dissolved>0.0 && rnd(fract(vUV+noise))<dissolved) discard;
    outputColour=vec4(vColour.rgb,1.0);
})";
const char* postVertex=R"(#version 300 es
out vec2 uv;
void main() {
    vec2 p=vec2(float((gl_VertexID<<1)&2),float(gl_VertexID&2));
    uv=p; gl_Position=vec4(p*2.0-1.0,0.0,1.0);
})";
const char* postFragment=R"(#version 300 es
precision highp float;
in vec2 uv;
uniform sampler2D scene;
uniform float fade, desaturate, dissolve, noise;
out vec4 outputColour;
void main() {
    float r=fract(sin(dot(fract(uv+noise),vec2(12.9898,78.233)))*43758.5453);
    if (r<dissolve) { outputColour=vec4(0.,0.,0.,1.); return; }
    vec3 c=texture(scene,uv).rgb;
    c=mix(c,vec3(dot(c,vec3(.299,.587,.114))),desaturate)*(1.0-fade);
    outputColour=vec4(c,1.0);
})";
}

View::View()
{
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion=2; attrs.alpha=false; attrs.depth=true; attrs.antialias=true;
    auto context=emscripten_webgl_create_context("#canvas",&attrs);
    if (context<=0 || emscripten_webgl_make_context_current(context)!=EMSCRIPTEN_RESULT_SUCCESS)
        throw std::runtime_error("WebGL 2 is required. Enable hardware acceleration or try another browser.");
    m_program=program(vertex,fragment); m_postProgram=program(postVertex,postFragment);
    glGenVertexArrays(1,&m_vao); glBindVertexArray(m_vao);
    glGenFramebuffers(1,&m_framebuffer); glGenTextures(1,&m_texture); glGenRenderbuffers(1,&m_depth);
}
View::~View()
{
    glDeleteProgram(m_program); glDeleteProgram(m_postProgram); glDeleteVertexArrays(1,&m_vao);
    glDeleteFramebuffers(1,&m_framebuffer); glDeleteTextures(1,&m_texture); glDeleteRenderbuffers(1,&m_depth);
}
void View::BeginScene()
{
    int w,h; emscripten_get_canvas_element_size("#canvas",&w,&h);
    if (w!=m_width || h!=m_height) {
        m_width=w; m_height=h;
        glBindTexture(GL_TEXTURE_2D,m_texture);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glBindRenderbuffer(GL_RENDERBUFFER,m_depth);
        glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,w,h);
        glBindFramebuffer(GL_FRAMEBUFFER,m_framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_texture,0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,m_depth);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Unable to allocate the WebGL scene framebuffer");
    }
    float aspect=static_cast<float>(w)/std::max(h,1);
    m_viewProjection=m_camera.GetViewMatrix()*XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov),aspect,NEAR_CLIP,FAR_CLIP);
    m_orthographic=XMMatrixOrthographicOffCenterLH(0,1000.f*aspect,0,1000,NEAR_CLIP,FAR_CLIP);
    m_noise=m_noise_enabled ? static_cast<float>(random_uint32())/4294967296.0f : 0.f;
    glBindFramebuffer(GL_FRAMEBUFFER,m_framebuffer); glViewport(0,0,w,h);
    auto c=m_vertexConstants.Palette[m_fill_colour_idx];
    glClearColor(c.x,c.y,c.z,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
    glBindVertexArray(m_vao); glUseProgram(m_program);
}
void View::Render(IGame* game)
{
    game->Render(this);
    glBindFramebuffer(GL_FRAMEBUFFER,0); glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);
    glUseProgram(m_postProgram);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,m_texture);
    glUniform1i(glGetUniformLocation(m_postProgram,"scene"),0);
    glUniform1f(glGetUniformLocation(m_postProgram,"fade"),m_pixelConstants.view_fade);
    glUniform1f(glGetUniformLocation(m_postProgram,"desaturate"),m_pixelConstants.view_desaturate);
    glUniform1f(glGetUniformLocation(m_postProgram,"dissolve"),m_pixelConstants.view_dissolve);
    glUniform1f(glGetUniformLocation(m_postProgram,"noise"),m_noise);
    glDrawArrays(GL_TRIANGLES,0,3);
}
void View::DrawModel(Model& model, const Model& linked)
{
    if (!model.m_pHeapVertices) {
        model.m_pHeapVertices=std::make_shared<WebBuffer>();
        glBindBuffer(GL_ARRAY_BUFFER,model.m_pHeapVertices->id);
        glBufferData(GL_ARRAY_BUFFER,model.m_pVertices->size()*sizeof(Vertex),model.m_pVertices->data(),GL_STATIC_DRAW);
    } else glBindBuffer(GL_ARRAY_BUFFER,model.m_pHeapVertices->id);
    if (!model.m_pHeapIndices) {
        model.m_pHeapIndices=std::make_shared<WebBuffer>();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,model.m_pHeapIndices->id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,model.m_pIndices->size()*sizeof(uint32_t),model.m_pIndices->data(),GL_STATIC_DRAW);
    } else glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,model.m_pHeapIndices->id);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,pos)));
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,normal)));
    glEnableVertexAttribArray(2); glVertexAttribIPointer(2,1,GL_UNSIGNED_INT,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,colour)));
    glEnableVertexAttribArray(3); glVertexAttribPointer(3,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,texcoord)));
    auto world=model.GetWorldMatrix(linked);
    auto wvp=world*(model.orthographic?m_orthographic:m_viewProjection);
    XMFLOAT4X4 w,wp; XMStoreFloat4x4(&w,world); XMStoreFloat4x4(&wp,wvp);
    // DirectX row-major row-vector matrices become GLSL column-vector matrices on upload.
    glUniformMatrix4fv(glGetUniformLocation(m_program,"world"),1,GL_FALSE,&w._11);
    glUniformMatrix4fv(glGetUniformLocation(m_program,"wvp"),1,GL_FALSE,&wp._11);
    glUniform4fv(glGetUniformLocation(m_program,"palette"),PALETTE_SIZE,&m_vertexConstants.Palette[0].x);
    auto eye=GetEyePosition(); glUniform3f(glGetUniformLocation(m_program,"eye"),eye.x,eye.y,eye.z);
    glUniform1i(glGetUniformLocation(m_program,"lighting"),model.lighting);
    glUniform1i(glGetUniformLocation(m_program,"fogColour"),m_vertexConstants.fog_colour_idx);
    glUniform1f(glGetUniformLocation(m_program,"fog"),m_vertexConstants.fog_density);
    glUniform1f(glGetUniformLocation(m_program,"zFade"),m_vertexConstants.z_fade);
    glUniform1f(glGetUniformLocation(m_program,"dissolved"),model.dissolved);
    glUniform1f(glGetUniformLocation(m_program,"noise"),m_noise);
    if (model.type==ModelType::Landscape) glDisable(GL_CULL_FACE);
    else { glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CW); }
    glDrawElements(GL_TRIANGLES,model.m_pIndices->size(),GL_UNSIGNED_INT,nullptr);
}
