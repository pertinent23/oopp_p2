#include "Game.hpp"
#include "Window.hpp"
#include "InputManager.hpp"
#include "BackgroundManager.hpp"
#include "TextRenderer.hpp"
#include "Player.hpp"
#include "ObstacleSpawner.hpp"
#include "CollisionManager.hpp"
#include "ScoreManager.hpp"
#include "Obstacle.hpp"
#include "Settings.hpp"
#include "Random.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>


/**
 * @brief Constructeur de la classe Game.
 * Initialise les gestionnaires de base et lance la première initialisation du jeu.
 */
Game::Game():
    flashTimer(0.0f),
    currentState(GameState::PLAYING),
    previousState(GameState::PLAYING),
    gameOverSelection(0),
    timeSurvived(0.0f),
    shakeTimer(0.0f)
{
    Random::init();
    scoreManager = std::make_unique<ScoreManager>();
    init();
}


Game::~Game() = default;


/**
 * @brief Prépare ou réinitialise l'environnement de jeu.
 * Crée la fenêtre, Thomas, et les générateurs d'obstacles.
 */
void Game::init()
{
    window = std::make_unique<gfx::Window>(
        "Carre Surfer",
        Settings::WINDOW_WIDTH,
        Settings::WINDOW_HEIGHT
    );
    
    // Adaptation dynamique : on récupère la vraie taille de l'écran (Wayland/X11)
    Settings::WINDOW_WIDTH  = window->width();
    Settings::WINDOW_HEIGHT = window->height();

    inputManager      = std::make_unique<InputManager>(*window);
    player            = std::make_unique<Player>(*inputManager);
    obstacleSpawner   = std::make_unique<ObstacleSpawner>();
    backgroundManager = std::make_unique<BackgroundManager>();
    
    obstacles.clear();
    
    std::cout << "--- Carre Surfer : Initialisation ---" << std::endl;
    std::cout << "Resolution : " << Settings::WINDOW_WIDTH << "x" << Settings::WINDOW_HEIGHT << std::endl;
    std::cout << "Record actuel : " << scoreManager->getHighScore() << std::endl;
    
    currentState      = GameState::PLAYING;
    previousState     = GameState::PLAYING;
    gameOverSelection = 0;
    timeSurvived      = 0.0f;
    shakeTimer        = 0.0f;
    flashTimer        = 0.0f;
    particles.clear();
    floatingTexts.clear();
}


/**
 * @brief Boucle principale (Main Loop).
 * Gère la machine à états finis (FSM) pour basculer entre le jeu, la pause et les menus.
 */
void Game::run()
{
    const float deltaTime = 1.0f / Settings::TARGET_FPS;

    while (window->isOpen())
    {
        window->pollEvents();
        
        // --- Navigation Globale (Touche Echap) ---
        if (inputManager->wasEscPressed())
        {
            if (currentState == GameState::PLAYING || currentState == GameState::PAUSED)
            {
                previousState     = currentState;
                currentState      = GameState::QUIT_CONFIRM;
                gameOverSelection = 1; // Sécurité : curseur sur "NON"
            }
            else if (currentState == GameState::QUIT_CONFIRM)
            {
                currentState = previousState; // Retour à l'état précédent
            }
        }

        // --- Machine à États ---

        if (currentState == GameState::PLAYING)
        {
            if (inputManager->wasSpacePressed())
            {
                currentState = GameState::PAUSED;
            }
            else
            {
                update(deltaTime);
                render();
            }
        }
        else if (currentState == GameState::PAUSED)
        {
            // Navigation dans le menu Pause
            if (inputManager->wasUpPressed())    gameOverSelection = 0;
            if (inputManager->wasDownPressed())  gameOverSelection = 1;

            if (inputManager->wasEnterPressed())
            {
                if (gameOverSelection == 0) 
                    currentState = GameState::PLAYING;
                else 
                {
                    previousState = currentState;
                    currentState  = GameState::QUIT_CONFIRM;
                    gameOverSelection = 1;
                }
            }
            else if (inputManager->wasSpacePressed())
            {
                currentState = GameState::PLAYING;
            }

            render();
            displayPause();
        }
        else if (currentState == GameState::GAME_OVER)
        {
            // Navigation dans l'écran de défaite
            if (inputManager->wasUpPressed())   gameOverSelection = 0;
            if (inputManager->wasDownPressed()) gameOverSelection = 1;

            if (inputManager->wasEnterPressed())
            {
                if (gameOverSelection == 0) init(); // Recommencer
                else 
                {
                    window->close();
                    break;
                }
            }
            else if (inputManager->wasSpacePressed())
            {
                init();
            }

            displayGameOver();
        }
        else if (currentState == GameState::QUIT_CONFIRM)
        {
            // Navigation dans l'écran de confirmation
            if (inputManager->wasUpPressed())   gameOverSelection = 0;
            if (inputManager->wasDownPressed()) gameOverSelection = 1;

            if (inputManager->wasEnterPressed())
            {
                if (gameOverSelection == 0) 
                {
                    window->close(); // Quitter définitivement
                    break;
                }
                else currentState = previousState;
            }

            render();
            displayQuitConfirm();
        }

        window->present();
        window->waitForNextFrame(Settings::TARGET_FPS);
    }
}


/**
 * @brief Met à jour la logique de jeu (Positions, Collisions, Spawning).
 */
void Game::update(float deltaTime)
{
    inputManager->update(deltaTime);
    player->update(deltaTime);
    
    float currentSpeed = obstacleSpawner->getCurrentSpeed();
    backgroundManager->update(deltaTime, currentSpeed);
    obstacleSpawner->update(deltaTime, obstacles);

    for (auto& obstacle : obstacles)
    {
        obstacle->update(deltaTime);
    }

    // Gestion des collisions et déclenchement du tremblement d'écran
    if (CollisionManager::handleCollisions(*player, obstacles))
    {
        shakeTimer = Settings::SHAKE_DURATION;
    }

    // Effet spécial : La Chouffe nettoie tout l'écran
    if (player->popClearScreen())
    {
        for (auto& obstacle : obstacles)
        {
            obstacle->deactivate(); 
        }
    }

    // Effet spécial : Feedback Kebab
    if (player->popKebabEffectTriggered())
    {
        triggerKebabFeedback();
    }

    updateFeedbacks(deltaTime);
    cleanDeadObstacles();

    // Transition vers Game Over si Thomas meurt
    if (player->isDead())
    {
        currentState      = GameState::GAME_OVER;
        gameOverSelection = 0;
        
        int finalScore = static_cast<int>(timeSurvived * Settings::SCORE_MULTIPLIER);
        bool isNewRecord = scoreManager->submitScore(finalScore);

        std::cout << "--- GAME OVER ---" << std::endl;
        std::cout << "Score final : " << finalScore << std::endl;
        if (isNewRecord)
        {
            std::cout << "NOUVEAU RECORD ! Felicitations." << std::endl;
        }
        std::cout << "-----------------" << std::endl;
    }
    else
    {
        timeSurvived += deltaTime;
    }
    
    handleShake(deltaTime);

    if (flashTimer > 0.0f)
    {
        flashTimer -= deltaTime;
    }
}


/**
 * @brief Nettoie la liste des obstacles en supprimant ceux qui sont inactifs.
 */
void Game::cleanDeadObstacles()
{
    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(),
            [](const std::unique_ptr<Obstacle>& o) { 
                return !o->isActive(); 
            }
        ),
        obstacles.end()
    );
}


/** @brief Gère la décroissance du tremblement de l'écran. */
void Game::handleShake(float deltaTime)
{
    if (shakeTimer > 0.0f)
    {
        shakeTimer -= deltaTime;
    }
}


/**
 * @brief Orchestre le dessin de tous les éléments à l'écran.
 */
void Game::render()
{
    float healthRatio = player->getHealth() / static_cast<float>(Settings::PLAYER_MAX_HEALTH);
    
    // Nettoyage de l'écran
    // On utilise un fond noir qui s'assombrit dynamiquement selon la santé.
    // Cela renforce l'immersion : quand Thomas est mal en point, le monde devient plus sombre.
    int bgVal = static_cast<int>(15 * healthRatio);
    window->clear(gfx::Color(bgVal, bgVal, bgVal)); 

    int offsetX = 0;
    int offsetY = 0;

    // Calcul du tremblement d'écran (Screen Shake)
    // Si le joueur vient de subir un dégât, on génère un décalage aléatoire 
    // qui sera appliqué à tous les objets dessinés ensuite.
    if (shakeTimer > 0.0f)
    {
        offsetX = Random::getInt(-Settings::SHAKE_INTENSITY, Settings::SHAKE_INTENSITY);
        offsetY = Random::getInt(-Settings::SHAKE_INTENSITY, Settings::SHAKE_INTENSITY);
    }

    // Dessin du décor (Trottoir/Lignes)
    backgroundManager->draw(*window, healthRatio);

    // Dessin des obstacles
    for (const auto& obstacle : obstacles)
    {
        obstacle->draw(*window, healthRatio, offsetX, offsetY); 
    }

    // Dessin du joueur
    player->draw(*window, healthRatio, offsetX, offsetY);

    // Dessin des Particules
    for (const auto& p : particles)
    {
        window->fillRect(
            static_cast<int>(p.x), static_cast<int>(p.y), 
            3, 3, 
            gfx::Color(255, 165, 0) // Orange Kebab
        );
    }

    // Dessin des Textes Flottants
    for (const auto& t : floatingTexts)
    {
        TextRenderer::drawText(
            *window, t.text, 
            static_cast<int>(t.x), static_cast<int>(t.y), 
            3, gfx::Color(0, 255, 0) // Vert soin
        );
    }

    // Dessin du Flash vert
    if (flashTimer > 0.0f)
    {
        int alpha = static_cast<int>(100 * (flashTimer / Settings::KEBAB_FLASH_DURATION));
        window->fillOverlay(0, 0, Settings::WINDOW_WIDTH, Settings::WINDOW_HEIGHT, gfx::Color(0, 255, 0, alpha));
    }

    // Dessin de l'Interface Utilisateur (HUD)
    drawUI();
}


/**
 * @brief Algorithme de dessin de cœur pixelisé par balayage vertical.
 * Utilise des équations de cercles décalés pour former les deux lobes.
 */
void Game::drawHeart(
    int x, 
    int y, 
    int size, 
    float fillRatio
)
{
    gfx::Color red(255, 50, 50);
    gfx::Color grey(80, 80, 80);
    
    size  = (size / 4) * 4; 
    int R = size / 4;
    int fillY = y + size - static_cast<int>(size * fillRatio);

    // Parcours horizontal de la zone du cœur
    for (int px = 0; px <= size; ++px)
    {
        int yTop, yBottom;

        // Équation des lobes
        // On divise le cœur en deux parties (gauche/droite) pour former les arrondis.
        if (px <= 2 * R) // Premier lobe (Lobe gauche)
        {
            int dx  = px - R;
            int dy  = static_cast<int>(std::sqrt(std::max(0, R * R - dx * dx)));
            yTop    = R - dy;
            yBottom = 2 * R + px;
        }
        else // Second lobe (Lobe droit)
        {
            int dx  = px - 3 * R;
            int dy  = static_cast<int>(std::sqrt(std::max(0, R * R - dx * dx)));
            yTop    = R - dy;
            yBottom = 6 * R - px;
        }
        
        int absTop    = y + yTop;
        int absBottom = y + yBottom;
        
        // Remplissage vertical progressif
        // On dessine une ligne verticale pour chaque colonne.
        // On compare la hauteur actuelle de la colonne avec le "niveau de liquide" (fillY).
        if (absBottom < fillY) // Zone supérieure (vide -> grise)
        {
            window->drawLine(x + px, absTop, x + px, absBottom, grey);
        }
        else if (absTop >= fillY) // Zone inférieure (pleine -> rouge)
        {
            window->drawLine(x + px, absTop, x + px, absBottom, red);
        }
        else // Zone de transition (entre le gris et le rouge)
        {
            window->drawLine(x + px, absTop, x + px, fillY, grey);
            window->drawLine(x + px, fillY, x + px, absBottom, red);
        }
    }
}


void Game::updateFeedbacks(float deltaTime)
{
    // Particules
    for (auto it = particles.begin(); it != particles.end(); )
    {
        it->x    += it->vx * deltaTime;
        it->y    += it->vy * deltaTime;
        it->life -= deltaTime;
        
        if (it->life <= 0) it = particles.erase(it);
        else ++it;
    }

    // Textes Flottants
    for (auto it = floatingTexts.begin(); it != floatingTexts.end(); )
    {
        it->y    -= Settings::FLOATING_TEXT_SPEED * deltaTime; 
        it->life -= deltaTime;
        
        if (it->life <= 0) it = floatingTexts.erase(it);
        else ++it;
    }
}


void Game::triggerKebabFeedback()
{
    flashTimer = Settings::KEBAB_FLASH_DURATION;

    // Texte flottant au-dessus du joueur
    floatingTexts.push_back({
        "+10 HP", 
        player->getPosition().x, 
        player->getPosition().y - 30.0f, 
        Settings::FLOATING_TEXT_LIFE
    });

    // Génération de particules orange
    for (int i = 0; i < Settings::PARTICLE_COUNT; ++i)
    {
        particles.push_back({
            player->getPosition().x + 15.0f,
            player->getPosition().y + 15.0f,
            Random::getFloat(-150.0f, 150.0f),
            Random::getFloat(-150.0f, 150.0f),
            Settings::PARTICLE_LIFE
        });
    }
}


/**
 * @brief Dessine l'interface HUD (Heads-Up Display).
 */
void Game::drawUI()
{
    // Barre de Vie (Cœurs dynamiques)
    // On divise la santé totale en 5 sections (cœurs).
    int maxHearts    = 5;
    float hpPerHeart = Settings::PLAYER_MAX_HEALTH / static_cast<float>(maxHearts);
    
    for (int i = 0; i < maxHearts; ++i)
    {
        float heartHP   = player->getHealth() - (i * hpPerHeart);
        float fillRatio = 0.0f;

        if (heartHP >= hpPerHeart) fillRatio = 1.0f; // Cœur plein
        else if (heartHP > 0.0f)   fillRatio = heartHP / hpPerHeart; // Cœur partiellement rempli
        
        drawHeart(20 + i * 40, 20, 28, fillRatio); 
    }

    // Barre de Score (Progression)
    // On dessine un rectangle gris de fond, puis un rectangle bleu par-dessus
    // pour montrer la progression du joueur vers l'objectif.
    int maxDistBar  = 500;
    int finalScore  = static_cast<int>(timeSurvived * Settings::SCORE_MULTIPLIER);
    float distRatio = std::min(1.0f, finalScore / static_cast<float>(Settings::MAX_SCORE_DISTANCE));
    int barX        = 230;
    
    window->fillRect(barX, 25, maxDistBar, 15, gfx::Color(80, 80, 80)); // Fond gris
    window->fillRect(barX, 25, static_cast<int>(maxDistBar * distRatio), 15, gfx::Color(0, 150, 255)); // Remplissage bleu
    window->drawRect(barX, 25, maxDistBar, 15, gfx::Color(255, 255, 255)); // Contour blanc pour la lisibilité

    // Marqueur du High Score
    // On dessine une petite barre dorée sur la barre de score pour matérialiser le record.
    int highScore = scoreManager->getHighScore();
    if (highScore > 0)
    {
        float hsRatio = std::min(1.0f, highScore / static_cast<float>(Settings::MAX_SCORE_DISTANCE));
        int hsPos     = barX + static_cast<int>(maxDistBar * hsRatio);
        window->fillRect(hsPos - 2, 15, 4, 35, gfx::Color(255, 215, 0)); 
    }

    // Commandes de jeu (Clignotantes)
    // On affiche les rappels de touches. Le clignotement attire l'attention
    // sans être trop intrusif grâce à un cycle de 1 seconde.
    if (std::fmod(timeSurvived, 1.0f) < 0.9f)
    {
        TextRenderer::drawText(
            *window, "[SPACE] PAUSE", 
            Settings::WINDOW_WIDTH - 340, 25, 3, 
            gfx::Color(150, 150, 150)
        );

        TextRenderer::drawText(
            *window, "[ESC] QUIT", 
            Settings::WINDOW_WIDTH - 140, 25, 3, 
            gfx::Color(150, 150, 150)
        );
    }
}


/** @brief Affiche l'écran de fin de partie. */
void Game::displayGameOver()
{
    // Fond d'écran de défaite (Bleu très sombre)
    window->clear(gfx::Color(10, 10, 15));
    
    // Message principal en grand et rouge
    TextRenderer::drawText(
        *window, "GAME OVER", 
        Settings::WINDOW_WIDTH / 2 - 144, 
        Settings::WINDOW_HEIGHT / 3, 8, 
        gfx::Color(255, 50, 50)
    );
    
    // Options du menu
    // La couleur dorée indique l'option actuellement sélectionnée par le joueur.
    gfx::Color colorRestart = (gameOverSelection == 0) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);
    gfx::Color colorQuit    = (gameOverSelection == 1) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);

    TextRenderer::drawText(
        *window, "> [SPACE] RESTART <", 
        Settings::WINDOW_WIDTH / 2 - 190, 
        Settings::WINDOW_HEIGHT / 2 + 20, 5, colorRestart
    );

    TextRenderer::drawText(
        *window, "> [ESC] QUIT <", 
        Settings::WINDOW_WIDTH / 2 - 140, 
        Settings::WINDOW_HEIGHT / 2 + 90, 5, colorQuit
    );
}


/** @brief Affiche l'écran de pause en superposition (Alpha blending). */
void Game::displayPause()
{
    // Voile semi-transparent (Alpha blending)
    // On dessine un grand rectangle bleu nuit transparent sur tout l'écran 
    // pour assombrir le jeu sans le cacher complètement.
    window->fillOverlay(0, 0, Settings::WINDOW_WIDTH, Settings::WINDOW_HEIGHT, gfx::Color(10, 10, 20, 150));
    
    // Dessin de l'icône de pause (Deux barres blanches verticales)
    // Placées au centre pour un aspect visuel classique de lecteur média
    window->fillRect(Settings::WINDOW_WIDTH / 2 - 40, Settings::WINDOW_HEIGHT / 3, 30, 100, gfx::Color(255, 50, 50));
    window->fillRect(Settings::WINDOW_WIDTH / 2 + 10, Settings::WINDOW_HEIGHT / 3, 30, 100, gfx::Color(255, 50, 50));

    // Options du menu de pause
    gfx::Color colorResume = (gameOverSelection == 0) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);
    gfx::Color colorQuit   = (gameOverSelection == 1) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);

    TextRenderer::drawText(
        *window, "> [SPACE] RESUME <", 
        Settings::WINDOW_WIDTH / 2 - 180, 
        Settings::WINDOW_HEIGHT / 2 + 60, 5, colorResume
    );

    TextRenderer::drawText(
        *window, "> [ESC] QUIT <", 
        Settings::WINDOW_WIDTH / 2 - 140, 
        Settings::WINDOW_HEIGHT / 2 + 130, 5, colorQuit
    );
}


/** @brief Affiche l'écran de confirmation de sortie. */
void Game::displayQuitConfirm()
{
    // Voile très sombre pour détacher le menu du jeu
    window->fillOverlay(0, 0, Settings::WINDOW_WIDTH, Settings::WINDOW_HEIGHT, gfx::Color(10, 10, 15, 200));

    // Question de confirmation
    TextRenderer::drawText(
        *window, "ARE YOU SURE?", 
        Settings::WINDOW_WIDTH / 2 - 182, 
        Settings::WINDOW_HEIGHT / 3, 7, 
        gfx::Color(255, 50, 50)
    );

    // Boutons de choix (YES/NO)
    gfx::Color colorYes = (gameOverSelection == 0) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);
    gfx::Color colorNo  = (gameOverSelection == 1) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);

    TextRenderer::drawText(
        *window, "> YES <", 
        Settings::WINDOW_WIDTH / 2 - 70, 
        Settings::WINDOW_HEIGHT / 2 + 20, 5, colorYes
    );

    TextRenderer::drawText(
        *window, "> NO <", 
        Settings::WINDOW_WIDTH / 2 - 60, 
        Settings::WINDOW_HEIGHT / 2 + 90, 5, colorNo
    );
}
