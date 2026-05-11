#pragma once

#include <vector>
#include <memory>
#include "Entity.hpp"


class Player;
class Obstacle;


/**
 * @class CollisionManager
 * @brief Système de détection de collisions AABB (Axis-Aligned Bounding Box).
 * 
 * Cette classe statique isole la logique géométrique des collisions pour
 * garantir une séparation claire des responsabilités (SOC).
 */
class CollisionManager 
{
    private:
        /**
         * @brief Vérifie l'intersection géométrique entre deux rectangles.
         * @param a Premier rectangle.
         * @param b Second rectangle.
         * @return Vrai si les rectangles se chevauchent.
         */
        static bool checkIntersection(
            const Entity::Rect& a, 
            const Entity::Rect& b
        );

    public:
        /**
         * @brief Analyse et traite les collisions entre le joueur et tous les obstacles.
         * @param player Thomas, notre héros.
         * @param obstacles Liste des obstacles présents à l'écran.
         * @return Vrai si Thomas a été blessé (utilisé pour l'effet de shake).
         */
        static bool handleCollisions(
            Player& player, 
            std::vector<std::unique_ptr<Obstacle>>& obstacles
        );
};
