#include "Kebab.hpp"
#include "Player.hpp"
#include "Window.hpp"


Kebab::Kebab(
    float x, 
    float y, 
    float speedX
) : Obstacle(x, y, 20.0f, 20.0f, speedX) 
{}


void Kebab::draw(
    gfx::Window& window, 
    float brightness, 
    int offsetX, 
    int offsetY
) const 
{
    // Couleurs orangées pour le Kebab
    int r  = static_cast<int>(255 * brightness);
    int g  = static_cast<int>(165 * brightness);
    int to = static_cast<int>(80 * brightness); 

    // Dessin du corps (Cercle orange)
    // Contrairement aux obstacles néfastes, le Kebab est rond et orange 
    // pour indiquer visuellement qu'il est un bonus (soin).
    window.fillCircle(
        static_cast<int>(position.x + size.x / 2) + offsetX,
        static_cast<int>(position.y + size.y / 2) + offsetY, 
        static_cast<int>(size.x / 2), 
        gfx::Color(r, g, 0)
    );

    // Dessin de la traînée de mouvement
    // On dessine 3 petites lignes orange foncé pour renforcer l'effet de défilement.
    int trailX = static_cast<int>(position.x + size.x + 5) + offsetX;
    int trailY = static_cast<int>(position.y) + offsetY;
    
    for (int i = 0; i < 3; ++i) 
    {
        window.fillRect(
            trailX, 
            trailY + 2 + (i * 7), 
            15, 1, 
            gfx::Color(to, to / 2, 0)
        );
    }
}


void Kebab::onCollision(Player& player) 
{
    player.heal(10);
    deactivate();
}
