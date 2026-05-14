#pragma once

#include <fstream>
#include <iostream>
#include <string>


class ScoreManager 
{
    private:
        int highScores[3]; ///< 0: Easy, 1: Intermediate, 2: Expert
        const std::string filename = "highscores.txt";

    public:
        ScoreManager() 
        {
            // Initialisation par défaut
            for (int i = 0; i < 3; ++i) highScores[i] = 0;

            std::ifstream file(filename);
            if (file.is_open()) 
            {
                for (int i = 0; i < 3; ++i) 
                {
                    if (!(file >> highScores[i])) break;
                }
            } 
        }

        int getHighScore(int difficulty) const 
        { 
            if (difficulty < 0 || difficulty >= 3) return 0;
            return highScores[difficulty]; 
        }

        bool submitScore(int score, int difficulty) 
        {
            if (difficulty < 0 || difficulty >= 3) return false;

            if (score > highScores[difficulty]) 
            {
                highScores[difficulty] = score;

                std::ofstream file(filename);
                if (file.is_open())
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        file << highScores[i] << " ";
                    }
                }

                return true; // Nouveau record !
            }

            return false;
        }
};
