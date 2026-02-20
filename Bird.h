#pragma once
#include <SFML/Graphics.hpp>

enum class BirdState {
    IDLE,
    FLAPPING,
    GLIDING,
    FALLING,
    DEAD,
    CELEBRATING
};

enum class WingState {
    UP,
    MID,
    DOWN,
    NEUTRAL
};

class Bird {
public:
    Bird(float x, float y);

    void update(float dt);
    void flap();
    void draw(sf::RenderWindow& window);
    void reset(float x, float y);
    void celebrate();
    void die();

    // Public wing animation for menu idle animation
    void animateWing(float dt) {
        wingTimer += dt;
        if (wingTimer > 0.2f) {
            wingTimer = 0.f;
            int next = (static_cast<int>(wingState) + 1) % 3;
            wingState = static_cast<WingState>(next);
        }
    }

    sf::FloatRect getBounds() const;
    float getY() const { return position.y; }
    float getX() const { return position.x; }
    bool isDead() const { return state == BirdState::DEAD; }
    bool isCelebrating() const { return state == BirdState::CELEBRATING; }

private:
    sf::Vector2f position;
    float velocity;
    float gravity;
    float flapStrength;
    float rotation;
    
    BirdState state;
    WingState wingState;
    float wingTimer;
    float wingAnimSpeed;
    float celebrateTimer;
    float celebrateBounce;
    
    // Body parts with more realistic proportions
    sf::ConvexShape body;
    sf::ConvexShape head;
    sf::CircleShape eye;
    sf::CircleShape pupil;
    sf::CircleShape eyeGlint;
    sf::ConvexShape beak;
    sf::ConvexShape beakBottom;
    
    // More detailed wings
    sf::ConvexShape wingLeft;
    sf::ConvexShape wingRight;
    std::vector<sf::ConvexShape> feathers;
    
    // Tail feathers
    sf::ConvexShape tailFeather[3];
    
    // Feet
    sf::ConvexShape foot[2];
    
    void updateVisuals();
    void updateWingAnimation(float dt);
    void updateState(float dt);
    void drawRealisticBird(sf::RenderWindow& window);
    sf::Vector2f getRotatedOffset(float x, float y, float angle);
    void createBodyShape();
    void createHeadShape();
    void createBeakShape();
    void createWingShapes();
    void createTailShape();
    void createFeetShape();
};