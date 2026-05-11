#pragma once

namespace Constants {
    // Résolution dynamique (mise à jour par le moteur Wayland)
    inline int WINDOW_WIDTH = 1920;
    inline int WINDOW_HEIGHT = 1080;
    constexpr int TARGET_FPS = 30;

    constexpr float PLAYER_SIZE = 40.0f; // Légèrement agrandi pour le 1080p
    constexpr int PLAYER_START_X = 200;
    constexpr int PLAYER_MAX_HEALTH = 100;
    constexpr float PLAYER_SPEED = 600.0f; // Ajusté pour le 1080p
    constexpr float STAGGER_INTERVAL = 2.0f; 
    constexpr float INVINCIBILITY_DURATION = 2.0f; 

    // Difficulté progressive
    constexpr float BASE_OBSTACLE_SPEED = 600.0f; 
    constexpr float MAX_OBSTACLE_SPEED = 1400.0f; 
    constexpr float BASE_SPAWN_INTERVAL_MAX = 1.0f; 
    
    // Probabilités (Total = 100)
    constexpr int PROB_KEBAB = 15;
    constexpr int PROB_CHOUFFE = 5; // Le nouveau Power-Up !
    constexpr int PROB_GARBAGE = 30;
    constexpr int PROB_SCOOTER = 10;
    constexpr int PROB_STUDENT = 40;
}
