#include "Garbage.hpp"
#include "Player.hpp"
#include "Window.hpp"


Garbage::Garbage(
    float x, 
    float y, 
    float speedX
) : Obstacle(x, y, 30.0f, 45.0f, speedX) 
{}


void Garbage::draw(
    gfx::Window& window, 
    float brightness, 
    int offsetX, 
    int offsetY
) const 
{
    int c  = static_cast<int>(150 * brightness);
    int tc = static_cast<int>(60 * brightness); 
    
    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY, 
        static_cast<int>(size.x), 
        static_cast<int>(size.y), 
        gfx::Color(c, c, c)
    );

    int trailX = static_cast<int>(position.x + size.x + 5) + offsetX;
    int trailY = static_cast<int>(position.y) + offsetY;
    
    for (int i = 0; i < 3; ++i) 
    {
        window.fillRect(
            trailX, 
            trailY + 5 + (i * 15), 
            30, 2, 
            gfx::Color(tc, tc, tc)
        );
    }
}


void Garbage::onCollision(Player& player) 
{
    player.takeDamage(10);
    player.triggerInvincibility(2.0f);
}
