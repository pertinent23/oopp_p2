#include "Player.hpp"
#include "Window.hpp"
#include "Random.hpp"

#include <algorithm>


Player::Player(InputManager& inputManager)
    : Entity(
        Constants::PLAYER_START_X, 
        Constants::WINDOW_HEIGHT / 2.0f, 
        Constants::PLAYER_SIZE, 
        Constants::PLAYER_SIZE
    ),
    input(inputManager),
    health(Constants::PLAYER_MAX_HEALTH),
    staggerTimer(Constants::STAGGER_INTERVAL),
    invincibilityTimer(0.0f),
    clearScreenRequested(false)
{}


void Player::update(float deltaTime)
{
    if (isDead()) 
    {
        return;
    }

    updateTimers(deltaTime);
    handleMovement(deltaTime);
    constrainToWindow();
}


void Player::updateTimers(float deltaTime)
{
    if (invincibilityTimer > 0.0f)
    {
        invincibilityTimer -= deltaTime;
    }

    staggerTimer -= deltaTime;
    
    if (staggerTimer <= 0.0f)
    {
        applyStagger();
        
        staggerTimer = Random::getFloat(
            Constants::STAGGER_INTERVAL * 0.5f, 
            Constants::STAGGER_INTERVAL * 1.5f
        );
    }
}


void Player::handleMovement(float deltaTime)
{
    velocity.x = 0.0f;
    velocity.y = 0.0f;

    if (input.isUpPressed())    velocity.y -= Constants::PLAYER_SPEED;
    if (input.isDownPressed())  velocity.y += Constants::PLAYER_SPEED;
    if (input.isLeftPressed())  velocity.x -= Constants::PLAYER_SPEED;
    if (input.isRightPressed()) velocity.x += Constants::PLAYER_SPEED;

    position += velocity * deltaTime;
}


void Player::applyStagger()
{
    int   staggerSteps = Random::getInt(1, 5);
    float staggerDist  = staggerSteps * 20.0f; 
    
    if (Random::getInt(0, 1) == 0)
    {
        position.y -= staggerDist;
    }
    else
    {
        position.y += staggerDist;
    }
}


void Player::constrainToWindow()
{
    position.x = std::max(0.0f, std::min(
        position.x, 
        static_cast<float>(Constants::WINDOW_WIDTH) - size.x
    ));

    position.y = std::max(60.0f, std::min(
        position.y, 
        static_cast<float>(Constants::WINDOW_HEIGHT) - size.y
    ));
}


void Player::draw(
    gfx::Window& window, 
    float brightness, 
    int offsetX, 
    int offsetY
) const
{
    // Vérification de l'état
    // Si Thomas est mort, on ne dessine rien.
    if (isDead()) 
    {
        return;
    }

    // Gestion du clignotement d'invincibilité
    // Si Thomas vient de prendre un choc, il clignote pour indiquer 
    // au joueur qu'il est temporairement protégé des nouveaux dégâts.
    if (isInvincible())
    {
        // On alterne le dessin une frame sur deux via le timer
        int blinkState = static_cast<int>(invincibilityTimer * 15) % 2; 
        if (blinkState == 0) 
        {
            return;
        }
    }

    // Dessin du corps (Carré blanc)
    // Thomas est représenté par un carré blanc pur qui s'assombrit
    // en même temps que le reste du jeu si sa santé diminue.
    int c = static_cast<int>(255 * brightness);
    
    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY, 
        static_cast<int>(size.x), 
        static_cast<int>(size.y), 
        gfx::Color(c, c, c)
    );
}


void Player::takeDamage(int amount)
{
    if (isInvincible()) 
    {
        return;
    }
    
    health = std::max(0, health - amount);
}


void Player::heal(int amount)
{
    health = std::min(Constants::PLAYER_MAX_HEALTH, health + amount);
}


void Player::triggerInvincibility(float durationSeconds)
{
    invincibilityTimer = durationSeconds;
}


void Player::applyConfusion(float durationSeconds)
{
    input.reverseControls(durationSeconds);
}


void Player::triggerClearScreen()
{
    clearScreenRequested = true;
}


bool Player::popClearScreen()
{
    bool temp = clearScreenRequested;
    clearScreenRequested = false;
    return temp;
}


int  Player::getHealth() const {
    return health;
}

bool Player::isInvincible() const {
    return invincibilityTimer > 0.0f;
}

bool Player::isDead() const {
    return health <= 0;
}
