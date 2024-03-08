#include "asset.h"

#include <fstream>


Assets ats = {};

const Texture &get_texture(const String &name) {
    assert(ats.textures.contains(name));
    return ats.textures[name];
}
Texture load_texture(const String &file, const String &name) {
    auto tex = load_texture_from_file(file);
    if(!tex.has_value()) {
        panic("Couldn't load texture " + file);
    }
    if(name.empty()) {
        ats.textures[file] = *tex;
    } else {
        ats.textures[name] = *tex;
    }
    return *tex;
}

void assets_destroy() {
    for(auto &[name, shader]: ats.shaders) {
        destroy_and_unload(*shader);
        delete shader;
    }
    ats.shaders.clear();
}
void destroy_and_unload(Shader &shader) {
    shader.name.clear();
    // rl::UnloadShader(shader.shader);
    glDeleteShader(shader.internal);
}
Shader *load_shader(const String &name, bool reload) {
    auto shader = raw_load_shader(name);
    Shader *ptr;
    if(!reload) {
        ptr = new Shader;
        *ptr = shader;
        ats.shaders[shader.name] = ptr;
    } else {
        ptr = ats.shaders[shader.name];
        destroy_and_unload(*ptr);
        *ptr = shader;
    }
    return ptr;
}

Shader *get_shader(const String &name) {
    auto it = ats.shaders.find(name);
    if(it == ats.shaders.end()) {
        panic("shader doesn't exist");
    }
    return it->second;
}
void assets_check_reload() {
    namespace fs = std::filesystem;
    for(auto &[_, it]: ats.shaders) {
        String path = String("assets/shaders/")+it->name+".shader";
        auto tpath = fs::path(path);
        u64 stamp  = fs::last_write_time(tpath).time_since_epoch().count();
        if(it->stamp < stamp) {
            println("reload shader: ", it->name);
            load_shader(it->name, true);
        }
    }
}
/* rl::Texture raw_load_texture(const String &name) {
    auto path = String("assets/basic/");
    auto suffix = ".png";
    path.append(name).append(suffix);
    return rl::LoadTexture(path.c_str());
} */
/* void load_texture(Assets &ats, String name) {
    auto tex = raw_load_texture(name);
    ats.textures["block"] = tex;
} */
Shader raw_load_shader(String name) {
    namespace fs = std::filesystem;
    
    String path = "assets/shaders/"+name+".shader";
    std::ifstream file(path);
    if(file.fail()) {
        panic("Shader file doesn't exist");
    }
    std::stringstream stream;
    stream << file.rdbuf();
    String s = stream.str();
    const String FRAGMENT = "#FRAGMENT";
    const String VERTEX = "#VERTEX";
    const String VERSION = "#version 330\n";
    auto vert = s.find(VERTEX);
    auto frag = s.find(FRAGMENT);

    if(frag == s.npos) {
        printerr("Couldn't reload shader read:\n" + s + "\n");
        release_assert(false);
    }

    bool use_default_vert = false;
    String vert_src;
    if(vert == s.npos) {
        use_default_vert = true;
    } else {
        vert_src = s.substr(vert, frag-vert).erase(0, VERTEX.size());
        vert_src.insert(0, VERSION);
    }    
    auto sub = s.substr(frag, s.size());
    String frag_src = sub.erase(sub.find(FRAGMENT), FRAGMENT.size());
    frag_src.insert(0, VERSION);
    
    auto tpath = fs::path(path);
    u64 stamp  = fs::last_write_time(tpath).time_since_epoch().count();
    
    if(use_default_vert) {
        panic("Todo");
        
    }
    
    GLint status;
    auto glvert = glCreateShader(GL_VERTEX_SHADER);
    auto glfrag = glCreateShader(GL_FRAGMENT_SHADER);
    auto shader_src = vert_src.c_str();
    glShaderSource(glvert, 1, &shader_src, nullptr);
    glCompileShader(glvert);
    glGetShaderiv(glvert, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE) {
        // void glGetShaderInfoLog(GLuint shader​, GLsizei maxLength​, GLsizei *length​, GLchar *infoLog​);
        String string(1024, '\0');     
        
        glGetShaderInfoLog(glvert, 1024, nullptr, &string[0]);
        println(string);
        panic("");
    }
    
    shader_src = frag_src.c_str();
    glShaderSource(glfrag, 1, &shader_src, nullptr);
    glCompileShader(glfrag);
    glGetShaderiv(glfrag, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        // void glGetShaderInfoLog(GLuint shader​, GLsizei maxLength​, GLsizei *length​, GLchar *infoLog​);
        String string(1024, '\0');     
        
        glGetShaderInfoLog(glfrag, 1024, nullptr, &string[0]);
        println(string);
        panic("");
    }
    auto shader = glCreateProgram();
    glAttachShader(shader, glvert);
    glAttachShader(shader, glfrag);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);
    glDeleteShader(glvert);
    glDeleteShader(glfrag);
    return Shader{stamp, name, shader};
}


