#pragma once

/**
 * @namespace Settings
 * @brief Regroupe tous les paramètres de configuration et d'équilibrage du jeu.
 */
namespace Settings 
{
    // --- Fenêtre et Performances ---
    inline int WINDOW_WIDTH  = 1920;
    inline int WINDOW_HEIGHT = 1080;
    constexpr int TARGET_FPS = 30;

    // --- Joueur (Thomas) ---
    constexpr float PLAYER_SIZE            = 40.0f;
    constexpr int   PLAYER_START_X         = 200;
    constexpr int   PLAYER_MAX_HEALTH      = 100;
    constexpr float PLAYER_SPEED           = 600.0f;
    constexpr float STAGGER_INTERVAL       = 2.0f; 
    constexpr float INVINCIBILITY_DURATION = 2.0f; 

    // --- Difficulté et Vitesse ---
    constexpr float BASE_OBSTACLE_SPEED      = 600.0f; 
    constexpr float MAX_OBSTACLE_SPEED       = 1400.0f; 
    constexpr float BASE_SPAWN_INTERVAL_MAX  = 1.0f; 
    constexpr float MIN_SPAWN_INTERVAL       = 0.3f; 
    constexpr int   GROUP_SPAWN_PROB         = 20;    // % de chance de spawn double
    constexpr float DIFFICULTY_GROWTH        = 15.0f; // Vitesse gagnée par seconde

    // --- Obstacles : Dimensions ---
    constexpr float KEBAB_SIZE     = 20.0f;
    constexpr float GARBAGE_WIDTH  = 30.0f;
    constexpr float GARBAGE_HEIGHT = 45.0f;
    constexpr float SCOOTER_WIDTH  = 15.0f;
    constexpr float SCOOTER_HEIGHT = 45.0f;
    constexpr float CHOUFFE_WIDTH  = 15.0f;
    constexpr float CHOUFFE_HEIGHT = 25.0f;
    constexpr float STUDENT_SIZE   = 40.0f;

    // --- Obstacles : Équilibrage (Dégâts/Soins) ---
    constexpr int   KEBAB_HEAL            = 10;
    constexpr int   GARBAGE_DAMAGE        = 10;
    constexpr int   SCOOTER_DAMAGE        = 30;
    constexpr int   STUDENT_DAMAGE        = 20;
    constexpr float CONFUSION_DURATION_MIN = 1.0f;
    constexpr float CONFUSION_DURATION_MAX = 5.0f;

    // --- Power-Ups et Bonus ---
    constexpr float KEBAB_BOOST_DURATION   = 0.5f; 
    constexpr float KEBAB_SAFETY_DURATION  = 0.5f; 
    constexpr float KEBAB_SPEED_MULTIPLIER = 1.3f;
    constexpr float KEBAB_SIZE_MULTIPLIER  = 1.2f;

    // --- Probabilités d'apparition (Total = 100) ---
    constexpr int PROB_KEBAB   = 15;
    constexpr int PROB_CHOUFFE = 5;
    constexpr int PROB_GARBAGE = 30;
    constexpr int PROB_SCOOTER = 10;
    constexpr int PROB_STUDENT = 40;

    // --- Effets Visuels ---
    constexpr float KEBAB_FLASH_DURATION  = 0.5f;
    constexpr int   PARTICLE_COUNT        = 15;
    constexpr float PARTICLE_LIFE         = 0.8f;
    constexpr float FLOATING_TEXT_LIFE    = 1.5f;
    constexpr float FLOATING_TEXT_SPEED   = 50.0f;
    constexpr float SHAKE_DURATION        = 1.0f;
    constexpr int   SHAKE_INTENSITY       = 10;

    // --- Progression et Parallaxe ---
    constexpr float SCORE_MULTIPLIER      = 10.0f;
    constexpr int   MAX_SCORE_DISTANCE    = 10000;
    constexpr float PARALLAX_FACTOR       = 0.3f;
}
