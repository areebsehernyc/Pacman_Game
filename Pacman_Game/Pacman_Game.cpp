#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include<iostream>
#include<vector> // Include vector header for using vectors
#include<math.h> 
#include<random>
using namespace std;
using namespace sf;

class Map {
public:
    RectangleShape border; // Single RectangleShape for the border
    RectangleShape upperLine;
    RectangleShape lowerLine;
    RectangleShape leftLine;
    RectangleShape rightLine;
    // Add this member variable to the Map class to define the inner square-like shape area
    FloatRect innerSquareBounds;

    Map(float windowWidth, float windowHeight, float outlineThickness) {

        // Create a single border around the window
        border.setSize(Vector2f(windowWidth - 2 * outlineThickness, windowHeight - 2 * outlineThickness));
        border.setFillColor(Color::Transparent);
        border.setOutlineThickness(-outlineThickness);
        border.setOutlineColor(Color::Cyan);
        border.setPosition(outlineThickness, outlineThickness);

        float mapWidth = 300; // Width of the square-like shape
        float mapHeight = 300; // Height of the square-like shape
        float sideLineLength = 230; // The length of the right and left sides

        // Creating the upper line
        upperLine.setSize(Vector2f(mapWidth, outlineThickness));
        upperLine.setFillColor(Color::Cyan);
        upperLine.setPosition((windowWidth - mapWidth) / 2, (windowHeight - mapHeight) / 2);

        // Creating the lower line
        lowerLine.setSize(Vector2f(mapWidth, outlineThickness));
        lowerLine.setFillColor(Color::Cyan);
        lowerLine.setPosition((windowWidth - mapWidth) / 2, upperLine.getPosition().y + mapHeight - outlineThickness);

        // Creating the left line
        leftLine.setSize(Vector2f(outlineThickness, sideLineLength));
        leftLine.setFillColor(Color::Cyan);
        leftLine.setPosition(upperLine.getPosition().x, lowerLine.getPosition().y - sideLineLength);

        // Creating the right line
        rightLine.setSize(Vector2f(outlineThickness, sideLineLength));
        rightLine.setFillColor(Color::Cyan);
        rightLine.setPosition(upperLine.getPosition().x + upperLine.getSize().x - outlineThickness, lowerLine.getPosition().y - sideLineLength);
        innerSquareBounds = FloatRect(
            upperLine.getPosition().x - outlineThickness,       // Expand to the left
            upperLine.getPosition().y - outlineThickness,       // Expand to the top
            mapWidth + 2 * outlineThickness,                    // Expand width
            leftLine.getSize().y + 2 * outlineThickness         // Expand height
        );

    }

    void draw(RenderWindow& window) {
        window.draw(border);
        window.draw(upperLine);
        window.draw(lowerLine);
        window.draw(leftLine);
        window.draw(rightLine);
    }

    // Update the collidesWith function as necessary to include new shapes
    bool collidesWith(const FloatRect& bounds) const {
        if (upperLine.getGlobalBounds().intersects(bounds) ||
            lowerLine.getGlobalBounds().intersects(bounds) ||
            leftLine.getGlobalBounds().intersects(bounds) ||
            rightLine.getGlobalBounds().intersects(bounds)) {
            return true;
        }

        // Check if bounds are outside the window border
        FloatRect borderBounds = border.getGlobalBounds();
        return !borderBounds.contains(bounds.left, bounds.top) ||
            !borderBounds.contains(bounds.left + bounds.width, bounds.top + bounds.height);
    }
    bool innerSquareCollidesWith(const FloatRect& bounds) const {
        if (upperLine.getGlobalBounds().intersects(bounds) ||
            lowerLine.getGlobalBounds().intersects(bounds) ||
            leftLine.getGlobalBounds().intersects(bounds) ||
            rightLine.getGlobalBounds().intersects(bounds)) {
            return true;
        }
        return false;
    }
    // Add this function inside the Map class to check if a point is inside the inner square
    bool isPointInsideInnerSquare(const Vector2f& point) const {
        return innerSquareBounds.contains(point);
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
    Vector2f getPosition() const {
        return sprite.getPosition();
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
        sprite.setPosition(x + sprite.getGlobalBounds().width / 2, y + sprite.getGlobalBounds().height / 2);
    }

    FloatRect getGlobalBounds() const {
        return sprite.getGlobalBounds();
    }

    void draw(RenderWindow& window) const {
        window.draw(sprite);
    }
    Sprite& getSprite() {
        return sprite;
    }

};
typedef Coin Cherry;
class Ghost : public Drawable {
private:
    Sprite sprite;
    Vector2f position;
    Vector2f direction;
    float speed;
    bool confined; // to check if the ghost is confined within the inner square area
    static mt19937 rng;
    Vector2f startPosition;
    bool chasing;
    Vector2f targetCorner; // Target corner for each ghost
public:
    // Update the constructor to set individual speeds for each ghost
    Ghost(const Texture& texture, const Vector2f& startPosition, const Vector2f& startDirection,
        float startSpeed, bool startsConfined = false) : startPosition(startPosition),
        direction(startDirection), speed(startSpeed), confined(startsConfined) {

        int rectWidth = texture.getSize().x / 4;
        int rectHeight = texture.getSize().y;
        int shrinkFactor = 60; // Amount to reduce the dimensions by

        sprite.setTexture(texture);
        sprite.setPosition(startPosition);
        sprite.setOrigin(rectWidth, rectHeight / 2); // Origin set to half of ghost sprite
        sprite.setTextureRect(IntRect(0, 0, rectWidth - shrinkFactor, rectHeight - shrinkFactor));
        //sprite.setTextureRect(IntRect(0, 0, texture.getSize().x / 4, texture.getSize().y));
    }
    void setChasing(bool isChasing) {
        chasing = isChasing;
    }
    void setTargetCorner(const Vector2f& target) {
        targetCorner = target;
    }

    // Function to randomly change the direction to Left/Right/Up/Down
    void randomDirection() {
        uniform_int_distribution<int> dist(0, 3);
        int dir = dist(rng);
        switch (dir) {
        case 0: direction = Vector2f(-1, 0); break; // Left
        case 1: direction = Vector2f(1, 0); break;  // Right
        case 2: direction = Vector2f(0, -1); break; // Up
        case 3: direction = Vector2f(0, 1); break;  // Down
        }
    }
    // Function to allow ghosts to bounce off walls (reflect direction)
    void bounceOffWalls(const Vector2f& nextMovement, const Map& map) {
        FloatRect nextBounds(sprite.getGlobalBounds().left + nextMovement.x, sprite.getGlobalBounds().top + nextMovement.y,
            sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);
        if (map.collidesWith(nextBounds)) {
            direction = Vector2f(-direction.x, direction.y); // Reflect X
        }
        if (confined && !map.isPointInsideInnerSquare(sprite.getPosition())) {
            direction = Vector2f(direction.x, -direction.y); // Reflect Y only for confined ghost
        }
    }

    Sprite& getSprite() {
        return sprite;
    }
    Vector2f& getPosition() {
        return position;
    }

    // Update the update method to implement the bouncing logic correctly
    void update(float dt, const Vector2f& targetPosition, const Map& map, bool chase) {
        Vector2f currentTarget = chase ? targetPosition : targetCorner;
        direction = currentTarget - sprite.getPosition();
        float length = sqrt((direction.x * direction.x) + (direction.y * direction.y));
        if (length != 0) direction /= length;

        // Movement without bouncing off walls (will be handled separately)
        Vector2f movement = direction * speed * dt;
        Vector2f newPosition = sprite.getPosition() + movement;

        // Update the sprite's position without collision detection
        sprite.move(movement);

        // Restrict the ghost to inner square if it's confined
        if (confined && !map.isPointInsideInnerSquare(sprite.getPosition())) {
            sprite.setPosition(startPosition);
        }
        else {
            // Check for collision with map border and inner square
            if (map.collidesWith(sprite.getGlobalBounds()) || (confined && !map.isPointInsideInnerSquare(newPosition))) {
                direction = -direction; // Reverse the direction
                sprite.move(-movement); // Move back into bounds
            }
        }
        if (chasing) {
            // Chasing logic using the provided code sample
            Vector2f start = sprite.getPosition(); // Current ghost position
            Vector2f delta = targetPosition - start; // Direction vector to Pac-Man

            // Normalize the direction vector
            float length = sqrt((delta.x * delta.x) + (delta.y * delta.y));
            if (length > 0) {
                delta /= length;
            }

            // Use a custom speed value based on ghost speed and elapsed deltaTime
            float moveDistance = speed * dt;
            sprite.move(delta * moveDistance); // Move the ghost towards Pac-Man

            // Bounce off walls if necessary
            bounceOffWalls(delta * moveDistance, map);
        }
        else {
            // Non-chasing ghost logic
            if (!confined) {
                // Movement logic for non-confined ghost
                if (rand() % 10 == 0) {
                    randomDirection(); // Randomly change the direction sometimes
                }
            }
            // Calculate the movement for non-chasing ghosts
            Vector2f movement = direction * speed * dt;
            sprite.move(movement);

            // Check for collision with map border and inner square
            bounceOffWalls(movement, map);
            // Confined ghost logic
            if (confined) {
                if (!map.isPointInsideInnerSquare(sprite.getPosition())) {
                    sprite.setPosition(startPosition);
                }
                else {
                    if (rand() % 10 == 0) {
                        randomDirection(); // Change direction occasionally
                    }
                    // Calculate the movement for confined ghost
                    Vector2f movement = direction * speed * dt;

                    // Movement with checking of bouncing off walls
                    // This already exists in your code, so we will not repeat all of it.
                    // Just make sure there are no conflicting resets of position.

                    // Move the ghost, or bounce off walls if needed
                    bounceOffWalls(movement, map);
                    sprite.move(movement);
                }
            }
        }

        position = sprite.getPosition(); // Update the ghost's position


        // Horizontal movement
        sprite.move(movement.x, 0);
        if (map.collidesWith(sprite.getGlobalBounds()) || (confined && map.innerSquareCollidesWith(sprite.getGlobalBounds()))) {
            sprite.move(-movement.x, 0);
            direction.x = -direction.x;
        }

        // Vertical movement
        sprite.move(0, movement.y);
        if (map.collidesWith(sprite.getGlobalBounds()) || (confined && map.innerSquareCollidesWith(sprite.getGlobalBounds()))) {
            sprite.move(0, -movement.y);
            direction.y = -direction.y;
        }

        // Reset the position if the ghost is outside the inner square bounds and is confined
        if (confined && !map.isPointInsideInnerSquare(sprite.getPosition())) {
            sprite.setPosition(startPosition);
        }
        if (confined) {
            if (!map.isPointInsideInnerSquare(sprite.getPosition())) {
                sprite.setPosition(startPosition);
            }
        }
        position = sprite.getPosition();
    }
    FloatRect getBoundingBox() const {
        return sprite.getGlobalBounds();
    }
    // Add a method to check if the ghost is confined, if needed
    bool isConfined() const {
        return confined;
    }
    void draw(RenderTarget& target, RenderStates states) const override {
        target.draw(sprite, states);
    }
};

mt19937 Ghost::rng(random_device{}());

int main() {
    RenderWindow window(VideoMode(800, 600), "Pacman Game - Map");
    const float outlineThickness = 7;  // The thickness of the outline (border)
    const float pacmanRadius = 20;     // Assuming frameWidth/2 is the radius
    const float extraSpacing = 0.0001f;      // To ensure coins do not touch the border or square
    // Define the startPosition only once here if you need to use it later, for example:
    Vector2f redGhostStartPosition(200.f, 100.f);
    // Initialize the map with an outline (border) thickness
    Map map(window.getSize().x, window.getSize().y, outlineThickness);
    // Calculate start positions for each ghost, particularly for the confined ghost
    float padding = 20.0f; // Add some padding so the ghost doesn't start at the exact edge
    Vector2f innerSquareTopLeft(
        map.innerSquareBounds.left + padding,
        map.innerSquareBounds.top + padding
    );
    Vector2f innerSquareBottomRight(
        map.innerSquareBounds.left + map.innerSquareBounds.width - padding,
        map.innerSquareBounds.top + map.innerSquareBounds.height - padding
    );

    // Choose any position inside the inner square bounds for the confined ghost to start
    // For this example, we choose the center of the inner square bounds
    Vector2f confinedGhostStartPosition =
        innerSquareTopLeft + 0.5f * (innerSquareBottomRight - innerSquareTopLeft);

    Texture pacmanTexture;
    if (!pacmanTexture.loadFromFile("Pacman_Spritesheet.png")) {
        cerr << "Failed to load Pacman spritesheet!" << endl;
        return EXIT_FAILURE;
    }

    Vector2f pacmanStartPosition(
        window.getSize().x / 2, // Center X position
        map.lowerLine.getPosition().y + map.lowerLine.getSize().y + pacmanRadius // Just below the square
    );
    Pacman pacman(pacmanTexture, pacmanStartPosition, 0.05f); // Correct the declaration

    Texture coinTexture;
    if (!coinTexture.loadFromFile("Coin.png")) {
        cerr << "Failed to load coin texture!" << endl;
        return EXIT_FAILURE;
    }
    Texture cherryTexture;
    if (!cherryTexture.loadFromFile("fruit.png")) {
        cerr << "Failed to load cherry texture!" << endl;
        return EXIT_FAILURE;
    }
    const float coinSpacing = 70.0f; // Spacing between coins 
    const float coinSize = coinTexture.getSize().x; // Assumes coins are square and uses the x dimension
    const float borderPadding = coinSize / 2 + extraSpacing; // Padding from the border
    const float squareShapePadding = borderPadding; // Padding from the square-like shape
    float availableWidth = window.getSize().x - 2 * borderPadding;

    vector<Coin> coins; // Define a vector to hold the coins
    // Manually specify the x coordinates for each coin in a row of 16 coins
// Remember to adjust positions manually to fit in your window as needed
    vector<float> xPositions = {
        35.f, 85.f, 135.f, 185.f, // ... and so on until you list all 16 x positions
        240.f, 290.f, 340.f, 390.f,
        450.f, 500.f, 550.f, 600.f,
        650.f, 700.f, 750.f
    };

    // Define the y positions for each row manually
    vector<float> yPositions = {
        // Row 1
        50.f,

        // Row 2
        100.f,
        170.f,//3
        240.f, //4
        310.f,//5
        380.f,//6
        450.f,//7
        520.f
        // Add more y positions for additional rows as needed...
    };
    // Now populate your coins vector using the defined positions, skipping the no-coin zone
    for (float y : yPositions) {
        for (float x : xPositions) {
            Coin newCoin(coinTexture);


            Vector2f coinTopLeft(x, y); // Fixed this line

            // Check if the bounds of the coin intersect with the expanded bounds of the square
            FloatRect coinBounds(coinTopLeft.x, coinTopLeft.y, coinSize, coinSize);
            if (!map.innerSquareBounds.intersects(coinBounds)) {
                newCoin.setPosition(coinTopLeft.x, coinTopLeft.y); // Set the coin's top-left position
                coins.push_back(newCoin); // Add the coin if it's not in the inner square area
            }
        }
    }
    // Manually set the desired Cherry size
    float desiredCherrySize = 170.f;

    // Calculate the scale factor for the cherry sprite
    float cherryScale = desiredCherrySize / cherryTexture.getSize().x;

    // Define a vector to hold the cherries
    vector<Cherry> cherries;
    float totalCherryWidth = 3.f * desiredCherrySize;
    // Define the positions manually
    vector<Vector2f> cherryPositions = {
        Vector2f(170.f, 250.f), // Cherry 1
        Vector2f(230.f, 250.f), // Cherry 2 
        Vector2f(290.f, 250.f), // Cherry 3
    };


    // Now populate the cherries vector using the defined positions
    for (auto& pos : cherryPositions) {
        Cherry cherry(cherryTexture);
        cherry.getSprite().setScale(cherryScale, cherryScale); // Set the cherry's scale
        cherry.setPosition(pos.x, pos.y);// Set the cherry's position using the predefined coordinates
        cherries.push_back(cherry);
    }

    // Load eat sound
    SoundBuffer eatBuffer;
    if (!eatBuffer.loadFromFile("eat.wav")) {
        cerr << "Failed to load eat sound!" << endl;
        return EXIT_FAILURE;
    }

    Sound eatSound;
    eatSound.setBuffer(eatBuffer);
    // Load cherry-eat sound
    SoundBuffer eatCherryBuffer;
    if (!eatCherryBuffer.loadFromFile("pacman_eatfruit.wav")) {
        cerr << "Failed to load eat cherry sound!" << endl;
        return EXIT_FAILURE;
    }
    Sound eatCherrySound;
    eatCherrySound.setBuffer(eatCherryBuffer);
    // Load a font
    Font font;
    if (!font.loadFromFile("ArcadeClassic.ttf")) { // Replace with your font file
        cerr << "Failed to load font!" << endl;
        return EXIT_FAILURE;
    }

    // Initialize score and life text
    Text scoreText, lifeText;
    scoreText.setFont(font);
    lifeText.setFont(font); // for life text
    scoreText.setCharacterSize(24); // in pixels, not points!
    lifeText.setCharacterSize(24); // in pixels, not points!
    scoreText.setFillColor(Color::White);
    lifeText.setFillColor(Color::White); // for life text
    scoreText.setPosition(window.getSize().x - 150.f, 10.f); // Set the position for the score text
    lifeText.setPosition(30.f, 10.f); // Set the position for the life text; adjust as needed
    // Initialize lives variable
    int lives = 3; // Starting number of lives for Pac-Man
    lifeText.setString("Lives: " + to_string(lives)); // Set the initial text for lives

    // Initialize score variable
    int score = 0;
    scoreText.setString("Score: " + to_string(score));

    // Loading the ghost sprite sheet and creating ghost characters
    Texture ghostTexture;
    if (!ghostTexture.loadFromFile("ghost.png")) {
        cerr << "Failed to load ghosts spritesheet!" << endl;
        return EXIT_FAILURE;
    }
    float ghostSize = 60.f; // Size you want for ghost
    float ghostScale = ghostSize / (ghostTexture.getSize().x / 4); // Assuming there are 4 ghosts in the row
    vector<Ghost> ghosts{
        // Confined ghost starts within your inner square bounds
    Ghost(ghostTexture, confinedGhostStartPosition, {0, -1}, 80, true),
    Ghost(ghostTexture, {100.0f, 100.0f}, {0, -1}, 60.0f, false),     // Chasing ghost
    Ghost(ghostTexture, {300.0f, 100.0f}, {0, -1}, 80.0f, false),     // Random moving ghost
    Ghost(ghostTexture, {400.0f, 100.0f}, {0, -1}, 80.0f, false),
    };

    vector<IntRect> ghostRects{
    IntRect(0, 0, 130, ghostTexture.getSize().y),   // First ghost
    IntRect(130, 0, 130, ghostTexture.getSize().y), // Second ghost
    IntRect(260, 0, 110, ghostTexture.getSize().y), // Third ghost
    IntRect(370, 0, 127, ghostTexture.getSize().y)  // Fourth ghost
    };


    // Apply the correct texture rectangle and scale for each ghost
    for (size_t i = 0; i < ghosts.size(); ++i) {
        ghosts[i].getSprite().setScale(ghostScale, ghostScale);
        ghosts[i].getSprite().setTextureRect(ghostRects[i]);
    }

    // Initialize "Game Over" text
    Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(Color::Red);
    gameOverText.setString("Game Over");
    gameOverText.setPosition(window.getSize().x / 2.f - gameOverText.getGlobalBounds().width / 2.f,
        window.getSize().y / 2.f - gameOverText.getGlobalBounds().height / 2.f);
    gameOverText.setStyle(Text::Bold);

    // Initialize game over flag
    bool gameOver = false;
    Clock clock;  // Clock to manage update timing
    // Starting positions and target corners for the ghosts
    Vector2f bottomRightCorner(window.getSize().x, window.getSize().y);
    Vector2f bottomLeftCorner(0, window.getSize().y);
    Vector2f bottomRightSquare(map.innerSquareBounds.left + map.innerSquareBounds.width,
        map.innerSquareBounds.top + map.innerSquareBounds.height);
    Vector2f startPositionInSquare = bottomRightSquare - Vector2f(50.f, 50.f); // Start position inside the square

    Ghost chasingGhost(ghostTexture, { 100.0f, 100.0f }, { 0, -1 }, 80.0f);
    chasingGhost.setTargetCorner(pacmanStartPosition); // pacmanStartPosition used for chasing

    Ghost bottomRightGhost(ghostTexture, { 300.0f, 100.0f }, { 0, -1 }, 80.0f);
    bottomRightGhost.setTargetCorner(bottomRightCorner);

    Ghost bottomLeftGhost(ghostTexture, { 400.0f, 100.0f }, { 0, -1 }, 80.0f);
    bottomLeftGhost.setTargetCorner(bottomLeftCorner);

    Ghost confinedGhost(ghostTexture, startPositionInSquare, { 0, -1 }, 80.0f, true);
    confinedGhost.setTargetCorner(bottomRightSquare);

    // Game loop
    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }
        }
        // Fix for deltaTime not being defined.
        float deltaTime = clock.restart().asSeconds(); // Place this inside the while-loop
        window.clear(Color::Black); // Clear the screen at the start of each frame
        bool anyChase = false;
        // Update and draw all ghosts
        for (auto& ghost : ghosts)
        {
            anyChase ? ghost.update(deltaTime, pacman.getPosition(), map, true)
                : ghost.update(deltaTime, pacman.getPosition(), map, anyChase);
            anyChase = !anyChase; // Only one ghost chases at a time
        }
        if (!gameOver)
        {
            // Update Pacman and check for coin collisions
            pacman.update(deltaTime, map);
            bool anyChase = false; // Indicates if the chasing ghost has been updated in this frame
            for (auto& ghost : ghosts)
            {
                if (!ghost.isConfined() && !anyChase) {
                    ghost.setChasing(true); // Set ghost to chase Pac-Man
                    anyChase = true;
                }
                else {
                    ghost.setChasing(false); // Set ghost to not chase Pac-Man
                }
                ghost.update(deltaTime, pacman.getPosition(), map, (&ghost == &chasingGhost));
                window.draw(ghost.getSprite());

                // Checking for collision with Pacman
                if (pacman.getBoundingBox().intersects(ghost.getBoundingBox()))
                {
                    // Deduct lives and check if the game is over
                    lives--;
                    lifeText.setString("Lives: " + to_string(lives));
                    if (lives <= 0)
                    {
                        cout << "Collide!";
                        //gameOver = true;
                    }
                    // Reset Pacman and ghosts to the starting positions if necessary
                    break;
                }
            }
            // Collision checks and updating score for coins
            for (auto it = coins.begin(); it != coins.end();)
            {
                if (pacman.getBoundingBox().intersects(it->getGlobalBounds()))
                {
                    eatSound.play();
                    it = coins.erase(it); // Remove the eaten coin
                    score += 100; // Add 100 points for each coin
                    scoreText.setString("Score: " + to_string(score)); // Update score text
                }
                else
                {
                    ++it;
                }
            }
            // Collision checks and updating score for cherries
            for (auto it = cherries.begin(); it != cherries.end();)
            {
                if (pacman.getBoundingBox().intersects(it->getGlobalBounds()))
                {
                    eatCherrySound.play();
                    it = cherries.erase(it); // Remove the eaten cherry
                    score += 300; // Add 300 points for each cherry
                    scoreText.setString("Score: " + to_string(score)); // Update score text
                }
                else
                {
                    ++it;
                }
            }
            // Setting the string for score and lives every frame is not necessary unless they change
            scoreText.setString("Score: " + to_string(score));
            lifeText.setString("Lives: " + to_string(lives));  // Set the initial text for lives
        }
        // Draw the rest of the game objects
        map.draw(window);
        window.draw(pacman);
        for (const auto& coin : coins) {
            coin.draw(window);
        }
        for (const auto& cherry : cherries) {
            cherry.draw(window);
        }
        window.draw(scoreText);
        window.draw(lifeText);
        if (gameOver) {
            window.draw(gameOverText);
        }
        window.display();
    }
    return 0;
}

