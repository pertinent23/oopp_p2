#include "DrunkStudent.hpp"
#include "Player.hpp"
#include "Window.hpp"
#include "Random.hpp"


DrunkStudent::DrunkStudent(
    float x, 
    float y, 
    float s, 
    float speedX
) : Obstacle(x, y, s, s, speedX) 
{}


void DrunkStudent::draw(
    gfx::Window& window, 
    float brightness, 
    int offsetX, 
    int offsetY
) const 
{
    int p  = static_cast<int>(180 * brightness);
    int tp = static_cast<int>(40 * brightness); 
    
    // Corps : Carré violet
    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY, 
        static_cast<int>(size.x), 
        static_cast<int>(size.y), 
        gfx::Color(p, p / 2, p)
    );

    // Trails : 3 lignes horizontales
    int trailX = static_cast<int>(position.x + size.x + 5) + offsetX;
    int trailY = static_cast<int>(position.y) + offsetY;
    
    for (int i = 0; i < 3; ++i) 
    {
        window.fillRect(
            trailX, 
            trailY + 2 + (i * (size.y / 3)), 
            20, 1, 
            gfx::Color(tp, 0, tp)
        );
    }
}


void DrunkStudent::onCollision(Player& player) 
{
    player.takeDamage(20);
    player.triggerInvincibility(2.0f);
    player.applyConfusion(Random::getFloat(1.0f, 5.0f));
}
