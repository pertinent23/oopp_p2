#pragma once

namespace gfx 
{ 
    class Window; 
}


/**
 * @class InputManager
 * @brief Gère les entrées clavier et les effets d'état sur les contrôles.
 * 
 * Cette classe permet de vérifier l'état des touches (enfoncée ou juste pressée)
 * et supporte l'inversion des contrôles (effet de la confusion).
 */
class InputManager 
{
    private:
        gfx::Window& window;           ///< Référence vers la fenêtre pour interroger le clavier
        bool         controlsReversed; ///< Vrai si l'effet de confusion est actif
        float        reverseTimer;     ///< Temps restant pour l'inversion des contrôles

    public:
        /**
         * @brief Constructeur.
         * @param win Fenêtre de rendu.
         */
        explicit InputManager(gfx::Window& win);

        /** @brief Met à jour les compteurs de temps internes. */
        void update(float deltaTime);

        /** 
         * @brief Active l'inversion des touches directionnelles.
         * @param durationSeconds Durée de l'effet.
         */
        void reverseControls(float durationSeconds);

        // --- Requêtes continues (Mouvement) ---

        bool isUpPressed() const;
        bool isDownPressed() const;
        bool isLeftPressed() const;
        bool isRightPressed() const;

        bool isSpacePressed() const;
        bool isEscPressed() const;

        // --- Requêtes ponctuelles (Menus / Actions uniques) ---

        bool wasSpacePressed() const;
        bool wasEscPressed() const;
        bool wasUpPressed() const;
        bool wasDownPressed() const;
        bool wasLeftPressed() const;
        bool wasRightPressed() const;
        bool wasEnterPressed() const;
};
