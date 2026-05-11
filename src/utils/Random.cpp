#include "Random.hpp"
#include <random>

namespace {
    // Moteur statique interne au fichier (concept de translation unit locale)
    std::mt19937 engine;
}

/**
 * @brief Initialise le générateur avec une graine basée sur l'horloge système.
 * Permet d'obtenir des séquences de nombres différentes à chaque lancement du programme.
 */
void Random::init() {
    std::random_device rd;
    engine.seed(rd());
}

/**
 * @brief Génère un entier pseudo-aléatoire dans l'intervalle [min, max] inclus.
 * @param min Borne inférieure inclusive.
 * @param max Borne supérieure inclusive.
 * @return Entier aléatoire.
 */
int Random::getInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(engine);
}

/**
 * @brief Génère un nombre flottant pseudo-aléatoire dans l'intervalle [min, max).
 * Note : La borne supérieure est exclue (comportement standard des distributions réelles).
 * @param min Borne inférieure inclusive.
 * @param max Borne supérieure exclusive.
 * @return Nombre flottant aléatoire.
 */
float Random::getFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(engine);
}
