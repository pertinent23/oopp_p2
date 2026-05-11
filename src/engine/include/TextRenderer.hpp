#pragma once

#include <string>


namespace gfx 
{ 
    class Window; 
    class Color; 
}


/**
 * @class TextRenderer
 * @brief Moteur de rendu de texte pixelisé (Police 3x5).
 * 
 * Permet d'afficher du texte sans dépendances externes (comme SDL_ttf) en
 * dessinant chaque caractère pixel par pixel via un dictionnaire interne.
 */
class TextRenderer 
{
    public:
        /**
         * @brief Affiche une chaîne de caractères à l'écran.
         * @param win Fenêtre de rendu.
         * @param text Texte à afficher (supporte A-Z, 0-9 et symboles).
         * @param x Position horizontale.
         * @param y Position verticale.
         * @param scale Taille des "pixels" de la police.
         * @param color Couleur du texte.
         */
        static void drawText(
            gfx::Window& win, 
            const std::string& text, 
            int x, int y, 
            int scale, 
            const gfx::Color& color
        );
};
