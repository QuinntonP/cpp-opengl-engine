#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

class Texture {
public:
    GLuint ID;
    int width, height, channels;

    Texture(const std::string& filepath);
    ~Texture();

    void bind(unsigned int slot = 0) const;
    void unbind() const;

private:
    bool load(const std::string& filepath);
};

#endif // TEXTURE_H
