#ifndef POKEMON_DATA_H
#define POKEMON_DATA_H

#include <vector>
#include <stdint.h>
#include <string>
#include <climits>

////////////////////////POKEDEX////////////////////////


typedef class moves{
   public:
    moves() : id(INT_MAX), identifier(), generation_id(INT_MAX), type_id(INT_MAX), power(0), pp(INT_MAX), accuracy(INT_MAX), priority(INT_MAX), target_id(INT_MAX), damage_class_id(INT_MAX), effect_id(INT_MAX), effect_chance(INT_MAX), contest_type_id(INT_MAX), is_default(INT_MAX), contest_effect_id(INT_MAX), super_contest_effect_id(INT_MAX) {}
    int id;
    std::string identifier;
    int generation_id;
    int type_id;
    int power;
    int pp;
    int accuracy;
    int priority;
    int target_id;
    int damage_class_id;
    int effect_id;
    int effect_chance;
    int contest_type_id;
    int is_default;
    int contest_effect_id;
    int super_contest_effect_id;
} moves_t;

typedef class pokemonCSV{
   public:
    pokemonCSV() : id(INT_MAX), identifier(), species_id(INT_MAX), height(INT_MAX), weight(INT_MAX), base_experience(INT_MAX), order(INT_MAX), is_default(INT_MAX) {}
    int id;
    std::string identifier;
    int species_id;
    int height;
    int weight;
    int base_experience;
    int order;
    int is_default;
} pokemonCSV_t;

typedef class experience_t{
   public:
    experience_t() : growth_rate_id(INT_MAX), level(INT_MAX), experience(INT_MAX) {}
    int growth_rate_id;
    int level;
    int experience;
} experience_t;

typedef class pokemon_moves{
   public:
    pokemon_moves() : pokemon_id(), version_group_id(INT_MAX), move_id(INT_MAX), pokemon_move_method_id(INT_MAX), level(INT_MAX), order(INT_MAX) {}
    int pokemon_id;
    int version_group_id;
    int move_id;
    int pokemon_move_method_id;
    int level;
    int order;
} pokemon_moves_t;

typedef class pokemon_species{
   public:
    pokemon_species() : id(INT_MAX), identifier(), generation_id(INT_MAX), evolves_from_species_id(INT_MAX), evolution_chain_id(INT_MAX), color_id(INT_MAX), shape_id(INT_MAX), habitat_id(INT_MAX), gender_rate(INT_MAX), capture_rate(INT_MAX), base_happiness(INT_MAX), is_baby(INT_MAX), hatch_counter(INT_MAX), has_gender_differences(INT_MAX), growth_rate_id(INT_MAX), forms_switchable(INT_MAX), is_legendary(INT_MAX), is_mythical(INT_MAX), order(INT_MAX), conquest_order(INT_MAX) {}
    int id;
    std::string identifier;
    int generation_id;
    int evolves_from_species_id;
    int evolution_chain_id;
    int color_id;
    int shape_id;
    int habitat_id;
    int gender_rate;
    int capture_rate;
    int base_happiness;
    int is_baby;
    int hatch_counter;
    int has_gender_differences;
    int growth_rate_id;
    int forms_switchable;
    int is_legendary;
    int is_mythical;
    int order;
    int conquest_order;
} pokemon_species_t;

typedef class pokemon_stats{
   public:
    pokemon_stats() : pokemon_id(INT_MAX), stat_id(INT_MAX), base_stat(INT_MAX), effort(INT_MAX) {}
    int pokemon_id;
    int stat_id;
    int base_stat;
    int effort;
} pokemon_stats_t;

typedef class pokemon_types{
   public:
    pokemon_types() : pokemon_id(INT_MAX), type_id(INT_MAX), slot(INT_MAX) {}
    int pokemon_id;
    int type_id;
    int slot;
} pokemon_types_t;

typedef class stats{
   public:
    stats() : id(INT_MAX), damage_class_id(INT_MAX), identifier(), is_battle_only(INT_MAX), game_index(INT_MAX) {}
    int id;
    int damage_class_id;
    std::string identifier;
    int is_battle_only;
    int game_index;
} stats_t;

typedef class type_names{
   public:     
    type_names() : type_id(INT_MAX), local_language_id(INT_MAX), name("") {}
    int type_id;
    int local_language_id;
    std::string name;
} type_names_t;

extern std::string pokedex[9];

extern std::vector<moves_t> movesVector;
extern std::vector<experience_t> experienceVector;
extern std::vector<pokemonCSV_t> pokemonCSVVector;
extern std::vector<pokemon_moves_t> pokemonMovesVector;
extern std::vector<pokemon_species_t> pokemonSpecVector;
extern std::vector<pokemon_stats_t> pokemonStatsVector;
extern std::vector<pokemon_types_t> pokemonTypesVector;
extern std::vector<stats> statsVector;
extern std::vector<type_names> typeNamesVector;

#endif