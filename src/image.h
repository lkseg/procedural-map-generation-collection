#pragma once
#include "util.h"
// #include "linalg.h"


struct Texture {
    u32 internal; // Rendering
    struct {
        i32 x;
        i32 y;
    } size;
};

struct Color {
    byte r, g, b, a;    

    Color();    
    Color(byte, byte, byte);
    Color(byte, byte, byte, byte);
    static Color xyz(f32, f32, f32);
    static const Color White;
    static const Color Black;
};


Color scale(const Color &, f32);
Color scale_rgb(const Color &, f32);
// scale_rgb
Color operator*(const Color &, f32);

Maybe<Texture> load_texture_from_file(const String &);
Maybe<Texture> load_texture(byte *, i32, i32);
void destroy_texture(Texture &);


