#include <stdio.h>
#include <cstdlib>
#include <time.h>
#include <stdint.h>

#include "queue.h"
#include "heap.h"
#include "display.h"
#include "pokemon_data.h"
#include "pokemon.h"
#include "character.h"
#include "terrain.h"
#include "map.h"
#include "battle.h"

#include <unistd.h>
#include <string.h>
#include <menu.h>
#include <ncurses.h>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <climits>
#include <vector>
#include <algorithm>
#include <cmath>

#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]]);

#define INF 9999

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

const char* topper ="------Welcome trainers, to Pokemon for C++!------";

typedef class heapNode{
    public:
     int weight;
     int mapx;
     int mapy;
     int totalDist;
     int id;
} heapNode;

typedef class distMap {
    public:
     int screen[21][80];
} distMap;

typedef class path {
  public:
   heap_node_t *hn;
   uint8_t pos[2];
   uint8_t from[2];
   int32_t cost;
   terrain_type_t terrain;
} path_t;

typedef enum dim {
  dim_x,
  dim_y,
  num_dims
} dim_t;

typedef int16_t pair_t[num_dims];

const char MOUNTAIN = '%';
const char LONGGRASS = ':';
const char CLEARING = '.';
const char FOREST = '^';
const char WATER = '~';
const char EXIT = '#';
const char CENTER = 'C';
const char PLAYER = '@';
const char MART = 'M';
const char *NPC_moves[8] = {"n","s","w","e","nw","se","ne","sw"};
const int NPC_cost[4][11] = {{INF, INF, 10, 50, 50, 15, 10, 15, 15, INF, INF},{INF, INF, 10, 50, 50, 20, 10, INF, INF, INF, INF},{INF, INF, 10, 50, 50, 20, 10, 30, INF, INF, INF},{INF, INF, 10, 10, 10, 20, 10, INF, INF, INF, 11}};

static int32_t path_cmp(const void *key, const void *with) {
  return ((path_t *) key)->cost - ((path_t *) with)->cost;
}

static int32_t turn_cmp(const void *key, const void *with) {
  return ((character_t *) key)->next_turn - ((character_t *) with)->next_turn;
}

//current world location
int currWorldRow, currWorldCol; 
//current exits for current screen
int currExitN;
int currExitS;
int currExitW;
int currExitE;

//2d array of pointers to map classes
map *world[402][402];
distMap *paths[2];

//location of player character
int player;

const char *trainers[6];

typedef int16_t pair_t[num_dims];

void moveNPCs(character_t *t); 

void initMap(int, int, int);

int displayStarterPokemon();


int pathFindCost(terrain_type_t t, int i){
    switch(t){
case ter_boulder:
            return NPC_cost[i][0];
            break;
        case ter_tree:
            return NPC_cost[i][1];
            break;
        case ter_path:
            return NPC_cost[i][2];
            break;
        case ter_mart:
            return NPC_cost[i][3];
            break;
        case ter_center:
            return NPC_cost[i][4];
            break;
        case ter_grass:
            return NPC_cost[i][5];
            break;
        case ter_clearing:
            return NPC_cost[i][6];
            break;
        case ter_mountain:
            return NPC_cost[i][7];
            break;
        case ter_forest:
            return NPC_cost[i][8];
            break;
        case ter_water:
            return NPC_cost[i][9];
            break;
        case ter_gate:
            return NPC_cost[i][10];
            break;
        default:
            return 0;
            break;
        }
        
}

void placeMartandCenter(int i, int a, terrain_type_t screen[21][80]){
    screen[i][a-1] = ter_mart;
    screen[i][a-2] = ter_mart;
    screen[i-1][a-1] = ter_mart;
    screen[i-1][a-2] = ter_mart;
    screen[i][a+1] = ter_center;
    screen[i][a+2] = ter_center;
    screen[i-1][a+1] = ter_center;
    screen[i-1][a+2] = ter_center;
}

void martCenterHelper(terrain_type_t screen[21][80]){
    int mandc = rand() % 17 + 2; //pokecenter and mart between 2 and 18
    for(int i = 0; i < 79; i++){
        if (screen[mandc][i] == ter_path){
            //checks surrounding area to make sure there are no paths interfering with possible mart and center placement.
            if (screen[mandc][i+1] != ter_path && screen[mandc][i+2] != ter_path && screen[mandc-1][i+1] != ter_path && screen[mandc-1][i+2] != ter_path && screen[mandc-1][i-1] != ter_path && screen[mandc-1][i-2] != ter_path){
                placeMartandCenter(mandc, i, screen);
                return;
            } 
            else {
                martCenterHelper(screen);
                return;
            }  
        }
    }
}
/*
   Creates a distance map from the location of the player character. 
   Used for hiker and rival characters to follow the player character
*/
static void dijkstras_path(terrain_type_t screen[21][80], int character){ //0 for hiker, 1 for rival
  heap_t h;
  uint32_t x, y;  
  path_t path[MAP_Y][MAP_X];
  path_t *p;

    for (y = 0; y < MAP_Y; y++) {
        for (x = 0; x < MAP_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
        }
    }


  paths[character] = (distMap *)malloc(sizeof(distMap));
  
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
        paths[character]->screen[y][x] = INF;
        path[y][x].cost = INF;
        path[y][x].terrain = screen[y][x];
    }
  }

  path[player % 79][player / 79].cost = 0;
  paths[character]->screen[player % 79][player / 79] = 0;

  heap_init(&h, path_cmp, NULL);
  
  for (y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) { 
        path[y][x].hn = heap_insert(&h, &path[y][x]);
    }
  }

  while ((p = (path_t *)heap_remove_min(&h))) {
    p->hn = NULL;

    int cost = pathFindCost(path[p->pos[dim_y] - 1][p->pos[dim_x]    ].terrain, character);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] - 1][p->pos[dim_x]    ] = ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y] - 1][p->pos[dim_x]    ]);
    }
   
    cost = pathFindCost(path[p->pos[dim_y]    ][p->pos[dim_x] - 1].terrain, character);
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y]][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y]][p->pos[dim_x] - 1] = ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y]    ][p->pos[dim_x] - 1]);
    }

    cost = pathFindCost(path[p->pos[dim_y]   ][p->pos[dim_x]  + 1].terrain, character);
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y]][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y]][p->pos[dim_x] + 1] =  ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y]    ][p->pos[dim_x] + 1]);
    }

    cost = pathFindCost(path[p->pos[dim_y] + 1][p->pos[dim_x]    ].terrain, character);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] + 1][p->pos[dim_x]    ] = ((p->cost + cost)); 
      heap_insert(&h, &path[p->pos[dim_y] + 1][p->pos[dim_x]    ]);
    }

    cost = pathFindCost(path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].terrain, character);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] - 1][p->pos[dim_x] - 1] = ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y] - 1][p->pos[dim_x] - 1]);
    }

    cost = pathFindCost(path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].terrain, character);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] + 1][p->pos[dim_x] + 1] = ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y] + 1][p->pos[dim_x] + 1]);
    }
    cost = pathFindCost(path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].terrain, character);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] - 1][p->pos[dim_x] + 1] =  ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y] - 1][p->pos[dim_x] + 1]);
    }

    cost = pathFindCost(path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].terrain, character);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] + 1][p->pos[dim_x] - 1] = ((p->cost + cost)); 
      heap_insert(&h, &path[p->pos[dim_y] + 1][p->pos[dim_x] - 1]);
    }
  }

  heap_delete(&h);
  return;
}

/*
   Creates road path connecting the east and west exits of a given map, and
   its north and south exits
*/
void roadPath(int a, int b, int c, int d, terrain_type_t screen[21][80]){    

    int mandc = rand() % 16 + 3;
    if (mandc == 12){
        mandc = mandc + 1;
    }
    //source is exitN targer is exitS
    int i, h;
    
    //move down
    for(i = 1; i < 12; i++){
        screen[i][a] = ter_path;
    }

    //check which exit is lower and move horizontally left or right
    if(a < b){
        for(int k = a; k <= b; k++){
            screen[i][k]= ter_path;
        }
    }
    else if(a > b){
        for(int k = a; k >= b; k--){
            screen[i][k]= ter_path;    
        }
    }

    // move down until exit
    for(int j = i+1; j < 20; j++){
        screen[j][b]= ter_path; 
    }
        
    //source is exitW target is exitE


    // move right
    for(h = 1; h < 40; h++){
        screen[c][h] = ter_path;
    }
    
    //check which exit is lower and move vertically up or down
    if(c < d){
        for(int k = c; k <= d; k++){
            screen[k][h]= ter_path;
        }
    }
    else if (c > d){
        for(int k = c; k >= d; k--){
            screen[k][h]= ter_path;    
        }
    }
    
    //move right until exit
    for(int j = h; j < 79; j++){
        screen[d][j] = ter_path;    
    }
   }

/*
   Manhattan probability function based on the distance from the center (200,200)
   of the world.
*/
double manhattanProb(){
    if(!(currWorldRow == 200 && currWorldCol == 200)){
        double d = abs(currWorldRow-200) + abs(currWorldCol-200);
        if(d > 200){
            return .05;
        }
        double equation = d * -45;
        equation /= 200.00;
        equation += 50;
        equation /= 100.00;
        return equation;
    }
    return 1.0;
}

// Seeds all terrain for a given map
void seeder(terrain_type_t screen[21][80]){
    //queue's size
    const int SIZE = 1580; 

    //initialize queue
    int head, tail;
    int queue[1580];


    initQueue(&head,&tail);
    
    //seed mountains, long grass and clearings
    int mountainsCoord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    int forestCoord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    int waterCoord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    int longGrassCoord1 = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    int longGrassCoord2 = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    int clearingCoord1 = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    int clearingCoord2 = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);


    screen[longGrassCoord1 % 79][longGrassCoord1 / 79] = ter_grass;
    screen[longGrassCoord2 % 79][longGrassCoord2 / 79] = ter_grass;
    screen[mountainsCoord % 79][mountainsCoord / 79] = ter_mountain;
    screen[clearingCoord1 % 79][clearingCoord1 / 79] = ter_clearing;
    screen[clearingCoord2 % 79][clearingCoord2 / 79] = ter_clearing;
    screen[forestCoord % 79][forestCoord / 79] = ter_forest;
    screen[waterCoord % 79][waterCoord / 79] = ter_water;
    
    enqueue(queue, &tail, longGrassCoord1);
    enqueue(queue, &tail, longGrassCoord2);
    enqueue(queue, &tail, forestCoord);
    enqueue(queue, &tail, waterCoord);
    enqueue(queue, &tail, mountainsCoord);
    enqueue(queue, &tail, clearingCoord1);
    enqueue(queue, &tail, clearingCoord2);


    while(!empty(head, tail)){
        int coord = dequeue(queue,&head);
        int x =  coord % 79;
        int y = coord / 79;
        terrain_type_t seed;
        //Check which biome and set seed to it
        if(screen[x][y] == ter_mountain)
            seed = ter_mountain;
        else if(screen[x][y] == ter_grass)
            seed = ter_grass;
        else if(screen[x][y] == ter_clearing)
            seed = ter_clearing;
        else if(screen[x][y] == ter_forest)
            seed = ter_forest;
        else if(screen[x][y] == ter_water)
            seed = ter_water;
        if(screen[x+1][y] == ter_default){
            screen[x+1][y] = seed;
            enqueue(queue, &tail, (y * 79 + (x+1)));
        }
        if(screen[x-1][y] == ter_default){
            screen[x-1][y] = seed;
            enqueue(queue, &tail, (y * 79 + (x-1)));
        }
        if(screen[x][y+1] == ter_default){
            screen[x][y+1] = seed;
            enqueue(queue, &tail, ((y+1) * 79 + x));
        }
        if(screen[x][y-1] == ter_default){
            screen[x][y-1] = seed;
            enqueue(queue, &tail, ((y-1) * 79 + x));
        }
    }

}


// Set exits for a newly created map and lines them up with existing maps
void setExits(){
    if(world[currWorldRow][currWorldCol+1] != NULL){
        currExitS = world[currWorldRow][currWorldCol+1]->exitS;
    }
    else{
        currExitS= rand() % 74 + 3;
    }
    if(world[currWorldRow][currWorldCol-1] != NULL){
        currExitN = world[currWorldRow][currWorldCol-1]->exitN;
    }
    else{
        currExitN= rand() % 74 + 3;
    }
    if(world[currWorldRow+1][currWorldCol] != NULL){
        currExitE = world[currWorldRow+1][currWorldCol]->exitW;
    }
    else{
        currExitE= rand() % 15 + 3;
    }
    if(world[currWorldRow-1][currWorldCol] != NULL){
        currExitW = world[currWorldRow-1][currWorldCol]->exitE;
    }
    else{
        currExitW= rand() % 15 + 3;
    }
}

int getPokemonStat(int level, int iv, int base, int hp){ //if hp == 0 then it is Otherstat, if 1 then it is HP
    if(!hp){ //Otherstat
        return (floor((((base + iv)*2) * level) / 100) + 5);
    }
    return (floor((((base + iv)*2) * level) / 100) + level + 10);
}

/*
  Spawns a new pokemon. Given an id, it will create the pokemon with the 
  corresponding PokeDex ID. If id == 0, it will create a random pokemon.
*/
pokemon_t spawnPokemon(int id){ //if 0, random pokemon is generated, else it is set to the given pokemon_id
    pokemon_t *newPokemon = new pokemon_t;
    int minimumLevel;
    int maximumLevel;
    int lvl;
    int pokemonId;

    if(id)
        pokemonId = id;
    else
        pokemonId = rand() % 1092 + 1;

    //set species_id
    int speciesId = pokemonId;
    newPokemon->species_id = pokemonId;

    //Determine level based on manhattan distance from origin
    int distance = abs(200 - currWorldRow) + abs(200 - currWorldCol);
    if(id > 0){
        lvl = 4;
    }
    else if(distance < 2){
        lvl = 1;
    }
    
    else{
        if(distance <= 200){
         minimumLevel = 1;
         maximumLevel = distance/2;    
        }
        else if(distance > 200){
            minimumLevel = (distance-200)/2;
            maximumLevel = 100; 
        }
        lvl = (rand() % (maximumLevel - minimumLevel + 1)) + minimumLevel;  
    }
    newPokemon->level = lvl;

    //Find moves for Pokemon
    int i;  
    std::vector<int> possibleMoves; //stored possible moves, moves_id

    if(pokemonId > 898){
        pokemonId += 9102;
    }
    //set pokemon_id
    newPokemon->id = pokemonId;

    for(i = 0; i < pokemonMovesVector.size(); i++){
        if(pokemonMovesVector.at(i).pokemon_id == pokemonId && pokemonMovesVector.at(i).pokemon_move_method_id == 1 && pokemonMovesVector.at(i).level <= lvl){
            if(std::find(possibleMoves.begin(), possibleMoves.end(), pokemonMovesVector.at(i).move_id) == possibleMoves.end()) // algorithm to find if move_id exists in vector
                possibleMoves.emplace_back(pokemonMovesVector.at(i).move_id);
        }
    }
    if(possibleMoves.size() < 2)
        newPokemon->pokemon_moves[0] = possibleMoves.at(0);
    else{
        int move1 = rand() % possibleMoves.size();
        int move2 = rand() % possibleMoves.size();
        while(possibleMoves.at(move1) == possibleMoves.at(move2)){
            move2 = rand() % possibleMoves.size();
        }
        newPokemon->pokemon_moves[0] = possibleMoves.at(move1);
        newPokemon->pokemon_moves[1] = possibleMoves.at(move2);
    }
    //Create IV stats for Pokemon
    int attackIV = rand() % 16;
    int defenseIV = rand() % 16;
    int hpIV = rand() % 16;
    int speedIV = rand() % 16;
    int specialAttackIV = rand() % 16;
    int specialDefenseIV = rand() % 16;

    //get base for each stat
    int hpBase = pokemonStatsVector.at((speciesId * 6) - 6).base_stat;
    int attackBase = pokemonStatsVector.at((speciesId * 6) - 5).base_stat;
    int defenseBase = pokemonStatsVector.at((speciesId * 6) - 4).base_stat;
    int specialAttackBase = pokemonStatsVector.at((speciesId * 6) - 3).base_stat;
    int specialDefenseBase = pokemonStatsVector.at((speciesId * 6) - 2).base_stat;
    int speedBase = pokemonStatsVector.at((speciesId * 6) - 1).base_stat;
    
    newPokemon->baseHP = getPokemonStat(lvl, hpIV, hpBase, 1);
    newPokemon->hp = getPokemonStat(lvl, hpIV, hpBase, 1);
    newPokemon->attack = getPokemonStat(lvl, attackIV, attackBase, 0);
    newPokemon->defense = getPokemonStat(lvl, defenseIV, defenseBase, 0);
    newPokemon->special_attack = getPokemonStat(lvl, specialAttackIV, specialAttackBase, 0);
    newPokemon->special_defense = getPokemonStat(lvl, specialDefenseIV, specialDefenseBase, 0);
    newPokemon->speed = getPokemonStat(lvl, speedIV, speedBase, 0);
    newPokemon->shiny = 0;

    return *newPokemon;
}

/*
   Creates player for each map traversed. If player already exists, copy that player to the
   next map according to directionFrom and the current world row and column
*/
void createPlayer(terrain_type_t screen[21][80], int directionFrom){
    character_t* pc = (character_t *)malloc(sizeof(character_t));
    if(world[200][200]->NPC[0].trainer_pokemon[0].level == 0){
        int starterId = displayStarterPokemon();
        pokemon_t starterPokemon;
        
        starterPokemon = spawnPokemon(starterId);
        pc->trainer_pokemon[0] = starterPokemon;
        for(int i = 1; i < 6; i++){
            pokemon_t emptyPokemon;
            pc->trainer_pokemon[i] = emptyPokemon;
        }
        pc->items[0] = 3; //player gets 3 potions
        pc->items[1] = 2; //player gets 2 revives
        pc->items[2] = 5; //player gets 5 pokeballs
    }
    else{
        std::copy(std::begin(world[200][200]->NPC[0].trainer_pokemon), std::end(world[200][200]->NPC[0].trainer_pokemon), std::begin(pc->trainer_pokemon));
        std::copy(std::begin(world[200][200]->NPC[0].items), std::end(world[200][200]->NPC[0].items), std::begin(pc->items));
    }
        
    pc->symbol = '@';
    if(directionFrom == 0){
        player = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(screen[player % 79][player / 79] != ter_path){
            player = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        pc->x = player / 79;
        pc->y = player % 79;
    }
    else if(directionFrom == 1){
        pc->x = world[currWorldRow][currWorldCol]->exitN;
        pc->y = 19;
    }
    else if(directionFrom == 2){
        pc->x = world[currWorldRow][currWorldCol]->exitS;
        pc->y = 1;
    }
    else if(directionFrom == 3){
        pc->x = 1;
        pc->y = world[currWorldRow][currWorldCol]->exitW;
    }
    else if(directionFrom == 4){
        pc->x = 78;
        pc->y = world[currWorldRow][currWorldCol]->exitE;
    } 
    pc->sequence_num = 0;
    pc->next_turn = world[currWorldRow][currWorldCol]->NPC[0].next_turn;
    world[currWorldRow][currWorldCol]->NPC[0] = *pc;
}


// Creates map by generating terrain, NPCs, PokeMarts and PokeCenters
void createMap(){

    world[currWorldRow][currWorldCol] = (map *)malloc(sizeof(map));
    world[currWorldRow][currWorldCol]->heapInit = 0;
    world[currWorldRow][currWorldCol]->exitS = currExitN;
    world[currWorldRow][currWorldCol]->exitN = currExitS;
    world[currWorldRow][currWorldCol]->exitE = currExitE;
    world[currWorldRow][currWorldCol]->exitW = currExitW;

    //create borders
    for(int l = 0; l < 80; l++){
        world[currWorldRow][currWorldCol]->screen[0][l] = ter_boulder;
        world[currWorldRow][currWorldCol]->screen[20][l] = ter_boulder;

        if(l == currExitS && currWorldRow != 400)
             world[currWorldRow][currWorldCol]->screen[20][l] = ter_gate;
        if(l == currExitN && currWorldRow != 0)
             world[currWorldRow][currWorldCol]->screen[0][l] = ter_gate;
    }
    for(int k = 0; k < 21; k++){
        world[currWorldRow][currWorldCol]->screen[k][0] = ter_boulder;
        world[currWorldRow][currWorldCol]->screen[k][79] = ter_boulder;
        if(k == currExitE && currWorldCol != 400)
             world[currWorldRow][currWorldCol]->screen[k][79] = ter_gate;
        if(k == currExitW && currWorldCol != 0)
             world[currWorldRow][currWorldCol]->screen[k][0] = ter_gate;
    }
    //create empty spaces using "-"
    for(int n = 1; n < 20; n++){
        for(int m = 1; m < 79; m++){
        world[currWorldRow][currWorldCol]->screen[n][m] = ter_default;
        }
    }

    seeder(world[currWorldRow][currWorldCol]->screen);
    roadPath(currExitN, currExitS, currExitW, currExitE, world[currWorldRow][currWorldCol]->screen);

    double randomNum = ((double) rand()) / RAND_MAX;
    
    if(randomNum < manhattanProb()){
        martCenterHelper(world[currWorldRow][currWorldCol]->screen);
    }
    dijkstras_path(world[currWorldRow][currWorldCol]->screen, 0);
    dijkstras_path(world[currWorldRow][currWorldCol]->screen, 1);

}


// Moves trainers without specified movement rules in a given direction
void moveFromDirection(character_t *t, int character){
    int direction = t->dir;
    int minX = t->x;
    int minY = t->y;

    if(direction == 0){
            t->y = t->y-1;
            t->x = t->x;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX], character);
        } 
    else if (direction == 2){
            t->y = t->y;
            t->x = t->x-1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX-1], character);
        }
    else if (direction == 4){
            t->y = t->y-1;
            t->x = t->x-1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX-1], character);
        }
    else if (direction == 1 ){
            t->y = t->y+1;
            t->x = t->x;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX], character);
        }
    else if (direction == 3){
            t->y = t->y;
            t->x = t->x+1;
             t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX+1], character);
        }
    else if (direction == 5){
            t->y = t->y+1;
            t->x = t->x+1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX+1], character);
        }
    else if (direction == 6){
            t->y = t->y-1;
            t->x = t->x+1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX+1], character);
        }
    else if (direction == 7){
            t->y = t->y+1;
            t->x = t->x-1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX-1], character);
        }
    else if (t->symbol != '@')
        mvprintw(22,0,"%s","cant move there");

}

void moveFollowers(character_t *t, int character){
    int minX = t->x;
    int minY = t->y;
    int min = INF;
    int a=0, b=0, c=0, d=0, e=0, f=0, g=0, h=0;
    for(int i = 0; i < numTrainers; i++){
        if(world[currWorldRow][currWorldCol]->NPC[i].sequence_num != t->sequence_num){
            character_t neighbor = world[currWorldRow][currWorldCol]->NPC[i];
            int neighborCoord = neighbor.x * 79 + neighbor.y;
            if(minX == neighbor.x && minY - 1 == neighbor.y){
                if(neighbor.symbol == '@' && t->dir == 0 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                    world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                    displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                    
                }
                a = 1;
            }   
            else if(minX-1 == neighbor.x && minY == neighbor.y){
                if(neighbor.symbol == '@' && t->dir == 2 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                    world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                    displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                }
                b = 1;
            }
            else if(minX-1 == neighbor.x && minY - 1 == neighbor.y){
                if(neighbor.symbol == '@'  && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                    world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                    displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                }
                c = 1;
                }
            else if(minX == neighbor.x && minY + 1 == neighbor.y){
                if(neighbor.symbol == '@' && t->dir == 1 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                    world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                    displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                }
                d = 1;
            }
            else if(minX+1 == neighbor.x &&  minY == neighbor.y){
                if(neighbor.symbol == '@' && t->dir == 3 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                    world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                    displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                } 
                e = 1;                        
            }
            else if(minX+1 == neighbor.x && minY + 1 == neighbor.y){
                if(neighbor.symbol == '@' && t->dir == 5 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                    world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                    displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                } 
                f = 1;                
            }
            else if(minX+1 == neighbor.x && minY - 1 == neighbor.y){
                if(neighbor.symbol == '@' && t->dir == 6 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                    world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                    displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                } 
                g = 1;                
            }
            else if(minX-1 == neighbor.x && minY + 1 == neighbor.y){
                if(neighbor.symbol == '@' && t->dir == 7 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                    world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                    displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                } 
                h = 1;                
            }
        }
    }
    // printf("%d %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g, h);
    if(min > paths[character]->screen[minY-1][minX] && a == 0){
        min = paths[character]->screen[minY-1][minX];
        t->dir = 0;
        t->y = minY-1;
        t->x = minX;
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX], character);
    }
    if(min > paths[character]->screen[minY][minX-1] && b == 0){
        t->y = minY;
        t->dir = 2;
        t->x = minX-1;
        min = paths[character]->screen[minY][minX-1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX-1], character);
    }
    if (min > paths[character]->screen[minY-1][minX-1] && c == 0){
        t->y = minY-1;
        t->dir = 4;
        t->x = minX-1;
        min = paths[character]->screen[minY-1][minX-1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX-1], character);
    }
    if (min > paths[character]->screen[minY+1][minX] && d == 0){
        t->y = minY+1;
        t->dir = 1;
        t->x = minX;
        min = paths[character]->screen[minY+1][minX];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX], character);
    }
    if (min > paths[character]->screen[minY][minX+1] && e == 0){
        t->y = minY;
        t->dir = 3;
        t->x = minX+1;
        min = paths[character]->screen[minY][minX+1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX+1], character);
    }
    if (min > paths[character]->screen[minY+1][minX+1] && f == 0){
        t->y = minY+1;
        t->dir = 5;
        t->x = minX+1;
        min = paths[character]->screen[minY+1][minX+1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX+1], character);
    }
    if (min > paths[character]->screen[minY-1][minX+1] && g == 0){
        t->y = minY-1;
        t->dir = 6;
        t->x = minX+1;
        min = paths[character]->screen[minY-1][minX+1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX+1], character);
    }
    if (min > paths[character]->screen[minY+1][minX-1] && h == 0){
        t->y = minY+1;
        t->x = minX-1;
        t->dir = 7;
        
        min = paths[character]->screen[minY+1][minX-1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX-1], character);
    }
    else
        t->next_turn = t->next_turn + 10;

}

int getWandererDirection(character_t *t){
    int direction = rand() % 8;
    int c = 0;
    while(c == 0){
        direction = rand() % 8;
        if(direction == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if(direction == 2){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 4){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 3){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 5){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                   c = 1;
            }
            else if (direction == 6){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 7){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
    }
    return direction;
}

int getExplorerDirection(character_t *t){
    int direction = rand() % 8;
    int c = 0;
    while(c == 0){
        direction = rand() % 8;
        if(direction == 0){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != ter_gate)
                c = 1;
        }
        else if(direction == 2){
            if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != ter_gate)
                c = 1;
        }
        else if (direction == 4){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != ter_gate)
                c = 1;
        }
        else if (direction == 1){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != ter_gate)
                c = 1;
        }
        else if (direction == 3){
            if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != ter_gate)
                c = 1;
        }
        else if (direction == 5){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != ter_gate)
                c = 1;
        }
        else if (direction == 6){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != ter_gate)
                c = 1;
        }
        else if (direction == 7){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != ter_gate)
                c = 1;
        }
    }
    return direction;
}

int movePC(character_t *t, char ch){ //if this returns a 0, then no gate traversal, if 1 then gate traversal north, if 
    int minX = t->x;
    int minY = t->y;
    int a=0, b=0, c=0, d=0, e=0, f=0, g=0, h=0;
    
    for(int i = 0; i < numTrainers; i++){
        if(world[currWorldRow][currWorldCol]->NPC[i].sequence_num != t->sequence_num){
            character_t neighbor = world[currWorldRow][currWorldCol]->NPC[i];
            int neighborCoord = neighbor.x * 79 + neighbor.y;
            if(minX == neighbor.x && minY - 1 == neighbor.y){
                if(ch == 'k' || ch == '8'){
                    if(neighbor.defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[neighbor.sequence_num].defeated = 1;
                        displayTrainerBattle(&neighbor, world[currWorldRow][currWorldCol]);
                    }
                }
                a = 1;
            }
            if(minX-1 == neighbor.x && minY == neighbor.y){
                if(ch == 'h' || ch == '4'){
                    if(neighbor.defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[neighbor.sequence_num].defeated = 1;
                        displayTrainerBattle(&neighbor, world[currWorldRow][currWorldCol]);
                    }
                }
                b = 1;
            }
            if(minX-1 == neighbor.x && minY - 1 == neighbor.y){
                if(ch == 'y' || ch == '7'){
                    if(neighbor.defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[neighbor.sequence_num].defeated = 1;
                        displayTrainerBattle(&neighbor, world[currWorldRow][currWorldCol]);
                    }
                }
                c = 1;
            }
            if(minX == neighbor.x && minY + 1 == neighbor.y){
                if(ch == 'j' || ch == '2'){
                    if(neighbor.defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[neighbor.sequence_num].defeated = 1;
                        displayTrainerBattle(&neighbor, world[currWorldRow][currWorldCol]);
                    }
                }
                d = 1;
            }
            if(minX+1 == neighbor.x &&  minY == neighbor.y){
                if(ch == 'l' || ch == '6'){
                    if(neighbor.defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[neighbor.sequence_num].defeated = 1;
                        displayTrainerBattle(&neighbor, world[currWorldRow][currWorldCol]);
                    }
                }
                e = 1;                        
            }
            if(minX+1 == neighbor.x && minY + 1 == neighbor.y){
                if(ch == 'n' || ch == '3'){
                    if(neighbor.defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[neighbor.sequence_num].defeated = 1;
                        displayTrainerBattle(&neighbor, world[currWorldRow][currWorldCol]);
                    }
                }
                f = 1;                
            }
            if(minX+1 == neighbor.x && minY - 1 == neighbor.y){
                if(ch == 'u' || ch == '9'){
                    if(neighbor.defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[neighbor.sequence_num].defeated = 1;
                        displayTrainerBattle(&neighbor, world[currWorldRow][currWorldCol]);
                    }
                }
                g = 1;                
            }
            if(minX-1 == neighbor.x && minY + 1 == neighbor.y){
                if(ch == 'b' || ch == '1'){
                    if(neighbor.defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[neighbor.sequence_num].defeated = 1;
                        displayTrainerBattle(&neighbor, world[currWorldRow][currWorldCol]);
                    }
                }
                h = 1;                
            }
        }
    }
    
    if(ch == 'k' || ch == '8'){ // up
        t->dir = 0;
        if(world[currWorldRow][currWorldCol]->screen[minY-1][minX] == ter_gate)
            return 1;
        else if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX], 3) != INF && a == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'j' || ch == '2'){ //down
        t->dir = 1;
        if(world[currWorldRow][currWorldCol]->screen[minY+1][minX] == ter_gate)
            return 2;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX], 3) != INF && d == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'l' || ch == '6'){ // right
        t->dir = 3;
        if(world[currWorldRow][currWorldCol]->screen[minY][minX+1] == ter_gate)
            return 3;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX+1], 3) != INF && e == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'h' || ch == '4'){ //left
        t->dir = 2;
        if(world[currWorldRow][currWorldCol]->screen[minY][minX-1] == ter_gate)
            return 4;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX-1], 3) != INF && b == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'y' || ch == '7'){
        t->dir = 4;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX-1], 3) != INF && c == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'n' || ch == '3'){
        t->dir = 5;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX+1], 3) != INF && f == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'u' || ch == '9'){
        t->dir = 6;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX+1], 3) != INF && g == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'b' || ch == '1'){
        t->dir = 7;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX-1], 3) != INF && h == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 't'){
        displayTrainerMenu(world[currWorldRow][currWorldCol]);
        t->next_turn = t->next_turn;
        
    }
    else if(ch == 'f'){
        return displayFlyMenu(currWorldRow, currWorldCol, world[currWorldRow][currWorldCol]);
        t->next_turn = t->next_turn;
    }
    else if(ch == 'B'){
        return displayItems(&t->trainer_pokemon[0], -1, world[currWorldRow][currWorldCol]);
        t->next_turn = t->next_turn;
    }
    else if(ch == '5' | ch == 0x20 || ch == '.'){
        t->next_turn = t->next_turn + 10;
    }    
    else if(ch == '>' && (world[currWorldRow][currWorldCol]->screen[minY][minX] == ter_mart ||world[currWorldRow][currWorldCol]->screen[minY][minX] == ter_center)){
        if(world[currWorldRow][currWorldCol]->screen[minY][minX] == ter_mart)
            displayMandCInterface(0, world[currWorldRow][currWorldCol]);
        else
            displayMandCInterface(1, world[currWorldRow][currWorldCol]);
    }
    else{
        displayWarning("Incorrect Key! Check manual for inputs.");
        t->next_turn = t->next_turn;
    }
    if(minX != t->x || minY != t->y){
        player = (((t->x % 78) + 1) * 79) + ((t->y % 19) + 1);
        free(paths[0]);
        free(paths[1]);
        dijkstras_path(world[currWorldRow][currWorldCol]->screen, 0);
        dijkstras_path(world[currWorldRow][currWorldCol]->screen, 1);
    }
    if(world[currWorldRow][currWorldCol]->screen[minY][minX] == ter_grass){
        int num = rand() % 10 + 1;
        if(num == 1){
            pokemon_t encounteredPokemon = spawnPokemon(0);
            displayPokemonBattle(world[currWorldRow][currWorldCol], &encounteredPokemon);
        }
    }
    return 0;
}

void moveNPCs(character_t *t){
    //if hiker
    if(t->symbol == 'h' && t->defeated == 0){
        moveFollowers(t, 0);
    }
    //if rival
    else if(t->symbol == 'r' && t->defeated == 0){
        moveFollowers(t, 1);
    }
    else{
        //if hit other npc, reverse 
        int minX = t->x;
        int minY = t->y;
        int a=0, b=0, c=0, d=0, e=0, f=0, g=0, h=0;

        for(int i = 0; i < numTrainers; i++){
            if(world[currWorldRow][currWorldCol]->NPC[i].sequence_num != t->sequence_num){
                character_t neighbor = world[currWorldRow][currWorldCol]->NPC[i];
                int neighborCoord = neighbor.x * 79 + neighbor.y;
                if(minX == neighbor.x && minY - 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 0 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                        displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                    }
                    a = 1;
                }   
                else if(minX-1 == neighbor.x && minY == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 2 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                        displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                    }
                    b = 1;
                }
                else if(minX-1 == neighbor.x && minY - 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 4 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                        displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                    }
                    c = 1;
                    }
                else if(minX == neighbor.x && minY + 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 1 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                        displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                    }
                    d = 1;
                }
                else if(minX+1 == neighbor.x &&  minY == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 3 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                        displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                    } 
                    e = 1;                        
                }
                else if(minX+1 == neighbor.x && minY + 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 5 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                        displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                    } 
                    f = 1;                
                }
                else if(minX+1 == neighbor.x && minY - 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 6 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                        displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                    } 
                    g = 1;                
                }
                else if(minX-1 == neighbor.x && minY + 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 7 && world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated == 0){
                        world[currWorldRow][currWorldCol]->NPC[t->sequence_num].defeated = 1;
                        displayTrainerBattle(t, world[currWorldRow][currWorldCol]);
                    } 
                    h = 1;                
                }
            }
        }

        if(t->symbol == 'w'){ //wanderer
            int direction = t->dir; //if hit IMPASSIBLE terrain random 
            if(direction == 0 && a == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    t->dir = getWandererDirection(t);
            }
            else if(direction == 2 && b == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 4 && c == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 1 && d == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 3 && e == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 5 && f == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 6 && g == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 7 && h == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    t->dir = getWandererDirection(t);
            }
            //now move from rand direction
            moveFromDirection(t, 2);
        }    
        else if(t->symbol == 'p'){ //pacers
            int  direction = t->dir;
            //if hit terrain reverse 
            if(direction == 0 && a == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_gate)
                    t->dir = 1;
            }
            else if(direction == 2 && b == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_gate)
                    t->dir = 3;
            }
            else if (direction == 4 && c == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_gate)
                    t->dir = 5;
            }
            else if (direction == 1 && d == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_gate)
                    t->dir = 0;
            }
            else if (direction == 3 && e == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_gate)
                    t->dir = 2;
            }
            else if (direction == 5 && f == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_gate)
                    t->dir = 4;
            }
            else if (direction == 6 && g == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_boulder|| world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_gate)
                    t->dir = 7;
            }
            else if (direction == 7 && h == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_gate)
                    t->dir = 6;
            }

        moveFromDirection(t, 2);
    }
    else { //explorer or defeated hiker or rival
        int direction = t->dir;//if hit IMPASSIBLE terrain random 
        if(direction == 0 && a == 0){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_gate)
                t->dir = getExplorerDirection(t);
        }
        else if(direction == 2 && b == 0){
            if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_gate)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 4 && c == 0){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_gate)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 1 && d == 0){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_gate)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 3 && e == 0){
        if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_gate)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 5 && f == 0){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_gate)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 6 && g == 0){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_gate)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 7 && h == 0){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_gate)
                t->dir = getExplorerDirection(t);
        }
        //now move from rand direction
        moveFromDirection(t, 2);
    }
    }
}

// Main movement loop for the entire game
void traffic(){
    character_t *c;
    char ch;
    int gate = 0; //1 north, 2 south, 3 east, 4 west

    if(world[currWorldRow][currWorldCol]->heapInit == 0){
        heap_init(&world[currWorldRow][currWorldCol]->mapHeap, turn_cmp, NULL);
        for(int i = 0; i < numTrainers; i++){
            world[currWorldRow][currWorldCol]->NPC[i].hn = heap_insert(&world[currWorldRow][currWorldCol]->mapHeap, &world[currWorldRow][currWorldCol]->NPC[i]);
        }  
        world[currWorldRow][currWorldCol]->heapInit = 1;
    }
    printMap(currWorldRow, currWorldCol, world[currWorldRow][currWorldCol]);
    refresh();
    while (gate == 0) {
        c = (character_t *)heap_remove_min(&world[currWorldRow][currWorldCol]->mapHeap);

        if(c->symbol == '@'){
            if((ch = getch()) != 'q'){
                gate = movePC(c, ch); 
                heap_insert(&world[currWorldRow][currWorldCol]->mapHeap, &world[currWorldRow][currWorldCol]->NPC[c->sequence_num]);
                printMap(currWorldRow, currWorldCol, world[currWorldRow][currWorldCol]);
                refresh();
            }
            else
                break;
        }
        else if(ch != 'q'){
            moveNPCs(c); 
            heap_insert(&world[currWorldRow][currWorldCol]->mapHeap, &world[currWorldRow][currWorldCol]->NPC[c->sequence_num]);
        }
      }

    if(gate == 1 && ch != 'q'){ //move north
        initMap(currWorldRow, currWorldCol-1, gate);
    }
    else if(gate == 2 && ch != 'q'){ //move south
        initMap(currWorldRow, currWorldCol+1, gate);
    }
    else if(gate == 3 && ch != 'q'){ //move east
        initMap(currWorldRow+1, currWorldCol, gate);
    }
    else if(gate == 4 && ch != 'q'){ //move west
        initMap(currWorldRow-1, currWorldCol, gate);
    }
    else if(gate == 5 && ch != 'q'){ //move west
        initMap(world[currWorldRow][currWorldCol]->nextY, 
        world[currWorldRow][currWorldCol]->nextX, 0);
    }
}

void spawnNPCs(int newNPC, int numT){
    int n;
    int newPokePossibility;
    if(newNPC == 0){ //Sentry
        character_t* sentry = (character_t *)malloc(sizeof(character_t));

        sentry->symbol = 's';
        int sentry_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[sentry_coord % 79][sentry_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[sentry_coord % 79][sentry_coord / 79] == ter_water || world[currWorldRow][currWorldCol]->screen[sentry_coord % 79][sentry_coord / 79] == ter_mountain){
            sentry_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        sentry->y = sentry_coord % 79;
        sentry->x = sentry_coord / 79;
        sentry->sequence_num = numTrainers - numT;
        sentry->defeated = 0;

        sentry->next_turn = INF;
        
        for(int i = 1; i < 6; i++){
            pokemon_t emptyPokemon;
            sentry->trainer_pokemon[i] = emptyPokemon;
        }

        pokemon_t sentryPokemon; 
        sentryPokemon = spawnPokemon(0);
        sentry->trainer_pokemon[0] = sentryPokemon;

        newPokePossibility = rand() % 100 + 1;
        n = 1; //amount of pokemon
        while(newPokePossibility < 60){
            if(n == 6)
                newPokePossibility = 80;
            else{
                pokemon_t newPokemon; 
                newPokemon = spawnPokemon(0);
                sentry->trainer_pokemon[n] = newPokemon;
                n++;
            }
        }
       
        world[currWorldRow][currWorldCol]->NPC[numTrainers - numT] = *sentry;
    }
    if(newNPC == 1){ //Wanderer
        character_t* wanderer =(character_t *) malloc(sizeof(character_t));

        wanderer->symbol = 'w';
        int wanderer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[wanderer_coord % 79][wanderer_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[wanderer_coord % 79][wanderer_coord / 79] == ter_forest || world[currWorldRow][currWorldCol]->screen[wanderer_coord % 79][wanderer_coord / 79] == ter_water || world[currWorldRow][currWorldCol]->screen[wanderer_coord % 79][wanderer_coord / 79] == ter_mountain){
            wanderer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        wanderer->y = wanderer_coord % 79;
        wanderer->x = wanderer_coord / 79;
        wanderer->sequence_num = numTrainers - numT;
        wanderer->dir = rand() % 8;
        wanderer->defeated = 0;

        wanderer->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[wanderer->y][wanderer->x], 1);
       
        for(int i = 1; i < 6; i++){
            pokemon_t emptyPokemon;
            wanderer->trainer_pokemon[i] = emptyPokemon;
        }

        pokemon_t wandererPokemon; 
        wandererPokemon = spawnPokemon(0);
        wanderer->trainer_pokemon[0] = wandererPokemon;

        newPokePossibility = rand() % 100 + 1;
        n = 1; //amount of pokemon
        while(newPokePossibility < 60){
            if(n == 6)
                newPokePossibility = 80;
            else{
                pokemon_t newPokemon; 
                newPokemon = spawnPokemon(0);
                wanderer->trainer_pokemon[n] = newPokemon;
                n++;
            }
        }
        
        world[currWorldRow][currWorldCol]->NPC[numTrainers - numT] = *wanderer;
    }
    if(newNPC == 2){ //Pacer
        character_t* pacer = (character_t *)malloc(sizeof(character_t));

        pacer->symbol = 'p';
        int pacer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[pacer_coord % 79][pacer_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[pacer_coord % 79][pacer_coord / 79] == ter_forest || world[currWorldRow][currWorldCol]->screen[pacer_coord % 79][pacer_coord / 79] == ter_water || world[currWorldRow][currWorldCol]->screen[pacer_coord % 79][pacer_coord / 79] == ter_mountain){
            pacer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        pacer->y = pacer_coord % 79;
        pacer->x = pacer_coord / 79;
        pacer->sequence_num = numTrainers - numT;
        pacer->dir = rand() % 8;
        pacer->defeated = 0;

        pacer->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[pacer->y][pacer->x], 1);

        for(int i = 1; i < 6; i++){
            pokemon_t emptyPokemon;
            pacer->trainer_pokemon[i] = emptyPokemon;
        }

        pokemon_t pacerPokemon; 
        pacerPokemon = spawnPokemon(0);
        pacer->trainer_pokemon[0] = pacerPokemon;

        newPokePossibility = rand() % 100 + 1;
        n = 1; //amount of pokemon
        while(newPokePossibility < 60){
            if(n == 6)
                newPokePossibility = 80;
            else{
                pokemon_t newPokemon; 
                newPokemon = spawnPokemon(0);
                pacer->trainer_pokemon[n] = newPokemon;
                n++;
            }
        }
       
        world[currWorldRow][currWorldCol]->NPC[numTrainers - numT] = *pacer;
    }
    if(newNPC == 3){ //Explorers
        character_t* explorer = (character_t *)malloc(sizeof(character_t));

        explorer->symbol = 'e';
        int explorer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[explorer_coord % 79][explorer_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[explorer_coord % 79][explorer_coord / 79] == ter_water|| world[currWorldRow][currWorldCol]->screen[explorer_coord % 79][explorer_coord / 79] == ter_forest || world[currWorldRow][currWorldCol]->screen[explorer_coord % 79][explorer_coord / 79] == ter_mountain){
            explorer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        explorer->y = explorer_coord % 79;
        explorer->x = explorer_coord / 79;
        explorer->sequence_num = numTrainers - numT;
        explorer->dir = rand() % 8;
        explorer->defeated = 0;
        
        explorer->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[explorer->y][explorer->x], 1);

        for(int i = 1; i < 6; i++){
            pokemon_t emptyPokemon;
            explorer->trainer_pokemon[i] = emptyPokemon;
        }

        pokemon_t explorerPokemon; 
        explorerPokemon = spawnPokemon(0);
        explorer->trainer_pokemon[0] = explorerPokemon;

        newPokePossibility = rand() % 100 + 1;
        n = 1; //amount of pokemon
        while(newPokePossibility < 60){
            if(n == 6)
                newPokePossibility = 80;
            else{
                pokemon_t newPokemon; 
                newPokemon = spawnPokemon(0);
                explorer->trainer_pokemon[n] = newPokemon;
                n++;
            }
        }
        
        world[currWorldRow][currWorldCol]->NPC[numTrainers - numT] = *explorer;
    }
}

void initNPCs(int numT){

    if(numT > 2){
        character_t* rival = (character_t *)malloc(sizeof(character_t));
        rival->symbol = 'r';
        int rival_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[rival_coord % 79][rival_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[rival_coord % 79][rival_coord / 79] == ter_forest || world[currWorldRow][currWorldCol]->screen[rival_coord % 79][rival_coord / 79] == ter_water || world[currWorldRow][currWorldCol]->screen[rival_coord % 79][rival_coord / 79] == ter_mountain){
            rival_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        rival->y = rival_coord % 79;
        rival->x = rival_coord / 79;
        rival->sequence_num = 2;
        rival->defeated = 0;

        rival->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[rival->y][rival->x], 1);

        for(int i = 1; i < 6; i++){
            pokemon_t emptyPokemon;
            rival->trainer_pokemon[i] = emptyPokemon;
        }

        pokemon_t rivalPokemon; 
        rivalPokemon = spawnPokemon(0);
        rival->trainer_pokemon[0] = rivalPokemon;

        int newPokePossibility = rand() % 100 + 1;
        int n = 1; //amount of pokemon
        while(newPokePossibility < 60){
            if(n == 6)
                newPokePossibility = 80;
            else{
                pokemon_t newPokemon; 
                newPokemon = spawnPokemon(0);
                rival->trainer_pokemon[n] = newPokemon;
                n++;
            }
        }

        character_t* hiker = (character_t *)malloc(sizeof(character_t));

        hiker->symbol = 'h';
        int hiker_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);

        while(world[currWorldRow][currWorldCol]->screen[hiker_coord % 79][hiker_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[hiker_coord % 79][hiker_coord / 79] == ter_water){
            hiker_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        hiker->y = hiker_coord % 79;
        hiker->x = hiker_coord / 79;
        hiker->sequence_num = 1;
        hiker->defeated = 0;

        hiker->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[hiker->y][hiker->x], 0);

        for(int i = 1; i < 6; i++){
            pokemon_t emptyPokemon;
            hiker->trainer_pokemon[i] = emptyPokemon;
        }

        pokemon_t hikerPokemon; 
        hikerPokemon = spawnPokemon(0);
        hiker->trainer_pokemon[0] = hikerPokemon;
        newPokePossibility = rand() % 100 + 1;
        n = 1; //amount of pokemon
        while(newPokePossibility < 60){
            if(n == 6)
                newPokePossibility = 80;
            else{
                pokemon_t newPokemon; 
                newPokemon = spawnPokemon(0);
                hiker->trainer_pokemon[n] = newPokemon;
                n++;
            }
        }
        world[currWorldRow][currWorldCol]->NPC[1] = *hiker;
        world[currWorldRow][currWorldCol]->NPC[2] = *rival;

        numT -= 3; //numT = 6
    }
    while(numT){
        int newNPC = rand() % 4;
        spawnNPCs(newNPC, numT);
        numT--;
    }

}

void initMap(int worldRow, int worldCol, int directionFrom){
    currWorldRow = worldRow;
    currWorldCol = worldCol;
    if(world[currWorldRow][currWorldCol] == NULL){
      setExits();
      createMap();
      initNPCs(numTrainers);  
    }
    createPlayer(world[currWorldRow][currWorldCol]->screen, directionFrom);
    printMap(currWorldRow, currWorldCol, world[currWorldRow][currWorldCol]);
    traffic();
}

// Parses the csv data for creating Pokemon
void parse(std::string filename, std::string path){
    std::string l;
    std::ifstream j;
    int count = 0;
    int numArray = 0;
    char c;

    if(filename.compare(pokedex[0]) == 0){
        j.open(path);

        getline(j, l);
        
        while(getline(j, l)){
            std::stringstream ss;
            ss << l;
            int string_length = l.length();
            moves_t move;
            while(ss.get(c)){
                string_length--;
                if(c == ','){ //is a comma
                    if(char(ss.peek()) == ','){ //(next is empty)
                        count++;
                    }
                }
                else if(isdigit(c) || c == '-'){ //is a number
                    std::string line;
                    line += c;
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += c;
                        string_length--;
                    }
                    switch(count){
                        case 0:
                            move.id = stoi(line);
                        case 2:
                            move.generation_id = stoi(line);
                            break;
                        case 3:
                            move.type_id = stoi(line);
                            break;
                        case 4:
                            move.power = stoi(line);
                            break;
                        case 5:
                            move.pp = stoi(line);
                            break;
                        case 6:
                            move.accuracy = stoi(line);
                            break;
                        case 7:
                            move.priority = stoi(line);
                            break;
                        case 8:
                            move.target_id = stoi(line);
                            break;
                        case 9:
                            move.damage_class_id = stoi(line);
                            break;
                        case 10:
                            move.effect_id = stoi(line);
                            break;
                        case 11:
                            move.effect_chance = stoi(line);
                            break;
                        case 12:
                            move.contest_type_id = stoi(line);
                            break;
                        case 13:
                            move.contest_effect_id = stoi(line);
                            break;
                        case 14:
                            move.super_contest_effect_id = stoi(line);
                            break;
                    }
                    count++;
                }
                    
                else if(isalpha(c)){ //is a string
                    std::string line;
                    line += char(c);
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += char(c);
                        string_length--;
                    }
                    move.identifier = line;
                    count++;
                }
            } 
            
            count = 0;
            movesVector.emplace_back(move);
            numArray++;
        }
    }
    else if(filename.compare(pokedex[1]) == 0){
        j.open(path);

        getline(j, l);
        
        while(getline(j, l)){
            std::stringstream ss;
            ss << l;
            int string_length = l.length();
            experience_t exp;
            while(ss.get(c)){
                string_length--;
                if(c == ','){ //is a comma
                    if(char(ss.peek()) == ','){ //(next is empty)
                        count++;
                    }
                }
                else if(isdigit(c) || c == '-'){ //is a number
                    std::string line;
                    line += c;
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += c;
                        string_length--;
                    }
                    switch(count){
                        case 0:
                            exp.growth_rate_id = stoi(line);
                        case 1:
                            exp.level = stoi(line);
                            break;
                        case 2:
                            exp.experience = stoi(line);
                            break;
                    }
                    count++;
                }
            } 
            count = 0;
            experienceVector.emplace_back(exp);
            numArray++;
        }
    }
    else if(filename.compare(pokedex[2]) == 0){
        j.open(path);

        getline(j, l);
        
        while(getline(j, l)){
            std::stringstream ss;
            ss << l;
            int string_length = l.length();
            pokemonCSV_t pokemon;
            while(ss.get(c)){
                string_length--;
                if(c == ','){ //is a comma
                    if(char(ss.peek()) == ','){ //(next is empty)
                        count++;
                    }
                }
                else if(isdigit(c) || c == '-'){ //is a number
                    std::string line;
                    line += c;
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += c;
                        string_length--;
                    }
                    switch(count){
                        case 0:
                            pokemon.id = stoi(line);
                        case 2:
                            pokemon.species_id = stoi(line);
                            break;
                        case 3:
                            pokemon.height = stoi(line);
                            break;
                        case 4:
                            pokemon.weight = stoi(line);
                            break;
                        case 5:
                            pokemon.base_experience = stoi(line);
                            break;
                        case 6:
                            pokemon.order = stoi(line);
                            break;
                        case 7:
                            pokemon.is_default = stoi(line);
                            break;
                    }
                    count++;
                }
                    
                else if(isalpha(c)){ //is a string
                    std::string line;
                    line += char(c);
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += char(c);
                        string_length--;
                    }
                    pokemon.identifier = line;
                    count++;
                }
            } 
            
            count = 0;
            pokemonCSVVector.emplace_back(pokemon);
            numArray++;
        }
    }
    else if(filename.compare(pokedex[3]) == 0){
        j.open(path);

        getline(j, l);
        
        while(getline(j, l)){
            std::stringstream ss;
            ss << l;
            int string_length = l.length();
            pokemon_moves_t pokemonMoves;
            while(ss.get(c)){
                string_length--;
                if(c == ','){ //is a comma
                    if(char(ss.peek()) == ','){ //(next is empty)
                        count++;
                    }
                }
                else if(isdigit(c) || c == '-'){ //is a number
                    std::string line;
                    line += c;
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += c;
                        string_length--;
                    }
                    switch(count){
                        case 0:
                            pokemonMoves.pokemon_id = stoi(line);
                            break;
                        case 1:
                            pokemonMoves.version_group_id = stoi(line);
                            break;
                        case 2:
                            pokemonMoves.move_id = stoi(line);
                            break;
                        case 3:
                            pokemonMoves.pokemon_move_method_id = stoi(line);
                            break;
                        case 4:
                            pokemonMoves.level = stoi(line);
                            break;
                        case 5:
                            pokemonMoves.order = stoi(line);
                            break;
                    }
                    count++;
                }
            
            }
            count = 0;
            pokemonMovesVector.emplace_back(pokemonMoves);
            numArray++;
        }
    }
    else if(filename.compare(pokedex[4]) == 0){
        j.open(path);

        getline(j, l);
        
        while(getline(j, l)){
            std::stringstream ss;
            ss << l;
            int string_length = l.length();
            pokemon_species_t pokemon_spec;
            while(ss.get(c)){
                string_length--;
                if(c == ','){ //is a comma
                    if(char(ss.peek()) == ','){ //(next is empty)
                        count++;
                    }
                }
                else if(isdigit(c) || c == '-'){ //is a number
                    std::string line;
                    line += c;
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += c;
                        string_length--;
                    }
                    switch(count){
                        case 0:
                            pokemon_spec.id = stoi(line);
                        case 2:
                            pokemon_spec.generation_id = stoi(line);
                            break;
                        case 3:
                            pokemon_spec.evolves_from_species_id = stoi(line);
                            break;
                        case 4:
                            pokemon_spec.evolution_chain_id = stoi(line);
                            break;
                        case 5:
                            pokemon_spec.color_id = stoi(line);
                            break;
                        case 6:
                            pokemon_spec.shape_id = stoi(line);
                            break;
                        case 7:
                            pokemon_spec.habitat_id = stoi(line);
                            break;
                        case 8:
                            pokemon_spec.gender_rate = stoi(line);
                            break;
                        case 9:
                            pokemon_spec.capture_rate = stoi(line);
                            break;
                        case 10:
                            pokemon_spec.base_happiness = stoi(line);
                            break;
                        case 11:
                            pokemon_spec.is_baby = stoi(line);
                            break;
                        case 12:
                            pokemon_spec.hatch_counter = stoi(line);
                            break;
                        case 13:
                            pokemon_spec.has_gender_differences = stoi(line);
                            break;
                        case 14:
                            pokemon_spec.growth_rate_id = stoi(line);
                            break;
                        case 15:
                            pokemon_spec.forms_switchable = stoi(line);
                            break;
                        case 16:
                            pokemon_spec.is_legendary = stoi(line);
                            break;
                        case 17:
                            pokemon_spec.is_mythical = stoi(line);
                            break;
                        case 18:
                            pokemon_spec.order = stoi(line);
                            break;
                        case 19:
                            pokemon_spec.conquest_order = stoi(line);
                            break;
                    }
                    count++;
                }
                    
                else if(isalpha(c)){ //is a string
                    std::string line;
                    line += char(c);
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += char(c);
                        string_length--;
                    }
                    pokemon_spec.identifier = line;
                    count++;
                }
            } 
            
            count = 0;
            pokemonSpecVector.emplace_back(pokemon_spec);
            numArray++;
        }
    }
    else if(filename.compare(pokedex[5]) == 0){
        j.open(path);

        getline(j, l);
        
        while(getline(j, l)){
            std::stringstream ss;
            ss << l;
            int string_length = l.length();
            pokemon_stats_t pokemonStats;
            while(ss.get(c)){
                string_length--;
                if(c == ','){ //is a comma
                    if(char(ss.peek()) == ','){ //(next is empty)
                        count++;
                    }
                }
                else if(isdigit(c) || c == '-'){ //is a number
                    std::string line;
                    line += c;
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += c;
                        string_length--;
                    }
                    switch(count){
                        case 0:
                            pokemonStats.pokemon_id = stoi(line);
                            break;
                        case 1:
                            pokemonStats.stat_id = stoi(line);
                            break;
                        case 2:
                            pokemonStats.base_stat = stoi(line);
                            break;
                        case 3:
                            pokemonStats.effort = stoi(line);
                            break;
                    }
                    count++;
                }
            }
            count = 0;
            pokemonStatsVector.emplace_back(pokemonStats);
            numArray++;
        }
    }
    else if(filename.compare(pokedex[6]) == 0){
        j.open(path);

        getline(j, l);
        
        while(getline(j, l)){
            std::stringstream ss;
            ss << l;
            int string_length = l.length();
            pokemon_types_t pokemonTypes;
            while(ss.get(c)){
                string_length--;
                if(c == ','){ //is a comma
                    if(char(ss.peek()) == ','){ //(next is empty)
                        count++;
                    }
                }
                else if(isdigit(c) || c == '-'){ //is a number
                    std::string line;
                    line += c;
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += c;
                        string_length--;
                    }
                    switch(count){
                        case 0:
                            pokemonTypes.pokemon_id = stoi(line);
                            break;
                        case 1:
                            pokemonTypes.type_id = stoi(line);
                            break;
                        case 2:
                            pokemonTypes.slot = stoi(line);
                            break;
                    }
                    count++;
                }
            }
            count = 0;
            pokemonTypesVector.emplace_back(pokemonTypes);
            numArray++;
        }
    }
    else if(filename.compare(pokedex[7]) == 0){
        j.open(path);

        getline(j, l);
        
        while(getline(j, l)){
            std::stringstream ss;
            ss << l;
            int string_length = l.length();
            stats statLine;
            while(ss.get(c)){
                string_length--;
                if(c == ','){ //is a comma
                    if(char(ss.peek()) == ','){ //(next is empty)
                        count++;
                    }
                }
                else if(isdigit(c) || c == '-'){ //is a number
                    std::string line;
                    line += c;
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += c;
                        string_length--;
                    }
                    switch(count){
                        case 0:
                            statLine.id = stoi(line);
                            break;
                        case 1:
                            statLine.damage_class_id = stoi(line);
                            break;
                        case 3:
                            statLine.is_battle_only = stoi(line);
                            break;
                        case 4:
                            statLine.game_index = stoi(line);
                            break;
                    }
                    count++;
                }
                    
                else if(isalpha(c)){ //is a string
                    std::string line;
                    line += char(c);
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += char(c);
                        string_length--;
                    }
                    statLine.identifier = line;
                    count++;
                }
            } 
            count = 0;
            statsVector.emplace_back(statLine);
            numArray++;
        }
    }  
    else if(filename.compare(pokedex[8]) == 0){
        j.open(path);

        getline(j, l);
        
        while(getline(j, l)){
            std::stringstream ss;
            ss << l;
            int string_length = l.length();
            type_names_t typeNames;
            while(ss.get(c)){
                string_length--;
                if(c == ','){ //is a comma
                    if(char(ss.peek()) == ','){ //(next is empty)
                        count++;
                    }
                }
                else if(isdigit(c)){ //is a number
                    std::string line;
                    line += c;
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += c;
                        string_length--;
                    }
                    switch(count){
                        case 0:
                            typeNames.type_id = stoi(line);
                            break;
                        case 1:
                            typeNames.local_language_id = stoi(line);
                            break;
                    }
                    count++;
                }
                    
                else{ //is a string
                    std::string line;
                    line += char(c);
                    while(char(ss.peek()) != ',' && string_length > 0){
                        ss.get(c);
                        line += char(c);
                        string_length--;
                    }
                    typeNames.name = line;
                    count++;
                }
            } 
            count = 0;
            typeNamesVector.emplace_back(typeNames);
            numArray++;
        }
    }
}

int main(int argc, char *argv[]){

    srand(time(NULL));

    std::string s;

    std::string line;
    std::string path;
    int j;

    for(j = 0; j < 9; j++){
        s = pokedex[j];
        path = "/mnt/m/pokemonCSV/" + s + ".csv";

        std::ifstream i;
        i.open(path);
        if(i.is_open()){
        parse(s,path);
        }
        else{
            path = "/share/cs327/pokedex/pokedex/data/csv/" + s + ".csv";
            i.open(path);
            if(i.is_open()){
                parse(s,path);
            } 
            else{
                std::string env = getenv("HOME");
                path = env + "/.poke327/pokedex/pokedex/data/csv/" + s + ".csv";
                i.open(path);
                if(i.is_open()){
                    parse(s,path);
                }  
                else  std::cout << "Error reading file" << std::endl;
            }
        }
    }
    initscr();
    // raw();
    noecho();
    keypad(stdscr, TRUE);
    initMap(200, 200, 0);
    endwin();

    return 0;
}
