#include "Bird.h"
#include <cmath>

const float PI = 3.14159265f;

Bird::Bird(float x, float y)
    : velocity(0.f), gravity(1500.f), flapStrength(-520.f),
      rotation(0.f), state(BirdState::IDLE), wingState(WingState::NEUTRAL),
      wingTimer(0.f), wingAnimSpeed(0.08f), celebrateTimer(0.f), celebrateBounce(0.f)
{
    position = { x, y };
    createBodyShape();
    createHeadShape();
    createBeakShape();
    createWingShapes();
    createTailShape();
    createFeetShape();
    updateVisuals();
}

void Bird::createBodyShape() {
    body.setPointCount(20);
    float bodyW = 48.f, bodyH = 38.f;
    for (int i = 0; i < 20; ++i) {
        float angle = (i / 20.f) * 2.f * PI;
        body.setPoint(i, sf::Vector2f(bodyW / 2.f * std::cos(angle),
                                      bodyH / 2.f * std::sin(angle)));
    }
    body.setFillColor(sf::Color(255, 220, 50));
    body.setOutlineColor(sf::Color(220, 180, 30));
    body.setOutlineThickness(2.5f);
}

void Bird::createHeadShape() {
    head.setPointCount(16);
    float headR = 22.f;
    for (int i = 0; i < 16; ++i) {
        float angle = (i / 16.f) * 2.f * PI;
        head.setPoint(i, sf::Vector2f(headR * std::cos(angle),
                                       headR * std::sin(angle)));
    }
    head.setFillColor(sf::Color(255, 230, 70));
    head.setOutlineColor(sf::Color(220, 180, 30));
    head.setOutlineThickness(2.f);

    eye.setRadius(7.f);
    eye.setOrigin(7.f, 7.f);
    eye.setFillColor(sf::Color::White);
    eye.setOutlineColor(sf::Color(40, 40, 40));
    eye.setOutlineThickness(1.5f);

    pupil.setRadius(3.5f);
    pupil.setOrigin(3.5f, 3.5f);
    pupil.setFillColor(sf::Color(20, 20, 20));

    eyeGlint.setRadius(2.f);
    eyeGlint.setOrigin(2.f, 2.f);
    eyeGlint.setFillColor(sf::Color(255, 255, 255, 200));
}

void Bird::createBeakShape() {
    beak.setPointCount(3);
    beak.setPoint(0, sf::Vector2f(0.f, 0.f));
    beak.setPoint(1, sf::Vector2f(18.f, -3.f));
    beak.setPoint(2, sf::Vector2f(18.f, 3.f));
    beak.setFillColor(sf::Color(255, 160, 40));
    beak.setOutlineColor(sf::Color(200, 120, 20));
    beak.setOutlineThickness(1.5f);

    beakBottom.setPointCount(3);
    beakBottom.setPoint(0, sf::Vector2f(0.f, 0.f));
    beakBottom.setPoint(1, sf::Vector2f(14.f, 2.f));
    beakBottom.setPoint(2, sf::Vector2f(14.f, 6.f));
    beakBottom.setFillColor(sf::Color(240, 140, 30));
    beakBottom.setOutlineColor(sf::Color(200, 120, 20));
    beakBottom.setOutlineThickness(1.5f);
}

void Bird::createWingShapes() {
    wingLeft.setPointCount(8);
    wingLeft.setPoint(0, sf::Vector2f(0.f, 0.f));
    wingLeft.setPoint(1, sf::Vector2f(-8.f, -6.f));
    wingLeft.setPoint(2, sf::Vector2f(-22.f, -14.f));
    wingLeft.setPoint(3, sf::Vector2f(-32.f, -16.f));
    wingLeft.setPoint(4, sf::Vector2f(-34.f, -10.f));
    wingLeft.setPoint(5, sf::Vector2f(-26.f, -4.f));
    wingLeft.setPoint(6, sf::Vector2f(-12.f, 2.f));
    wingLeft.setPoint(7, sf::Vector2f(-4.f, 4.f));
    wingLeft.setFillColor(sf::Color(255, 200, 40));
    wingLeft.setOutlineColor(sf::Color(210, 160, 20));
    wingLeft.setOutlineThickness(2.f);

    wingRight.setPointCount(6);
    wingRight.setPoint(0, sf::Vector2f(0.f, 0.f));
    wingRight.setPoint(1, sf::Vector2f(8.f, -4.f));
    wingRight.setPoint(2, sf::Vector2f(18.f, -8.f));
    wingRight.setPoint(3, sf::Vector2f(20.f, -4.f));
    wingRight.setPoint(4, sf::Vector2f(14.f, 0.f));
    wingRight.setPoint(5, sf::Vector2f(4.f, 2.f));
    wingRight.setFillColor(sf::Color(245, 190, 30));
    wingRight.setOutlineColor(sf::Color(210, 160, 20));
    wingRight.setOutlineThickness(1.5f);

    feathers.clear();
    for (int i = 0; i < 4; ++i) {
        sf::ConvexShape feather;
        feather.setPointCount(4);
        feather.setFillColor(sf::Color(220, 170, 20, 180));
        feathers.push_back(feather);
    }
}

void Bird::createTailShape() {
    for (int i = 0; i < 3; ++i) {
        tailFeather[i].setPointCount(4);
        tailFeather[i].setFillColor(sf::Color(255, 200, 40));
        tailFeather[i].setOutlineColor(sf::Color(210, 160, 20));
        tailFeather[i].setOutlineThickness(1.5f);
    }
}

void Bird::createFeetShape() {
    for (int i = 0; i < 2; ++i) {
        foot[i].setPointCount(3);
        foot[i].setPoint(0, sf::Vector2f(0.f, 0.f));
        foot[i].setPoint(1, sf::Vector2f(-3.f, 8.f));
        foot[i].setPoint(2, sf::Vector2f(3.f, 8.f));
        foot[i].setFillColor(sf::Color(255, 140, 60));
        foot[i].setOutlineColor(sf::Color(200, 100, 30));
        foot[i].setOutlineThickness(1.f);
    }
}

// ─── FIX: flap now always works unless bird is DEAD ──────────────────────
void Bird::flap() {
    if (state != BirdState::DEAD) {
        // Cancel any ongoing celebration immediately
        if (state == BirdState::CELEBRATING) {
            celebrateTimer = 999.f; // force celebration to end
        }
        velocity    = flapStrength;
        state       = BirdState::FLAPPING;
        wingTimer   = 0.f;
        wingState   = WingState::UP;
    }
}

// ─── FIX: celebrate is now a SHORT visual-only effect (0.5s) ─────────────
void Bird::celebrate() {
    if (state == BirdState::DEAD) return;
    state          = BirdState::CELEBRATING;
    celebrateTimer = 0.f;
    // Do NOT change velocity — bird keeps flying normally
}

void Bird::die() {
    state     = BirdState::DEAD;
    wingState = WingState::DOWN;
}

void Bird::update(float dt) {
    updateState(dt);
    updateWingAnimation(dt);

    if (state == BirdState::CELEBRATING) {
        celebrateTimer += dt;
        celebrateBounce = std::sin(celebrateTimer * 10.f) * 4.f;

        // ─── FIX: normal physics during celebration ───────────────────
        velocity   += gravity * dt;
        position.y += velocity * dt;

        // Gentle wobble rotation
        rotation = std::sin(celebrateTimer * 6.f) * 10.f;

        // Auto-end celebration after 0.5 seconds → return to normal flight
        if (celebrateTimer >= 0.5f) {
            state    = BirdState::GLIDING;
            rotation = 0.f;
        }
    } else if (state == BirdState::DEAD) {
        velocity   += gravity * dt;
        position.y += velocity * dt;
        rotation   += 280.f * dt;
        if (rotation > 90.f) rotation = 90.f;
    } else {
        velocity   += gravity * dt;
        position.y += velocity * dt;

        float targetRot = velocity * 0.05f;
        if (targetRot < -30.f) targetRot = -30.f;
        if (targetRot >  90.f) targetRot =  90.f;

        float rotSpeed = (state == BirdState::FLAPPING) ? 20.f : 8.f;
        rotation += (targetRot - rotation) * rotSpeed * dt;
    }

    updateVisuals();
}

void Bird::updateState(float dt) {
    // ─── FIX: allow state transitions out of CELEBRATING ─────────────
    if (state == BirdState::DEAD) return;
    if (state == BirdState::CELEBRATING) return; // handled in update()

    if (state == BirdState::FLAPPING) {
        wingTimer += dt;
        if (wingTimer > 0.15f) state = BirdState::GLIDING;
    } else if (state == BirdState::GLIDING) {
        if (velocity > 100.f) state = BirdState::FALLING;
    } else if (state == BirdState::FALLING) {
        if (velocity < 0.f) state = BirdState::GLIDING;
    }
}

void Bird::updateWingAnimation(float dt) {
    wingTimer += dt;

    if (state == BirdState::CELEBRATING) {
        if (wingTimer > wingAnimSpeed * 0.5f) {
            wingTimer = 0.f;
            wingState = (wingState == WingState::UP) ? WingState::DOWN : WingState::UP;
        }
    } else if (state == BirdState::FLAPPING) {
        if      (wingTimer < 0.05f) wingState = WingState::UP;
        else if (wingTimer < 0.10f) wingState = WingState::MID;
        else                        wingState = WingState::DOWN;
    } else if (state == BirdState::GLIDING) {
        wingState = WingState::NEUTRAL;
    } else if (state == BirdState::FALLING) {
        if (wingTimer > wingAnimSpeed * 2.f) wingTimer = 0.f;
        wingState = (wingTimer < wingAnimSpeed) ? WingState::MID : WingState::NEUTRAL;
    } else if (state == BirdState::DEAD) {
        wingState = WingState::DOWN;
    }
}


void Bird::reset(float x, float y) {
    position       = { x, y };
    velocity       = 0.f;
    rotation       = 0.f;
    state          = BirdState::IDLE;
    wingState      = WingState::NEUTRAL;
    wingTimer      = 0.f;
    celebrateTimer = 0.f;
    celebrateBounce= 0.f;
    updateVisuals();
}

sf::FloatRect Bird::getBounds() const {
    return sf::FloatRect(position.x - 22.f, position.y - 18.f, 44.f, 36.f);
}

sf::Vector2f Bird::getRotatedOffset(float x, float y, float angle) {
    float rad   = angle * PI / 180.f;
    float cos_a = std::cos(rad);
    float sin_a = std::sin(rad);
    return sf::Vector2f(x * cos_a - y * sin_a, x * sin_a + y * cos_a);
}

void Bird::updateVisuals() {
    body.setPosition(position);
    body.setRotation(rotation);

    sf::Vector2f headOff = getRotatedOffset(18.f, -5.f, rotation);
    head.setPosition(position + headOff);
    head.setRotation(rotation);

    sf::Vector2f eyeOff = getRotatedOffset(26.f, -10.f, rotation);
    eye.setPosition(position + eyeOff);
    pupil.setPosition(eye.getPosition() + sf::Vector2f(2.f, 0.5f));
    eyeGlint.setPosition(pupil.getPosition() + sf::Vector2f(-1.5f, -1.5f));

    sf::Vector2f beakOff = getRotatedOffset(32.f, -5.f, rotation);
    beak.setPosition(position + beakOff);
    beak.setRotation(rotation);

    sf::Vector2f beakBotOff = getRotatedOffset(32.f, 0.f, rotation);
    beakBottom.setPosition(position + beakBotOff);
    beakBottom.setRotation(rotation);

    float wingAngleLeft = 0.f, wingAngleRight = 0.f;
    sf::Vector2f wingOffsetLeft(-8.f, -2.f), wingOffsetRight(8.f, 0.f);

    switch (wingState) {
        case WingState::UP:
            wingAngleLeft = -45.f; wingAngleRight = -35.f;
            wingOffsetLeft.y -= 8.f; break;
        case WingState::MID:
            wingAngleLeft = -15.f; wingAngleRight = -10.f;
            wingOffsetLeft.y -= 4.f; break;
        case WingState::DOWN:
            wingAngleLeft = 25.f; wingAngleRight = 20.f;
            wingOffsetLeft.y += 6.f; break;
        case WingState::NEUTRAL:
            wingAngleLeft = 0.f; wingAngleRight = 0.f; break;
    }

    sf::Vector2f wlOff = getRotatedOffset(wingOffsetLeft.x, wingOffsetLeft.y, rotation);
    wingLeft.setPosition(position + wlOff);
    wingLeft.setRotation(rotation + wingAngleLeft);

    sf::Vector2f wrOff = getRotatedOffset(wingOffsetRight.x, wingOffsetRight.y, rotation);
    wingRight.setPosition(position + wrOff);
    wingRight.setRotation(rotation + wingAngleRight);

    for (int i = 0; i < 3; ++i) {
        float len = 16.f - i * 2.f;
        tailFeather[i].setPoint(0, sf::Vector2f(0.f, 0.f));
        tailFeather[i].setPoint(1, sf::Vector2f(-len * 0.4f, -2.f));
        tailFeather[i].setPoint(2, sf::Vector2f(-len, -4.f + i));
        tailFeather[i].setPoint(3, sf::Vector2f(-len * 0.6f, 2.f));

        sf::Vector2f tailOff = getRotatedOffset(-20.f, -2.f + i * 2.f, rotation);
        tailFeather[i].setPosition(position + tailOff);
        tailFeather[i].setRotation(rotation + (-25.f + i * 25.f));
    }

    if (state != BirdState::DEAD) {
        for (int i = 0; i < 2; ++i) {
            float fx = (i == 0) ? -6.f : 6.f;
            sf::Vector2f footOff = getRotatedOffset(fx, 14.f, rotation);
            foot[i].setPosition(position + footOff);
            foot[i].setRotation(rotation);
        }
    }
}

void Bird::draw(sf::RenderWindow& window) {
    for (int i = 2; i >= 0; --i) window.draw(tailFeather[i]);

    if (state != BirdState::DEAD) {
        for (int i = 0; i < 2; ++i) window.draw(foot[i]);
    }

    window.draw(wingRight);
    window.draw(body);
    window.draw(wingLeft);
    window.draw(head);
    window.draw(beakBottom);
    window.draw(beak);
    window.draw(eye);
    window.draw(pupil);
    window.draw(eyeGlint);
}