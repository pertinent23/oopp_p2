#pragma once
#include "Obstacle.hpp"

class Kebab : public Obstacle {
    public:
        Kebab(float x, float y, float speedX);

        void draw(
            gfx::Window& window, 
            float brightness = 1.0f,
            int offsetX = 0,
            int offsetY = 0
        ) const override;

        void onCollision(Player& player) override;
};
