#pragma once

#include "Entity.hpp"
#include "InputManager.hpp"
#include "Settings.hpp"
#include "Obstacle.hpp"
#include <vector>
#include <memory>


/**
 * @class Player
 * @brief Représente Thomas, le personnage contrôlé par le joueur.
 * 
 * Cette classe gère les déplacements, la santé, les effets d'état (confusion, invincibilité)
 * et les mécaniques spéciales comme le nettoyage d'écran.
 */
class Player : public Entity 
{
    protected:
        InputManager& input;         ///< Référence vers le gestionnaire d'entrées
        int   health;                ///< Points de vie actuels
        float staggerTimer;          ///< Chronomètre pour l'effet de décalage aléatoire
        float invincibilityTimer;    ///< Temps d'invincibilité restant après un choc
        float kebabEffectTimer;      ///< Temps restant pour le boost Kebab (Vitesse + Taille + Invincibilité)
        bool  clearScreenRequested;  ///< Drapeau pour le power-up Chouffe
        bool  kebabEffectTriggered;  ///< Drapeau pour prévenir Game de déclencher les particules/flash

        const std::vector<std::unique_ptr<Obstacle>>* currentObstacles = nullptr; ///< Réf pour l'IA

        /** @brief Gère les déplacements. Rendu virtuel pour SmartPlayer. */
        virtual void handleMovement(float deltaTime);

        /** @brief Applique un petit décalage aléatoire à la position (Simule la fatigue). */
        void applyStagger();

        /** @brief Met à jour les différents compteurs de temps. */
        void updateTimers(float deltaTime);

        /** @brief Empêche le joueur de sortir des limites de l'écran. */
        void constrainToWindow();

    public:
        /**
         * @brief Initialise Thomas au centre de l'écran.
         * @param inputManager Gestionnaire d'entrées pour le contrôle.
         */
        explicit Player(InputManager& inputManager);

        /** @brief Logique principale du joueur. */
        void update(float deltaTime) override;

        /** @brief Permet de passer les obstacles à l'IA avant update. */
        void setObstacles(const std::vector<std::unique_ptr<Obstacle>>& obs) {
            currentObstacles = &obs;
        }

        /** @brief Affiche Thomas sous forme de carré blanc (clignotant si invincible). */
        void draw(
            gfx::Window& window, 
            float brightness = 1.0f, 
            int offsetX = 0, 
            int offsetY = 0
        ) const override;

        /** @brief Applique des dégâts au joueur. */
        void takeDamage(int amount);

        /** @brief Soigne le joueur. */
        void heal(int amount);

        /** @brief Active l'invincibilité temporaire. */
        void triggerInvincibility(float durationSeconds);

        /** @brief Active le boost Kebab (Vitesse + Taille + Invincibilité). */
        void triggerKebabBoost(float durationSeconds);

        /** @brief Inverse les contrôles du joueur. */
        void applyConfusion(float durationSeconds);

        /** @brief Active la destruction de tous les obstacles à l'écran. */
        void triggerClearScreen();

        /** 
         * @brief Vérifie et consomme la requête de nettoyage d'écran.
         * @return Vrai si le nettoyage a été demandé.
         */
        bool popClearScreen();

        /** @brief Vérifie et consomme le drapeau de déclenchement d'effet Kebab. */
        bool popKebabEffectTriggered();

        /** @return Points de vie actuels. */
        int getHealth() const;

        /** @return Vrai si le joueur est dans son état d'invincibilité post-choc. */
        bool isInvincible() const;

        /** @return Vrai si la santé est tombée à zéro. */
        bool isDead() const;
};
