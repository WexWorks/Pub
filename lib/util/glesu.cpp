// Copyright (c) 2011 by WexWorks, LLC -- All Rights Reserved

#include "util/glesu.h"
#include "util/debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

namespace ww {
namespace glesu {

void CheckError(const char* op) {
  for (GLint error = glGetError(); error; error = glGetError()) {
    Error("after %s() glError (0x%x)\n", op, error);
  }
}

char *LoadTextFile(const char *filename) {
  char *text = NULL;
  FILE *file = NULL;
  long char_count = 0;

  file = fopen(filename, "r");
  if (file == NULL) {
    Error("Cannot read file '%s': %s.\n", filename, strerror(errno));
    exit(-1);
  }

  fseek(file, 0, SEEK_END);
  char_count = ftell(file);
  fseek(file, 0, SEEK_SET);
  text = (char *) calloc(char_count + 1, sizeof(char));
  fread(text, sizeof(char), char_count, file);
  text[char_count] = '\0';
  fclose(file);

  return text;

}

GLuint CompileShader(GLenum shader_type, const char *source_code) {
  GLuint shader = glCreateShader(shader_type);
  if (!shader) {
    Error("Cannot create shader type %d", shader_type);
    return 0;
  }

  glShaderSource(shader, 1, &source_code, NULL);
  glCompileShader(shader);
  CheckError("glCompileShader");
  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen) {
      char* buf = (char*) malloc(infoLen);
      if (buf) {
        glGetShaderInfoLog(shader, infoLen, NULL, buf);
        Error("Could not compile shader %d:\n\t%s\n", shader_type, buf);
        free(buf);
      }
      glDeleteShader(shader);
      shader = 0;
    }
  }

  return shader;
}

GLuint CreateProgram(GLuint vp, GLuint fp) {
  if (!vp || !fp) {
    Error("Invalid shaders %d and %d", vp, fp);
    return 0;
  }

  GLuint program = glCreateProgram();
  if (!program) {
    Error("Cannot create program");
    return 0;
  }

  glAttachShader(program, vp);
  CheckError("glAttachShader vp");
  glAttachShader(program, fp);
  CheckError("glAttachShader fp");
  glLinkProgram(program);
  GLint linkStatus = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus != GL_TRUE) {
    GLint bufLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
    if (bufLength) {
      char* buf = (char*) malloc(bufLength);
      if (buf) {
        glGetProgramInfoLog(program, bufLength, NULL, buf);
        Error("Could not link program:\n%s\n", buf);
        free(buf);
      }
    }
    glDeleteProgram(program);
    program = 0;
  }
  CheckError("CreateProgram");

  return program;
}

class ProgramCache {
public:
  ProgramCache() :
    path_(NULL), program_count_(0), program_alloc_(0), program_(NULL), name_(NULL) {
#if LINUX || ANDROID
    sep_ = ':';
#endif
#if WINDOWS
    sep_ = ';';
#endif
  }

  virtual ~ProgramCache() {
    free(path_);
    free(program_);
    free(name_);
  }

  bool AddPath(const char *path) {
    free(path_);
    char *oldpath = path_;
    int oldlen = path_ ? strlen(path_) : 0;
    int newlen = oldlen + strlen(path) + 1;
    path_ = (char *)malloc(newlen);
    if (path_ == NULL) {
      path_ = oldpath;
      return false;
    }
    if (oldpath) {
      strcpy(path_, oldpath);
      path_[oldlen] = sep_;
      strcpy(&path_[oldlen+1], path);
      free(oldpath);
    } else {
      strcpy(path_, path);
    }
    return true;
  }

  bool Invalidate() {
    memset(program_, 0, program_count_ * sizeof(GLuint));
    return true;
  }

  GLuint Lookup(const char *name) {
    for (int i = 0; i < program_count_; ++i) {
      if (!strcmp(name, name_[i])) {
        if (!program_[i])
          program_[i] = Load(name);
        return program_[i];
      }
    }
    if (Add(name)) {
      program_[program_count_ - 1] = Load(name);
      return program_[program_count_ - 1];
    }
    return 0;
  }

private:
  ProgramCache(const ProgramCache&);
  void operator=(const ProgramCache&);

  bool Add(const char *name) {
    if (++program_count_ > program_alloc_) {
      program_alloc_ = program_alloc_ ? 2 * program_alloc_ : 32;
      program_ = (GLuint *) realloc(program_, program_alloc_ * sizeof(GLuint));
      name_ = (const char **) realloc(name_, program_alloc_ * sizeof(char *));
    }
    program_[program_count_ - 1] = 0;
    name_[program_count_ - 1] = name;
    return true;
  }

  GLuint Load(const char *name) {
    char path[4096];
    if (path_) {
      strcpy(path, path_);
    } else {
      path[0] = '.';
      path[1] = 0;
    }
    GLuint program = 0;
    for (const char *p = strtok(path, &sep_); p; p = strtok(NULL, &sep_)) {
      char basename[1024];
      sprintf(basename, "%s/%s", p, name);
      char filename[1024];
      sprintf(filename, "%s.vp", basename);
      const char *code = glesu::LoadTextFile(filename);
      if (!code)
        return false;
      GLuint vp = glesu::CompileShader(GL_VERTEX_SHADER, code);
      if (!vp)
        return false;
      sprintf(filename, "%s.fp", basename);
      code = glesu::LoadTextFile(filename);
      if (!code)
        return false;
      GLuint fp = glesu::CompileShader(GL_FRAGMENT_SHADER, code);
      if (!fp)
        return false;
      program = glesu::CreateProgram(vp, fp);
      if (!program)
        return false;
    }
    return program;
  }

  char *path_;
  char sep_;
  int program_count_;
  int program_alloc_;
  GLuint *program_;
  const char **name_;
};

static ProgramCache program_cache_;

bool AddShaderPath(const char *path) {
  return program_cache_.AddPath(path);
}

bool InvalidateProgramCache() {
  return program_cache_.Invalidate();
}

GLuint LoadProgram(const char *name) {
  return program_cache_.Lookup(name);
}

} // glesu
} // ww
