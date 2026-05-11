#pragma once

#include <fstream>
#include <iostream>
#include <string>


class ScoreManager 
{
    private:
        int highScore;
        const std::string filename = "highscore.txt";

    public:
        ScoreManager() 
        {
            std::ifstream file(filename);

            if (file.is_open() && (file >> highScore)) 
            {
                // Lecture réussie
            } 
            else 
            {
                highScore = 0;
            }
        }

        int getHighScore() const 
        { 
            return highScore; 
        }

        bool submitScore(int score) 
        {
            if (score > highScore) 
            {
                highScore = score;

                std::ofstream file(filename);

                if (file.is_open())
                {
                    file << highScore;
                }

                return true; // Nouveau record !
            }

            return false;
        }
};
