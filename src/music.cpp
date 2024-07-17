#include "music.h"


std::vector<Mix_Music*> music;
std::vector<Mix_Chunk*> sounds;

int volume;

int loadMusic(const char* filename){
    Mix_Music *m = NULL;
    m = Mix_LoadMUS(filename);
    if(m == NULL){
        printf("Failed to load music. SLD_Mixer error %s\n", Mix_GetError());
    }
    music.push_back(m);
    return music.size()-1;
}
int loadSound(const char* filename){
    Mix_Chunk *m = NULL;
    m = Mix_LoadWAV(filename);
    if(m == NULL){
        printf("Failed to load sound. SLD_Mixer error %s\n", Mix_GetError());
    }
    sounds.push_back(m);
    return sounds.size()-1;
}
void setVolume(int v){
    volume = (MIX_MAX_VOLUME * v) / 100;
}

int playMusic(int m){

    Mix_Volume(1,volume);
    Mix_PlayMusic(music[m], -1);

    return 0;
}

int playSound(int s){
    Mix_Volume(-1, volume);
    Mix_PlayChannel(-1, sounds[s], 0);
    return 0;
}

int initMixer(){
    Mix_Init(MIX_INIT_MP3);
    SDL_Init(SDL_INIT_AUDIO);
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 256) < 0){
        printf("SDL_Mixer couldn't init. ERR: %s\n", Mix_GetError());
    }
    setVolume(80);
    return 0;
}
int quitMixer(){
    for(int s = 0; s < music.size(); s++){
        Mix_FreeMusic(music[s]);
        music[s] = NULL;
    }
    return 0;
}

void togglePlay(){
    if(Mix_PausedMusic() == 1){
        Mix_ResumeMusic();
    }
    else{
        Mix_PauseMusic();
    }
}
void playAndWait(int channel, int s, int loops)
{
    channel = Mix_PlayChannel(channel, sounds[s], loops);
    if (channel < 0) {
        return; // error
    }
    while (Mix_Playing(channel) != 0) {
        SDL_Delay(0);
    }
}