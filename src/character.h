#ifndef CHARACTER_H
#define CHARACTER_H

#include "heap.h"
#include "pokemon.h"

typedef class character {
  public:
   heap_node_t *hn;
   char symbol; 
   int dir;
   int next_turn;
   int sequence_num;
   int x;
   int y;
   int defeated;
   int pokedollars;
   pokemon_t trainer_pokemon[6];
   int items[3]; //0 = potions, 1 = revives, 2 = pokeballs
} character_t;

#endif