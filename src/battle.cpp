#include "battle.h"

void executeMove(moves_t move, pokemon_t *defendingPokemon, pokemon_t *attackingPokemon){
    WINDOW *maintextwin;
    int maintextwinWidth = 48;
    int maintextwinLen = 6;
    int missProb = rand() % 100;

    std::string attackingPokeName = pokemonCSVVector.at(attackingPokemon->species_id - 1).identifier;
    std::string defendingPokeName = pokemonCSVVector.at(defendingPokemon->species_id - 1).identifier;

    maintextwin = newwin(maintextwinLen, maintextwinWidth, (21 + maintextwinLen) / 2 - 2, (80 - maintextwinWidth ) / 2);
    box(maintextwin,0,0);

    if(missProb >= move.accuracy){
        char buffer[1024] = "";
        strcat(buffer, attackingPokeName.c_str());
        strcat(buffer, " missed their attack!");
    
        mvwprintw(maintextwin, 2, (maintextwinWidth - strlen(buffer)) / 2,"%s",buffer);
        refresh();
        wrefresh(maintextwin);   
        sleep(2);
    }
    else{
        char buffer[1024] = "";
        strcat(buffer, attackingPokeName.c_str());
        strcat(buffer, " used ");
        strcat(buffer, move.identifier.c_str());
        strcat(buffer, "!");

        int level = attackingPokemon->level;
        int power = move.power;
        int attack = attackingPokemon->attack;
        int critical = 1;
        int defense = defendingPokemon->defense;
        int random = rand() % (100 - 85 + 1) + 85;
        int type = 1;
        int stab = 1;

        int attackingPokemonTypes[2] = {0, 0};
        int j = 0;

        //See if type of move is a type of the pokemon
        for(int i = 0; i < pokemonTypesVector.size(); i++){
            if(pokemonTypesVector.at(i).pokemon_id == attackingPokemon->id){
                attackingPokemonTypes[j] = pokemonTypesVector.at(i).type_id;
                j++;
            }
            if(j == 2){
                i = pokemonTypesVector.size();
            }
        }
        if(move.type_id == attackingPokemonTypes[0] || move.type_id == attackingPokemonTypes[1]){
            stab = 1.5;
        }

        //Calculate critical hit
        int critRand = rand() % 256;
        if(critRand < attackingPokemon->speed/2){
            critical = 1.5;
            mvwprintw(maintextwin, 3, (maintextwinWidth - strlen("and it's a Critical Hit!")) / 2,"%s","and it's a Critical Hit!");
        }

        int equation = ((((((2*level)/5) + 2) * power * attack/defense) / 50) + 2) * critical * stab * type;
        double totalDamage = (double(random)/100) * equation;
        if(totalDamage == 0){
            totalDamage = 2;
        }
        if(defendingPokemon->hp - totalDamage < 0)
            defendingPokemon->hp = 0;
        else
            defendingPokemon->hp -= totalDamage;

        mvwprintw(maintextwin, 2, (maintextwinWidth - strlen(buffer)) / 2,"%s",buffer);
        refresh();
        wrefresh(maintextwin);   
        sleep(2);
    }

}

void executeMoveCycle(int moveId, pokemon_t *enemyPokemon, pokemon_t *playerPokemon){

    int enemyRandMove = rand() % 3;
    while(enemyPokemon->pokemon_moves[enemyRandMove] == -1){
        enemyRandMove = rand() % 3;
    }

    
    moves_t enemyMove = movesVector.at(enemyPokemon->pokemon_moves[enemyRandMove] - 1);
    
    if(moveId == 0){
        executeMove(enemyMove, playerPokemon, enemyPokemon);
    }
    else{
        moves_t playerMove =  movesVector.at(moveId - 1);
        if(playerMove.priority == enemyMove.priority){
            if(enemyPokemon->speed == playerPokemon->speed){
                int randTurn = rand() % 2;
                if(randTurn == 1){
                    executeMove(enemyMove, playerPokemon, enemyPokemon);
                    if(playerPokemon->hp > 0)
                        executeMove(playerMove, enemyPokemon, playerPokemon);
                    }
                else{
                    executeMove(playerMove, enemyPokemon, playerPokemon);
                    if(enemyPokemon->hp > 0)
                        executeMove(enemyMove, playerPokemon, enemyPokemon);
                }
            }
            else if(enemyPokemon->speed > playerPokemon->speed){
                executeMove(enemyMove, playerPokemon, enemyPokemon);
                if(playerPokemon->hp > 0)
                    executeMove(playerMove, enemyPokemon, playerPokemon);
            }
            else{
                executeMove(playerMove, enemyPokemon, playerPokemon);
                if(enemyPokemon->hp > 0)
                    executeMove(enemyMove, playerPokemon, enemyPokemon);
            }
        }
        else if(playerMove.priority > enemyMove.priority){
            executeMove(playerMove, enemyPokemon, playerPokemon);
            if(enemyPokemon->hp > 0)
                executeMove(enemyMove, playerPokemon, enemyPokemon);
        }
        else{
            executeMove(enemyMove, playerPokemon, enemyPokemon);
            if(playerPokemon->hp > 0)
                executeMove(playerMove, enemyPokemon, playerPokemon);
        }
    }
    
}

int healPokemon(int item, pokemon_t *hurtPokemon, map *currentScreen){
    if(hurtPokemon->hp == hurtPokemon->maxHp)
        return 0; //Already full health
    if(item == 1){ //Potion
        if(hurtPokemon->hp + 20 > hurtPokemon->maxHp){
            hurtPokemon->hp = hurtPokemon->maxHp;  
        }
        else
            hurtPokemon->hp = hurtPokemon->hp + 20;
        currentScreen->NPC[0].items[0]-=1;
        return 1;
    }
    else{ //Revive
        if(hurtPokemon->hp == 0){
            hurtPokemon->hp = hurtPokemon->maxHp;
            currentScreen->NPC[0].items[1]-=1;
            return 2;
        }
        return 0;
    }
}

int getExperience(pokemon_t *playerPokemon, pokemon_t *enemyPokemon, int trainer){
    int a, b, l, s;
    a = 1;
    s = 2;
    b = playerPokemon->baseExp;
    l = enemyPokemon->level;
    if(trainer){ //trainer == 1
        a = 1.5;
    }
    return (a * b * l) / (7 * s);
}

int addExperience(pokemon_t *playerPokemon, int experience){ //adds experience to pokemon and tests if it needs to level up
    playerPokemon->expCurrent += experience;
    int lvl = playerPokemon->level;
    if(playerPokemon->expCurrent >= playerPokemon->expNeeded){
        playerPokemon->level++; //New level
        playerPokemon->expCurrent = playerPokemon->expCurrent - playerPokemon->expNeeded;
        playerPokemon->expNeeded = experienceVector.at(((playerPokemon->growthRate - 1)* 100 + playerPokemon->level) - 1).experience;

        playerPokemon->hp = getPokemonStat(lvl, playerPokemon->iv_hp, playerPokemon->base_hp, 1);
        playerPokemon->attack = getPokemonStat(lvl, playerPokemon->iv_attack, playerPokemon->base_attack, 0);
        playerPokemon->defense = getPokemonStat(lvl, playerPokemon->iv_defense, playerPokemon->base_defense, 0);
        playerPokemon->special_attack = getPokemonStat(lvl, playerPokemon->iv_special_attack, playerPokemon->base_special_attack, 0);
        playerPokemon->special_defense = getPokemonStat(lvl, playerPokemon->iv_special_defense, playerPokemon->base_special_defense, 0);
        playerPokemon->speed = getPokemonStat(lvl, playerPokemon->iv_speed, playerPokemon->base_speed, 0);
        
        std::vector possibleMoves = getMoves(playerPokemon, 0);
        
        if(possibleMoves.size() > 0){
             for(int i = 0; i < 4; i++){
                if(playerPokemon->pokemon_moves[i] == -1){
                    playerPokemon->pokemon_moves[i] = possibleMoves.at(0);
                    std::string text = pokemonCSVVector.at(playerPokemon->species_id - 1).identifier + " learned " +  movesVector.at(possibleMoves.at(0) - 1).identifier;
                    writeToMainTextWin(text, 2);
                    return 1;
                }
             }
            std::string text = pokemonCSVVector.at(playerPokemon->species_id - 1).identifier + " wants to learn " +  movesVector.at(possibleMoves.at(0) - 1).identifier;
            writeToMainTextWin(text, 2);
            displayMoves(playerPokemon, playerPokemon, possibleMoves.at(0));
        }
        return 1; //level up
    }
    return 0; //no level up
}
int getPokemonStat(int level, int iv, int base, int hp){ //if hp == 0 then it is Otherstat, if 1 then it is HP
    if(!hp){ //Otherstat
        return (floor((((base + iv)*2) * level) / 100) + 5);
    }
    return (floor((((base + iv)*2) * level) / 100) + level + 10);
}

std::vector<int> getMoves(pokemon_t *playerPokemon, int newPokemon){
    int i; 
    std::vector<int> possibleMoves; //stored possible moves, moves_id
    int pokemonId = playerPokemon->id;
    int lvl = playerPokemon->level;
    for(i = 0; i < pokemonMovesVector.size(); i++){
        if(newPokemon){
            if(pokemonMovesVector.at(i).pokemon_id == pokemonId && pokemonMovesVector.at(i).pokemon_move_method_id == 1 && pokemonMovesVector.at(i).level <= lvl){
                if(std::find(possibleMoves.begin(), possibleMoves.end(), pokemonMovesVector.at(i).move_id) == possibleMoves.end()) // algorithm to find if move_id exists in vector
                    possibleMoves.emplace_back(pokemonMovesVector.at(i).move_id);
            }
        }
        else{
            if(pokemonMovesVector.at(i).pokemon_id == pokemonId && pokemonMovesVector.at(i).pokemon_move_method_id == 1 && pokemonMovesVector.at(i).level == lvl){
                if(std::find(possibleMoves.begin(), possibleMoves.end(), pokemonMovesVector.at(i).move_id) == possibleMoves.end()) // algorithm to find if move_id exists in vector
                    possibleMoves.emplace_back(pokemonMovesVector.at(i).move_id);
            }
        }
    }
    return possibleMoves;
}

void getMoney(character_t *player, pokemon_t *enemyPokemon){
    int basePayout = enemyPokemon->level * ((rand() % 8 + 4) * 10);
    player->pokedollars+= basePayout;
    std::string text = "You got $" + std::to_string(basePayout) + " for winning!";
    writeToMainTextWin(text, 2);
}