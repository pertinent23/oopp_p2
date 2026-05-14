#pragma once

#include "Player.hpp"

/**
 * @class SmartPlayer
 * @brief Une intelligence artificielle basique qui joue à Carré Surfer.
 * 
 * Hérite de Player mais surcharge handleMovement pour prendre des 
 * décisions basées sur la position des obstacles au lieu du clavier.
 */
class SmartPlayer : public Player 
{
    private:
        float targetY;
        float targetX;

        /** 
         * @brief Analyse l'environnement immédiat.
         * Identifie le danger le plus proche sur la trajectoire Y, 
         * ainsi que le bonus le plus proche disponible.
         */
        void evaluateEnvironment(
            Obstacle*& outDanger, 
            float& outDangerDist, 
            Obstacle*& outKebab,
            Obstacle*& outChouffe
        ) const;

        /** 
         * @brief Heuristique de prise de décision.
         * Détermine l'ordonnée Y vers laquelle Thomas doit se diriger.
         */
        float determineTargetY(
            Obstacle* danger, 
            float dangerDist, 
            Obstacle* kebab,
            Obstacle* chouffe,
            float currentSpeed
        ) const;

        /** 
         * @brief Détermine l'abscisse X vers laquelle Thomas doit se diriger.
         * Avance s'il y a un bonus, recule s'il y a un danger.
         */
        float determineTargetX(
            Obstacle* danger, 
            float dangerDist, 
            Obstacle* kebab,
            Obstacle* chouffe
        ) const;

        /** 
         * @brief Calcule les vecteurs de vélocité pour atteindre la cible en douceur.
         */
        void applyVelocities(float targetXPos, float targetYPos, float currentSpeed);

        /** @brief Surpasse la logique de mouvement pour utiliser l'IA. */
        void handleMovement(float deltaTime) override;

    public:
        /** @brief Constructeur. */
        explicit SmartPlayer(InputManager& inputManager);
};
