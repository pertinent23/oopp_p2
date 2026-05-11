#include "InputManager.hpp"
#include "Window.hpp"

#include <algorithm>


InputManager::InputManager(
    gfx::Window& win
) : window(win), controlsReversed(false), reverseTimer(0.0f) {}


void InputManager::update(float deltaTime) 
{
    if (controlsReversed) 
    {
        reverseTimer -= deltaTime;
        
        if (reverseTimer <= 0.0f) 
        {
            controlsReversed = false;
            reverseTimer     = 0.0f;
        }
    }
}


void InputManager::reverseControls(float durationSeconds) 
{
    controlsReversed = true;
    reverseTimer     = durationSeconds;
}


bool InputManager::isUpPressed() const 
{
    auto key = controlsReversed ? gfx::Key::Down : gfx::Key::Up;
    return window.isKeyDown(key);
}


bool InputManager::isDownPressed() const 
{
    auto key = controlsReversed ? gfx::Key::Up : gfx::Key::Down;
    return window.isKeyDown(key);
}


bool InputManager::isLeftPressed() const 
{
    auto key = controlsReversed ? gfx::Key::Right : gfx::Key::Left;
    return window.isKeyDown(key);
}


bool InputManager::isRightPressed() const 
{
    auto key = controlsReversed ? gfx::Key::Left : gfx::Key::Right;
    return window.isKeyDown(key);
}


bool InputManager::isSpacePressed() const 
{
    return window.isKeyDown(gfx::Key::Space);
}


bool InputManager::isEscPressed() const 
{
    return window.isKeyDown(gfx::Key::Escape);
}


bool InputManager::wasSpacePressed() const 
{
    return window.wasKeyPressed(gfx::Key::Space);
}


bool InputManager::wasEscPressed() const 
{
    return window.wasKeyPressed(gfx::Key::Escape);
}


bool InputManager::wasUpPressed() const 
{
    return window.wasKeyPressed(gfx::Key::Up);
}


bool InputManager::wasDownPressed() const 
{
    return window.wasKeyPressed(gfx::Key::Down);
}


bool InputManager::wasLeftPressed() const 
{
    return window.wasKeyPressed(gfx::Key::Left);
}


bool InputManager::wasRightPressed() const 
{
    return window.wasKeyPressed(gfx::Key::Right);
}


bool InputManager::wasEnterPressed() const 
{
    return window.wasKeyPressed(gfx::Key::Enter);
}
