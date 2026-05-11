#pragma once

#include <vector>
#include <memory>
#include "Obstacle.hpp"


/**
 * @class ObstacleSpawner
 * @brief Gère la génération dynamique et la difficulté du jeu.
 * 
 * Cette classe fait apparaître des obstacles de manière aléatoire et augmente
 * progressivement la vitesse de défilement pour accroître la difficulté.
 */
class ObstacleSpawner 
{
    private:
        float spawnTimer;   ///< Temps restant avant la prochaine apparition
        float currentSpeed; ///< Vitesse de défilement actuelle (pixels par seconde)

        /** 
         * @brief Instancie un obstacle aléatoire selon des probabilités définies.
         * @return Pointeur unique vers le nouvel obstacle.
         */
        std::unique_ptr<Obstacle> createRandomObstacle() const;

    public:
        /** @brief Initialise le spawner avec les paramètres de base. */
        ObstacleSpawner();

        /** 
         * @brief Met à jour la difficulté et fait apparaître des obstacles si nécessaire.
         * @param deltaTime Temps écoulé.
         * @param obstacles Liste des obstacles actifs à compléter.
         */
        void update(
            float deltaTime, 
            std::vector<std::unique_ptr<Obstacle>>& obstacles
        );

        /** @return La vitesse actuelle du jeu. */
        float getCurrentSpeed() const;
};
