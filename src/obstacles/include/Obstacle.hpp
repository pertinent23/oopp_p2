#pragma once

#include "Entity.hpp"


class Player; 


/**
 * @class Obstacle
 * @brief Classe de base pour tous les objets que le joueur doit éviter ou ramasser.
 * 
 * Hérite d'Entity et ajoute la gestion de l'activation/désactivation pour le nettoyage
 * automatique par le moteur de jeu.
 */
class Obstacle : public Entity 
{
    protected:
        bool active; ///< État de l'obstacle (faux s'il doit être supprimé)
    
    public:
        /**
         * @brief Constructeur d'un obstacle.
         * @param x Position horizontale.
         * @param y Position verticale.
         * @param w Largeur.
         * @param h Hauteur.
         * @param speedX Vitesse de défilement vers la gauche.
         */
        Obstacle(
            float x, 
            float y, 
            float w, 
            float h, 
            float speedX
        );
    
        /**
         * @brief Gère le déplacement vers la gauche et la désactivation hors-écran.
         */
        void update(float deltaTime) override;
        
        /**
         * @brief Méthode abstraite appelée lors d'une collision avec le joueur.
         * @param player Référence vers le joueur pour appliquer les effets.
         */
        virtual void onCollision(Player& player) = 0;
    
        /** @return Vrai si l'obstacle est toujours en jeu. */
        bool isActive() const;
    
        /** @brief Marque l'obstacle comme inactif pour suppression immédiate. */
        void deactivate();
};
