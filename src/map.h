#ifndef MAP_H
#define MAP_H

#define MAP_X 80
#define MAP_Y 21
#define numTrainers 6
#include "terrain.h"
#include "heap.h"
#include "character.h"

//Individual map structure
typedef class Map {
    public:
     Map() : heapInit(0) {}
     terrain_type_t screen[21][80];
     heap mapHeap;
     int heapInit; //if 0 -> not initialized, if 1 -> initialized
     character_t NPC[9999];
     int exitN;
     int exitS;
     int exitW;
     int exitE;
     int x; 
     int y;
     int nextY; //next maps to be taken to
     int nextX;
} map;

#endif