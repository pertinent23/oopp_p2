#pragma once

/**
 * @file Random.hpp
 * @brief Utilitaire pour la génération de nombres aléatoires
 * 
 * Centralise le moteur aléatoire <random> du C++ moderne pour éviter
 * d'utiliser rand() qui est déprécié et moins performant.
 */
class Random {
    public:
        /**
         * @brief Initialise la graine aléatoire (Seed)
         */
        static void init();

        /**
         * @brief Génère un entier aléatoire compris entre min et max (inclus)
         * @param min Valeur minimale
         * @param max Valeur maximale
         * @return int L'entier généré
         */
        static int getInt(int min, int max);

        /**
         * @brief Génère un nombre flottant aléatoire compris entre min et max
         * @param min Valeur minimale
         * @param max Valeur maximale
         * @return float Le flottant généré
         */
        static float getFloat(float min, float max);
};
