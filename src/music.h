#ifndef MUSIC_H
#define MUSIC_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <vector>

extern int volume;

extern std::vector<Mix_Music*> music;
extern std::vector<Mix_Chunk*> sounds;

int loadMusic(const char* filename);
int loadSound(const char* filename);
void setVolume(int v);

int playMusic(int m);
int playSound(int s);

int initMixer();
int quitMixer();

void togglePlay();

void playAndWait(int channel, int s, int loops);
#endif