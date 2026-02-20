#include "Game.h"
#include <cmath>
#include <cstdlib>

const float PI_I = 3.14159265f;

// ─── buildSkySystem() ────────────────────────────────────────────────────
void Game::buildSkySystem() {
    // Start at MORNING
    currentScenario   = SkyScenario::MORNING;
    scenarioTimer     = 0.f;
    scenarioDuration  = 30.f;   // each scenario lasts 30 real seconds
    isBlending        = false;
    blendT            = 0.f;
    scenarioLabelTimer= 0.f;

    getScenarioColors(currentScenario, sky.top, sky.mid, sky.bottom);
    sky.timeOfDay        = 6.f;  // sunrise
    sky.transitionSpeed  = 0.01f;

    skyVertices.setPrimitiveType(sf::Quads);
    skyVertices.resize(4);
    skyVertices[0].position = sf::Vector2f(0, 0);
    skyVertices[1].position = sf::Vector2f((float)windowWidth, 0);
    skyVertices[2].position = sf::Vector2f((float)windowWidth, (float)windowHeight);
    skyVertices[3].position = sf::Vector2f(0, (float)windowHeight);

    // Sun / small moon
    celestialBody.setRadius(32.f);
    celestialBody.setOrigin(32.f, 32.f);
    celestialBody.setFillColor(sf::Color(241, 196, 15));

    celestialGlow.setRadius(50.f);
    celestialGlow.setOrigin(50.f, 50.f);
    celestialGlow.setFillColor(sf::Color(255, 200, 50, 70));

    // Large moon (MOON scenario)
    bigMoon.setRadius(52.f);
    bigMoon.setOrigin(52.f, 52.f);
    bigMoon.setFillColor(sf::Color(240, 245, 255));

    bigMoonGlow.setRadius(80.f);
    bigMoonGlow.setOrigin(80.f, 80.f);
    bigMoonGlow.setFillColor(sf::Color(220, 230, 255, 40));
}

// ─── buildClouds() ───────────────────────────────────────────────────────
void Game::createCloudTexture(int index) {
    sf::RenderTexture rt;
    if (!rt.create(140, 70)) return;
    rt.clear(sf::Color::Transparent);
    int numCircles = 6 + (std::rand() % 3);
    for (int i = 0; i < numCircles; ++i) {
        sf::CircleShape puff;
        float r = 18.f + (std::rand() % 16);
        puff.setRadius(r);
        puff.setOrigin(r, r);
        puff.setPosition(30.f + (std::rand() % 80), 30.f + (std::rand() % 20));
        int brightness = 240 + (std::rand() % 16);
        puff.setFillColor(sf::Color(brightness, brightness, brightness, 210));
        rt.draw(puff);
    }
    rt.display();
    cloudTextures[index].loadFromImage(rt.getTexture().copyToImage());
}

void Game::buildClouds() {
    for (int i = 0; i < 3; ++i) createCloudTexture(i);
    for (int i = 0; i < 5; ++i) spawnCloud();
    cloudSpawnTimer = 0.f;
}

void Game::spawnCloud() {
    Cloud c;
    c.depth  = 0.3f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.7f;
    c.speed  = 18.f * c.depth;
    c.alpha  = 140.f + 50.f * c.depth;
    int texI = std::rand() % 3;
    c.texture= &cloudTextures[texI];
    c.sprite.setTexture(*c.texture);
    c.sprite.setColor(sf::Color(255,255,255,static_cast<sf::Uint8>(c.alpha)));
    float sc = 0.6f + c.depth * 0.5f;
    c.sprite.setScale(sc, sc);
    c.x = static_cast<float>(windowWidth) + 100.f;
    c.y = 40.f + (std::rand() % (windowHeight / 2 - 80));
    c.sprite.setPosition(c.x, c.y);
    clouds.push_back(c);
}

// ─── buildStars() ────────────────────────────────────────────────────────
void Game::buildStars() {
    stars.clear();
    // 80 stars for density in BLACK_NIGHT
    for (int i = 0; i < 80; ++i) {
        Star s;
        float r = 1.f + (std::rand() % 2);
        s.shape.setRadius(r);
        s.shape.setOrigin(r, r);
        s.shape.setFillColor(sf::Color(255, 255, 255, 180));
        s.shape.setPosition(
            static_cast<float>(std::rand() % windowWidth),
            static_cast<float>(std::rand() % (windowHeight / 2)));
        s.twinklePhase = static_cast<float>(std::rand() % 100) / 10.f;
        s.twinkleSpeed = 0.5f + (static_cast<float>(std::rand()) / RAND_MAX) * 1.5f;
        stars.push_back(s);
    }
}

// ─── buildMountains() ────────────────────────────────────────────────────
void Game::buildMountains() {
    // Far layer
    for (int i = 0; i < 8; ++i) {
        Mountain m;
        m.shape.setPointCount(3);
        float baseW = 180.f + (std::rand() % 100);
        float height= 120.f + (std::rand() % 80);
        m.shape.setPoint(0, sf::Vector2f(0, 0));
        m.shape.setPoint(1, sf::Vector2f(baseW/2.f, -height));
        m.shape.setPoint(2, sf::Vector2f(baseW, 0));
        m.shape.setFillColor(sf::Color(52, 73, 94, 200));
        m.shape.setOutlineColor(sf::Color(44, 62, 80));
        m.shape.setOutlineThickness(1.f);
        m.baseX = i * 200.f - 100.f;
        m.speed = 30.f;
        m.shape.setPosition(m.baseX, static_cast<float>(windowHeight) - 80.f);
        mountains[0].push_back(m);
    }
    // Near layer
    for (int i = 0; i < 8; ++i) {
        Mountain m;
        m.shape.setPointCount(3);
        float baseW = 200.f + (std::rand() % 120);
        float height= 140.f + (std::rand() % 100);
        m.shape.setPoint(0, sf::Vector2f(0, 0));
        m.shape.setPoint(1, sf::Vector2f(baseW/2.f, -height));
        m.shape.setPoint(2, sf::Vector2f(baseW, 0));
        m.shape.setFillColor(sf::Color(127, 140, 141, 220));
        m.shape.setOutlineColor(sf::Color(95, 106, 106));
        m.shape.setOutlineThickness(1.5f);
        m.baseX = i * 220.f - 100.f;
        m.speed = 60.f;
        m.shape.setPosition(m.baseX, static_cast<float>(windowHeight) - 80.f);
        mountains[1].push_back(m);
    }
}

// ─── buildGround() ───────────────────────────────────────────────────────
void Game::buildGround() {
    ground.setSize({static_cast<float>(windowWidth * 2), 80.f});
    ground.setFillColor(sf::Color(222, 196, 119));
    ground.setOutlineColor(sf::Color(180, 140, 90));
    ground.setOutlineThickness(3.f);
    groundX[0] = 0.f;
    groundX[1] = static_cast<float>(windowWidth);

    for (int i = 0; i < 10; ++i) {
        groundDecor[i].setSize({30.f + (std::rand() % 20),
                                 4.f + (std::rand() % 3)});
        groundDecor[i].setFillColor(sf::Color(100, 200, 60, 180));
    }
}

// ─── buildUI() ───────────────────────────────────────────────────────────
void Game::buildUI() {
    float W = static_cast<float>(windowWidth);

    soundButton.bg.setSize({42.f, 28.f});
    soundButton.bg.setFillColor(sf::Color(0,0,0,120));
    soundButton.bg.setOutlineColor(sf::Color::White);
    soundButton.bg.setOutlineThickness(1.5f);
    soundButton.bg.setPosition(W - 52.f, 10.f);

    soundButton.label.setFont(fontMain);
    soundButton.label.setCharacterSize(15);
    soundButton.label.setFillColor(sf::Color::White);
    soundButton.label.setString(soundEnabled ? "ON" : "OFF");
    soundButton.label.setPosition(W - 44.f, 14.f);

    soundButton.bounds  = soundButton.bg.getGlobalBounds();
    soundButton.hovered = false;

    touchRipple.setFillColor(sf::Color::Transparent);
    touchRipple.setOutlineColor(sf::Color(255,255,255,200));
    touchRipple.setOutlineThickness(2.f);
    rippleAlpha = 0.f;
}

// ─── spawnBackgroundBird() ───────────────────────────────────────────────
void Game::spawnBackgroundBird() {
    BackgroundBird bb;
    bb.x = static_cast<float>(windowWidth) + 50.f;
    bb.y = 80.f + (std::rand() % (windowHeight / 3));
    bb.speed     = 40.f + (std::rand() % 40);
    bb.wingPhase = 0.f;
    for (int i = 0; i < 2; ++i) {
        bb.shape[i].setPointCount(3);
        bb.shape[i].setFillColor(sf::Color(80,80,80,160));
    }
    bgBirds.push_back(bb);
}