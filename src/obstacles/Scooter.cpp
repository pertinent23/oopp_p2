#include "Scooter.hpp"
#include "Player.hpp"
#include "Window.hpp"
#include "Settings.hpp"


Scooter::Scooter(
    float x, 
    float y, 
    float speedX
) : Obstacle(x, y, Settings::SCOOTER_WIDTH, Settings::SCOOTER_HEIGHT, speedX) 
{}


void Scooter::draw(
    gfx::Window& window, 
    float brightness, 
    int offsetX, 
    int offsetY
) const 
{
    // Vert pour le scooter (évoquant les flottes de scooters électriques en ville)
    int g  = static_cast<int>(200 * brightness);
    int tg = static_cast<int>(50 * brightness); 
    
    // Dessin du corps (Rectangle fin et vertical)
    // Le scooter est étroit pour être difficile à éviter par son mouvement rapide.
    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY, 
        static_cast<int>(size.x), 
        static_cast<int>(size.y), 
        gfx::Color(0, g, 0)
    );

    // Dessin des traînées de vitesse
    // On dessine de longues traînées vertes pour souligner la finesse de l'objet.
    int trailX = static_cast<int>(position.x + size.x + 5) + offsetX;
    int trailY = static_cast<int>(position.y) + offsetY;
    
    for (int i = 0; i < 3; ++i) 
    {
        window.fillRect(
            trailX, 
            trailY + 5 + (i * 15), 
            30, 2, 
            gfx::Color(0, tg, 0)
        );
    }
}


void Scooter::onCollision(Player& player) 
{
    player.takeDamage(Settings::SCOOTER_DAMAGE);
    player.triggerInvincibility(Settings::INVINCIBILITY_DURATION);
}
