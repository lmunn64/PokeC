#include "display.h"

int aButton;

void writeToMainTextWin(std::string text, int where){
    WINDOW *maintextwin;
    int maintextwinWidth = 48;
    int maintextwinLen = 6;

    maintextwin = newwin(maintextwinLen, maintextwinWidth, (21 + maintextwinLen) / 2 - 2, (80 - maintextwinWidth ) / 2);
    box(maintextwin,0,0);

    mvwprintw(maintextwin, where, (maintextwinWidth - strlen(text.c_str())) / 2,"%s",text.c_str());
    touchwin(maintextwin);
    wrefresh(maintextwin);
    sleep(2);
    wclear(maintextwin);
    box(maintextwin, 0, 0);
    wrefresh(maintextwin); 
}

int displayStarterPokemon(){
    ITEM **my_items;
	int c, pokemonId, loop;				
	MENU *my_menu;
    WINDOW *my_menu_win;
    int n_choices, i;
	/* Create items */
    n_choices = 3;
    my_items = (ITEM **)calloc(10, sizeof(ITEM *));
    
    my_items[0] = new_item("Treecko", ": Grass");
    my_items[1] = new_item("Torchic",": Fire");
    my_items[2] = new_item("Mudkip", ": Water");


	/* Create menu */
	my_menu = new_menu((ITEM **)my_items);
    int winWidth = 72;
    int winLen = 10;
	/* Create the window to be associated with the menu */
    my_menu_win = newwin(winLen, winWidth, (MAP_Y - winLen) / 2, (MAP_X - winWidth) / 2);
    keypad(my_menu_win, TRUE);
    // menu_opts_off(my_menu, O_SHOWDESC);
	/* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 70, 4, 1));
	set_menu_format(my_menu, 6, 3);
			
	/* Set menu mark to the string " * " */
        set_menu_mark(my_menu, " * ");

	/* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0);
	print_in_middle(my_menu_win, 1, 0, winWidth, "Choose a Pokemon!", COLOR_PAIR(1));
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, 70);
	mvwaddch(my_menu_win, 2, 71, ACS_RTEE);
        
	/* Post the menu */
	post_menu(my_menu);
	wrefresh(my_menu_win);
    loop = 0;
	while(loop == 0){
        c = getch();
        switch(c){
            case 'a':
                menu_driver(my_menu, REQ_LEFT_ITEM);
                break;
            case 'd':
                menu_driver(my_menu, REQ_RIGHT_ITEM);
                break;
            case 10: /* ENTER */
                playSound(aButton);
                clrtoeol();
                if(item_name(current_item(my_menu)) == "Treecko")
                    pokemonId = 252;
                else if(item_name(current_item(my_menu)) == "Torchic")
                    pokemonId = 255;
                else if (item_name(current_item(my_menu)) == "Mudkip")
                    pokemonId = 258;
                pos_menu_cursor(my_menu);
                loop = 1;
                break;
            }
            wrefresh(my_menu_win);
	}
    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    delwin(my_menu_win);
    for(i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
    return pokemonId;
}

/*
    Function to display a wild Pokemon battle
*/
void displayPokemonBattle(map *currentScreen, pokemon_t *enemyPokemon){
    WINDOW *win;
    WINDOW *maintextwin;
    WINDOW *playerpokewin;
    WINDOW *enemypokewin;

    //Battle menu window
    ITEM **battle_items;
    MENU *battle_menu;
    WINDOW *battle_menu_win;
    int n_choices, i, flee, loop, experience;

    refresh();

    //Initialize window sizes
    int winWidth = 62;
    int winLen = 16;
    int maintextwinWidth = 48;
    int maintextwinLen = 6;
    int ppwinWidth = 22;
    int ppwinLen = 4;
    int battlewinWidth = 22;
    int battlewinLen = 6;

    char *title;
    char c;
    //Initialize battle menu
    battle_items = (ITEM **)calloc(10, sizeof(ITEM *));
    
    battle_items[0] = new_item("Fight" ,"");
    battle_items[1] = new_item("Pkmn" ,"");
    battle_items[2] = new_item("Item" ,"");
    battle_items[3] = new_item("Run" ,"");

    battle_menu = new_menu((ITEM **)battle_items);
    menu_opts_off(battle_menu, O_SHOWDESC);
    battle_menu_win = newwin(battlewinLen, battlewinWidth, (21 + maintextwinLen) / 2 - 2, (80 - battlewinWidth) / 2 + 13);
    keypad(battle_menu_win, TRUE);

    set_menu_win(battle_menu, battle_menu_win);
    set_menu_sub(battle_menu, derwin(battle_menu_win, 2, 18, 2, 1));
	set_menu_format(battle_menu, 2, 2);
    set_menu_mark(battle_menu, " * ");
    box(battle_menu_win, 0, 0);

    std::string playerPokeName = pokemonCSVVector.at(currentScreen->NPC[0].trainer_pokemon[0].species_id - 1).identifier;
    int playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
    int playerPokeLevel = currentScreen->NPC[0].trainer_pokemon[0].level;
    
    pokemon_t encounteredPokemon = *enemyPokemon;

    std::string pokeName = pokemonCSVVector.at(encounteredPokemon.species_id - 1).identifier;
    int pokeLevel = encounteredPokemon.level;
  
    int pokeHP = encounteredPokemon.hp;

    win = newwin(winLen, winWidth, (21 - winLen) / 2, (80 - winWidth) / 2);
    maintextwin = newwin(maintextwinLen, maintextwinWidth, (21 + maintextwinLen) / 2 - 2, (80 - maintextwinWidth ) / 2);
    playerpokewin = newwin(ppwinLen, ppwinWidth, (21 + ppwinLen) / 2 - ppwinLen - 1, (80 - ppwinWidth + 25) / 2);
    enemypokewin = newwin(ppwinLen, ppwinWidth, ((21 + ppwinLen) / 2) - ((ppwinLen*2) + 1), (80 - ppwinWidth - 24) / 2);

    box(maintextwin,0,0);
    box(win,0,0);
    box(enemypokewin,0,0);
    box(playerpokewin,0,0);


    mvwprintw(win, 0, (winWidth - strlen("A Wild Pokemon Appears!")) / 2, "A Wild Pokemon Appears!");
    std::string text = "A wild " + pokeName + " appears!";
    mvwprintw(maintextwin, 2, (maintextwinWidth - strlen(text.c_str())) / 2,"%s",text.c_str());

    mvwprintw(playerpokewin, 1, 2, "%s", playerPokeName.c_str());
    mvwprintw(playerpokewin, 2, 2, "%s %d", "HP:", playerPokeHP);
    mvwprintw(playerpokewin, 2, ppwinWidth / 2 + 1, "%s %d", "Lvl:", playerPokeLevel);
    mvwprintw(enemypokewin, 1, 2, "%s", pokeName.c_str());
    mvwprintw(enemypokewin, 2, 2, "%s %d", "HP:", pokeHP);
    mvwprintw(enemypokewin, 2, ppwinWidth / 2 + 1, "%s %d", "Lvl:", pokeLevel); 
   
    refresh();

    wrefresh(win);
    wrefresh(maintextwin);   
    wrefresh(playerpokewin);  
    wrefresh(enemypokewin);   
    //Play intro sound effect
    togglePlay();
    int music = loadSound("music/wild_pokemon_appears_intro.wav");
    playAndWait(-1, music, 0);
    wclear(maintextwin);
    box(maintextwin,0,0);
    wrefresh(maintextwin);
    // sleep(2);
    post_menu(battle_menu);
    wrefresh(battle_menu_win);
    loop = 0;
    flee = 0;
    togglePlay();
    music = loadMusic("music/wild_pokemon_appears.mp3");
    playMusic(music); 
    experience = 0;
    while(loop == 0){
        c = getch();
        switch(c){
            case 's':
                menu_driver(battle_menu, REQ_DOWN_ITEM);
                break;
            case 'w':
                menu_driver(battle_menu, REQ_UP_ITEM);
                break;
            case 'a':
                menu_driver(battle_menu, REQ_LEFT_ITEM);
                break;
            case 'd':
                menu_driver(battle_menu, REQ_RIGHT_ITEM);
                break;
            case 10:
                playSound(aButton);
                clrtoeol();
                if(item_name(current_item(battle_menu)) == "Fight"){
                    unpost_menu(battle_menu);
                    int fought = displayMoves(&encounteredPokemon, &currentScreen->NPC[0].trainer_pokemon[0], 0);
                    if(fought == 1){
                        int playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
                        int pokeHP = encounteredPokemon.hp;
                        mvwprintw(playerpokewin, 2, 2, "%s", "HP:    ");
                        mvwprintw(enemypokewin, 2, 2, "%s", "HP:    ");
                        mvwprintw(playerpokewin, 2, 2, "%s %3d", "HP:", playerPokeHP);
                        mvwprintw(enemypokewin, 2, 2, "%s %3d", "HP:", pokeHP);
                        refresh();
 
                        wrefresh(playerpokewin);  
                        wrefresh(enemypokewin);   
                    }
                    redrawwin(enemypokewin);
                    redrawwin(maintextwin);
                    redrawwin(playerpokewin);
                    post_menu(battle_menu);
                    redrawwin(battle_menu_win);
                    refresh();
                }
                else if(item_name(current_item(battle_menu)) == "Item"){
                    unpost_menu(battle_menu);
                    int pokeCaught = displayItems(&encounteredPokemon, 1, currentScreen);
                    if(pokeCaught == 1){
                        loop = 1;
                        post_menu(battle_menu);
                    }
                    else{
                        int playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
                        mvwprintw(playerpokewin, 2, 2, "%s", "HP:     ");
                        mvwprintw(playerpokewin, 2, 2, "%s %d", "HP:", playerPokeHP);
                        refresh();
                        wrefresh(playerpokewin);
                        redrawwin(maintextwin);
                        redrawwin(playerpokewin);
                        post_menu(battle_menu);
                        redrawwin(battle_menu_win);
                        refresh();
                    }
                    
                }
                else if(item_name(current_item(battle_menu)) == "Pkmn"){
                    unpost_menu(battle_menu);
                    int swap = displayPokemon(0, 0, currentScreen);
                    if(swap == 1){
                        executeMoveCycle(0, &encounteredPokemon, &currentScreen->NPC[0].trainer_pokemon[0]);
                        playerPokeName = pokemonCSVVector.at(currentScreen->NPC[0].trainer_pokemon[0].species_id - 1).identifier;
                        int playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
                        int playerPokeLevel = currentScreen->NPC[0].trainer_pokemon[0].level;
                        wclear(playerpokewin);
                        box(playerpokewin,0,0);
                        mvwprintw(playerpokewin, 1, 2, "%s", playerPokeName.c_str());
                        mvwprintw(playerpokewin, 2, 2, "%s", "HP:    ");
                        mvwprintw(playerpokewin, 2, 2, "%s %d", "HP:", playerPokeHP);
                        mvwprintw(playerpokewin, 2, ppwinWidth / 2 + 1, "%s %d", "Lvl:", playerPokeLevel);
                        wrefresh(playerpokewin);
                        
                    }
                    redrawwin(maintextwin);
                    redrawwin(playerpokewin);
                    post_menu(battle_menu);
                    redrawwin(battle_menu_win);
                    refresh();
                }
                else if(item_name(current_item(battle_menu)) == "Run"){
                    if(!flee)
                        flee = rand() % 10 + 1;
                    if(flee == 5){
                        unpost_menu(battle_menu);
                        delwin(battle_menu_win);

                        std::string text = "You can't run from this battle!";
                        writeToMainTextWin(text, 2);

                        post_menu(battle_menu);
                        wrefresh(battle_menu_win);
                        post_menu(battle_menu);
                        redrawwin(battle_menu_win);
                        refresh();
                    }
                    else{
                        loop = 1;
                        int sound = loadSound("music/SFX_RUN.wav");
                        playSound(sound);
                        sleep(1);
                    }    
                        
                }
                break;
        }
        if(encounteredPokemon.hp == 0){
                int sound = loadSound("music/SFX_FAINT_THUD.wav");
                playSound(sound);
                unpost_menu(battle_menu);
                delwin(battle_menu_win);
                int victory = loadMusic("music/victory.mp3");
                playMusic(victory);
                char buffer[1024] = "";
                experience = getExperience(&currentScreen->NPC[0].trainer_pokemon[0], &encounteredPokemon, 0);
                std::string text = pokeName + "has passed out.";
                writeToMainTextWin(text, 2);
                text = playerPokeName + " gained " + std::to_string(experience) + " experience points.";
                writeToMainTextWin(text, 2);
                if(addExperience(&currentScreen->NPC[0].trainer_pokemon[0], experience)){
                    text = playerPokeName + " grew to level " + std::to_string(currentScreen->NPC[0].trainer_pokemon[0].level);
                    writeToMainTextWin(text, 2);
                }
                loop = 1;
        }
        if(currentScreen->NPC[0].trainer_pokemon[0].hp == 0){
            int allDead = 0;
            for(int i = 0; i < 6; i++){
                if(currentScreen->NPC[0].trainer_pokemon[i].hp == 0)
                    allDead++;
            }
            if(allDead == 6){
                unpost_menu(battle_menu);
                delwin(battle_menu_win);
                std::string text = "You've lost the battle. Game Over.";
                writeToMainTextWin(text, 2);
                endwin();
                exit(0);
            }
            else{
                int sound = loadSound("music/SFX_FAINT_THUD.wav");
                playSound(sound);
                unpost_menu(battle_menu);
                delwin(battle_menu_win);
                std::string text = playerPokeName + " has passed out.";
                writeToMainTextWin(text, 2);
                text = "Choose another Pokemon.";
                writeToMainTextWin(text, 2);
    
                displayPokemon(0, 1, currentScreen);

                playerPokeName = pokemonCSVVector.at(currentScreen->NPC[0].trainer_pokemon[0].species_id - 1).identifier;
                playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
                playerPokeLevel = currentScreen->NPC[0].trainer_pokemon[0].level;
                mvwprintw(maintextwin, 2, 1, "%s", "                                              "); 
                mvwprintw(maintextwin, 3, 1, "%s", "                                              "); 
                wclear(playerpokewin);
                box(playerpokewin,0,0);
                mvwprintw(playerpokewin, 1, 2, "%s", playerPokeName.c_str());
                mvwprintw(playerpokewin, 2, 2, "%s", "HP:     ");
                mvwprintw(playerpokewin, 2, 2, "%s %d", "HP:", playerPokeHP);
                mvwprintw(playerpokewin, 2, ppwinWidth / 2 + 1, "%s %d", "Lvl:", playerPokeLevel);
                wrefresh(playerpokewin);
                wrefresh(maintextwin);
                redrawwin(maintextwin);
                redrawwin(playerpokewin);
                post_menu(battle_menu);
                redrawwin(battle_menu_win);
                refresh();
            }
        }
        wrefresh(maintextwin);    
        wrefresh(playerpokewin);  
        wrefresh(enemypokewin); 
        wrefresh(win);
        wrefresh(battle_menu_win); 
    }
    
    /* Unpost and free all the memory taken up */
    wclear(win);
    wclear(maintextwin);
    wclear(playerpokewin);
    wclear(enemypokewin);

    unpost_menu(battle_menu);
    free_menu(battle_menu);
    delwin(battle_menu_win);

    wrefresh(win);
    wrefresh(maintextwin);
    wrefresh(playerpokewin);
    wrefresh(enemypokewin);

    delwin(win);
    delwin(maintextwin);
    delwin(playerpokewin);
    delwin(enemypokewin);
    printMap(currentScreen->y, currentScreen->x, currentScreen);
    music = loadMusic("music/BG.mp3");
    playMusic(music); 
    refresh();
}

/*
    Function to display a trainer Pokemon battle
*/
void displayTrainerBattle(character_t *trainer, map *currentScreen){
    WINDOW *win;
    WINDOW *maintextwin;
    WINDOW *playerpokewin;
    WINDOW *enemypokewin;

    //Battle menu window
    ITEM **battle_items;
    MENU *battle_menu;
    WINDOW *battle_menu_win;
    int n_choices, i, loop, experience;

    refresh();

    //Initialize window sizes
    int winWidth = 62;
    int winLen = 16;
    int maintextwinWidth = 48;
    int maintextwinLen = 6;
    int ppwinWidth = 22;
    int ppwinLen = 4;
    int battlewinWidth = 22;
    int battlewinLen = 6;

    char *title;
    char c;
    //Initialize battle menu
    battle_items = (ITEM **)calloc(10, sizeof(ITEM *));

    battle_items[0] = new_item("Fight" ,"");
    battle_items[1] = new_item("Pkmn" ,"");
    battle_items[2] = new_item("Item" ,"");
    battle_items[3] = new_item("Run" ,"");

    battle_menu = new_menu((ITEM **)battle_items);
    menu_opts_off(battle_menu, O_SHOWDESC);
    battle_menu_win = newwin(battlewinLen, battlewinWidth, (21 + maintextwinLen) / 2 - 2, (80 - battlewinWidth) / 2 + 13);
    keypad(battle_menu_win, TRUE);

    set_menu_win(battle_menu, battle_menu_win);
    set_menu_sub(battle_menu, derwin(battle_menu_win, 2, 18, 2, 1));
	set_menu_format(battle_menu, 2, 2);
    set_menu_mark(battle_menu, " * ");
    box(battle_menu_win, 0, 0);

    std::string playerPokeName = pokemonCSVVector.at(currentScreen->NPC[0].trainer_pokemon[0].species_id - 1).identifier;
    int playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
    int playerPokeLevel = currentScreen->NPC[0].trainer_pokemon[0].level;
        
    pokemon_t *enemyPokemon = &trainer->trainer_pokemon[0];

    std::string pokeName = pokemonCSVVector.at(enemyPokemon->species_id - 1).identifier;
    int pokeLevel = enemyPokemon->level;
    
    int pokeHP = enemyPokemon->hp;

    win = newwin(winLen, winWidth, (21 - winLen) / 2, (80 - winWidth) / 2);
    maintextwin = newwin(maintextwinLen, maintextwinWidth, (21 + maintextwinLen) / 2 - 2, (80 - maintextwinWidth ) / 2);
    playerpokewin = newwin(ppwinLen, ppwinWidth, (21 + ppwinLen) / 2 - ppwinLen - 1, (80 - ppwinWidth + 25) / 2);
    enemypokewin = newwin(ppwinLen, ppwinWidth, ((21 + ppwinLen) / 2) - ((ppwinLen*2) + 1), (80 - ppwinWidth - 24) / 2);

    box(maintextwin,0,0);
    box(win,0,0);
    box(enemypokewin,0,0);
    box(playerpokewin,0,0);

    char buffer[1024] = "Trainer sends out ";
    char buffer2[1024]= "Level: ";
    char buffer3[1024]= "Moves: ";

    strcat(buffer, pokeName.c_str());
    strcat(buffer, "!");

    mvwprintw(win, 0, (winWidth - strlen("A Trainer wants to Battle!")) / 2, "A Trainer wants to Battle!");

   
    mvwprintw(maintextwin, 2, (maintextwinWidth - strlen(buffer)) / 2,"%s",buffer);

    mvwprintw(playerpokewin, 1, 2, "%s", playerPokeName.c_str());
    mvwprintw(playerpokewin, 2, 2, "%s %d", "HP:", playerPokeHP);
    mvwprintw(playerpokewin, 2, ppwinWidth / 2 + 1, "%s %d", "Lvl:", playerPokeLevel);

    mvwprintw(enemypokewin, 1, 2, "%s", pokeName.c_str());
    mvwprintw(enemypokewin, 2, 2, "%s %d", "HP:", pokeHP);
    mvwprintw(enemypokewin, 2, ppwinWidth / 2 + 1, "%s %d", "Lvl:", pokeLevel);        
    refresh();

    wrefresh(win);
    wrefresh(maintextwin);   
    wrefresh(playerpokewin);  
    wrefresh(enemypokewin);   
    
	//Play intro sound effect
    togglePlay();
    int music = loadSound("music/trainer_battle_intro.wav");
    playAndWait(-1, music, 0);
    // sleep(2);

    mvwprintw(maintextwin, 2, 1, "%s", "                                              "); 
    wrefresh(maintextwin); 

    post_menu(battle_menu);
    wrefresh(battle_menu_win);
    loop = 0;
    togglePlay();
    music = loadMusic("music/trainer_battle.mp3");
    playMusic(music); 
    experience = 0;
    while(c != 27 && loop != 1){
        
        c = getch();
        switch(c){
            case 's':
                menu_driver(battle_menu, REQ_DOWN_ITEM);
                break;
            case 'w':
                menu_driver(battle_menu, REQ_UP_ITEM);
                break;
            case 'a':
                menu_driver(battle_menu, REQ_LEFT_ITEM);
                break;
            case 'd':
                menu_driver(battle_menu, REQ_RIGHT_ITEM);
                break;
            case 10:
                playSound(aButton);
                clrtoeol();
                if(item_name(current_item(battle_menu)) == "Fight"){
                    unpost_menu(battle_menu);
                    int fought = displayMoves(enemyPokemon, &currentScreen->NPC[0].trainer_pokemon[0], 0);
                    if(fought == 1){
                        int playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
                        int pokeHP = enemyPokemon->hp;
                        mvwprintw(playerpokewin, 2, 2, "%s", "HP:     ");
                        mvwprintw(enemypokewin, 2, 2, "%s", "HP:     ");
                        mvwprintw(playerpokewin, 2, 2, "%s %d", "HP:", playerPokeHP);
                        mvwprintw(enemypokewin, 2, 2, "%s %d", "HP:", pokeHP);
                        refresh();
 
                        wrefresh(playerpokewin);  
                        wrefresh(enemypokewin);   
                    }
                    redrawwin(enemypokewin);
                    redrawwin(maintextwin);
                    redrawwin(playerpokewin);
                    post_menu(battle_menu);
                    redrawwin(battle_menu_win);
                    refresh();
                }
                else if(item_name(current_item(battle_menu)) == "Item"){
                    unpost_menu(battle_menu);
                    displayItems(enemyPokemon, 0, currentScreen);
                    int playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
                    mvwprintw(playerpokewin, 2, 2, "%s", "HP:     ");
                    mvwprintw(playerpokewin, 2, 2, "%s %d", "HP:", playerPokeHP);
                    refresh();
                    wrefresh(playerpokewin);
                    redrawwin(maintextwin);
                    redrawwin(playerpokewin);
                    post_menu(battle_menu);
                    redrawwin(battle_menu_win);
                    refresh();
                }
                else if(item_name(current_item(battle_menu)) == "Pkmn"){
                    unpost_menu(battle_menu);
                    int swap = displayPokemon(0, 0, currentScreen);
                    if(swap == 1){
                        executeMoveCycle(0, enemyPokemon, &currentScreen->NPC[0].trainer_pokemon[0]);
                        std::string playerPokeName = pokemonCSVVector.at(currentScreen->NPC[0].trainer_pokemon[0].species_id - 1).identifier;
                        int playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
                        int playerPokeLevel = currentScreen->NPC[0].trainer_pokemon[0].level;
                        wclear(playerpokewin);
                        box(playerpokewin,0,0);
                        mvwprintw(enemypokewin, 1, 2, "%s", playerPokeName.c_str());
                        mvwprintw(playerpokewin, 2, 2, "%s", "HP:    ");
                        mvwprintw(playerpokewin, 2, 2, "%s %d", "HP:", playerPokeHP);
                        mvwprintw(playerpokewin, 2, ppwinWidth / 2 + 1, "%s %d", "Lvl:", playerPokeLevel);
                        wrefresh(playerpokewin);
                        
                    }
                    redrawwin(maintextwin);
                    redrawwin(playerpokewin);
                    post_menu(battle_menu);
                    redrawwin(battle_menu_win);
                    refresh();
                }
                else if(item_name(current_item(battle_menu)) == "Run"){
                    unpost_menu(battle_menu);
                    delwin(battle_menu_win);
                    std::string text ="You can't run from this battle!";
                    writeToMainTextWin(text, 2);
                    
                    post_menu(battle_menu);
                    wrefresh(battle_menu_win);
                    post_menu(battle_menu);
                    redrawwin(battle_menu_win);
                    refresh();
                }
                // pos_menu_cursor(battle_menu);
                break;
        }
        if(enemyPokemon->hp == 0){
            unpost_menu(battle_menu);
            delwin(battle_menu_win);
            std::string text = pokeName + "has passed out.";
            writeToMainTextWin(text, 2);
            int sound = loadSound("music/SFX_FAINT_THUD.wav");
            playSound(sound);
            experience = experience + getExperience(&currentScreen->NPC[0].trainer_pokemon[0], enemyPokemon, 1);
            int allDead = 0;
            for(int i = 0; i < 6; i++){
                if(trainer->trainer_pokemon[i].hp == 0)
                    allDead++;
                else    
                    i = 6;
            }
            if(allDead == 6){
                int victory = loadMusic("music/victory.mp3");
                playMusic(victory);
                std::string text ="You win!";
                writeToMainTextWin(text, 2);
                text = playerPokeName + " gained " + std::to_string(experience) + " experience points";
                writeToMainTextWin(text, 2);
                if(addExperience(&currentScreen->NPC[0].trainer_pokemon[0], experience)){
                    text = playerPokeName + " grew to level " + std::to_string(currentScreen->NPC[0].trainer_pokemon[0].level);
                    writeToMainTextWin(text, 2);
                }
                getMoney(&currentScreen->NPC[0], enemyPokemon);
                loop = 1;
            }
            else{
                enemyPokemon = &trainer->trainer_pokemon[allDead];
                pokeName = pokemonCSVVector.at(enemyPokemon->species_id - 1).identifier;
                int pokeLevel = enemyPokemon->level;
                int pokeHP = enemyPokemon->hp;
                std::string text = "Trainer sends out " + pokeName + "!";
                writeToMainTextWin(text, 2);

                mvwprintw(enemypokewin, 1, 2, "%s", "                  ");
                mvwprintw(enemypokewin, 1, 2, "%s", pokeName.c_str());
                mvwprintw(enemypokewin, 2, 2, "%s %d", "HP:", pokeHP);
                mvwprintw(enemypokewin, 2, ppwinWidth / 2 + 1, "%s %d", "Lvl:", pokeLevel);  

                wrefresh(enemypokewin);
                wrefresh(maintextwin); 
                redrawwin(maintextwin);
                redrawwin(enemypokewin);
                post_menu(battle_menu);
                redrawwin(battle_menu_win); 
                
                refresh();
            }

        }
        if(currentScreen->NPC[0].trainer_pokemon[0].hp == 0){
            int allDead = 0;
            for(int i = 0; i < 6; i++){
                if(currentScreen->NPC[0].trainer_pokemon[i].hp == 0)
                    allDead++;
            }
            if(allDead == 6){
                unpost_menu(battle_menu);
                delwin(battle_menu_win);
                std::string text ="You've lost the battle. Game Over.";
                writeToMainTextWin(text, 2);
                endwin();
                exit(0);
            }
            else{
                unpost_menu(battle_menu);
                delwin(battle_menu_win);
                int sound = loadSound("music/SFX_FAINT_THUD.wav");
                playSound(sound);
                std::string text = playerPokeName + " has passed out.";
                writeToMainTextWin(text, 2);
                text = "Choose another Pokemon.";
                writeToMainTextWin(text, 2);
                displayPokemon(0, 1, currentScreen);

                std::string playerPokeName = pokemonCSVVector.at(currentScreen->NPC[0].trainer_pokemon[0].species_id - 1).identifier;
                int playerPokeHP = currentScreen->NPC[0].trainer_pokemon[0].hp;
                int playerPokeLevel = currentScreen->NPC[0].trainer_pokemon[0].level;
                wclear(playerpokewin);
                box(playerpokewin,0,0);
                        
                mvwprintw(maintextwin, 2, 1, "%s", "                                              "); 
                mvwprintw(maintextwin, 3, 1, "%s", "                                              "); 
                mvwprintw(playerpokewin, 1, 2, "%s", "                  ");
                mvwprintw(playerpokewin, 1, 2, "%s", playerPokeName.c_str());
                mvwprintw(playerpokewin, 2, 2, "%s", "HP:     ");
                mvwprintw(playerpokewin, 2, 2, "%s %d", "HP:", playerPokeHP);
                mvwprintw(playerpokewin, 2, ppwinWidth / 2 + 1, "%s %d", "Lvl:", playerPokeLevel);
               
                wrefresh(playerpokewin);
                wrefresh(maintextwin);
                redrawwin(maintextwin);
                redrawwin(playerpokewin);
                post_menu(battle_menu);
                redrawwin(battle_menu_win);
                refresh();
            }
        }
        wrefresh(maintextwin);    
        wrefresh(playerpokewin);  
        wrefresh(enemypokewin); 
        wrefresh(win);
        wrefresh(battle_menu_win); 
        refresh();
    }
    
    /* Unpost and free all the memory taken up */
    wclear(win);
    wclear(maintextwin);
    wclear(playerpokewin);
    wclear(enemypokewin);

    unpost_menu(battle_menu);
    free_menu(battle_menu);
    delwin(battle_menu_win);

    wrefresh(win);
    wrefresh(maintextwin);
    wrefresh(playerpokewin);
    wrefresh(enemypokewin);

    delwin(win);
    delwin(maintextwin);
    delwin(playerpokewin);
    delwin(enemypokewin);
    music = loadMusic("music/BG.mp3");
    playMusic(music); 
    printMap(currentScreen->y, currentScreen->x, currentScreen);
    refresh();
}

/*
    Function to display a given pokemons moves during a battle
*/
int displayMoves(pokemon_t *trainerPokemon, pokemon_t *playerPokemon, int newMove){
    char c;
    ITEM **moves_items;
    MENU *moves_menu;
    WINDOW *moves_menu_win;
    int n_choices, i, j, loop, fight; //fight is 0 if there was no move selected, and 1 if a move was executed
    int moveswinWidth = 48;
    int moveswinLen = 6;
    n_choices = 4;
    moves_items = (ITEM **)calloc(10, sizeof(ITEM *));
    j = 0;
    for(i = 0; i < 4; i++){
        if(playerPokemon->pokemon_moves[i] > 0){
            const char *s = movesVector.at(playerPokemon->pokemon_moves[i] - 1).identifier.c_str();
            moves_items[j] = new_item(s ,"");
            j++;
        }
    }

    moves_menu = new_menu((ITEM **)moves_items);
    menu_opts_off(moves_menu, O_SHOWDESC);
    moves_menu_win = newwin(moveswinLen, moveswinWidth, 21 / 2 + 1, (80 - moveswinWidth) / 2);
    keypad(moves_menu_win, TRUE);

    set_menu_win(moves_menu, moves_menu_win);
    set_menu_sub(moves_menu, derwin(moves_menu_win, 2, 46, 2, 1));
	set_menu_format(moves_menu, 2, 2);
    set_menu_mark(moves_menu, " * ");
    box(moves_menu_win, 0, 0);

    post_menu(moves_menu);
    wrefresh(moves_menu_win);
    loop = 0;
    fight = 0;
    while(loop == 0){
        c = getch();
        switch(c){
             case 's':
                menu_driver(moves_menu, REQ_DOWN_ITEM);
                break;
            case 'w':
                menu_driver(moves_menu, REQ_UP_ITEM);
                break;
            case 'a':
                menu_driver(moves_menu, REQ_LEFT_ITEM);
                break;
            case 'd':
                menu_driver(moves_menu, REQ_RIGHT_ITEM);
                break;
            case 27: 
                loop = 1;
                break;
            case 10:
                playSound(aButton);
                clrtoeol();
               
                for(i = 0; i < 4; i++){
                    if(playerPokemon->pokemon_moves[i] > 0){
                        if(item_name(current_item(moves_menu)) == movesVector.at(playerPokemon->pokemon_moves[i] - 1).identifier.c_str()){
                            if(newMove){
                                playerPokemon->pokemon_moves[i] = newMove;
                                loop = 1;
                            }
                            else{
                                executeMoveCycle(playerPokemon->pokemon_moves[i], trainerPokemon, playerPokemon);
                                loop = 1;
                                fight = 1;
                            }
                            
                        }
                    }
                }
                break;
        }
        wrefresh(moves_menu_win);
    }
    unpost_menu(moves_menu);
    for (i = 0; i < n_choices; i++){
        free_item(moves_items[i]);
    }
    free_menu(moves_menu);
    wborder(moves_menu_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(moves_menu_win);
    delwin(moves_menu_win);
    refresh();
    return fight;
}
   
/*
    Function to display a the players pokemon
*/
int displayPokemon(int item, int deadSwap, map *currentScreen){
    char c;
    ITEM **pokemon_items;
    WINDOW *maintextwin;
    MENU *pokemon_menu;
    WINDOW *pokemon_menu_win;
    int n_choices, i, actionTaken, loop;
    int pokemonwinWidth = 60;
    int pokemonwinLen = 6;
    n_choices = 4;
    pokemon_items = (ITEM **)calloc(10, sizeof(ITEM *));
    
    for(i = 0; i < 4; i++){
        if(currentScreen->NPC[0].trainer_pokemon[i].level > 0){
            const char *name = pokemonCSVVector.at(currentScreen->NPC[0].trainer_pokemon[i].species_id - 1).identifier.c_str();
            pokemon_items[i] = new_item(name ,"");
        }
    }

    pokemon_menu = new_menu((ITEM **)pokemon_items);
    menu_opts_off(pokemon_menu, O_SHOWDESC);
    pokemon_menu_win = newwin(pokemonwinLen, pokemonwinWidth, 21 / 2 + 1, (80 - pokemonwinWidth) / 2);
    keypad(pokemon_menu_win, TRUE);
    maintextwin = newwin(pokemonwinLen, pokemonwinWidth, (21 + pokemonwinLen) / 2 - 2, (80 - pokemonwinWidth ) / 2);
    set_menu_win(pokemon_menu, pokemon_menu_win);
    set_menu_sub(pokemon_menu, derwin(pokemon_menu_win, 2, 58, 2, 1));
	set_menu_format(pokemon_menu, 2, 3);
    set_menu_mark(pokemon_menu, " * ");
    box(pokemon_menu_win, 0, 0);
    box(maintextwin,0,0);
    post_menu(pokemon_menu);
    wrefresh(pokemon_menu_win);
    actionTaken = 0; //if actionTaken is 0, there was nothing changed
    loop = 0;
    int healed = -1;
    while(loop == 0){
        c = getch();
        switch(c){
             case 's':
                menu_driver(pokemon_menu, REQ_DOWN_ITEM);
                break;
            case 'w':
                menu_driver(pokemon_menu, REQ_UP_ITEM);
                break;
            case 'a':
                menu_driver(pokemon_menu, REQ_LEFT_ITEM);
                break;
            case 'd':
                menu_driver(pokemon_menu, REQ_RIGHT_ITEM);
                break;
            case 27:
                loop = 1;
                break;
            case 10:
                clrtoeol();
                playSound(aButton);
                for(i = 0; i < 6; i++){
                    if((current_item(pokemon_menu)) == pokemon_items[i]){
                        if(item > 0){
                            healed = healPokemon(item, &currentScreen->NPC[0].trainer_pokemon[i], currentScreen);
                            actionTaken = 1;
                            loop = 1;
                        }
                        else{
                            if(currentScreen->NPC[0].trainer_pokemon[i].hp == 0){
                                char buffer[1024] = "";
                                std::string playerPokeName = pokemonCSVVector.at(currentScreen->NPC[0].trainer_pokemon[i].species_id - 1).identifier;
                                strcat(buffer, playerPokeName.c_str());
                                strcat(buffer, " has passed out.");
                                mvwprintw(maintextwin, 2, (pokemonwinWidth - strlen(buffer)) / 2,"%s",buffer);
                                mvwprintw(maintextwin, 3, (pokemonwinWidth - strlen("Choose another Pokemon.")) / 2,"%s","Choose another Pokemon.");
                                touchwin(maintextwin);
                                wrefresh(maintextwin);
                                sleep(2);
                                mvwprintw(maintextwin, 2, 1, "%s", "                                              "); 
                                mvwprintw(maintextwin, 3, 1, "%s", "                                              "); 
                                wrefresh(maintextwin); 
                                post_menu(pokemon_menu);
                                wrefresh(pokemon_menu_win);
                                post_menu(pokemon_menu);
                                redrawwin(pokemon_menu_win);
                                refresh();
                            }
                            else{
                                std::swap(currentScreen->NPC[0].trainer_pokemon[0], currentScreen->NPC[0].trainer_pokemon[i]);
                                if(deadSwap != 1){
                                    actionTaken = 1;
                                    int sound = loadSound("music/SFX_SWAP.wav");
                                    playSound(sound);
                                    sleep(1);
                                }
                                    
                                loop = 1;
                            }
                            
                        }  
                    }
                }
                if(healed == 0){
                    unpost_menu(pokemon_menu);
                    delwin(pokemon_menu_win);
                    std::string text = "This pokemon is already healed!";
                    writeToMainTextWin(text, 2);
                    post_menu(pokemon_menu);
                    wrefresh(pokemon_menu_win);
                    post_menu(pokemon_menu);
                    redrawwin(pokemon_menu_win);
                    refresh();
                }
                else if (healed > 0){
                    int sound = loadSound("music/SFX_HEAL_UP.wav");
                    playSound(sound);
                    sleep(1);
                    actionTaken = 1;
                }
                break;
        }
        wrefresh(pokemon_menu_win);
    }
    unpost_menu(pokemon_menu);
    for (i = 0; i < n_choices; i++){
        free_item(pokemon_items[i]);
    }
    free_menu(pokemon_menu);
    wborder(pokemon_menu_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(pokemon_menu_win);
    delwin(pokemon_menu_win);
    refresh();
    return actionTaken;
}

/*
    Function to display a warning above the screen to the player
*/
void displayWarning(const char* message){
    mvprintw(0, (MAP_X - strlen(message)) / 2, "%s", message);
    refresh();
    usleep(1100000);
    mvprintw(0, (MAP_X - strlen("------Welcome trainers, to Pokemon for C!------")) / 2, "%s", "------Welcome trainers, to Pokemon for C!------");
    refresh();
}

/*
    Function to print map to console
*/
void printMap(int currentRow, int currentColumn, map *currentScreen){
    int check;
    int x = currentRow - 200;
    int y = currentColumn - 200;
    start_color();
    init_pair(ter_clearing, COLOR_GREEN, COLOR_BLACK);
    init_pair(ter_grass, COLOR_YELLOW, COLOR_BLACK);
    init_pair(ter_path, COLOR_WHITE, COLOR_BLACK);
    init_pair(ter_water, COLOR_CYAN, COLOR_BLACK);
    init_pair(ter_mountain, 8, COLOR_BLACK);
    init_pair(ter_player, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(ter_mart, COLOR_BLUE, COLOR_BLACK);
    init_pair(ter_center, COLOR_RED, COLOR_BLACK);
    init_pair(ter_tree, 10, COLOR_BLACK);
    mvprintw(0, (MAP_X - strlen("------Welcome trainers, to Pokemon for C!------")) / 2, "%s", "------Welcome trainers, to Pokemon for C!------");
    mvprintw(22, (MAP_X - 40) / 2, "%s %d %s %d %s", "-----You are at Coordinates", y, "x", x, "-----");
    for(int i = 1; i < MAP_Y+1; i++){
        for(int j = 0; j < MAP_X; j++){
            check = 1;
            for(int k = 0; k < numTrainers; k++){
                if(currentScreen->NPC[k].y == i-1 && currentScreen->NPC[k].x == j){
                    mvprintw(i, j, "%c", currentScreen->NPC[k].symbol);
                    check = 0;
                    break;
                }
            }
            if(check == 1){
                switch (currentScreen->screen[i-1][j]){
                    case ter_boulder:
                    case ter_mountain:
                        attron(COLOR_PAIR(ter_mountain));
                        mvprintw(i, j, "%c", '%');
                        attroff(COLOR_PAIR(ter_mountain));
                        break;
                    case ter_tree:
                    case ter_forest:
                        attron(COLOR_PAIR(ter_tree));
                        mvprintw(i, j, "%c", '^');
                        attroff(COLOR_PAIR(ter_tree));
                        break;
                    case ter_path:
                    case ter_gate:
                        attron(COLOR_PAIR(ter_path));
                        mvprintw(i, j, "%c", '#');
                        attroff(COLOR_PAIR(ter_path));
                        break;
                    case ter_mart:
                        attron(COLOR_PAIR(ter_mart));
                        mvprintw(i, j, "%c", 'M');
                        attroff(COLOR_PAIR(ter_mart));
                        break;
                    case ter_center:
                       attron(COLOR_PAIR(ter_center));
                        mvprintw(i, j, "%c", 'C');
                        attroff(COLOR_PAIR(ter_center));
                        break;
                    case ter_grass:
                        attron(COLOR_PAIR(ter_grass));
                        mvprintw(i, j, "%c", ':');
                        attroff(COLOR_PAIR(ter_grass));
                        break;
                    case ter_clearing:
                        attron(COLOR_PAIR(ter_clearing));
                        mvprintw(i, j, "%c", '.');
                        attroff(COLOR_PAIR(ter_clearing));
                        break;
                    case ter_water:
                        attron(COLOR_PAIR(ter_water));
                        mvprintw(i, j, "%c", '~');
                        attroff(COLOR_PAIR(ter_water));
                        break;
                    }
                }
            }
    }
}

/*
    Function to print and allow input to fly to another map
*/
int displayFlyMenu(int currentRow, int currentColumn, map *currentScreen){
    WINDOW *win;
    refresh();
    int winWidth = 32;
    int winLen = 10;
    char *title;
    
    char strX[10];
    char strY[10];
    int x;
    int y;
    std::stringstream ssX;
    std::stringstream ssY;

    echo();

    win = newwin(10, 32, (MAP_Y - winLen) / 2, (MAP_X - winWidth) / 2);
    box(win,0,0);

    mvwprintw(win, 0, (winWidth - strlen("Fly to any map in the world!")) / 2, "Fly to any map in the world!");
    mvwprintw(win, 2, (winWidth - 12) / 2, "X Coordinate:");
    move(MAP_Y / 2 - 2, MAP_X/ 2 - 1);
    refresh();
    wrefresh(win);
    getstr(strX);

    ssX << strX;
    ssX >> x;

    mvwprintw(win, 4, (winWidth - 12) / 2, "Y Coordinate:");
    move(MAP_Y / 2, MAP_X/ 2 - 1);
    refresh();
    wrefresh(win);
    getstr(strY);

    ssY << strY;
    ssY >> y;

    
    wclear(win);
    wrefresh(win);
    printMap(currentRow, currentColumn, currentScreen);

    delwin(win);
    noecho();
    
    if(y > 200 || x > 200 || x < -200 || y < -200 ){
        displayWarning("-Inputs must be betweeen -200 and 200-");
        return 0;
    }
    else if(abs(y+200) == currentRow && abs(x+200) == currentColumn){
        displayWarning("----You are on this map already----");
        return 0;
    }
    else{
        currentScreen->nextY = abs(y+200);
        currentScreen->nextX = abs(x+200);
        return 5;
    }
        
}

/*
    Function that prints PokeMart and PokeCenter displays
*/
void displayMandCInterface(int display, map *currentScreen){
    WINDOW *win, *moneywin;
    ITEM **my_items;
    int n_choices;
    MENU *my_menu;  
    refresh();

    int potionPrice = 300;
    int revivePrice = 1500;
    int pokeballPrice = 200;

    int winWidth = 30;
    int winLen = 10;
    char *title;
    char c;
    int music;

    

    if(display == 0){
        n_choices = 5;
        my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
        
        win = newwin(winLen, winWidth, (MAP_Y - winLen) / 2, (MAP_X - winWidth) / 2);
        box(win,0,0);
        //Set up money window
        moneywin = newwin(3, 8, (21 + winLen) / 2 - 6, (80 - winWidth ) / 2 + winWidth+ 2);
        box(moneywin,0,0);
        mvwprintw(moneywin, 1, 1,"%s %d", "$:", currentScreen->NPC[0].pokedollars);

        my_items[0] = new_item("Potion", std::to_string(potionPrice).c_str());
        my_items[1] = new_item("Pokeball", std::to_string(pokeballPrice).c_str());
        my_items[2] = new_item("Revive", std::to_string(revivePrice).c_str());

        my_menu = new_menu((ITEM **)my_items);

        keypad(win, TRUE);
        // menu_opts_off(my_menu, O_SHOWDESC);
        /* Set main window and sub window */
        set_menu_win(my_menu, win);
        set_menu_sub(my_menu, derwin(win, 3, 28, 4, 1));
	    set_menu_format(my_menu, 3, 1);
        set_menu_mark(my_menu, "      * ");
        mvwprintw(win, 0, (winWidth - strlen("Welcome to the PokeMart!")) / 2, "Welcome to the PokeMart!");
        music = loadMusic("music/pokemon_center.mp3");
        playMusic(music); 
      
        post_menu(my_menu);
	    wrefresh(win);
        wrefresh(moneywin);
        int loop = 0;
        while(loop == 0){
            c = getch();
            switch(c){
                case 'w':
                    menu_driver(my_menu, REQ_UP_ITEM);
                    break;
                case 's':
                    menu_driver(my_menu, REQ_DOWN_ITEM);
                    break;
                case 27:
                    loop = 1;
                    break;
                case 10: /* ENTER */
                    playSound(aButton);
                    clrtoeol();
                    if(item_name(current_item(my_menu)) == "Potion"){
                        if (currentScreen->NPC[0].pokedollars >= potionPrice){
                            currentScreen->NPC[0].pokedollars-= potionPrice;
                            currentScreen->NPC[0].items[0]+=1;
                            mvwprintw(moneywin, 1, 1,"%s %d", "$:", currentScreen->NPC[0].pokedollars);
                        }
                        else{
                            displayWarning("You don't have enough money");
                            int sound = loadSound("music/SFX_DENIED.wav");
                            playSound(sound);
                        }
                    }
                    else if(item_name(current_item(my_menu)) == "Pokeball"){
                        if (currentScreen->NPC[0].pokedollars >= pokeballPrice){
                            currentScreen->NPC[0].pokedollars-= pokeballPrice;
                            currentScreen->NPC[0].items[3]+=1;
                            mvwprintw(moneywin, 1, 1,"%s %d", "$:", currentScreen->NPC[0].pokedollars);
                        }
                        else{
                            displayWarning("You don't have enough money");
                            int sound = loadSound("music/SFX_DENIED.wav");
                            playSound(sound);
                        }
                    }
                    else if (item_name(current_item(my_menu)) == "Revive"){
                        if (currentScreen->NPC[0].pokedollars >= revivePrice){
                            currentScreen->NPC[0].pokedollars-= revivePrice;
                            currentScreen->NPC[0].items[2]+=1;
                            mvwprintw(moneywin, 1, 1,"%s %d", "$:", currentScreen->NPC[0].pokedollars);
                        }
                        else{
                           displayWarning("You don't have enough money");
                           int sound = loadSound("music/SFX_DENIED.wav");
                            playSound(sound);
                        }
                    }
                    pos_menu_cursor(my_menu);
                    break;
                }
                wrefresh(win);
                wrefresh(moneywin);
        }
        /* Unpost and free all the memory taken up */
        unpost_menu(my_menu);
        for(int i = 0; i < n_choices; ++i){
            free_item(my_items[i]);
        }
        free_menu(my_menu);
        delwin(win);
        delwin(moneywin);
        wrefresh(win);
        refresh();
        music = loadMusic("music/BG.mp3");
        playMusic(music); 
        
    }
    else{
        n_choices = 2;
        win = newwin(winLen, winWidth, (MAP_Y - winLen) / 2, (MAP_X - winWidth) / 2);
        box(win,0,0);
        my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
        my_items[0] = new_item("Heal Team", "");

        my_menu = new_menu((ITEM **)my_items);

        keypad(win, TRUE);
        // menu_opts_off(my_menu, O_SHOWDESC);
        /* Set main window and sub window */
        set_menu_win(my_menu, win);
        set_menu_sub(my_menu, derwin(win, 3, 28, 4, 1));
	    set_menu_format(my_menu, 3, 1);
        set_menu_mark(my_menu, "       * ");         
        mvwprintw(win, 0, (winWidth - strlen("Welcome to the PokeCenter!")) / 2, "Welcome to the PokeCenter!");
        mvwprintw(win, 2, (winWidth - 32) / 2, "Lets heal up your team!");
        
        music = loadMusic("music/pokemon_center.mp3");
        playMusic(music);
        post_menu(my_menu);
        wrefresh(win);
        while(c != 27){
            c = getch();
            switch(c){
                case 10:
                int sound = loadSound("music/SFX_HEAL_UP.wav");
                playSound(sound);
                for(int i = 0; i < 6; i++){
                    currentScreen->NPC[0].trainer_pokemon[i].hp =currentScreen->NPC[0].trainer_pokemon[i].maxHp;
                } 
            }
            wrefresh(win);
        }
        /* Unpost and free all the memory taken up */
        unpost_menu(my_menu);
        for(int i = 0; i < n_choices; ++i){
            free_item(my_items[i]);
        }
        free_menu(my_menu);
        delwin(win);
        wrefresh(win);
        refresh();
        music = loadMusic("music/BG.mp3");
        playMusic(music); 
    }
    
}

/*
    Function that prints all trainers and relative positions to player
*/
void displayTrainerMenu(map *currentScreen){
    ITEM **my_items;
	int c;				
	MENU *my_menu;
    WINDOW *my_menu_win;
    int n_choices, i;
    const char *trainers[6];  

    //fill the trainer array to be displayed
    for(i = 0; i < numTrainers; i++){
        if(currentScreen->NPC[i + 1].symbol == 'r')
            trainers[i] = "Rival";
        if(currentScreen->NPC[i + 1].symbol == 'h')
            trainers[i] = "Hiker";
        if(currentScreen->NPC[i + 1].symbol == 's')
            trainers[i] = "Sentry";
        if(currentScreen->NPC[i + 1].symbol == 'e')
            trainers[i] = "Explorer";
        if(currentScreen->NPC[i + 1].symbol == 'p')
            trainers[i] = "Pacer";
        if(currentScreen->NPC[i + 1].symbol == 'w')
            trainers[i] = "Wanderer";
    }
    trainers[numTrainers] = (char *)NULL;

	/* Create items */
    n_choices = 7;

    char buffer[n_choices][50];

    my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
    for(i = 0; i < n_choices; ++i){
        int x = currentScreen->NPC[i+1].x - currentScreen->NPC[0].x;
        int y = currentScreen->NPC[i+1].y - currentScreen->NPC[0].y;
        if(currentScreen->NPC[i+1].y < currentScreen->NPC[0].y){
            if((currentScreen->NPC[i+1].x < currentScreen->NPC[0].x))
                sprintf(buffer[i], "%d north and %d west", abs(y), abs(x));
            else
                sprintf(buffer[i], "%d north and %d east", abs(y), abs(x));
        }
        else {
            if((currentScreen->NPC[i+1].x < currentScreen->NPC[0].x))
                sprintf(buffer[i], "%d south and %d west", abs(y), abs(x));
            else
                sprintf(buffer[i], "%d south and %d east", abs(y), abs(x));
        }        
        my_items[i] = new_item(trainers[i], buffer[i]);
    }

	/* Create menu */
	my_menu = new_menu((ITEM **)my_items);
    int winWidth = 40;
    int winLen = 10;
	/* Create the window to be associated with the menu */
    my_menu_win = newwin(10, 40, (MAP_Y - winLen) / 2, (MAP_X - winWidth) / 2);
    keypad(my_menu_win, TRUE);
     
	/* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 38, 3, 1));
	set_menu_format(my_menu, 5, 1);
			
	/* Set menu mark to the string " * " */
        set_menu_mark(my_menu, " * ");

	/* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0);
	print_in_middle(my_menu_win, 1, 0, 40, "Trainer List", COLOR_PAIR(1));
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
	mvwaddch(my_menu_win, 2, 39, ACS_RTEE);
        
	/* Post the menu */
	post_menu(my_menu);
	wrefresh(my_menu_win);
	
	refresh();

	while((c = wgetch(my_menu_win)) != 27){
     
       switch(c){
        case KEY_DOWN:
            menu_driver(my_menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(my_menu, REQ_UP_ITEM);
            break;
        case KEY_NPAGE:
            menu_driver(my_menu, REQ_SCR_DPAGE);
            break;
        case KEY_PPAGE:
            menu_driver(my_menu, REQ_SCR_UPAGE);
            break;
        }
        wrefresh(my_menu_win);
	}
    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    delwin(my_menu_win);
    for(i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
}

/*
    Function that displays and allows use of items in players bag
*/
int displayItems(pokemon_t *wildPokemon, int wild, map *currentScreen){ //if wild == 0, not wild, if 1, wild.
    char c;
    ITEM **items_items;
    MENU *items_menu;
    WINDOW *items_menu_win;
    WINDOW *maintextwin, *moneywin;
    int n_choices, i, loop, pokemonCaught;
    int itemswinWidth = 48;
    int itemswinLen = 6;
    n_choices = 3;

    items_items = (ITEM **)calloc(10, sizeof(ITEM *));
    char reviveNum[5] = "x";
    char pokeballNum[5] = "x";
    char potionNum[5] = "x";

    std::string playerPotion = std::to_string(currentScreen->NPC[0].items[0]);
    std::string playerRevive = std::to_string(currentScreen->NPC[0].items[1]);
    std::string playerPokeball = std::to_string(currentScreen->NPC[0].items[2]);
    
    //concat actual number of items in bag
    strcat(reviveNum, playerRevive.c_str());
    strcat(pokeballNum, playerPokeball.c_str());
    strcat(potionNum, playerPotion.c_str());

    items_items[0] = new_item("Potion" ,potionNum);
    items_items[1] = new_item("Revive" ,reviveNum);
    items_items[2] = new_item("Pokeball" ,pokeballNum);

    items_menu = new_menu((ITEM **)items_items);
    // menu_opts_off(items_menu, O_SHOWDESC);
    items_menu_win = newwin(itemswinLen, itemswinWidth, 21 / 2 + 1, (80 - itemswinWidth) / 2);
    keypad(items_menu_win, TRUE);
    moneywin = newwin(3, 8, (21 + itemswinLen) / 2 - 5, (80 - itemswinWidth ) / 2 + itemswinWidth - 7);
    maintextwin = newwin(itemswinLen, itemswinWidth, (21 + itemswinLen) / 2 - 2, (80 - itemswinWidth ) / 2);
    box(maintextwin,0,0);
    box(moneywin,0,0);
    mvwprintw(moneywin, 1, 1,"%s %d", "$:", currentScreen->NPC[0].pokedollars);
    set_menu_win(items_menu, items_menu_win);
    set_menu_sub(items_menu, derwin(items_menu_win, 2, 46, 2, 1));
	set_menu_format(items_menu, 2, 2);
    set_menu_mark(items_menu, " * ");
    box(items_menu_win, 0, 0);

    post_menu(items_menu);
    wrefresh(items_menu_win);
    if(wild < 0){
        wrefresh(moneywin);
    }
    
    loop = 0;
    pokemonCaught = 0;
    while(loop != 1){
        c = getch();
        switch(c){
             case 's':
                menu_driver(items_menu, REQ_DOWN_ITEM);
                break;
            case 'w':
                menu_driver(items_menu, REQ_UP_ITEM);
                break;
            case 'a':
                menu_driver(items_menu, REQ_LEFT_ITEM);
                break;
            case 'd':
                menu_driver(items_menu, REQ_RIGHT_ITEM);
                break;
            case 27:
                loop = 1;
                break;
            case 10:
                
                playSound(aButton);
                clrtoeol();
                if(item_name(current_item(items_menu)) == "Pokeball"){
                    if(wild == -1){
                        unpost_menu(items_menu);
                        std::string text = "Can't use this here!";
                        writeToMainTextWin(text, 2);
                        post_menu(items_menu);
                        redrawwin(items_menu_win);
                    }
                    if(wild == 1 && currentScreen->NPC[0].items[2] > 0){
                        int sound = loadSound("music/SFX_BALL_TOSS.wav");
                        playSound(sound);
                        sleep(1);
                        for(i = 0; i < 6; i++){
                            if(currentScreen->NPC[0].trainer_pokemon[i].level == 0){
                                currentScreen->NPC[0].trainer_pokemon[i] = *wildPokemon;
                                i = 6;
                            }
                        }
                        unpost_menu(items_menu);
                        std::string pokeName = pokemonCSVVector.at(wildPokemon->species_id - 1).identifier;
                        
                        char buffer[1024] = "You caught a ";
                        strcat(buffer, pokeName.c_str());
                        strcat(buffer, "!");
                        mvwprintw(maintextwin, 2, (itemswinWidth - strlen(buffer)) / 2,"%s",buffer);
                        wrefresh(maintextwin);
                        redrawwin(maintextwin);
                        togglePlay();
                        int music = loadSound("music/pokemon_caught.wav");
                        playAndWait(1, music, 0); 
                        currentScreen->NPC[0].items[2] -= 1;
                        post_menu(items_menu);
                        redrawwin(items_menu_win);
                        
                        loop = 1;
                        pokemonCaught = 1;
                    }
                    else if(!wild && currentScreen->NPC[0].items[2] > 0){
                        unpost_menu(items_menu);
                        std::string text = "You can't catch this pokemon!";
                        writeToMainTextWin(text, 2);
                        post_menu(items_menu);
                        redrawwin(items_menu_win);
                    }
                }
                else if(item_name(current_item(items_menu)) == "Potion"){
                    unpost_menu(items_menu);
                    int actionTaken = displayPokemon(1, 0, currentScreen);
                    if(actionTaken == 1 && wild != -1)
                        executeMoveCycle(0, wildPokemon, &currentScreen->NPC[0].trainer_pokemon[0]);
                    redrawwin(maintextwin);
                    post_menu(items_menu);
                    redrawwin(items_menu_win);
                    loop = 1;
                }
                else if(item_name(current_item(items_menu)) == "Revive"){
                    unpost_menu(items_menu);
                    int actionTaken = displayPokemon(2, 0, currentScreen);
                    if(actionTaken == 1 && wild != -1)
                        executeMoveCycle(0, wildPokemon, &currentScreen->NPC[0].trainer_pokemon[0]);
                    redrawwin(maintextwin);
                    post_menu(items_menu);
                    redrawwin(items_menu_win);
                    loop = 1;
                }
                break;
            
        }
        wrefresh(items_menu_win);
    }
    unpost_menu(items_menu);
    for (i = 0; i < n_choices; i++){
        free_item(items_items[i]);
    }
    free_menu(items_menu);
    wborder(items_menu_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(items_menu_win);
    delwin(items_menu_win);
    delwin(moneywin);
    refresh();
    return pokemonCaught;
}


/*
    Function that prints text in middle of window
*/
void print_in_middle(WINDOW *win, int starty, int startx, int width, const char *string, chtype color)
{	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}