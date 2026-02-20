#include "Game.h"
#include <cmath>
#include <sstream>
#include <iomanip>

const float PI_R = 3.14159265f;

// ─── Main Render ──────────────────────────────────────────────────────────
void Game::render() {
    window.clear();

    renderSky();
    renderCelestial();
    if (scenarioHasStars(currentScenario)) renderStars();
    renderMountains();
    renderClouds();
    renderBackgroundBirds();
    renderGround();

    if (state == GameState::PLAYING  ||
        state == GameState::GAMEOVER ||
        state == GameState::PAUSED) {
        for (auto& p : pipes) p.draw(window);
    }

    bird.draw(window);

    // White flash on death
    if (flashAlpha > 0.f) {
        sf::RectangleShape flash;
        flash.setSize({static_cast<float>(windowWidth), static_cast<float>(windowHeight)});
        flash.setFillColor(sf::Color(255,255,255, static_cast<sf::Uint8>(flashAlpha)));
        window.draw(flash);
    }

    if (state == GameState::MENU)     renderMenu();
    if (state == GameState::PLAYING)  { renderHUD(); renderScenarioLabel(); }
    if (state == GameState::PAUSED)   { renderHUD(); renderPause(); }
    if (state == GameState::GAMEOVER) renderGameOver();

    renderTouchIndicator();
    window.display();
}

// ─── Sky ──────────────────────────────────────────────────────────────────
void Game::renderSky() {
    float midY = windowHeight * 0.45f;
    sf::VertexArray sky3(sf::Quads, 8);

    sky3[0] = sf::Vertex(sf::Vector2f(0, 0),                              sky.top);
    sky3[1] = sf::Vertex(sf::Vector2f((float)windowWidth, 0),             sky.top);
    sky3[2] = sf::Vertex(sf::Vector2f((float)windowWidth, midY),          sky.mid);
    sky3[3] = sf::Vertex(sf::Vector2f(0, midY),                           sky.mid);

    sky3[4] = sf::Vertex(sf::Vector2f(0, midY),                           sky.mid);
    sky3[5] = sf::Vertex(sf::Vector2f((float)windowWidth, midY),          sky.mid);
    sky3[6] = sf::Vertex(sf::Vector2f((float)windowWidth, (float)windowHeight), sky.bottom);
    sky3[7] = sf::Vertex(sf::Vector2f(0, (float)windowHeight),            sky.bottom);

    window.draw(sky3);
}

// ─── Celestial ────────────────────────────────────────────────────────────
void Game::renderCelestial() {
    // Arc position driven by timeOfDay
    float t     = sky.timeOfDay / 24.f;
    float angle = t * 2.f * PI_R - (PI_R / 2.f);
    float cx    = windowWidth * 0.5f;
    float cy    = windowHeight * 0.75f;
    float ar    = windowWidth * 0.55f;
    float x     = cx + ar * std::cos(angle);
    float y     = cy + ar * std::sin(angle);

    if (currentScenario == SkyScenario::MOON) {
        // Large central moon for MOON scenario
        float mx = windowWidth  * 0.65f;
        float my = windowHeight * 0.25f;

        sf::CircleShape glow(80.f);
        glow.setOrigin(80.f, 80.f);
        glow.setPosition(mx, my);
        glow.setFillColor(sf::Color(220, 230, 255, 40));
        window.draw(glow);

        sf::CircleShape moon(52.f);
        moon.setOrigin(52.f, 52.f);
        moon.setPosition(mx, my);
        moon.setFillColor(sf::Color(240, 245, 255));
        moon.setOutlineColor(sf::Color(200, 215, 240, 120));
        moon.setOutlineThickness(4.f);
        window.draw(moon);

        // Craters
        struct Crater { float ox, oy, r; };
        Crater craters[] = {{-18,8,6},{10,-15,4},{20,12,5},{-5,-5,3}};
        for (auto& cr : craters) {
            sf::CircleShape c(cr.r);
            c.setOrigin(cr.r, cr.r);
            c.setPosition(mx + cr.ox, my + cr.oy);
            c.setFillColor(sf::Color(200, 208, 228));
            window.draw(c);
        }
        return; // skip normal sun/moon arc for MOON scenario
    }

    if (currentScenario == SkyScenario::MOONLIT_NIGHT ||
        currentScenario == SkyScenario::BLACK_NIGHT   ||
        currentScenario == SkyScenario::NIGHT         ||
        currentScenario == SkyScenario::STATS) {
        // Moon
        celestialBody.setFillColor(sf::Color(236, 240, 241));
        celestialGlow.setFillColor(sf::Color(200, 220, 255, 55));
    } else {
        // Sun
        celestialBody.setFillColor(sf::Color(241, 196, 15));
        celestialGlow.setFillColor(sf::Color(255, 220, 80, 70));
    }

    celestialGlow.setPosition(x, y);
    celestialBody.setPosition(x, y);
    if (y < windowHeight - 80.f) {
        window.draw(celestialGlow);
        window.draw(celestialBody);
    }
}

// ─── Stars ────────────────────────────────────────────────────────────────
void Game::renderStars() {
    // More stars for BLACK_NIGHT
    float densityMul = (currentScenario == SkyScenario::BLACK_NIGHT) ? 1.f : 0.6f;

    for (size_t i = 0; i < stars.size(); ++i) {
        if (currentScenario != SkyScenario::BLACK_NIGHT && i % 2 == 0) continue;
        auto& s = stars[i];
        float alpha = (80.f + 120.f * std::sin(s.twinklePhase)) * densityMul;
        sf::Color c = s.shape.getFillColor();
        c.a = static_cast<sf::Uint8>(std::min(255.f, alpha));
        s.shape.setFillColor(c);
        window.draw(s.shape);
    }
}

// ─── Mountains ────────────────────────────────────────────────────────────
void Game::renderMountains() {
    // Tint mountains based on scenario
    sf::Color farColor  = sf::Color(52, 73, 94, 200);
    sf::Color nearColor = sf::Color(127, 140, 141, 220);

    if (currentScenario == SkyScenario::MORNING || currentScenario == SkyScenario::EVENING) {
        farColor  = sf::Color(120, 60, 30, 180);
        nearColor = sf::Color(160, 90, 50, 210);
    } else if (scenarioHasStars(currentScenario)) {
        farColor  = sf::Color(20, 30, 55, 200);
        nearColor = sf::Color(30, 45, 75, 220);
    }

    for (auto& m : mountains[0]) {
        auto shape = m.shape;
        shape.setFillColor(farColor);
        window.draw(shape);
    }
    for (auto& m : mountains[1]) {
        auto shape = m.shape;
        shape.setFillColor(nearColor);
        window.draw(shape);
    }
}

// ─── Clouds ───────────────────────────────────────────────────────────────
void Game::renderClouds() {
    std::sort(clouds.begin(), clouds.end(),
        [](const Cloud& a, const Cloud& b){ return a.depth < b.depth; });
    for (auto& c : clouds) window.draw(c.sprite);
}

// ─── Background Birds ─────────────────────────────────────────────────────
void Game::renderBackgroundBirds() {
    for (auto& bb : bgBirds) {
        float wAngle = std::sin(bb.wingPhase) * 20.f;
        bb.shape[0].setPoint(0, sf::Vector2f(0,0));
        bb.shape[0].setPoint(1, sf::Vector2f(-8, -5+wAngle));
        bb.shape[0].setPoint(2, sf::Vector2f(-6, 0));
        bb.shape[0].setPosition(bb.x, bb.y);
        bb.shape[1].setPoint(0, sf::Vector2f(0,0));
        bb.shape[1].setPoint(1, sf::Vector2f(8, -5+wAngle));
        bb.shape[1].setPoint(2, sf::Vector2f(6, 0));
        bb.shape[1].setPosition(bb.x, bb.y);
        window.draw(bb.shape[0]);
        window.draw(bb.shape[1]);
    }
}

// ─── Ground ───────────────────────────────────────────────────────────────
void Game::renderGround() {
    float groundY = static_cast<float>(windowHeight) - 80.f;

    // Ground colour changes with scenario
    sf::Color groundCol = sf::Color(222, 196, 119);
    sf::Color grassCol  = sf::Color(100, 200, 60);
    if (scenarioHasStars(currentScenario)) {
        groundCol = sf::Color(160, 140,  80);
        grassCol  = sf::Color( 50, 120,  30);
    }
    ground.setFillColor(groundCol);

    for (int i = 0; i < 2; ++i) {
        ground.setPosition(groundX[i], groundY);
        window.draw(ground);
    }
    sf::RectangleShape grass;
    grass.setSize({static_cast<float>(windowWidth * 2), 14.f});
    grass.setFillColor(grassCol);
    for (int i = 0; i < 2; ++i) {
        grass.setPosition(groundX[i], groundY);
        window.draw(grass);
    }
    for (int i = 0; i < 10; ++i) {
        float x = groundX[0] + (i * 80.f);
        if (x > windowWidth) x -= windowWidth * 2;
        groundDecor[i].setPosition(x, groundY + 8.f);
        window.draw(groundDecor[i]);
    }
}

// ─── HUD ──────────────────────────────────────────────────────────────────
void Game::renderHUD() {
    float W = static_cast<float>(windowWidth);

    // ── Score: top-centre ─────────────────────────────────────────────────
    scoreText.setString(std::to_string(score));
    sf::FloatRect sb = scoreText.getLocalBounds();
    scoreText.setOrigin(sb.left + sb.width / 2.f, sb.top);
    scoreText.setPosition(W / 2.f, 14.f);
    window.draw(scoreText);

    // ── Best: top-left ────────────────────────────────────────────────────
    hiScoreText.setString("Best: " + std::to_string(highScore));
    hiScoreText.setOrigin(0.f, 0.f);
    hiScoreText.setPosition(10.f, 12.f);
    window.draw(hiScoreText);

    // ── Difficulty badge: below best ──────────────────────────────────────
    const char* dStr[] = {"EASY","NORMAL","HARD"};
    sf::Color   dCol[] = {sf::Color(80,255,80), sf::Color(255,255,80), sf::Color(255,80,80)};
    sf::Text diffBadge;
    diffBadge.setFont(fontMain);
    diffBadge.setCharacterSize(13);
    diffBadge.setFillColor(dCol[static_cast<int>(difficulty)]);
    diffBadge.setOutlineColor(sf::Color(20,20,20));
    diffBadge.setOutlineThickness(1.5f);
    diffBadge.setString(dStr[static_cast<int>(difficulty)]);
    diffBadge.setPosition(10.f, 34.f);
    window.draw(diffBadge);

    // ── Sound button: top-right ───────────────────────────────────────────
    soundButton.bg.setPosition(W - 52.f, 10.f);
    soundButton.label.setPosition(W - 44.f, 15.f);
    window.draw(soundButton.bg);
    window.draw(soundButton.label);
}

// ─── Scenario Label ───────────────────────────────────────────────────────
void Game::renderScenarioLabel() {
    if (scenarioLabelTimer <= 0.f) return;

    const char* names[] = {
        "Morning", "Day", "Evening", "Night",
        "Moonlit Night", "Black Night", "Stats", "Moon"
    };
    int idx = static_cast<int>(currentScenario);
    if (idx < 0 || idx >= static_cast<int>(SkyScenario::COUNT)) return;

    float alpha = std::min(1.f, scenarioLabelTimer); // fade out in last 1s
    sf::Uint8 a = static_cast<sf::Uint8>(255 * alpha);

    sf::Text label;
    label.setFont(fontMain);
    label.setCharacterSize(22);
    label.setFillColor(sf::Color(255, 240, 180, a));
    label.setOutlineColor(sf::Color(60, 40, 0, a));
    label.setOutlineThickness(2.f);
    label.setStyle(sf::Text::Bold);
    label.setString(std::string("~ ") + names[idx] + " ~");

    sf::FloatRect lb = label.getLocalBounds();
    label.setOrigin(lb.left + lb.width / 2.f, lb.top);
    label.setPosition(static_cast<float>(windowWidth) / 2.f, 72.f);
    window.draw(label);
}

// ─── Menu ─────────────────────────────────────────────────────────────────
void Game::renderMenu() {
    float W  = static_cast<float>(windowWidth);
    float H  = static_cast<float>(windowHeight);
    float cX = W / 2.f;
    float cY = H / 2.f;

    // Semi-transparent panel
    sf::RectangleShape panel;
    panel.setSize({360.f, 400.f});
    panel.setOrigin(180.f, 200.f);
    panel.setPosition(cX, cY);
    panel.setFillColor(sf::Color(0,0,0,150));
    panel.setOutlineColor(sf::Color(255,220,50,220));
    panel.setOutlineThickness(3.f);
    window.draw(panel);

    // Title — centred, 40px above panel centre
    sf::FloatRect tb = titleText.getLocalBounds();
    titleText.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
    titleText.setPosition(cX, cY - 155.f);
    window.draw(titleText);

    // Best score
    std::string hss = "Best: " + std::to_string(highScore);
    hiScoreText.setString(hss);
    sf::FloatRect hb = hiScoreText.getLocalBounds();
    hiScoreText.setOrigin(hb.left + hb.width/2.f, hb.top);
    hiScoreText.setPosition(cX, cY - 95.f);
    window.draw(hiScoreText);

    // Difficulty selector heading
    sf::Text dtitle;
    dtitle.setFont(fontMain);
    dtitle.setCharacterSize(20);
    dtitle.setFillColor(sf::Color(210,210,210));
    dtitle.setString("Select Difficulty:");
    sf::FloatRect dt = dtitle.getLocalBounds();
    dtitle.setOrigin(dt.left + dt.width/2.f, dt.top);
    dtitle.setPosition(cX, cY - 55.f);
    window.draw(dtitle);

    // Difficulty options
    for (int i = 0; i < 3; ++i) {
        bool sel = (i == selectedDifficulty);
        diffText[i].setCharacterSize(sel ? 32 : 25);
        diffText[i].setOutlineThickness(sel ? 3.f : 1.5f);
        sf::FloatRect fr = diffText[i].getLocalBounds();
        diffText[i].setOrigin(fr.left + fr.width/2.f, fr.top + fr.height/2.f);
        diffText[i].setPosition(cX, cY - 5.f + i * 46.f);

        if (sel) {
            sf::Text arrow;
            arrow.setFont(fontMain);
            arrow.setCharacterSize(22);
            arrow.setFillColor(sf::Color::White);
            arrow.setString(">>>");
            sf::FloatRect ar = arrow.getLocalBounds();
            arrow.setOrigin(0.f, ar.top + ar.height/2.f);
            arrow.setPosition(cX - 110.f, cY - 5.f + i * 46.f);
            window.draw(arrow);
        }
        window.draw(diffText[i]);
    }

    // Instructions
    sf::FloatRect ib = instructText.getLocalBounds();
    instructText.setOrigin(ib.left + ib.width/2.f, ib.top);
    instructText.setPosition(cX, cY + 148.f);
    window.draw(instructText);

    // Controls hint
    sf::Text ctrl;
    ctrl.setFont(fontMain);
    ctrl.setCharacterSize(15);
    ctrl.setFillColor(sf::Color(190,190,190));
    ctrl.setString("W/S or Arrow Keys  |  ENTER to start");
    sf::FloatRect cr = ctrl.getLocalBounds();
    ctrl.setOrigin(cr.left + cr.width/2.f, cr.top);
    ctrl.setPosition(cX, cY + 175.f);
    window.draw(ctrl);
}

// ─── Pause ────────────────────────────────────────────────────────────────
void Game::renderPause() {
    sf::RectangleShape overlay;
    overlay.setSize({static_cast<float>(windowWidth), static_cast<float>(windowHeight)});
    overlay.setFillColor(sf::Color(0,0,0,140));
    window.draw(overlay);

    float cX = windowWidth  / 2.f;
    float cY = windowHeight / 2.f;

    sf::FloatRect pb = pauseText.getLocalBounds();
    pauseText.setOrigin(pb.left + pb.width/2.f, pb.top + pb.height/2.f);
    pauseText.setPosition(cX, cY - 55.f);
    window.draw(pauseText);

    sf::Text resume;
    resume.setFont(fontMain);
    resume.setCharacterSize(22);
    resume.setFillColor(sf::Color::White);
    resume.setOutlineColor(sf::Color(40,40,40));
    resume.setOutlineThickness(2.f);
    resume.setString("ESC to Resume\nM for Menu");
    sf::FloatRect rb = resume.getLocalBounds();
    resume.setOrigin(rb.left + rb.width/2.f, rb.top);
    resume.setPosition(cX, cY + 10.f);
    window.draw(resume);
}

// ─── Game Over ────────────────────────────────────────────────────────────
void Game::renderGameOver() {
    float cX = windowWidth  / 2.f;
    float cY = windowHeight / 2.f;

    sf::RectangleShape panel;
    panel.setSize({350.f, 340.f});
    panel.setOrigin(175.f, 170.f);
    panel.setPosition(cX, cY);
    panel.setFillColor(sf::Color(0,0,0,170));
    panel.setOutlineColor(sf::Color(255,80,80,240));
    panel.setOutlineThickness(3.f);
    window.draw(panel);

    // GAME OVER title
    sf::FloatRect gb = gameoverText.getLocalBounds();
    gameoverText.setOrigin(gb.left + gb.width/2.f, gb.top + gb.height/2.f);
    gameoverText.setPosition(cX, cY - 128.f);
    window.draw(gameoverText);

    // Score
    sf::Text sc;
    sc.setFont(fontMain);
    sc.setCharacterSize(36);
    sc.setFillColor(sf::Color::White);
    sc.setOutlineColor(sf::Color(40,40,40));
    sc.setOutlineThickness(2.f);
    sc.setString("Score: " + std::to_string(score));
    sf::FloatRect scb = sc.getLocalBounds();
    sc.setOrigin(scb.left + scb.width/2.f, scb.top);
    sc.setPosition(cX, cY - 72.f);
    window.draw(sc);

    // Best
    sf::Text best;
    best.setFont(fontMain);
    best.setCharacterSize(26);
    best.setFillColor(sf::Color(255,230,80));
    best.setOutlineColor(sf::Color(80,50,0));
    best.setOutlineThickness(2.f);
    best.setString("Best: " + std::to_string(highScore));
    sf::FloatRect bb = best.getLocalBounds();
    best.setOrigin(bb.left + bb.width/2.f, bb.top);
    best.setPosition(cX, cY - 24.f);
    window.draw(best);

    // Medal
    sf::CircleShape medal(26.f);
    medal.setOrigin(26.f, 26.f);
    medal.setPosition(cX, cY + 48.f);
    if      (score >= 30) medal.setFillColor(sf::Color(255,215,0));
    else if (score >= 15) medal.setFillColor(sf::Color(192,192,192));
    else if (score >= 5)  medal.setFillColor(sf::Color(205,127,50));
    else                  medal.setFillColor(sf::Color(80,80,80));
    medal.setOutlineColor(sf::Color(60,40,0));
    medal.setOutlineThickness(2.5f);
    window.draw(medal);

    sf::Text mt;
    mt.setFont(fontMain);
    mt.setCharacterSize(13);
    mt.setFillColor(sf::Color(30,30,30));
    mt.setStyle(sf::Text::Bold);
    if      (score >= 30) mt.setString("GOLD");
    else if (score >= 15) mt.setString("SILVER");
    else if (score >= 5)  mt.setString("BRONZE");
    else                  mt.setString("----");
    sf::FloatRect mtb = mt.getLocalBounds();
    mt.setOrigin(mtb.left + mtb.width/2.f, mtb.top + mtb.height/2.f);
    mt.setPosition(cX, cY + 46.f);
    window.draw(mt);

    // Restart
    sf::FloatRect rb = restartText.getLocalBounds();
    restartText.setOrigin(rb.left + rb.width/2.f, rb.top);
    restartText.setPosition(cX, cY + 92.f);
    window.draw(restartText);
}

// ─── Touch Indicator ──────────────────────────────────────────────────────
void Game::renderTouchIndicator() {
    if (rippleAlpha > 0.f && platform == PlatformMode::MOBILE) {
        touchRipple.setRadius(rippleAlpha * 0.4f);
        touchRipple.setOrigin(rippleAlpha * 0.4f, rippleAlpha * 0.4f);
        touchRipple.setPosition(lastTouchPos);
        sf::Uint8 a = static_cast<sf::Uint8>(200.f * (1.f - rippleAlpha / 100.f));
        touchRipple.setOutlineColor(sf::Color(255,255,255,a));
        window.draw(touchRipple);
    }
}