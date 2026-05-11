#pragma once

/**
 * @file Vector2D.hpp
 * @brief Structure atomique représentant un vecteur 2D mathématique
 */

struct Vector2D {
    float x;
    float y;

    /**
     * @brief Constructeur par défaut (0, 0)
     */
    Vector2D();

    /**
     * @brief Constructeur avec coordonnées
     * @param x Composante X
     * @param y Composante Y
     */
    Vector2D(float x, float y);

    // Surcharge des opérateurs pour des opérations mathématiques fluides
    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator-(const Vector2D& other) const;
    Vector2D operator*(float scalar) const;
    Vector2D& operator+=(const Vector2D& other);
};
