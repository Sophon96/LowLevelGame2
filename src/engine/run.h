#ifndef RUN_H
#define RUN_H

#define LLG_GL_VER_MAJOR 3
#define LLG_GL_VER_MINOR 3

struct Config {
    double deltaTimeLimit = 3.0;
    bool limitFPS = true;
    bool vsync = true;
    int maxFPS = 100;
    int physicsTPS = 60;
};

struct WindowSize {
    int width = 1920 / 2;
    int height = 1080 / 2;
};

struct StatePackage {
    Config *config;
    WindowSize *windowSize;
};

int run();

#endif //RUN_H