// *******************************
// ShaderMgrSAM.h - Version 1.4 - September 3, 2020
//
// ShaderMgrSAM.cpp code defines, compiles and manages shaders
//           for the SimpleAnimModern.cpp program
//
// Author: Sam Buss
//
// Software is "as-is" and carries no warranty. It may be used without
//  restriction, but if you modify it, please change the filenames to
//  prevent confusion between different versions.
// Bug reports: Sam Buss, sbuss@ucsd.edu
// *******************************


#pragma once

void setup_shaders();
unsigned int setup_shader_vertfrag(const char* vertexShaderSource, const char* fragmentShaderSource);

GLuint check_compilation_shader(GLuint shader);
GLuint check_link_status(GLuint program);



