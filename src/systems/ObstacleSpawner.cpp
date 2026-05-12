#include "ObstacleSpawner.hpp"
#include "Kebab.hpp"
#include "Garbage.hpp"
#include "Scooter.hpp"
#include "DrunkStudent.hpp"
#include "Chouffe.hpp"
#include "Settings.hpp"
#include "Random.hpp"

#include <algorithm>


ObstacleSpawner::ObstacleSpawner() 
    : spawnTimer(0.0f), currentSpeed(Settings::BASE_OBSTACLE_SPEED) 
{
}


float ObstacleSpawner::getCurrentSpeed() const 
{
    return currentSpeed;
}


std::unique_ptr<Obstacle> ObstacleSpawner::createRandomObstacle() const 
{
    float startX = static_cast<float>(Settings::WINDOW_WIDTH);
    
    float startY = Random::getFloat(
        100.0f, 
        static_cast<float>(Settings::WINDOW_HEIGHT) - 50.0f
    );

    int randVal = Random::getInt(1, 100);

    if (randVal <= Settings::PROB_KEBAB) 
    {
        return std::make_unique<Kebab>(startX, startY, currentSpeed);
    }
    randVal -= Settings::PROB_KEBAB;

    if (randVal <= Settings::PROB_CHOUFFE) 
    {
        return std::make_unique<Chouffe>(startX, startY, currentSpeed);
    }
    randVal -= Settings::PROB_CHOUFFE;

    if (randVal <= Settings::PROB_GARBAGE) 
    {
        return std::make_unique<Garbage>(startX, startY, currentSpeed);
    }
    randVal -= Settings::PROB_GARBAGE;

    if (randVal <= Settings::PROB_SCOOTER) 
    {
        return std::make_unique<Scooter>(startX, startY, currentSpeed);
    }
    
    float studentSize = Random::getFloat(
        Settings::STUDENT_SIZE * 0.5f, 
        Settings::STUDENT_SIZE
    );
    
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
    // Augmentation progressive de la difficulté
    if (currentSpeed < Settings::MAX_OBSTACLE_SPEED) 
    {
        currentSpeed += Settings::DIFFICULTY_GROWTH * deltaTime;
    }

    spawnTimer -= deltaTime;
    
    if (spawnTimer <= 0.0f) 
    {
        obstacles.push_back(createRandomObstacle());
        
        // Plus on va vite, plus ça spawn rapidement !
        float speedRatio = (currentSpeed - Settings::BASE_OBSTACLE_SPEED) / 
                           (Settings::MAX_OBSTACLE_SPEED - Settings::BASE_OBSTACLE_SPEED);
        
        float maxInterval = std::max(
            Settings::MIN_SPAWN_INTERVAL, 
            Settings::BASE_SPAWN_INTERVAL_MAX - (speedRatio * 1.5f)
        );
        
        spawnTimer = Random::getFloat(maxInterval * 0.5f, maxInterval);
    }
}
