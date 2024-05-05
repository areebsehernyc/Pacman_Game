#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include<iostream>
using namespace std;
using namespace sf;

// Define a class for the Map (T shape and single Cyan outline around the window)
class Map {
public:
    RectangleShape border; // Single RectangleShape for the border
    RectangleShape top;
    RectangleShape left;

    Map(float windowWidth, float windowHeight, float outlineThickness) {
        // Create a single border around the window
        border.setSize(Vector2f(windowWidth - 2 * outlineThickness, windowHeight - 2 * outlineThickness));
        border.setFillColor(Color::Transparent);
        border.setOutlineThickness(-outlineThickness); // Negative value for inner outline
        border.setOutlineColor(Color::Cyan);
        border.setPosition(outlineThickness, outlineThickness);

        float topWidth = 400; // Increased width for a longer top bar of T
        float topHeight = 70;
        float leftWidth = 70;
        float leftHeight = 300;

        // Configure T shape with cyan outline and black fill
        top.setSize(Vector2f(topWidth, topHeight));
        top.setFillColor(Color::Black);
        top.setOutlineThickness(-outlineThickness); // Negative value for inner outline
        top.setOutlineColor(Color::Cyan);
        top.setPosition((windowWidth - topWidth) / 2, (windowHeight - (topHeight + leftHeight)) / 2);

        left.setSize(Vector2f(leftWidth, leftHeight));
        left.setFillColor(Color::Black);
        left.setOutlineThickness(-outlineThickness); // Negative value for inner outline
        left.setOutlineColor(Color::Cyan);
        left.setPosition((windowWidth - leftWidth) / 2, top.getPosition().y + top.getSize().y);
    }

    void draw(RenderWindow& window) {
        window.draw(border); // Draw the border first
        window.draw(top);
        window.draw(left);
    }
public:
    bool collidesWith(const FloatRect& bounds) const {
        // Check for collision with the T shape and outline.
        if (top.getGlobalBounds().intersects(bounds) || left.getGlobalBounds().intersects(bounds)) {
            return true;
        }

        // Check if bounds are outside the window border
        FloatRect borderBounds = border.getGlobalBounds();
        return !borderBounds.contains(bounds.left, bounds.top) ||
            !borderBounds.contains(bounds.left + bounds.width, bounds.top + bounds.height);
    }

};
class Pacman : public Drawable {
    // Function to get the sprite rotation based on the current direction
    float getRotation() const {
        if (direction.x > 0) return 0;       // Right
        if (direction.x < 0) return 180;     // Left
        if (direction.y > 0) return 90;      // Down
        if (direction.y < 0) return -90;     // Up
        return sprite.getRotation(); // No change in direction
    }
public:
    Pacman(const Texture& texture, const Vector2f& startPosition, float animationSpeed)
        : sprite(texture, IntRect(0, 0, 40, 40)), position(startPosition),
        speed(215.0f), animationTimer(0.0f), numFrames(5), currentFrame(0),
        frameWidth(50), openMouth(true), animationSpeed(animationSpeed) {
        sprite.setOrigin(20, 20); // Assuming 40x40 size frames
        sprite.setPosition(startPosition);
    }

    FloatRect getBoundingBox() const {
        return sprite.getGlobalBounds();
    }

    void update(float dt, const Map& map) {
        direction = Vector2f(); // Placeholder for the direction based on user input
        // Add code here for input handling and setting the direction variable based on arrow keys

        // Example movement update (to be replaced with proper keyboard handling)
        if (Keyboard::isKeyPressed(Keyboard::Left)) {
            direction.x = -1;
        }
        else if (Keyboard::isKeyPressed(Keyboard::Right)) {
            direction.x = 1;
        }
        else if (Keyboard::isKeyPressed(Keyboard::Up)) {
            direction.y = -1;
        }
        else if (Keyboard::isKeyPressed(Keyboard::Down)) {
            direction.y = 1;
        }

        Vector2f movement = direction * speed * dt;


        // Attempt horizontal movement
        sprite.move(movement.x, 0);
        if (map.collidesWith(sprite.getGlobalBounds())) {
            sprite.move(-movement.x, 0); // Undo movement
        }

        // Attempt vertical movement
        sprite.move(0, movement.y);
        if (map.collidesWith(sprite.getGlobalBounds())) {
            sprite.move(0, -movement.y); // Undo movement
        }


        // Animation
        animationTimer += dt;
        if (animationTimer >= animationSpeed) {
            animationTimer = 0.0f;

            // Logic to loop pacman frames, assuming frames are in a single row
            if (openMouth && currentFrame == numFrames - 1) {
                openMouth = false;
            }
            else if (!openMouth && currentFrame == 0) {
                openMouth = true;
            }

            currentFrame = openMouth ? currentFrame + 1 : currentFrame - 1;
            sprite.setTextureRect(IntRect(currentFrame * frameWidth, 0, frameWidth, 40));
        }
        sprite.setRotation(getRotation());
    }

private:
    virtual void draw(RenderTarget& target, RenderStates states) const override {
        target.draw(sprite, states);
    }

    Sprite sprite;
    Vector2f position;
    Vector2f direction;
    float speed;
    float animationTimer;
    float animationSpeed;
    int numFrames;
    int currentFrame;
    int frameWidth;
    bool openMouth;
};
class Coin {
private:
    Sprite sprite;

public:
    Coin(const Texture& texture) {
        sprite.setTexture(texture);
        // Set the origin to the center of the coin if necessary. Adjust the scale if the coin is too big.
        // sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
    }

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }

    FloatRect getGlobalBounds() const {
        return sprite.getGlobalBounds();
    }

    void draw(RenderWindow& window) const {
        window.draw(sprite);
    }
};

int main() {
    RenderWindow window(VideoMode(800, 600), "Pacman Game - Map");
    const float outlineThickness = 7;  // The thickness of the outline (border)
    const float pacmanRadius = 20;     // Assuming frameWidth/2 is the radius
    const float small_offset = 0.10f;  // A small offset to ensure not touching the outline


    // Initialize the map with an outline (border) thickness
    Map map(window.getSize().x, window.getSize().y, outlineThickness);


    Texture pacmanTexture;
    if (!pacmanTexture.loadFromFile("Pacman_Spritesheet.png")) {
        cerr << "Failed to load Pacman spritesheet!" << endl;
        return EXIT_FAILURE;
    }

    // Calculate starting position for Pacman inside the outline
 // and without touching it once it's centered on its new position
    Vector2f pacmanStartPosition(
        outlineThickness + pacmanRadius + small_offset, // X position inside the left border
        window.getSize().y - outlineThickness - pacmanRadius - small_offset // Y position above the bottom border
    );

    Pacman pacman(pacmanTexture, pacmanStartPosition, 0.05f); // Animation speed as needed

    Texture coinTexture;
    if (!coinTexture.loadFromFile("Coin.png")) {
        cerr << "Failed to load coin texture!" << endl;
        return EXIT_FAILURE;
    }
    
    // Define the size of the coins based on the texture dimensions
    Vector2f coinSize(coinTexture.getSize());

    // Create vector to hold Coin objects
    vector<Coin> coins;

    // Slightly increased spacing to reduce coin density
    float spacingFactor = 1.025f;
    float coinSpacing = 67.0f * spacingFactor;

    // Correcting startX to make sure coins are not placed within the T bar on the right
    float rightTGap = map.left.getPosition().x + map.left.getSize().x + outlineThickness;

    // Calculate the starting points and adjust for spacing
    float startX = outlineThickness + coinSpacing / 2.0f;
    float startY = outlineThickness + coinSpacing / 2.0f;

    // Arrange coins considering the adjustment for the right side of T and less coin density
    for (float x = startX; x <= window.getSize().x - startX; x += coinSpacing) {
        for (float y = startY; y <= window.getSize().y - startY; y += coinSpacing) {
            // Calculate coin bounds accounting for the coin size
            FloatRect coinBounds(x - coinSize.x / 2.0f, y - coinSize.y / 2.0f, coinSize.x, coinSize.y);

            // Check if the x position is within the right gap of the T and y is not colliding with the T
            if (!map.collidesWith(coinBounds) && (x <= rightTGap || y <= map.top.getPosition().y || y >= map.top.getPosition().y + map.top.getSize().y)) {
                Coin coin(coinTexture);
                coin.setPosition(x, y);
                coins.push_back(coin);
            }
        }
    }


    // Load eat sound
    SoundBuffer eatBuffer;
    if (!eatBuffer.loadFromFile("eat.wav")) {
        cerr << "Failed to load eat sound!" << endl;
        return EXIT_FAILURE;
    }

    Sound eatSound;
    eatSound.setBuffer(eatBuffer);

    Clock clock;  // Clock to manage update timing

    // Game loop
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }
        float deltaTime = clock.restart().asSeconds();
        pacman.update(deltaTime, map);
        // Check if Pacman eats any coin and play the sound
        for (auto it = coins.begin(); it != coins.end(); ) {
            if (pacman.getBoundingBox().intersects(it->getGlobalBounds())) {
                eatSound.play(); // Play the sound
                it = coins.erase(it); // Remove the coin when eaten
            }
            else {
                ++it;
            }
        }
        // Draw calls
        window.clear(Color::Black);
        map.draw(window);
        for (const auto& coin : coins) {
            coin.draw(window);
        }
        window.draw(pacman);

        window.display();
    }

    return 0;
}