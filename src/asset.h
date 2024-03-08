#ifndef ASSET_H
#define ASSET_H

#include "util.h"
#include "renderer.h"
#include "image.h"

#include <filesystem>
#include <GLES3/gl3.h>


struct Assets {
    // Will be dynamically reloaded
    Map<String, Shader *> shaders = Map<String, Shader *>(1000);    
    Map<String, Texture> textures = Map<String, Texture>(1000);
};

Texture load_texture(const String &file, const String &name = "");
Shader *load_shader(const String &, bool reload = false);
Shader *get_shader(const String &);
const Texture &get_texture(const String &);
void assets_check_reload();
void assets_destroy();
void destroy_and_unload(Shader &);
Shader raw_load_shader(String);


#endif // ASSET_H