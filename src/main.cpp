#include "Game.hpp"

#include <iostream>
#include <exception>


int main(int argc, char* argv[]) 
{
    bool demoMode = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "-smart")
        {
            demoMode = true;
        }
    }

    try 
    {
        Game game(demoMode);
        game.run();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Erreur fatale : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
