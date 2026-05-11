#pragma once
#include "Obstacle.hpp"

class DrunkStudent : public Obstacle {
    public:
        DrunkStudent(float x, float y, float size, float speedX);
    
        void draw(
            gfx::Window& window,
            float brightness = 1.0f,
            int offsetX = 0,
            int offsetY = 0
        ) const override;
        
        void onCollision(Player& player) override;
};
