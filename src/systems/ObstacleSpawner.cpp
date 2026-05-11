#include "ObstacleSpawner.hpp"
#include "Kebab.hpp"
#include "Garbage.hpp"
#include "Scooter.hpp"
#include "DrunkStudent.hpp"
#include "Chouffe.hpp"
#include "Constants.hpp"
#include "Random.hpp"

#include <algorithm>


ObstacleSpawner::ObstacleSpawner() 
    : spawnTimer(0.0f), currentSpeed(Constants::BASE_OBSTACLE_SPEED) 
{
}


float ObstacleSpawner::getCurrentSpeed() const 
{
    return currentSpeed;
}


std::unique_ptr<Obstacle> ObstacleSpawner::createRandomObstacle() const 
{
    float startX = static_cast<float>(Constants::WINDOW_WIDTH);
    
    float startY = Random::getFloat(
        100.0f, 
        static_cast<float>(Constants::WINDOW_HEIGHT) - 50.0f
    );

    int randVal = Random::getInt(1, 100);

    if (randVal <= Constants::PROB_KEBAB) 
    {
        return std::make_unique<Kebab>(startX, startY, currentSpeed);
    }
    randVal -= Constants::PROB_KEBAB;

    if (randVal <= Constants::PROB_CHOUFFE) 
    {
        return std::make_unique<Chouffe>(startX, startY, currentSpeed);
    }
    randVal -= Constants::PROB_CHOUFFE;

    if (randVal <= Constants::PROB_GARBAGE) 
    {
        return std::make_unique<Garbage>(startX, startY, currentSpeed);
    }
    randVal -= Constants::PROB_GARBAGE;

    if (randVal <= Constants::PROB_SCOOTER) 
    {
        return std::make_unique<Scooter>(startX, startY, currentSpeed);
    }
    
    float studentSize = Random::getFloat(20.0f, 40.0f);
    
    return std::make_unique<DrunkStudent>(
        startX, 
        startY, 
        studentSize, 
        currentSpeed
    );
}


void ObstacleSpawner::update(
    float deltaTime, 
    std::vector<std::unique_ptr<Obstacle>>& obstacles
) 
{
    // Augmentation progressive de la difficulté (+15 pixels/s chaque seconde)
    if (currentSpeed < Constants::MAX_OBSTACLE_SPEED) 
    {
        currentSpeed += 15.0f * deltaTime;
    }

    spawnTimer -= deltaTime;
    
    if (spawnTimer <= 0.0f) 
    {
        obstacles.push_back(createRandomObstacle());
        
        // Plus on va vite, plus ça spawn rapidement !
        float speedRatio = (currentSpeed - Constants::BASE_OBSTACLE_SPEED) / 
                           (Constants::MAX_OBSTACLE_SPEED - Constants::BASE_OBSTACLE_SPEED);
        
        float maxInterval = std::max(
            0.3f, 
            Constants::BASE_SPAWN_INTERVAL_MAX - (speedRatio * 1.5f)
        );
        
        spawnTimer = Random::getFloat(maxInterval * 0.5f, maxInterval);
    }
}
