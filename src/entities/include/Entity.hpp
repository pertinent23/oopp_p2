#pragma once

#include "Vector2D.hpp"


namespace gfx 
{ 
    class Window; 
}


/**
 * @class Entity
 * @brief Classe de base abstraite pour tous les objets mobiles du jeu (Joueur, Obstacles).
 * 
 * Cette classe définit les propriétés fondamentales de position, taille et vitesse,
 * ainsi que les méthodes virtuelles pures pour la mise à jour et l'affichage.
 */
class Entity 
{
    protected:
        Vector2D position; ///< Position actuelle de l'entité dans le monde
        Vector2D size;     ///< Dimensions (largeur, hauteur) de l'entité
        Vector2D velocity; ///< Vecteur vitesse actuel

    public:
        /**
         * @struct Rect
         * @brief Structure simple représentant un rectangle pour la gestion des collisions.
         */
        struct Rect 
        { 
            float x, y, w, h; 
        };

        /**
         * @brief Constructeur de l'entité.
         * @param x Position horizontale initiale.
         * @param y Position verticale initiale.
         * @param w Largeur.
         * @param h Hauteur.
         */
        Entity(
            float x, 
            float y, 
            float w, 
            float h
        );

        virtual ~Entity() = default;

        /**
         * @brief Met à jour l'état de l'entité (logique, mouvement).
         * @param deltaTime Temps écoulé depuis la dernière frame.
         */
        virtual void update(float deltaTime) = 0;

        /**
         * @brief Affiche l'entité à l'écran.
         * @param window Référence vers la fenêtre de rendu.
         * @param brightness Ratio de luminosité (0.0 à 1.0) basé sur la santé du joueur.
         * @param offsetX Décalage X pour l'effet de tremblement (shake).
         * @param offsetY Décalage Y pour l'effet de tremblement (shake).
         */
        virtual void draw(
            gfx::Window& window, 
            float brightness = 1.0f, 
            int offsetX = 0, 
            int offsetY = 0
        ) const = 0;

        /** @return Référence constante vers la position de l'entité. */
        const Vector2D& getPosition() const;

        /** @return Référence constante vers la taille de l'entité. */
        const Vector2D& getSize() const;

        /** @return La hitbox de l'entité sous forme de Rect. */
        Rect getHitbox() const;
};
