#ifndef POKEMON_H
#define POKEMON_H

#include <cstdlib>
#include <climits>

typedef class pokemon {
  public:
   pokemon() : id(INT_MAX), species_id(INT_MAX), level(0), attack(INT_MAX), defense(INT_MAX), special_attack(INT_MAX), special_defense(INT_MAX), speed(INT_MAX), hp(0), maxHp(INT_MAX), shiny(INT_MAX),  pokemon_moves{-1,-1,-1,-1}, iv_attack(INT_MAX), iv_defense(INT_MAX), iv_special_attack(INT_MAX), iv_special_defense(INT_MAX), iv_speed(INT_MAX), iv_hp(INT_MAX),
   base_attack(INT_MAX), base_hp(INT_MAX), base_defense(INT_MAX), base_special_attack(INT_MAX), base_special_defense(INT_MAX), base_speed(INT_MAX)
 {}
   int id;
   int species_id;

   int level;
   int moves; //Amount of moves
   int attack;
   int defense;
   int special_attack;
   int special_defense;
   int speed;
   int hp;

   int iv_attack;
   int iv_defense;
   int iv_special_attack;
   int iv_special_defense;
   int iv_speed;
   int iv_hp;

   int base_attack;
   int base_hp;
   int base_defense;
   int base_special_attack;
   int base_special_defense;
   int base_speed;

   int maxHp; //How much hp this pokemon can have
   int baseExp;
   int expNeeded; // how much experience needed to level up
   int expCurrent; //how much experience currently have
   int growthRate;
   int shiny; //0 for no, 1 for yes
   int pokemon_moves[4];
} pokemon_t;

#endif