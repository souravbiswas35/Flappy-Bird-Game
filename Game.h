#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include "Bird.h"
#include "Pipe.h"

enum class GameState   { MENU, PLAYING, PAUSED, GAMEOVER };
enum class Difficulty  { EASY, NORMAL, HARD };
enum class PlatformMode{ DESKTOP, MOBILE };

// Sky scenarios that cycle during gameplay
enum class SkyScenario {
    MORNING,        // warm pink/orange sunrise
    DAY,            // bright blue sky
    EVENING,        // golden hour orange
    NIGHT,          // dark blue + stars
    MOONLIT_NIGHT,  // navy + bright moon + stars
    BLACK_NIGHT,    // near-black + many stars
    STATS,          // brief tinted pause with score
    MOON,           // deep purple + large moon
    COUNT
};

class Game {
public:
    Game();
    void run();

private:
    // ── Order here MUST match the constructor initializer list ────────────

    // Window (initialized first in ctor)
    int windowWidth, windowHeight;
    float scaleFactor;
    PlatformMode platform;
    sf::RenderWindow window;

    static const int BASE_WIDTH  = 600;
    static const int BASE_HEIGHT = 800;

    // Game objects
    Bird bird;

    // State
    GameState  state;
    Difficulty difficulty;

    // Timers / counters
    float pipeSpawnTimer;
    float pipeSpawnInterval;
    int   score;
    int   highScore;
    int   selectedDifficulty;

    // Flash
    float flashAlpha;

    // Sounds
    bool soundEnabled;

    // Sky scenario system
    struct SkyGradient {
        sf::Color top, mid, bottom;
        float timeOfDay;
        float transitionSpeed;
    };
    SkyGradient sky;
    sf::VertexArray skyVertices;

    SkyScenario currentScenario;
    float       scenarioTimer;
    float       scenarioDuration;
    float       blendT;
    bool        isBlending;
    sf::Color   fromTop, fromMid, fromBot;
    sf::Color   toTop,   toMid,   toBot;
    float       scenarioLabelTimer;

    // Clouds
    struct Cloud {
        sf::Texture* texture;
        sf::Sprite   sprite;
        float x, y, speed, depth, alpha;
    };
    std::vector<Cloud> clouds;
    sf::Texture cloudTextures[3];
    float cloudSpawnTimer;

    // Celestial bodies
    sf::CircleShape celestialBody;
    sf::CircleShape celestialGlow;
    sf::CircleShape bigMoon;
    sf::CircleShape bigMoonGlow;

    // Stars
    struct Star {
        sf::CircleShape shape;
        float twinklePhase;
        float twinkleSpeed;
    };
    std::vector<Star> stars;

    // Background birds
    struct BackgroundBird {
        sf::ConvexShape shape[2];
        float x, y, speed, wingPhase;
    };
    std::vector<BackgroundBird> bgBirds;
    float bgBirdSpawnTimer;

    // Ground
    sf::RectangleShape ground;
    sf::RectangleShape groundDecor[10];
    float groundX[2];
    float groundSpeed;

    // Mountains
    struct Mountain {
        sf::ConvexShape shape;
        float baseX, speed;
    };
    std::vector<Mountain> mountains[2];

    // Fonts & texts
    sf::Font fontMain, fontScore;
    sf::Text scoreText, hiScoreText, titleText, instructText;
    sf::Text diffText[3], pauseText, gameoverText, restartText;

    // Pipes
    std::vector<Pipe> pipes;

    // Touch / ripple
    sf::CircleShape touchRipple;
    float rippleAlpha;
    sf::Vector2f lastTouchPos;

    // Sounds (buffers + sounds)
    sf::SoundBuffer flapBuf, dieBuf, scoreBuf, swooshBuf;
    sf::Sound       flapSnd, dieSnd, scoreSnd, swooshSnd;

    // UI
    struct Button {
        sf::RectangleShape bg;
        sf::Text           label;
        sf::FloatRect      bounds;
        bool               hovered;
    };
    Button soundButton;
    Button menuButtons[3];

    // ── Methods ───────────────────────────────────────────────────────────
    void processEvents();
    void update(float dt);
    void render();
    void handleResize();
    void updateScaling();

    void updateMenu(float dt);
    void updatePlaying(float dt);
    void updateGameOver(float dt);
    void updateSky(float dt);
    void updateClouds(float dt);
    void updateStars(float dt);
    void updateBackgroundBirds(float dt);
    void updateMountains(float dt);

    void advanceScenario();
    void getScenarioColors(SkyScenario s,
                           sf::Color& top, sf::Color& mid, sf::Color& bot);
    sf::Color lerpColor(sf::Color a, sf::Color b, float t);
    bool scenarioHasStars(SkyScenario s);

    void renderSky();
    void renderCelestial();
    void renderStars();
    void renderMountains();
    void renderClouds();
    void renderBackgroundBirds();
    void renderGround();
    void renderHUD();
    void renderMenu();
    void renderPause();
    void renderGameOver();
    void renderTouchIndicator();
    void renderScenarioLabel();

    void spawnPipe();
    void resetGame();
    void loadHighScore();
    void saveHighScore();
    void setDifficulty(Difficulty d);
    void buildSounds();
    void buildFonts();
    void buildSkySystem();
    void buildClouds();
    void buildStars();
    void buildMountains();
    void buildGround();
    void buildUI();
    void spawnCloud();
    void spawnBackgroundBird();
    sf::Vector2f scale(float x, float y);
    float scaleX(float x);
    float scaleY(float y);
    float scaleSize(float size);
    void detectPlatform();
    void createCloudTexture(int index);
};