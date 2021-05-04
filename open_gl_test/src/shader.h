#ifndef SHADERS_H
#define SHADERS_H

#include "main.h"
#include "glm/glm.hpp"

extern GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
extern glm::vec3 load_texture(int frame_idx, GLuint handle, const char* where);
extern void load_cubemap(int frame_idx, GLuint handle, const std::vector<std::string>& file_locs);

#endif