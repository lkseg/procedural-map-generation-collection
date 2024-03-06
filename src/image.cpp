#define STB_IMAGE_IMPLEMENTATION
#include "image.h"
#include "stb_image.h"

#include <GLFW/glfw3.h>



const Color Color::White = Color{255,255,255,255};
const Color Color::Black = Color{0,0,0,255};


Maybe<Texture> load_texture(const String &file) {
    const String DIR = "./assets/textures/";
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load((DIR+file).c_str(), &image_width, &image_height, NULL, 4);
    
    if (image_data == nullptr)
        return {};

    auto ret = load_texture(image_data, image_width, image_height);
    stbi_image_free(image_data);
    return ret;
}

Maybe<Texture> load_texture(byte *data, i32 width, i32 height) {
GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    


    return Texture{.internal = image_texture, .size = {width, height}};
}
void destroy_texture(Texture &texture) {
    glDeleteTextures(1, &texture.internal);
}


Color::Color() {}

Color::Color(byte _r, byte _g, byte _b): r(_r), g(_g), b(_b), a(255) {}
Color::Color(byte _r, byte _g, byte _b, byte _a): r(_r), g(_g), b(_b), a(_a) {}

Color Color::xyz(f32 r, f32 g, f32 b) {
    return Color(r*255.f, g*255.f, b*255.f);
}
Color scale(const Color &color, f32 f) {
    byte r = f32(color.r) * f;
    byte g = f32(color.g) * f;
    byte b = f32(color.b) * f;
    byte a = f32(color.a) * f;
    return {r, g, b, a};    
}

Color scale_rgb(const Color &color, f32 f) {
    byte r = f32(color.r) * f;
    byte g = f32(color.g) * f;
    byte b = f32(color.b) * f;
    return {r, g, b, color.a};    
}
Color operator*(const Color &color, f32 f) {
    return scale_rgb(color, f);
}
