#pragma once

#include <vector>
#include "Vector2D.hpp"
#include "Constants.hpp"
#include "Window.hpp"


class BackgroundManager 
{
    private:
        struct Line 
        {
            Vector2D pos;
            float    width;
            float    height;
        };

        std::vector<Line> lines;

    public:
        BackgroundManager() 
        {
            // Lignes de trottoir fines et sombres
            for (int i = 0; i < 20; i++) 
            {
                float y1 = static_cast<float>(
                    100 + (i * 55) % (Constants::WINDOW_HEIGHT - 150)
                );

                float y2 = static_cast<float>(
                    130 + (i * 73) % (Constants::WINDOW_HEIGHT - 150)
                );

                lines.push_back({
                    {static_cast<float>(i * 120), y1}, 50.0f, 3.0f
                });

                lines.push_back({
                    {static_cast<float>(i * 120 + 60), y2}, 40.0f, 2.0f
                });
            }
        }

        void update(
            float deltaTime, 
            float currentSpeed
        ) 
        {
            float speed = currentSpeed * 0.3f; // Défile lentement en arrière-plan

            for (auto& line : lines) 
            {
                line.pos.x -= speed * deltaTime;

                if (line.pos.x + line.width < 0) 
                {
                    line.pos.x += Constants::WINDOW_WIDTH + 200; 
                }
            }
        }

        void draw(
            gfx::Window& window, 
            float brightness
        ) const 
        {
            int c = static_cast<int>(40 * brightness); // Effet de traînée très sombre

            for (const auto& line : lines) 
            {
                window.fillRect(
                    static_cast<int>(line.pos.x), 
                    static_cast<int>(line.pos.y), 
                    static_cast<int>(line.width), 
                    static_cast<int>(line.height), 
                    gfx::Color(c, c, c)
                );
            }
        }
};
