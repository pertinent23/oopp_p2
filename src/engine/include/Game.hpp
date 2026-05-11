#pragma once

#include <vector>
#include <memory>


namespace gfx 
{ 
    class Window; 
}

class Player;
class InputManager;
class ObstacleSpawner;
class Obstacle;
class ScoreManager;
class BackgroundManager;


/**
 * @class Game
 * @brief Le cœur du moteur "Carre Surfer".
 * 
 * Cette classe orchestre la boucle principale, la machine à états,
 * la mise à jour des systèmes et le rendu global.
 */
class Game
{
    private:
        std::unique_ptr<gfx::Window>      window;            ///< Fenêtre de rendu
        std::unique_ptr<InputManager>     inputManager;      ///< Gestionnaire des touches
        std::unique_ptr<Player>           player;            ///< Thomas, le héros
        std::unique_ptr<ObstacleSpawner>  obstacleSpawner;   ///< Générateur d'obstacles
        std::unique_ptr<ScoreManager>     scoreManager;      ///< Gestion des scores et records
        std::unique_ptr<BackgroundManager> backgroundManager; ///< Arrière-plan animé
        
        std::vector<std::unique_ptr<Obstacle>> obstacles;    ///< Liste des obstacles actifs
    
        /**
         * @enum GameState
         * @brief Les différents états possibles de l'application.
         */
        enum class GameState
        {
            PLAYING,        ///< Jeu actif
            PAUSED,         ///< Menu de pause
            GAME_OVER,      ///< Écran de défaite
            QUIT_CONFIRM    ///< Menu de confirmation de sortie
        };
    
        GameState currentState;     ///< État actuel
        GameState previousState;    ///< État mémorisé (pour retour après confirmation)
        
        int   gameOverSelection;    ///< Index de sélection dans les menus (0 ou 1)
        float timeSurvived;         ///< Chronomètre de survie
        float shakeTimer;           ///< Temps restant pour l'effet de tremblement
    
        /** @brief Initialise ou réinitialise tous les systèmes. */
        void init();
    
        /** @brief Met à jour la logique du jeu. */
        void update(float deltaTime);
    
        /** @brief Effectue le rendu de la scène de jeu. */
        void render();
    
        /** @brief Supprime les obstacles inactifs de la mémoire. */
        void cleanDeadObstacles();
    
        /** @brief Met à jour le chronomètre de tremblement de l'écran. */
        void handleShake(float deltaTime);
        
        /** @brief Affiche l'écran de Game Over. */
        void displayGameOver();
    
        /** @brief Affiche l'écran de Pause avec effet transparent. */
        void displayPause();
    
        /** @brief Affiche l'écran "Are you sure?". */
        void displayQuitConfirm();
        
        /**
         * @brief Dessine un cœur de vie pixelisé.
         * @param x Position X.
         * @param y Position Y.
         * @param size Taille du cœur.
         * @param fillRatio Ratio de remplissage (0.0 à 1.0).
         */
        void drawHeart(
            int x, 
            int y, 
            int size, 
            float fillRatio
        );
    
        /** @brief Affiche l'interface utilisateur (Santé, Score, Record). */
        void drawUI();
    
    public:
        /** @brief Constructeur : Prépare les systèmes de base. */
        Game();
    
        /** @brief Destructeur par défaut. */
        ~Game();
    
        /** @brief Lance la boucle infinie du jeu. */
        void run();
};
