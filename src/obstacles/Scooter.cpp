#include "Scooter.hpp"
#include "Player.hpp"
#include "Window.hpp"


Scooter::Scooter(
    float x, 
    float y, 
    float speedX
) : Obstacle(x, y, 15.0f, 45.0f, speedX) 
{}


void Scooter::draw(
    gfx::Window& window, 
    float brightness, 
    int offsetX, 
    int offsetY
) const 
{
    int g  = static_cast<int>(200 * brightness);
    int tg = static_cast<int>(50 * brightness); 
    
    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY, 
        static_cast<int>(size.x), 
        static_cast<int>(size.y), 
        gfx::Color(0, g, 0)
    );

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
    player.takeDamage(30);
    player.triggerInvincibility(2.0f);
}
