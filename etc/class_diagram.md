# Diagramme de Classes : Carré Surfer

Ce diagramme illustre l'architecture orientée objet (POO) du jeu, découpée en modules logiques (packages) pour respecter le principe de responsabilité unique.

```mermaid
classDiagram
    %% --- Package: Engine ---
    class Game {
        -Player* player
        -std::vector~Obstacle*~ obstacles
        -CollisionManager collisionManager
        -ObstacleSpawner obstacleSpawner
        -InputManager inputManager
        -int scoreSeconds
        -bool isGameOver
        +run()
        +update(float deltaTime)
        +render()
        +reset()
    }

    class InputManager {
        -bool isReversed
        -float reverseTimer
        +update(float deltaTime)
        +setReversed(float duration)
        +isUpPressed()
        +isDownPressed()
        +isLeftPressed()
        +isRightPressed()
    }

    %% --- Package: Entities ---
    class Entity {
        <<Abstract>>
        #Vector2D position
        #Vector2D size
        #Vector2D velocity
        +update(float deltaTime)*
        +draw()*
        +getHitbox() Rect
    }

    class Player {
        -int health
        -float staggerTimer
        -float invincibilityTimer
        -bool isInvincible
        +update(float deltaTime)
        +draw()
        +takeDamage(int amount)
        +heal(int amount)
        +triggerInvincibility(float duration)
        +getHealth() int
    }

    %% --- Package: Obstacles ---
    class Obstacle {
        <<Abstract>>
        #int damage
        +update(float deltaTime)
        +draw()*
        +onCollision(Player& player)*
    }

    class Kebab {
        +draw()
        +onCollision(Player& player)
    }

    class Garbage {
        +draw()
        +onCollision(Player& player)
    }

    class Scooter {
        +draw()
        +onCollision(Player& player)
    }

    class DrunkStudent {
        +draw()
        +onCollision(Player& player)
    }

    %% --- Package: Systems ---
    class CollisionManager {
        +checkCollisions(Player& player, std::vector~Obstacle*~& obstacles)
        -checkIntersection(Rect a, Rect b) bool
    }

    class ObstacleSpawner {
        -float spawnTimer
        +update(float deltaTime, std::vector~Obstacle*~& obstacles)
        -spawnRandomObstacle() Obstacle*
    }

    %% --- Package: Utils ---
    class Vector2D {
        +float x
        +float y
        +add(Vector2D other)
        +multiply(float scalar)
    }

    %% Relations
    Entity <|-- Player
    Entity <|-- Obstacle
    Obstacle <|-- Kebab
    Obstacle <|-- Garbage
    Obstacle <|-- Scooter
    Obstacle <|-- DrunkStudent

    Game *-- Player
    Game *-- ObstacleSpawner
    Game *-- CollisionManager
    Game *-- InputManager
    Game o-- Obstacle
```

## Explication des Modules (Packages)

L'architecture est découpée en 5 modules principaux pour garantir une bonne séparation des responsabilités (Single Responsibility Principle) et faciliter le travail à deux via Git :

1. **`engine` (Le moteur du jeu)**
   - `Game` : Classe centrale. Gère la boucle principale du jeu (`update`, `render`), possède les autres managers et l'état global du jeu (Score, Game Over).
   - `InputManager` : Isole la logique de gestion des touches du clavier. C'est ici que l'on implémentera l'inversion des touches causée par l'étudiant ivre, pour ne pas polluer la classe `Player` ou `Game` avec cette logique.

2. **`entities` (Les entités de base)**
   - `Entity` : Classe abstraite contenant la logique de base de tout objet visible à l'écran (position, taille, vitesse, `getHitbox()`).
   - `Player` : Hérite de `Entity`. Implémente la logique spécifique à Thomas : sa santé, son état d'invincibilité, et sa tendance à tituber (stagger).

3. **`obstacles` (Les obstacles spécifiques)**
   - `Obstacle` : Hérite de `Entity`. Classe de base pour tous les objets que le joueur croise. Elle définit une méthode virtuelle pure `onCollision()` que chaque sous-classe implémentera.
   - `Kebab`, `Garbage`, `Scooter`, `DrunkStudent` : Classes concrètes qui héritent d'`Obstacle`. Le polymorphisme permet au jeu de traiter tous les obstacles de manière générique (`std::vector<Obstacle*>`), tout en exécutant un effet spécifique au contact.

4. **`systems` (La logique transversale)**
   - `CollisionManager` : Gère de façon isolée la physique et la détection de collisions entre les boîtes de collision (hitboxes) du joueur et des obstacles.
   - `ObstacleSpawner` : Responsable du frai (spawn) aléatoire des obstacles à l'écran, selon les probabilités dictées dans l'énoncé (20% kebab, etc.).

5. **`utils` (Les utilitaires)**
   - `Vector2D` : Structure simple pour gérer des positions et vecteurs proprement (mathématiques 2D).
   - `Constants` : Regroupe toutes les valeurs magiques (tailles, couleurs, vitesses) pour éviter les variables globales éparpillées.
