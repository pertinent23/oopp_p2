#include "Player.hpp"
#include "Window.hpp"
#include "Random.hpp"
#include "Settings.hpp"

#include <algorithm>


Player::Player(InputManager& inputManager)
    : Entity(
        Settings::PLAYER_START_X, 
        Settings::WINDOW_HEIGHT / 2.0f, 
        Settings::PLAYER_SIZE, 
        Settings::PLAYER_SIZE
    ),
    input(inputManager),
    health(Settings::PLAYER_MAX_HEALTH),
    staggerTimer(Settings::STAGGER_INTERVAL),
    invincibilityTimer(0.0f),
    kebabEffectTimer(0.0f),
    clearScreenRequested(false),
    kebabEffectTriggered(false)
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

    if (kebabEffectTimer > 0.0f)
    {
        kebabEffectTimer -= deltaTime;
        
        // Si le boost vient de se terminer à cette frame
        if (kebabEffectTimer <= 0.0f)
        {
            // On donne une invincibilité de sécurité (clignotement)
            triggerInvincibility(Settings::KEBAB_SAFETY_DURATION); 
        }
    }

    staggerTimer -= deltaTime;
    
    if (staggerTimer <= 0.0f)
    {
        applyStagger();
        
        staggerTimer = Random::getFloat(
            Settings::STAGGER_INTERVAL * 0.5f, 
            Settings::STAGGER_INTERVAL * 1.5f
        );
    }
}


void Player::handleMovement(float deltaTime)
{
    velocity.x = 0.0f;
    velocity.y = 0.0f;

    float speed = Settings::PLAYER_SPEED;
    if (kebabEffectTimer > 0.0f) speed *= Settings::KEBAB_SPEED_MULTIPLIER; 

    if (input.isUpPressed())    velocity.y -= speed;
    if (input.isDownPressed())  velocity.y += speed;
    if (input.isLeftPressed())  velocity.x -= speed;
    if (input.isRightPressed()) velocity.x += speed;

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
        static_cast<float>(Settings::WINDOW_WIDTH) - size.x
    ));

    position.y = std::max(60.0f, std::min(
        position.y, 
        static_cast<float>(Settings::WINDOW_HEIGHT) - size.y
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
    
    // Gestion de la taille (Pulse Kebab)
    // Thomas grossit si l'effet Kebab est actif.
    float drawW = size.x;
    float drawH = size.y;
    float drawX = position.x + offsetX;
    float drawY = position.y + offsetY;

    if (kebabEffectTimer > 0.0f)
    {
        drawW *= Settings::KEBAB_SIZE_MULTIPLIER;
        drawH *= Settings::KEBAB_SIZE_MULTIPLIER;
        // On centre l'agrandissement
        drawX -= (drawW - size.x) / 2.0f;
        drawY -= (drawH - size.y) / 2.0f;
    }

    window.fillRect(
        static_cast<int>(drawX), 
        static_cast<int>(drawY), 
        static_cast<int>(drawW), 
        static_cast<int>(drawH), 
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
    health = std::min(Settings::PLAYER_MAX_HEALTH, health + amount);
}


void Player::triggerInvincibility(float durationSeconds)
{
    invincibilityTimer = durationSeconds;
}


void Player::triggerKebabBoost(float durationSeconds)
{
    kebabEffectTimer     = durationSeconds;
    kebabEffectTriggered = true;
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


bool Player::popKebabEffectTriggered()
{
    bool temp = kebabEffectTriggered;
    kebabEffectTriggered = false;
    return temp;
}


int  Player::getHealth() const {
    return health;
}

bool Player::isInvincible() const {
    return invincibilityTimer > 0.0f || kebabEffectTimer > 0.0f;
}

bool Player::isDead() const {
    return health <= 0;
}
