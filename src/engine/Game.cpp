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
#include "SmartPlayer.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>


/**
 * @brief Constructeur de la classe Game.
 * Initialise les gestionnaires de base et lance la première initialisation du jeu.
 */
Game::Game(bool demoMode):
    flashTimer(0.0f),
    currentState(GameState::PLAYING),
    previousState(GameState::PLAYING),
    menuSelection(0),
    timeSurvived(0.0f),
    shakeTimer(0.0f),
    isDemoMode(demoMode)
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
    
    Settings::WINDOW_WIDTH  = window->width();
    Settings::WINDOW_HEIGHT = window->height();

    inputManager = std::make_unique<InputManager>(*window);
    backgroundManager = std::make_unique<BackgroundManager>();
    
    std::cout << "--- Carre Surfer : Initialisation ---" << std::endl;
    std::cout << "Resolution : " << Settings::WINDOW_WIDTH << "x" << Settings::WINDOW_HEIGHT << std::endl;
    std::cout << "Record actuel : " << scoreManager->getHighScore() << std::endl;
    
    currentState      = GameState::MAIN_MENU;
    previousState     = GameState::MAIN_MENU;
    menuSelection     = 0;
    timeSurvived      = 0.0f;
    shakeTimer        = 0.0f;
    flashTimer        = 0.0f;
    particles.clear();
    floatingTexts.clear();
}

void Game::resetGame()
{
    if (isDemoMode)
    {
        player = std::make_unique<SmartPlayer>(*inputManager);
    }
    else
    {
        player = std::make_unique<Player>(*inputManager);
    }

    obstacleSpawner   = std::make_unique<ObstacleSpawner>();
    
    obstacles.clear();
    timeSurvived      = 0.0f;
    shakeTimer        = 0.0f;
    flashTimer        = 0.0f;
    particles.clear();
    floatingTexts.clear();

    currentState      = GameState::PLAYING;
    previousState     = GameState::PLAYING;
}

void Game::applyDifficulty(int level)
{
    if (level == 0) // EASY
    {
        Settings::BASE_OBSTACLE_SPEED = 600.0f;
        Settings::MAX_OBSTACLE_SPEED = 1400.0f;
        Settings::BASE_SPAWN_INTERVAL_MAX = 1.0f;
        Settings::MIN_SPAWN_INTERVAL = 0.3f;
        Settings::GROUP_SPAWN_PROB = 20;
        Settings::DIFFICULTY_GROWTH = 15.0f;
    }
    else if (level == 1) // INTERMEDIATE
    {
        Settings::BASE_OBSTACLE_SPEED = 800.0f;
        Settings::MAX_OBSTACLE_SPEED = 1800.0f;
        Settings::BASE_SPAWN_INTERVAL_MAX = 0.8f;
        Settings::MIN_SPAWN_INTERVAL = 0.2f;
        Settings::GROUP_SPAWN_PROB = 35;
        Settings::DIFFICULTY_GROWTH = 25.0f;
    }
    else if (level == 2) // EXPERT
    {
        Settings::BASE_OBSTACLE_SPEED = 1000.0f;
        Settings::MAX_OBSTACLE_SPEED = 2500.0f;
        Settings::BASE_SPAWN_INTERVAL_MAX = 0.6f;
        Settings::MIN_SPAWN_INTERVAL = 0.1f;
        Settings::GROUP_SPAWN_PROB = 50;
        Settings::DIFFICULTY_GROWTH = 40.0f;
    }
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
            if (currentState == GameState::PLAYING || currentState == GameState::PAUSED || currentState == GameState::MAIN_MENU)
            {
                previousState     = currentState;
                currentState      = GameState::QUIT_CONFIRM;
                menuSelection = 1; // Sécurité : curseur sur "NON"
            }
            else if (currentState == GameState::QUIT_CONFIRM)
            {
                currentState = previousState; // Retour à l'état précédent
            }
        }

        // --- Machine à États ---

        if (currentState == GameState::MAIN_MENU)
        {
            if (inputManager->wasUpPressed())
            {
                menuSelection = std::max(0, menuSelection - 1);
            }

            if (inputManager->wasDownPressed())
            {
                menuSelection = std::min(2, menuSelection + 1);
            }

            if (inputManager->wasEnterPressed())
            {
                applyDifficulty(menuSelection);
                resetGame();
            }
            else
            {
                // Animation du fond pour le menu principal
                backgroundManager->update(deltaTime, 200.0f);
                displayMainMenu();
            }
        }
        else if (currentState == GameState::PLAYING)
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
            if (inputManager->wasUpPressed())    menuSelection = 0;
            if (inputManager->wasDownPressed())  menuSelection = 1;

            if (inputManager->wasEnterPressed())
            {
                if (menuSelection == 0) 
                    currentState = GameState::PLAYING;
                else 
                {
                    previousState = currentState;
                    currentState  = GameState::QUIT_CONFIRM;
                    menuSelection = 1;
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
            if (inputManager->wasUpPressed())   menuSelection = std::max(0, menuSelection - 1);
            if (inputManager->wasDownPressed()) menuSelection = std::min(2, menuSelection + 1);

            if (inputManager->wasEnterPressed())
            {
                if (menuSelection == 0) resetGame(); // Recommencer
                else if (menuSelection == 1) 
                {
                    currentState = GameState::MAIN_MENU;
                    menuSelection = 0;
                }
                else 
                {
                    window->close();
                    break;
                }
            }
            else if (inputManager->wasSpacePressed())
            {
                resetGame();
            }

            displayGameOver();
        }
        else if (currentState == GameState::QUIT_CONFIRM)
        {
            // Navigation dans l'écran de confirmation
            if (inputManager->wasUpPressed())   menuSelection = 0;
            if (inputManager->wasDownPressed()) menuSelection = 1;

            if (inputManager->wasEnterPressed())
            {
                if (menuSelection == 0) 
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
    
    // On passe les obstacles au joueur (utile pour l'IA en mode démo)
    player->setObstacles(obstacles);
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
        menuSelection     = 0;
        
        int finalScore = static_cast<int>(timeSurvived * Settings::SCORE_MULTIPLIER);
        
        bool isNewRecord = false;
        // On n'enregistre le score que si on n'est PAS en mode démo
        if (!isDemoMode)
        {
            isNewRecord = scoreManager->submitScore(finalScore);
        }

        std::cout << "--- GAME OVER ---" << std::endl;
        std::cout << "Score final : " << finalScore << std::endl;
        if (isDemoMode)
        {
            std::cout << "[Mode Demo] Le score n'est pas sauvegarde." << std::endl;
        }
        else if (isNewRecord)
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
    // Sécurité : Si le joueur n'est pas encore créé (ex: dans le Menu Principal), on utilise des valeurs par défaut.
    float healthRatio = player ? (player->getHealth() / static_cast<float>(Settings::PLAYER_MAX_HEALTH)) : 1.0f;
    
    // Nettoyage de l'écran
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

    // Dessin du joueur (si initialisé)
    if (player)
    {
        player->draw(*window, healthRatio, offsetX, offsetY);
    }

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

    if (player)
    {
        // Dessin du Flash vert
        if (flashTimer > 0.0f)
        {
            int alpha = static_cast<int>(100 * (flashTimer / Settings::KEBAB_FLASH_DURATION));
            window->fillOverlay(0, 0, Settings::WINDOW_WIDTH, Settings::WINDOW_HEIGHT, gfx::Color(0, 255, 0, alpha));
        }

        // Dessin de l'Interface Utilisateur (HUD)
        drawUI();
    }
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
    if (!player) return;

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
        Settings::WINDOW_HEIGHT / 4, 8, 
        gfx::Color(255, 50, 50)
    );
    
    // Options du menu
    gfx::Color colorRestart = (menuSelection == 0) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);
    gfx::Color colorMenu    = (menuSelection == 1) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);
    gfx::Color colorQuit    = (menuSelection == 2) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);

    TextRenderer::drawText(
        *window, "> [SPACE] RESTART <", 
        Settings::WINDOW_WIDTH / 2 - 190, 
        Settings::WINDOW_HEIGHT / 2, 5, colorRestart
    );

    TextRenderer::drawText(
        *window, "> MAIN MENU <", 
        Settings::WINDOW_WIDTH / 2 - 110, 
        Settings::WINDOW_HEIGHT / 2 + 70, 5, colorMenu
    );

    TextRenderer::drawText(
        *window, "> [ESC] QUIT <", 
        Settings::WINDOW_WIDTH / 2 - 140, 
        Settings::WINDOW_HEIGHT / 2 + 140, 5, colorQuit
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
    gfx::Color colorResume = (menuSelection == 0) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);
    gfx::Color colorQuit   = (menuSelection == 1) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);

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
    gfx::Color colorYes = (menuSelection == 0) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);
    gfx::Color colorNo  = (menuSelection == 1) ? gfx::Color(255, 215, 0) : gfx::Color(100, 100, 100);

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


/** @brief Affiche l'écran du Menu Principal. */
void Game::displayMainMenu()
{
    // Fond dynamique de rue
    window->clear(gfx::Color(20, 20, 25));
    backgroundManager->draw(*window, 1.0f); // Dessine le background (trottoirs, rue)
    
    // Voile assombrissant pour faire ressortir le texte
    window->fillOverlay(0, 0, Settings::WINDOW_WIDTH, Settings::WINDOW_HEIGHT, gfx::Color(0, 0, 0, 180));
    
    // Titre du jeu
    TextRenderer::drawText(
        *window, "CARRE SURFER", 
        Settings::WINDOW_WIDTH / 2 - 200, 
        Settings::WINDOW_HEIGHT / 4, 10, 
        gfx::Color(0, 200, 255)
    );

    TextRenderer::drawText(
        *window, "SELECT DIFFICULTY", 
        Settings::WINDOW_WIDTH / 2 - 180, 
        Settings::WINDOW_HEIGHT / 3 + 50, 5, 
        gfx::Color(200, 200, 200)
    );
    
    // Options
    gfx::Color colorEasy  = (menuSelection == 0) ? gfx::Color(0, 255, 0)   : gfx::Color(100, 100, 100);
    gfx::Color colorInter = (menuSelection == 1) ? gfx::Color(255, 165, 0) : gfx::Color(100, 100, 100);
    gfx::Color colorExp   = (menuSelection == 2) ? gfx::Color(255, 50, 50) : gfx::Color(100, 100, 100);

    TextRenderer::drawText(
        *window, "> EASY <", 
        Settings::WINDOW_WIDTH / 2 - 90, 
        Settings::WINDOW_HEIGHT / 2 + 20, 5, colorEasy
    );
    TextRenderer::drawText(
        *window, "> INTERMEDIATE <", 
        Settings::WINDOW_WIDTH / 2 - 160, 
        Settings::WINDOW_HEIGHT / 2 + 90, 5, colorInter
    );
    TextRenderer::drawText(
        *window, "> EXPERT <", 
        Settings::WINDOW_WIDTH / 2 - 100, 
        Settings::WINDOW_HEIGHT / 2 + 160, 5, colorExp
    );
}
