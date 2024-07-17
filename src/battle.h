#ifndef BATTLE_H
#define BATTLE_H

#include "pokemon.h"
#include "character.h"
#include "pokemon_data.h"
#include "display.h"
#include "map.h"
#include <menu.h>
#include <ncurses.h>
#include <cstring>
#include <unistd.h> 
#include <cmath>
#include <vector>
#include <algorithm>


void executeMove(moves_t move, pokemon_t *defendingPokemon, pokemon_t *attackingPokemon);

void executeMoveCycle(int moveId, pokemon_t *enemyPokemon, pokemon_t *playerPokemon);

int healPokemon(int item, pokemon_t *hurtPokemon, map *currentScreen);

int getExperience(pokemon_t *playerPokemon, pokemon_t *enemyPokemon, int trainer);

int addExperience(pokemon_t *playerPokemon, int exp);

int getPokemonStat(int level, int iv, int base, int hp);

std::vector<int> getMoves(pokemon_t *playerPokemon, int newPokemon);

void getMoney(character_t *player, pokemon_t *enemyPokemon);
#endif