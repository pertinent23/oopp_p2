#include "DrunkStudent.hpp"
#include "Player.hpp"
#include "Window.hpp"
#include "Random.hpp"
#include "Settings.hpp"


DrunkStudent::DrunkStudent(
    float x, 
    float y, 
    float s, 
    float speedX
) : Obstacle(x, y, s, s, speedX) 
{}


void DrunkStudent::draw(
    gfx::Window& window, 
    float brightness, 
    int offsetX, 
    int offsetY
) const 
{
    // Calcul de la couleur violette (p) et de la traînée (tp) en fonction de la santé
    int p  = static_cast<int>(180 * brightness);
    int tp = static_cast<int>(40 * brightness); 
    
    // Dessin du corps principal de l'étudiant
    // On utilise un carré violet pour le différencier des autres obstacles.
    // L'offsetX/Y permet d'appliquer l'effet de tremblement (shake) lors des collisions.
    window.fillRect(
        static_cast<int>(position.x) + offsetX, 
        static_cast<int>(position.y) + offsetY, 
        static_cast<int>(size.x), 
        static_cast<int>(size.y), 
        gfx::Color(p, p / 2, p)
    );

    // Dessin de l'effet de vitesse (Trails)
    // On dessine 3 lignes horizontales derrière l'objet pour simuler le mouvement rapide.
    int trailX = static_cast<int>(position.x + size.x + 5) + offsetX;
    int trailY = static_cast<int>(position.y) + offsetY;
    
    for (int i = 0; i < 3; ++i) 
    {
        // On espace les lignes verticalement pour couvrir toute la hauteur du carré
        window.fillRect(
            trailX, 
            trailY + 2 + (i * (size.y / 3)), 
            20, 1, 
            gfx::Color(tp, 0, tp)
        );
    }
}


void DrunkStudent::onCollision(Player& player) 
{
    player.takeDamage(Settings::STUDENT_DAMAGE);
    player.triggerInvincibility(Settings::INVINCIBILITY_DURATION);
    player.applyConfusion(Random::getFloat(
        Settings::CONFUSION_DURATION_MIN, 
        Settings::CONFUSION_DURATION_MAX
    ));
}
