#include "Game.h"
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

const float PI = 3.14159265f;

// ─── Sound Synthesis ──────────────────────────────────────────────────────
static sf::SoundBuffer makeTone(float freq, float dur, float vol = 1.f,
                                 bool decay = false) {
    const unsigned SR = 44100;
    unsigned samples = static_cast<unsigned>(SR * dur);
    std::vector<sf::Int16> buf(samples);
    for (unsigned i = 0; i < samples; ++i) {
        float t   = static_cast<float>(i) / SR;
        float env = decay ? std::exp(-t * 15.f) : std::exp(-t * 3.f);
        float s   = std::sin(2.f * PI * freq * t) * env * vol * 32000.f;
        buf[i]    = static_cast<sf::Int16>(std::max(-32767.f, std::min(32767.f, s)));
    }
    sf::SoundBuffer sb;
    sb.loadFromSamples(buf.data(), samples, 1, SR);
    return sb;
}

// ─── Constructor ──────────────────────────────────────────────────────────
// Initializer list order MUST match member declaration order in Game.h
Game::Game()
    : windowWidth(BASE_WIDTH), windowHeight(BASE_HEIGHT),
      scaleFactor(1.0f),
      platform(PlatformMode::DESKTOP),
      window(sf::VideoMode(BASE_WIDTH, BASE_HEIGHT),
             "Flappy Bird",
             sf::Style::Titlebar | sf::Style::Close),
      bird(BASE_WIDTH * 0.25f, BASE_HEIGHT * 0.4f),
      state(GameState::MENU),
      difficulty(Difficulty::NORMAL),
      pipeSpawnTimer(0.f),
      pipeSpawnInterval(2.4f),
      score(0), highScore(0), selectedDifficulty(1),
      flashAlpha(0.f),
      soundEnabled(true),
      currentScenario(SkyScenario::MORNING),
      scenarioTimer(0.f), scenarioDuration(30.f),
      blendT(0.f), isBlending(false),
      scenarioLabelTimer(0.f),
      cloudSpawnTimer(0.f),
      bgBirdSpawnTimer(0.f),
      groundSpeed(160.f),
      rippleAlpha(0.f)
{
    window.setFramerateLimit(60);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    detectPlatform();
    updateScaling();
    buildFonts();
    buildSounds();
    buildSkySystem();
    buildClouds();
    buildStars();
    buildMountains();
    buildGround();
    buildUI();
    loadHighScore();
    setDifficulty(Difficulty::NORMAL);
}

// ─── Platform / Scaling ───────────────────────────────────────────────────
void Game::detectPlatform() {
    platform = (windowWidth < 600 || windowHeight < 800)
             ? PlatformMode::MOBILE : PlatformMode::DESKTOP;
}
void Game::updateScaling() {
    scaleFactor = std::min(
        static_cast<float>(windowWidth)  / BASE_WIDTH,
        static_cast<float>(windowHeight) / BASE_HEIGHT);
}
sf::Vector2f Game::scale(float x, float y) { return sf::Vector2f(x*scaleFactor, y*scaleFactor); }
float Game::scaleX(float x)    { return x * scaleFactor; }
float Game::scaleY(float y)    { return y * scaleFactor; }
float Game::scaleSize(float s) { return s * scaleFactor; }

// ─── Fonts ────────────────────────────────────────────────────────────────
void Game::buildFonts() {
    const char* paths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/segoeui.ttf", nullptr
    };
    for (int i = 0; paths[i]; ++i)
        if (fontMain.loadFromFile(paths[i])) break;
    fontScore = fontMain;

    scoreText.setFont(fontScore);
    scoreText.setCharacterSize(52);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setOutlineColor(sf::Color(30, 30, 30));
    scoreText.setOutlineThickness(4.f);
    scoreText.setStyle(sf::Text::Bold);

    hiScoreText.setFont(fontMain);
    hiScoreText.setCharacterSize(18);
    hiScoreText.setFillColor(sf::Color(255, 230, 100));
    hiScoreText.setOutlineColor(sf::Color(60, 40, 0));
    hiScoreText.setOutlineThickness(2.f);

    titleText.setFont(fontMain);
    titleText.setCharacterSize(56);
    titleText.setFillColor(sf::Color(255, 230, 50));
    titleText.setOutlineColor(sf::Color(180, 100, 0));
    titleText.setOutlineThickness(5.f);
    titleText.setStyle(sf::Text::Bold);
    titleText.setString("FLAPPY BIRD");

    instructText.setFont(fontMain);
    instructText.setCharacterSize(21);
    instructText.setFillColor(sf::Color::White);
    instructText.setOutlineColor(sf::Color(40, 40, 40));
    instructText.setOutlineThickness(2.f);
    instructText.setString("SPACE or TAP to Flap");

    const char*  dLabels[] = { "  EASY", "NORMAL", "  HARD" };
    sf::Color    dColors[] = {
        sf::Color(100,255,100),
        sf::Color(255,255,100),
        sf::Color(255,100,100)
    };
    for (int i = 0; i < 3; ++i) {
        diffText[i].setFont(fontMain);
        diffText[i].setCharacterSize(26);
        diffText[i].setFillColor(dColors[i]);
        diffText[i].setOutlineColor(sf::Color(30,30,30));
        diffText[i].setOutlineThickness(2.f);
        diffText[i].setString(dLabels[i]);
    }

    pauseText.setFont(fontMain);
    pauseText.setCharacterSize(52);
    pauseText.setFillColor(sf::Color::White);
    pauseText.setOutlineColor(sf::Color(40,40,40));
    pauseText.setOutlineThickness(4.f);
    pauseText.setStyle(sf::Text::Bold);
    pauseText.setString("PAUSED");

    gameoverText.setFont(fontMain);
    gameoverText.setCharacterSize(52);
    gameoverText.setFillColor(sf::Color(255, 80, 80));
    gameoverText.setOutlineColor(sf::Color(80, 0, 0));
    gameoverText.setOutlineThickness(4.f);
    gameoverText.setStyle(sf::Text::Bold);
    gameoverText.setString("GAME OVER");

    restartText.setFont(fontMain);
    restartText.setCharacterSize(20);
    restartText.setFillColor(sf::Color::White);
    restartText.setOutlineColor(sf::Color(40,40,40));
    restartText.setOutlineThickness(2.f);
    restartText.setString("SPACE / TAP to Restart     M for Menu");
}

// ─── Sounds ───────────────────────────────────────────────────────────────
void Game::buildSounds() {
    flapBuf   = makeTone(520.f, 0.12f, 0.6f, true);
    dieBuf    = makeTone(180.f, 0.40f, 0.8f, false);
    scoreBuf  = makeTone(880.f, 0.15f, 0.5f, true);
    swooshBuf = makeTone(300.f, 0.18f, 0.4f, true);
    flapSnd.setBuffer(flapBuf);
    dieSnd.setBuffer(dieBuf);
    scoreSnd.setBuffer(scoreBuf);
    swooshSnd.setBuffer(swooshBuf);
}

// ─── Difficulty ───────────────────────────────────────────────────────────
void Game::setDifficulty(Difficulty d) {
    difficulty = d;
    switch (d) {
        case Difficulty::EASY:
            Pipe::SPEED = 120.f; Pipe::GAP = 220.f; pipeSpawnInterval = 3.0f; break;
        case Difficulty::NORMAL:
            Pipe::SPEED = 160.f; Pipe::GAP = 185.f; pipeSpawnInterval = 2.4f; break;
        case Difficulty::HARD:
            Pipe::SPEED = 210.f; Pipe::GAP = 145.f; pipeSpawnInterval = 2.0f; break;
    }
    groundSpeed = Pipe::SPEED;
}

// ─── Game Reset ───────────────────────────────────────────────────────────
void Game::resetGame() {
    bird.reset(BASE_WIDTH * 0.25f, BASE_HEIGHT * 0.4f);
    pipes.clear();
    pipeSpawnTimer = 0.f;
    score          = 0;
    flashAlpha     = 0.f;
    scenarioTimer  = 0.f;
    // Advance to next scenario each restart — cycles through all 8
    advanceScenario();
    setDifficulty(difficulty);
}

// ─── Pipe Spawning ────────────────────────────────────────────────────────
void Game::spawnPipe() {
    float minY = 140.f;
    float maxY = static_cast<float>(windowHeight) - 80.f - Pipe::GAP * 0.5f;
    float gapY = minY + (static_cast<float>(std::rand()) / RAND_MAX) * (maxY - minY);
    pipes.emplace_back(static_cast<float>(windowWidth + 20), gapY,
                       static_cast<float>(windowHeight - 80));
}

// ─── High Score IO ────────────────────────────────────────────────────────
void Game::loadHighScore() { std::ifstream f("highscore.dat"); if (f.is_open()) f >> highScore; }
void Game::saveHighScore() { std::ofstream f("highscore.dat"); if (f.is_open()) f << highScore; }

// ─── Events ───────────────────────────────────────────────────────────────
void Game::processEvents() {
    sf::Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed)  { window.close(); return; }
        if (ev.type == sf::Event::Resized) { handleResize(); }

        if (ev.type == sf::Event::MouseButtonPressed &&
            ev.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mp(static_cast<float>(ev.mouseButton.x),
                            static_cast<float>(ev.mouseButton.y));
            lastTouchPos = mp;
            rippleAlpha  = 100.f;

            if (soundButton.bg.getGlobalBounds().contains(mp)) {
                soundEnabled = !soundEnabled;
                soundButton.label.setString(soundEnabled ? "ON" : "OFF");
                continue;
            }
            if (state == GameState::PLAYING && !bird.isDead()) {
                bird.flap();
                if (soundEnabled) flapSnd.play();
            } else if (state == GameState::GAMEOVER) {
                resetGame();
                state = GameState::PLAYING;
                if (soundEnabled) swooshSnd.play();
            }
        }

        if (ev.type == sf::Event::KeyPressed) {
            auto key = ev.key.code;
            if (state == GameState::MENU) {
                if (key == sf::Keyboard::Up   || key == sf::Keyboard::W)
                    selectedDifficulty = (selectedDifficulty + 2) % 3;
                if (key == sf::Keyboard::Down || key == sf::Keyboard::S)
                    selectedDifficulty = (selectedDifficulty + 1) % 3;
                if (key == sf::Keyboard::Return || key == sf::Keyboard::Space) {
                    setDifficulty(static_cast<Difficulty>(selectedDifficulty));
                    resetGame();
                    state = GameState::PLAYING;
                    if (soundEnabled) swooshSnd.play();
                }
            } else if (state == GameState::PLAYING) {
                if (key == sf::Keyboard::Space && !bird.isDead()) {
                    bird.flap();
                    if (soundEnabled) flapSnd.play();
                }
                if (key == sf::Keyboard::Escape) state = GameState::PAUSED;
            } else if (state == GameState::PAUSED) {
                if (key == sf::Keyboard::Escape) state = GameState::PLAYING;
                if (key == sf::Keyboard::M)      state = GameState::MENU;
            } else if (state == GameState::GAMEOVER) {
                if (key == sf::Keyboard::Space) {
                    resetGame();
                    state = GameState::PLAYING;
                    if (soundEnabled) swooshSnd.play();
                }
                if (key == sf::Keyboard::M) state = GameState::MENU;
            }
        }
    }
}

void Game::handleResize() {
    windowWidth  = window.getSize().x;
    windowHeight = window.getSize().y;
    updateScaling();
    window.setView(sf::View(sf::FloatRect(0, 0,
        static_cast<float>(windowWidth),
        static_cast<float>(windowHeight))));
}

// ─── Update ───────────────────────────────────────────────────────────────
void Game::update(float dt) {
    updateSky(dt);
    updateClouds(dt);
    updateStars(dt);
    updateMountains(dt);
    updateBackgroundBirds(dt);

    if (rippleAlpha > 0.f) { rippleAlpha -= 200.f*dt; if (rippleAlpha<0.f) rippleAlpha=0.f; }
    if (flashAlpha  > 0.f) { flashAlpha  -= 400.f*dt; if (flashAlpha <0.f) flashAlpha =0.f; }
    if (scenarioLabelTimer > 0.f) scenarioLabelTimer -= dt;

    if (state == GameState::MENU)     updateMenu(dt);
    if (state == GameState::PLAYING)  updatePlaying(dt);
    if (state == GameState::GAMEOVER) updateGameOver(dt);
}

void Game::updateMenu(float dt) {
    bird.update(dt * 0.f);
    bird.animateWing(dt);
    for (int i = 0; i < 2; ++i) {
        groundX[i] -= groundSpeed * 0.5f * dt;
        if (groundX[i] <= -windowWidth) groundX[i] += windowWidth * 2.f;
    }
}

void Game::updatePlaying(float dt) {
    bird.update(dt);

    for (int i = 0; i < 2; ++i) {
        groundX[i] -= groundSpeed * dt;
        if (groundX[i] <= -windowWidth) groundX[i] += windowWidth * 2.f;
    }

    pipeSpawnTimer += dt;
    if (pipeSpawnTimer >= pipeSpawnInterval) {
        pipeSpawnTimer = 0.f;
        spawnPipe();
    }

    for (auto& p : pipes) {
        p.update(dt);
        if (p.hasPassed(bird.getX())) {
            p.markPassed();
            score++;
            if (score > highScore) highScore = score;
            if (soundEnabled) scoreSnd.play();

            if (score % 10 == 0) {
                Pipe::SPEED       = std::min(Pipe::SPEED + 5.f, 320.f);
                groundSpeed       = Pipe::SPEED;
                Pipe::GAP         = std::max(Pipe::GAP   - 3.f, 110.f);
                pipeSpawnInterval = std::max(pipeSpawnInterval - 0.05f, 1.5f);
            }
            // celebrate is now visual-only — does NOT block input
            if (score % 5 == 0) bird.celebrate();
        }
    }

    pipes.erase(std::remove_if(pipes.begin(), pipes.end(),
        [](const Pipe& p){ return p.isOffScreen(); }), pipes.end());

    sf::FloatRect bb = bird.getBounds();
    for (auto& p : pipes) {
        if (p.checkCollision(bb) && !bird.isDead()) {
            bird.die(); if (soundEnabled) dieSnd.play(); flashAlpha = 255.f;
        }
    }
    if (bird.getY() < 0.f && !bird.isDead()) {
        bird.die(); if (soundEnabled) dieSnd.play(); flashAlpha = 255.f;
    }
    float groundY = static_cast<float>(windowHeight) - 80.f;
    if (bird.getY() >= groundY) {
        if (!bird.isDead()) { bird.die(); if (soundEnabled) dieSnd.play(); flashAlpha = 255.f; }
        state = GameState::GAMEOVER;
        saveHighScore();
    }
}

void Game::updateGameOver(float dt) {
    bird.update(dt);
    for (int i = 0; i < 2; ++i) {
        groundX[i] -= groundSpeed * 0.3f * dt;
        if (groundX[i] <= -windowWidth) groundX[i] += windowWidth * 2.f;
    }
}

// ─── Sky Scenario System ──────────────────────────────────────────────────
sf::Color Game::lerpColor(sf::Color a, sf::Color b, float t) {
    return sf::Color(
        static_cast<sf::Uint8>(a.r + (b.r - a.r) * t),
        static_cast<sf::Uint8>(a.g + (b.g - a.g) * t),
        static_cast<sf::Uint8>(a.b + (b.b - a.b) * t), 255);
}

void Game::getScenarioColors(SkyScenario s,
                              sf::Color& top, sf::Color& mid, sf::Color& bot) {
    switch (s) {
        case SkyScenario::MORNING:
            top=sf::Color(255,160, 80); mid=sf::Color(255,200,130); bot=sf::Color(255,230,180); break;
        case SkyScenario::DAY:
            top=sf::Color( 52,152,219); mid=sf::Color(142,202,230); bot=sf::Color(220,240,255); break;
        case SkyScenario::EVENING:
            top=sf::Color(180, 80, 20); mid=sf::Color(255,140, 40); bot=sf::Color(255,200, 80); break;
        case SkyScenario::NIGHT:
            top=sf::Color( 20, 30, 70); mid=sf::Color( 30, 40, 90); bot=sf::Color( 40, 55,110); break;
        case SkyScenario::MOONLIT_NIGHT:
            top=sf::Color( 10, 20, 60); mid=sf::Color( 20, 35, 80); bot=sf::Color( 35, 55,100); break;
        case SkyScenario::BLACK_NIGHT:
            top=sf::Color(  5,  5, 15); mid=sf::Color( 10, 10, 25); bot=sf::Color( 15, 15, 40); break;
        case SkyScenario::STATS:
            top=sf::Color( 30, 10, 60); mid=sf::Color( 50, 20, 80); bot=sf::Color( 70, 30,100); break;
        case SkyScenario::MOON:
            top=sf::Color( 25, 10, 55); mid=sf::Color( 40, 20, 75); bot=sf::Color( 60, 35, 95); break;
        default:
            top=sf::Color( 52,152,219); mid=sf::Color(142,202,230); bot=sf::Color(220,240,255); break;
    }
}

bool Game::scenarioHasStars(SkyScenario s) {
    return s == SkyScenario::NIGHT        ||
           s == SkyScenario::MOONLIT_NIGHT ||
           s == SkyScenario::BLACK_NIGHT   ||
           s == SkyScenario::STATS         ||
           s == SkyScenario::MOON;
}

void Game::advanceScenario() {
    int next = (static_cast<int>(currentScenario) + 1)
             % static_cast<int>(SkyScenario::COUNT);
    currentScenario = static_cast<SkyScenario>(next);
    fromTop = sky.top; fromMid = sky.mid; fromBot = sky.bottom;
    getScenarioColors(currentScenario, toTop, toMid, toBot);
    isBlending         = true;
    blendT             = 0.f;
    scenarioTimer      = 0.f;
    scenarioLabelTimer = 2.5f;
}

void Game::updateSky(float dt) {
    if (state == GameState::PLAYING) {
        scenarioTimer += dt;
        if (scenarioTimer >= scenarioDuration) advanceScenario();
    }

    if (isBlending) {
        blendT += dt / 3.0f;
        if (blendT >= 1.f) {
            blendT = 1.f; isBlending = false;
            sky.top = toTop; sky.mid = toMid; sky.bottom = toBot;
        } else {
            sky.top    = lerpColor(fromTop, toTop, blendT);
            sky.mid    = lerpColor(fromMid, toMid, blendT);
            sky.bottom = lerpColor(fromBot, toBot, blendT);
        }
    }

    sky.timeOfDay += 0.02f * dt * 60.f;
    if (sky.timeOfDay >= 24.f) sky.timeOfDay = 0.f;
}

void Game::updateClouds(float dt) {
    cloudSpawnTimer += dt;
    if (cloudSpawnTimer > 4.f) { cloudSpawnTimer = 0.f; spawnCloud(); }
    for (auto& c : clouds) { c.x -= c.speed * dt; c.sprite.setPosition(c.x, c.y); }
    clouds.erase(std::remove_if(clouds.begin(), clouds.end(),
        [](const Cloud& c){ return c.x < -200.f; }), clouds.end());
}

void Game::updateStars(float dt) {
    for (auto& s : stars) s.twinklePhase += s.twinkleSpeed * dt;
}

void Game::updateMountains(float dt) {
    for (int layer = 0; layer < 2; ++layer)
        for (auto& m : mountains[layer]) {
            m.baseX -= m.speed * dt;
            if (m.baseX < -300.f) m.baseX += 1760.f;
            m.shape.setPosition(m.baseX, static_cast<float>(windowHeight) - 80.f);
        }
}

void Game::updateBackgroundBirds(float dt) {
    bgBirdSpawnTimer += dt;
    if (bgBirdSpawnTimer > 8.f) { bgBirdSpawnTimer = 0.f; spawnBackgroundBird(); }
    for (auto& bb : bgBirds) { bb.x -= bb.speed * dt; bb.wingPhase += 5.f * dt; }
    bgBirds.erase(std::remove_if(bgBirds.begin(), bgBirds.end(),
        [](const BackgroundBird& b){ return b.x < -50.f; }), bgBirds.end());
}

// ─── Run ──────────────────────────────────────────────────────────────────
void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = std::min(clock.restart().asSeconds(), 0.05f);
        processEvents();
        update(dt);
        render();
    }
    saveHighScore();
}