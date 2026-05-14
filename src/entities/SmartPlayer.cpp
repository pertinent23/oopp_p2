#include "SmartPlayer.hpp"
#include "Kebab.hpp"
#include "Chouffe.hpp"
#include <iostream>


/**
 * @brief Constructeur de l'Intelligence Artificielle.
 * Initialise le joueur virtuel au centre de l'écran et affiche un message
 * dans la console pour confirmer l'activation du mode démo.
 * 
 * @param inputManager Référence au gestionnaire d'entrées (ignoré par l'IA).
 */
SmartPlayer::SmartPlayer(
    InputManager& inputManager
) : Player(inputManager), 
    targetY(Settings::WINDOW_HEIGHT / 2.0f),
    targetX(Settings::PLAYER_START_X)
{
    std::cout << "[SmartPlayer] Mode Demo IA active !" << std::endl;
}


/**
 * @brief Algorithme principal appelé à chaque frame.
 * Coordonne les sous-méthodes heuristiques pour déterminer le comportement.
 */
void SmartPlayer::handleMovement(float deltaTime)
{
    velocity.x = 0.0f;
    velocity.y = 0.0f;

    float speed = Settings::PLAYER_SPEED;
    if (kebabEffectTimer > 0.0f) 
    {
        speed *= Settings::KEBAB_SPEED_MULTIPLIER;
    }

    if (currentObstacles)
    {
        Obstacle* dangerousObstacle = nullptr;
        Obstacle* nearestKebab = nullptr;
        Obstacle* nearestChouffe = nullptr;
        float nearestDangerDist = 99999.0f;

        // Analyse de l'environnement (Lookahead amélioré)
        evaluateEnvironment(dangerousObstacle, nearestDangerDist, nearestKebab, nearestChouffe);

        // Prise de décision heuristique avec priorités et ANTICIPATION
        targetY = determineTargetY(dangerousObstacle, nearestDangerDist, nearestKebab, nearestChouffe, speed);
        targetX = determineTargetX(dangerousObstacle, nearestDangerDist, nearestKebab, nearestChouffe);

        // Lissage du mouvement
        applyVelocities(targetX, targetY, speed);
    }

    // Application finale de la vélocité sur la position (Euler Integration)
    position += velocity * deltaTime;
}


/**
 * @brief Analyse l'environnement pour repérer les dangers et les bonus.
 * L'IA scanne les objets situés uniquement devant elle.
 */
void SmartPlayer::evaluateEnvironment(
    Obstacle*& outDanger, 
    float& outDangerDist, 
    Obstacle*& outKebab,
    Obstacle*& outChouffe
) const
{
    outDanger = nullptr;
    outKebab = nullptr;
    outChouffe = nullptr;
    outDangerDist = 99999.0f;
    
    float minKebabDist = 99999.0f;
    float minChouffeDist = 99999.0f;

    for (const auto& obsPtr : *currentObstacles)
    {
        Obstacle* obs = obsPtr.get();
        if (!obs->isActive()) continue;
        
        float obsX = obs->getPosition().x;
        float obsW = obs->getSize().x;
        float playerRight = position.x + size.x;

        // Si l'obstacle est devant nous
        if (obsX + obsW > position.x)
        {
            float dist = obsX - playerRight;
            
            // Identification précise du type d'objet
            Kebab* kebab = dynamic_cast<Kebab*>(obs);
            Chouffe* chouffe = dynamic_cast<Chouffe*>(obs);

            if (kebab)
            {
                if (dist > 0 && dist < 1000.0f && dist < minKebabDist)
                {
                    minKebabDist = dist;
                    outKebab = obs;
                }
            }
            else if (chouffe)
            {
                if (dist > 0 && dist < 1000.0f && dist < minChouffeDist)
                {
                    minChouffeDist = dist;
                    outChouffe = obs;
                }
            }
            else // C'est un obstacle nuisible
            {
                // ANTICIPATION : vision élargie
                float obsTop = obs->getPosition().y;
                float obsBottom = obsTop + obs->getSize().y;
                float visionCorridorTop = position.y - 120.0f;
                float visionCorridorBottom = position.y + size.y + 120.0f;
                bool inVisionCorridor = (visionCorridorTop < obsBottom) && (visionCorridorBottom > obsTop);
                
                if (inVisionCorridor && dist > -50.0f && dist < outDangerDist)
                {
                    outDangerDist = dist;
                    outDanger = obs;
                }
            }
        }
    }
}


/**
 * @brief Détermine la meilleure position Y vers laquelle se diriger.
 */
float SmartPlayer::determineTargetY(
    Obstacle* danger, 
    float dangerDist, 
    Obstacle* kebab,
    Obstacle* chouffe,
    float currentSpeed
) const
{
    float newTargetY = position.y;

    // --- PRIORITÉ 1 : PROTECTION (Avec calcul de faisabilité pour les bonus) ---
    if (danger && dangerDist < 850.0f)
    {
        float obsCenterY = danger->getPosition().y + danger->getSize().y / 2.0f;
        float playerCenterY = position.y + size.y / 2.0f;

        // ANALYSE : Est-ce qu'on peut se permettre de prendre un bonus AVANT de fuir ?
        bool canGrabBonusFirst = false;
        Obstacle* bestBonus = kebab ? kebab : chouffe;

        if (bestBonus)
        {
            // Calcul du temps restant avant l'impact et avant le bonus
            float distToBonus = bestBonus->getPosition().x - (position.x + size.x);
            float obsSpeed = std::abs(danger->getVelocity().x);
            if (obsSpeed < 10.0f)
            {
                obsSpeed = Settings::BASE_OBSTACLE_SPEED;
            }

            float timeToBonus = distToBonus / obsSpeed;
            float timeToDanger = dangerDist / obsSpeed;

            // Calcul du temps nécessaire pour s'enfuir APRÈS avoir ramassé le bonus
            float bonusCenterY = bestBonus->getPosition().y + bestBonus->getSize().y / 2.0f;
            
            // On veut atteindre une position Y sécurisée (marge de 40px par rapport à l'obstacle)
            float safeY = (playerCenterY < obsCenterY) ? (danger->getPosition().y - size.y - 40.0f) 
                                                       : (danger->getPosition().y + danger->getSize().y + 40.0f);
            
            float distToSafeY = std::abs(bonusCenterY - safeY);
            float timeToEscape = distToSafeY / currentSpeed;

            // OPTION 2 : On ne prend le bonus que si on a le temps de s'échapper ENSUITE
            // On ajoute une petite marge de sécurité de 0.1s
            if (timeToBonus + timeToEscape + 0.1f < timeToDanger)
            {
                canGrabBonusFirst = true;
            }
        }

        if (canGrabBonusFirst)
        {
            // On fonce vers le bonus !
            newTargetY = bestBonus->getPosition().y + bestBonus->getSize().y / 2.0f - size.y / 2.0f;
        }
        else
        {
            // Esquive classique par réflexe
            if (playerCenterY < obsCenterY)
            {
                newTargetY = position.y - (currentSpeed * 0.4f);
            }
            else
            {
                newTargetY = position.y + (currentSpeed * 0.4f);
            }
        }
    }
    // PRIORITÉ 2 : KEBAB (Soin hors danger)
    else if (kebab)
    {
        float bonusCenterY = kebab->getPosition().y + kebab->getSize().y / 2.0f;
        newTargetY = bonusCenterY - size.y / 2.0f;
    }
    // PRIORITÉ 3 : CHOUFFE (Score hors danger)
    else if (chouffe)
    {
        float bonusCenterY = chouffe->getPosition().y + chouffe->getSize().y / 2.0f;
        newTargetY = bonusCenterY - size.y / 2.0f;
    }

    return newTargetY;
}


/**
 * @brief Détermine la meilleure position X vers laquelle se diriger.
 */
float SmartPlayer::determineTargetX(
    Obstacle* danger, 
    float dangerDist, 
    Obstacle* kebab,
    Obstacle* chouffe
) const
{
    float minX = Settings::PLAYER_START_X;
    float maxX = Settings::WINDOW_WIDTH / 2.0f;
    
    // Si danger proche, on reste à gauche quoi qu'il arrive
    if (danger && dangerDist < 850.0f) return minX;

    // Choix du bonus prioritaire
    Obstacle* priorityBonus = kebab ? kebab : chouffe;

    if (priorityBonus)
    {
        float bonusCenterY = priorityBonus->getPosition().y + priorityBonus->getSize().y / 2.0f;
        float playerCenterY = position.y + size.y / 2.0f;
        
        // On n'avance que si on est déjà aligné verticalement avec le bonus choisi
        if (std::abs(playerCenterY - bonusCenterY) < 30.0f)
        {
            return maxX;
        }
    }

    return minX;
}



/**
 * @brief Convertit les cibles de position en vélocité pour un mouvement fluide.
 */
void SmartPlayer::applyVelocities(float targetXPos, float targetYPos, float currentSpeed)
{
    // Calcul des distances par rapport aux cibles
    float distY = targetYPos - position.y;
    float distX = targetXPos - position.x;

    // "Zone morte" (Deadzone) : On arrête de bouger si on est assez proche
    // Cela évite les micro-oscillations permanentes.
    // "Zone morte" (Deadzone) : Réduite à 4 pixels pour plus de précision
    float deadZone = 4.0f;

    // --- Mouvement Vertical avec amortissement ---
    if (std::abs(distY) > deadZone) 
    {
        // Plus on est proche de la cible, plus on ralentit (Amortissement linéaire)
        // On utilise une zone d'amortissement de 100 pixels.
        float damping = std::min(1.0f, std::abs(distY) / 100.0f);
        velocity.y = (distY > 0 ? currentSpeed : -currentSpeed) * damping;
    }

    // --- Mouvement Horizontal avec amortissement ---
    if (std::abs(distX) > deadZone) 
    {
        float damping = std::min(1.0f, std::abs(distX) / 100.0f);
        velocity.x = (distX > 0 ? currentSpeed : -currentSpeed) * damping;
    }
}
