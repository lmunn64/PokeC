#ifndef DISPLAY_H
#define DISPLAY_H

#include "pokemon.h"
#include "character.h"
#include <menu.h>
#include <ncurses.h>
#include <unistd.h>
#include <sstream>
#include "terrain.h"
#include "map.h"
#include <cstring>
#include "pokemon_data.h"
#include "battle.h"
#include "music.h"
extern int aButton;
void writeToMainTextWin(std::string text, int where);

int displayStarterPokemon();

void displayPokemonBattle(map *currentScreen,pokemon_t *enemyPokemon);

void displayTrainerBattle(character_t *trainer, map *currentScreen);

int displayMoves(pokemon_t *trainerPokemon, pokemon_t *playerPokemon, int newMove);

int displayPokemon(int item, int deadSwap, map *currentScreen);

void displayWarning(const char* message);

void printMap(int currentRow, int currentColumn, map *currentScreen);

int displayFlyMenu(int currentRow, int currentColumn, map *currentScreen);

void displayMandCInterface(int display, map *currentScreen);

void displayTrainerMenu(map *currentScreen);

int displayItems(pokemon_t *wildPokemon, int wild, map *currentScreen);

void displayLevelUp(pokemon_t *trainerPokemon);

void print_in_middle(WINDOW *win, int starty, int startx, int width, const char *string, chtype color);
#endif