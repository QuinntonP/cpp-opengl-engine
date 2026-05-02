#include "Engine/texture.h"
#include "Engine/stb_image.h"
#include <iostream>

Texture::Texture(const std::string& filepath) : ID(0), width(0), height(0), channels(0) {
    load(filepath);
}

Texture::~Texture() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}

bool Texture::load(const std::string& filepath) {
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return false;
    }

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    // Set texture parameters for pixelated/retro look
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Use NEAREST filtering for crisp pixels (no blurring)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Determine format
    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    std::cout << "Loaded texture: " << filepath << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
    return true;
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
