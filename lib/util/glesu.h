// Copyright (c) 2011 by WexWorks, LLC -- All Rights Reserved

#ifndef GLESU_H_
#define GLESU_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

namespace ww {
namespace glesu {

void CheckError(const char* op);
char *LoadTextFile(const char *filename);
GLuint CompileShader(GLenum shader_type, const char *source_code);
GLuint CreateProgram(GLuint vp, GLuint fp);
bool AddShaderPath(const char *path); // Add path to search list used in LoadProgram
GLuint LoadProgram(const char *name); // load name.{fp,vp} and cache compiled result
bool InvalidateProgramCache();

} // namespace glesu
} // namespace ww

#endif   // GLESU_H_
