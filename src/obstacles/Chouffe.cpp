#include "Chouffe.hpp"
#include "Player.hpp"
#include "Window.hpp"


Chouffe::Chouffe(
    float x, 
    float y, 
    float speedX
) : Obstacle(x, y, 15.0f, 25.0f, speedX) 
{}


void Chouffe::draw(
    gfx::Window& window, 
    float brightness, 
    int offsetX, 
    int offsetY
) const 
{
    int r = static_cast<int>(210 * brightness);
    int g = static_cast<int>(105 * brightness);
    int b = static_cast<int>(30 * brightness); 

    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY + 5, 
        static_cast<int>(size.x), 
        static_cast<int>(size.y) - 5, 
        gfx::Color(r, g, b)
    );
    
    int w = static_cast<int>(255 * brightness);
    
    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY, 
        static_cast<int>(size.x), 
        5, 
        gfx::Color(w, w, w)
    );

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
