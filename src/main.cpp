#include "Game.hpp"

#include <iostream>
#include <exception>


int main() 
{
    try 
    {
        Game game;
        game.run();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Erreur fatale : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
