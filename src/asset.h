#ifndef ASSET_H
#define ASSET_H

#include "util.h"
#include "renderer.h"

#include <filesystem>
#include <GLES3/gl3.h>


struct Assets {
    Map<String, Shader *> shaders = Map<String, Shader *>(1000);    
    // Map<String, rl::Texture2D> textures = Map<String, rl::Texture2D>(1000);
    // Map<String, rl::Model> models = Map<String, rl::Model>(1000);
};

Shader *load_shader(const String &, bool reload = false);
Shader *get_shader(const String &);

void assets_check_reload();
void assets_destroy();
void destroy_and_unload(Shader &);
Shader raw_load_shader(String);


#endif // ASSET_H