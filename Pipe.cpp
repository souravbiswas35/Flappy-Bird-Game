#include "Pipe.h"

float Pipe::GAP   = 165.f;
float Pipe::WIDTH = 72.f;
float Pipe::SPEED = 180.f;

static const float CAP_H = 28.f;

Pipe::Pipe(float x, float gapY, float windowHeight)
    : x(x), gapY(gapY), windowHeight(windowHeight), passed(false)
{
    sf::Color pipeGreen(78, 192, 46);
    sf::Color pipeOutline(40, 120, 20);
    sf::Color capGreen(58, 160, 30);

    // Top body
    topBody.setFillColor(pipeGreen);
    topBody.setOutlineColor(pipeOutline);
    topBody.setOutlineThickness(3.f);

    // Top cap
    topCap.setSize({ WIDTH + 12.f, CAP_H });
    topCap.setFillColor(capGreen);
    topCap.setOutlineColor(pipeOutline);
    topCap.setOutlineThickness(3.f);

    // Bottom body
    botBody.setFillColor(pipeGreen);
    botBody.setOutlineColor(pipeOutline);
    botBody.setOutlineThickness(3.f);

    // Bottom cap
    botCap.setSize({ WIDTH + 12.f, CAP_H });
    botCap.setFillColor(capGreen);
    botCap.setOutlineColor(pipeOutline);
    botCap.setOutlineThickness(3.f);

    updateShapes();
}

void Pipe::update(float dt) {
    x -= SPEED * dt;
    updateShapes();
}

void Pipe::draw(sf::RenderWindow& window) {
    window.draw(topBody);
    window.draw(topCap);
    window.draw(botBody);
    window.draw(botCap);
}

bool Pipe::isOffScreen() const {
    return x + WIDTH < 0.f;
}

bool Pipe::checkCollision(const sf::FloatRect& birdBounds) const {
    sf::FloatRect topRect = topBody.getGlobalBounds();
    sf::FloatRect botRect = botBody.getGlobalBounds();
    sf::FloatRect topCapRect = topCap.getGlobalBounds();
    sf::FloatRect botCapRect = botCap.getGlobalBounds();
    return birdBounds.intersects(topRect)    ||
           birdBounds.intersects(botRect)    ||
           birdBounds.intersects(topCapRect) ||
           birdBounds.intersects(botCapRect);
}

bool Pipe::hasPassed(float birdX) const {
    return !passed && (x + WIDTH < birdX);
}

void Pipe::updateShapes() {
    float topH   = gapY - GAP / 2.f;
    float botY   = gapY + GAP / 2.f;
    float botH   = windowHeight - botY;
    float capX   = x - 6.f;

    // Top body
    topBody.setSize({ WIDTH, topH });
    topBody.setPosition(x, 0.f);

    // Top cap (sits at the bottom of top pipe)
    topCap.setPosition(capX, topH - CAP_H);

    // Bottom body
    botBody.setSize({ WIDTH, botH });
    botBody.setPosition(x, botY);

    // Bottom cap (sits at the top of bottom pipe)
    botCap.setPosition(capX, botY);
}
