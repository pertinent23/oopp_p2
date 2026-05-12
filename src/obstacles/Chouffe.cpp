#include "Chouffe.hpp"
#include "Player.hpp"
#include "Window.hpp"
#include "Settings.hpp"


Chouffe::Chouffe(
    float x, 
    float y, 
    float speedX
) : Obstacle(x, y, Settings::CHOUFFE_WIDTH, Settings::CHOUFFE_HEIGHT, speedX) 
{}


void Chouffe::draw(
    gfx::Window& window, 
    float brightness, 
    int offsetX, 
    int offsetY
) const 
{
    // Couleurs ambrées pour la bière et blanc pour la mousse
    int r = static_cast<int>(210 * brightness);
    int g = static_cast<int>(105 * brightness);
    int b = static_cast<int>(30 * brightness); 

    // Dessin du liquide (Rectangle ambré)
    // On dessine le bas du verre rempli de bière. 
    // On décale le haut de 5 pixels pour laisser de la place à la mousse.
    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY + 5, 
        static_cast<int>(size.x), 
        static_cast<int>(size.y) - 5, 
        gfx::Color(r, g, b)
    );
    
    // Dessin de la mousse (Petit rectangle blanc sur le dessus)
    // Cela permet d'identifier immédiatement l'objet comme une boisson.
    int w = static_cast<int>(255 * brightness);
    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY, 
        static_cast<int>(size.x), 
        5, 
        gfx::Color(w, w, w)
    );

    // Dessin des traînées de vitesse
    // Traînées courtes et ambrées pour un effet visuel cohérent.
    int trailX = static_cast<int>(position.x + size.x + 5) + offsetX;
    int trailY = static_cast<int>(position.y) + offsetY;
    
    for (int i = 0; i < 3; ++i) 
    {
        window.fillRect(
            trailX, 
            trailY + 5 + (i * 8), 
            15, 1, 
            gfx::Color(r / 2, g / 2, 0)
        );
    }
}


void Chouffe::onCollision(Player& player) 
{
    player.triggerClearScreen(); 
    deactivate();
}
