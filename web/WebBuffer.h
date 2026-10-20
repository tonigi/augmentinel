#pragma once
struct WebBuffer
{
    GLuint id{};
    WebBuffer() { glGenBuffers(1, &id); }
    ~WebBuffer() { glDeleteBuffers(1, &id); }
    WebBuffer(const WebBuffer&) = delete;
    WebBuffer& operator=(const WebBuffer&) = delete;
};
