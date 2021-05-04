// // Include standard headers
#ifndef MAIN_H
#define MAIN_H
#define MAX_PARTICLES 100

#include <iostream>
#include <fstream>
#include <nanogui/nanogui.h> // already includes  #include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include "misc/getopt.h" // getopt for windows
#else
#include <getopt.h>
#include <unistd.h>
#endif

#include <unordered_set>

typedef uint32_t gid_t;

using namespace std;
using namespace nanogui;

#endif