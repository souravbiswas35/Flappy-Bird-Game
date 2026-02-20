#pragma once
#include <SFML/Graphics.hpp>

class Pipe {
public:
    static float GAP;       // gap between top and bottom pipe
    static float WIDTH;
    static float SPEED;

    Pipe(float x, float gapY, float windowHeight);

    void update(float dt);
    void draw(sf::RenderWindow& window);
    bool isOffScreen() const;
    bool checkCollision(const sf::FloatRect& birdBounds) const;
    bool hasPassed(float birdX) const;
    void markPassed() { passed = true; }
    float getX() const { return x; }

private:
    float x, gapY, windowHeight;
    bool  passed;

    sf::RectangleShape topBody, topCap;
    sf::RectangleShape botBody, botCap;

    void updateShapes();
};
