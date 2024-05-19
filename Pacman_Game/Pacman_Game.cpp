#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include<iostream>
#include<vector>
#include<math.h>
#include<random>
#include<functional>
#include <chrono> // Add this for sleep functionality
#include <thread> // Add this for sleep functionality

class Pacman;
class Ghost;
class Coin;
typedef Coin Fruit;
class Map;

using namespace std;
using namespace sf;
struct Level {
	int number;
	int chasingGhosts;
	Color mapColor;
	string fruitTexturePath;
	int fruitScore;
	vector<Vector2f> coinPositions;
	vector<Vector2f> fruitPositions;
};
Level level1 = { 1, 1, Color::Cyan, {},400 };
Level level2 = { 2, 2, Color::Blue, {},700 };
Level level3 = { 3, 3, Color::White, {},1000 };
Level currentLevel = level1;
// Global scope for persistent texture reference and fruits
Texture fruitTexture;
vector<Fruit> fruits;
// Forward declarations of functions.
void resetGameForNextLevel(Pacman& pacman, vector<Ghost>& ghosts, vector<Coin>& coins,
	Map& map, Texture& coinTexture,
	const Vector2f& pacmanStartPosition, const Vector2f& redGhostStartPosition,
	const Vector2f& confinedGhostStartPosition,
	const float coinSize, const vector<Vector2f>& levelCoinPositions);

vector<Coin> coins;
vector<float> xPositions = {
  35.f, 85.f, 135.f, 185.f,
  240.f, 290.f, 340.f, 390.f,
  450.f, 500.f, 550.f, 600.f,
  650.f, 700.f, 750.f
};

vector<float> yPositions = {
  50.f,
  100.f,
  170.f,
  240.f,
  310.f,
  380.f,
  450.f,
  520.f
};

class Map {
public:
	RectangleShape border;
	RectangleShape upperLine;
	RectangleShape lowerLine;
	RectangleShape leftLine;
	RectangleShape rightLine;
	FloatRect innerSquareBounds;

	Map(float windowWidth, float windowHeight, float outlineThickness) {
		border.setSize(Vector2f(windowWidth - 2 * outlineThickness, windowHeight - 2 * outlineThickness));
		border.setFillColor(Color::Transparent);
		border.setOutlineThickness(-outlineThickness);
		border.setOutlineColor(Color::Cyan);
		border.setPosition(outlineThickness, outlineThickness);

		float mapWidth = 300;
		float mapHeight = 300;
		float sideLineLength = 230;

		upperLine.setSize(Vector2f(mapWidth, outlineThickness));
		upperLine.setFillColor(Color::Cyan);
		upperLine.setPosition((windowWidth - mapWidth) / 2, (windowHeight - mapHeight) / 2);

		lowerLine.setSize(Vector2f(mapWidth, outlineThickness));
		lowerLine.setFillColor(Color::Cyan);
		lowerLine.setPosition((windowWidth - mapWidth) / 2, upperLine.getPosition().y + mapHeight - outlineThickness);

		leftLine.setSize(Vector2f(outlineThickness, sideLineLength));
		leftLine.setFillColor(Color::Cyan);
		leftLine.setPosition(upperLine.getPosition().x, lowerLine.getPosition().y - sideLineLength);

		rightLine.setSize(Vector2f(outlineThickness, sideLineLength));
		rightLine.setFillColor(Color::Cyan);
		rightLine.setPosition(upperLine.getPosition().x + upperLine.getSize().x - outlineThickness, lowerLine.getPosition().y - sideLineLength);
		innerSquareBounds = FloatRect(
			upperLine.getPosition().x - outlineThickness,  // Expand to the left
			upperLine.getPosition().y - outlineThickness,  // Expand to the top
			mapWidth + 2 * outlineThickness,            // Expand width
			leftLine.getSize().y + 2 * outlineThickness   // Expand height
		);
	}

	void draw(RenderWindow& window) {
		window.draw(border);
		window.draw(upperLine);
		window.draw(lowerLine);
		window.draw(leftLine);
		window.draw(rightLine);
	}
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
	bool isPointInsideInnerSquare(const Vector2f& point) const {
		return innerSquareBounds.contains(point);
	}
};
class Pacman : public Drawable {
	float getRotation() const {
		if (direction.x > 0) return 0;  // Right
		if (direction.x < 0) return 180; // Left
		if (direction.y > 0) return 90;  // Down
		if (direction.y < 0) return -90; // Up
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
	void setPosition(const Vector2f& newPosition) {
		sprite.setPosition(newPosition); // Update the private member variable if necessary
		position = newPosition;
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
typedef Coin Fruit;

class Ghost : public Drawable {
private:
	Sprite sprite;
	Vector2f position;
	Vector2f direction;
	float speed;
	bool confined;
	static mt19937 rng;
	Vector2f startPosition;
	bool chasing = false;
	Vector2f targetCorner;
	float directionChangeCooldownTime = 1.7f;
	float chasingSpeed;
	float nonChasingSpeed;

	bool isOutOfWindowBounds(const RenderWindow& window) {
		FloatRect bounds = sprite.getGlobalBounds();
		return bounds.left < 0 || bounds.top < 0 ||
			bounds.left + bounds.width > window.getSize().x ||
			bounds.top + bounds.height > window.getSize().y;
	}
	Vector2f calculateDirection(const Vector2f& targetPosition) {
		Vector2f dir = targetPosition - sprite.getPosition();
		float length = sqrt((dir.x * dir.x) + (dir.y * dir.y));
		if (length != 0) dir /= length;
		return dir;
	}
	void keepInsideWindow(const RenderWindow& window) {
		if (isOutOfWindowBounds(window)) {
			FloatRect bounds = sprite.getGlobalBounds();
			if (bounds.left < 0) position.x = 0;
			if (bounds.top < 0) position.y = 0;
			if (bounds.left + bounds.width > window.getSize().x) position.x = window.getSize().x - bounds.width;
			if (bounds.top + bounds.height > window.getSize().y) position.y = window.getSize().y - bounds.height;
			sprite.setPosition(position);
		}
	}
public:
	Ghost(const Texture& texture, const Vector2f& startPosition,
		const Vector2f& startDirection, float startSpeed,
		bool startsConfined = false)
		: startPosition(startPosition), direction(startDirection),
		speed(startSpeed), confined(startsConfined) {
		sprite.setTexture(texture);
		sprite.setPosition(startPosition);
		sprite.setOrigin(texture.getSize().x / 4, texture.getSize().y / 2);
		sprite.setTextureRect(
			IntRect(0, 0, texture.getSize().x / 4, texture.getSize().y));
	}
	float directionChangeCooldown = 0;
	void setChasing(bool isChasing) { chasing = isChasing; }
	bool isChasing() const { return chasing; }

	void randomDirection() {
		if (directionChangeCooldown <= 0) {
			uniform_int_distribution<int> dist(0, 3);
			int dir = dist(rng);
			switch (dir) {
			case 0:
				direction = Vector2f(-1, 0);
				break; // Left
			case 1:
				direction = Vector2f(1, 0);
				break; // Right
			case 2:
				direction = Vector2f(0, -1);
				break; // Up
			case 3:
				direction = Vector2f(0, 1);
				break; // Down
			}
			directionChangeCooldown = directionChangeCooldownTime;
		}
	}
	Sprite& getSprite() { return sprite; }
	void setTargetCorner(const Vector2f& target) { targetCorner = target; }

	bool isOutOfWindowBounds(const RenderWindow& window, const FloatRect& nextPosition) {
		return nextPosition.left < 0 || nextPosition.top < 0 ||
			nextPosition.left + nextPosition.width > window.getSize().x ||
			nextPosition.top + nextPosition.height > window.getSize().y;
	}
	Vector2f getNewValidDirection(const RenderWindow& window) {
		std::vector<Vector2f> possibleDirections = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
		shuffle(begin(possibleDirections), end(possibleDirections), rng);  // Randomize the direction order

		for (const auto& dir : possibleDirections) {
			Vector2f newDirection = dir * (speed * directionChangeCooldownTime);
			if (!isOutOfWindowBounds(window, nextPositionBounds(getBoundingBox(), newDirection))) {
				return dir;
			}
		}

		return Vector2f(0, 0);  // No valid direction found; you might want to handle this case differently
	}
	// Create a new method for calculating potential new position
	FloatRect nextPositionBounds(const FloatRect& currentBounds, const Vector2f& movement) {
		return FloatRect(currentBounds.left + movement.x, currentBounds.top + movement.y,
			currentBounds.width, currentBounds.height);
	}
	// Update the update method to implement the bouncing logic correctly
	void update(float dt, const RenderWindow& window, const Pacman& pacman) {
		Vector2f targetPosition = pacman.getPosition();
		float currentSpeed = chasing ? chasingSpeed : nonChasingSpeed;

		// Adjust the direction towards the target position if chasing
		if (chasing) {
			Vector2f directionTowardsTarget = targetPosition - sprite.getPosition();
			float length = sqrtf(directionTowardsTarget.x * directionTowardsTarget.x + directionTowardsTarget.y * directionTowardsTarget.y);
			if (length != 0) {
				directionTowardsTarget /= length;
			}
			direction = directionTowardsTarget;
		}
		else {
			randomDirection();
		}

		if (directionChangeCooldown > 0) {
			directionChangeCooldown -= dt;
		}

		Vector2f movement = direction * speed * dt;
		if (isOutOfWindowBounds(window, nextPositionBounds(getBoundingBox(), movement))) {
			direction = getNewValidDirection(window);
		}

		sprite.move(direction * speed * dt);
		position = sprite.getPosition();

	}
	// Setter method to adjust direction change cooldown time
	void setDirectionChangeCooldownTime(float cooldownTime) {
		directionChangeCooldownTime = cooldownTime;
	}
	FloatRect getBoundingBox() const { return sprite.getGlobalBounds(); }
	void setPosition(const Vector2f& newPosition) {
		sprite.setPosition(newPosition);
		position = newPosition;
	}
	bool isConfined() const { return confined; }
	void draw(RenderTarget& target, RenderStates states) const override {
		target.draw(sprite, states);
	}
	Vector2f getPosition() {
		return sprite.getPosition();
	}
};
mt19937 Ghost::rng(random_device{}());
// Add a helper function to calculate distance between two points
float distance(const Vector2f& a, const Vector2f& b) {
	return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}

// Check for proper touching between Pacman and Ghost using their radii
bool properTouch(const Vector2f& pacmanPos, const Vector2f& ghostPos, float pacmanRadius, float ghostRadius) {
	float dist = distance(pacmanPos, ghostPos);
	return dist <= (pacmanRadius + ghostRadius);
}
int main() {
	RenderWindow window(VideoMode(800, 600), "Pacman Game - Map");
	const float outlineThickness = 7;
	const float pacmanRadius = 20;
	const float extraSpacing = 0.0001f;
	Vector2f redGhostStartPosition(200.f, 100.f);
	Map map(window.getSize().x, window.getSize().y, outlineThickness);
	// Inside the game loop where you draw the map:
	map.border.setOutlineColor(currentLevel.mapColor);

	float padding = 20.0f;
	Vector2f innerSquareTopLeft(
		map.innerSquareBounds.left + padding,
		map.innerSquareBounds.top + padding
	);
	Vector2f innerSquareBottomRight(
		map.innerSquareBounds.left + map.innerSquareBounds.width - padding,
		map.innerSquareBounds.top + map.innerSquareBounds.height - padding
	);
	Vector2f confinedGhostStartPosition =
		innerSquareTopLeft + 0.5f * (innerSquareBottomRight - innerSquareTopLeft);

	Texture pacmanTexture;
	if (!pacmanTexture.loadFromFile("Pacman_Spritesheet.png")) {
		cerr << "Failed to load Pacman spritesheet!" << endl;
		return EXIT_FAILURE;
	}


	Vector2f pacmanStartPosition(
		window.getSize().x / 2,
		map.lowerLine.getPosition().y + map.lowerLine.getSize().y + pacmanRadius
	);
	Pacman pacman(pacmanTexture, pacmanStartPosition, 0.05f);

	Texture coinTexture;
	if (!coinTexture.loadFromFile("Coin.png")) {
		cerr << "Failed to load coin texture!" << endl;
		return EXIT_FAILURE;
	}

	const float coinSpacing = 70.0f;
	const float coinSize = coinTexture.getSize().x;
	const float borderPadding = coinSize / 2 + extraSpacing;
	const float squareShapePadding = borderPadding;
	float availableWidth = window.getSize().x - 2 * borderPadding;

	vector<Coin> coins;

	vector<float> xPositions = {
	 35.f, 85.f, 135.f, 185.f,
	 240.f, 290.f, 340.f, 390.f,
	 450.f, 500.f, 550.f, 600.f,
	 650.f, 700.f, 750.f
	};

	vector<float> yPositions = {
	  50.f,
	  100.f,
	  170.f,
	  240.f,
	  310.f,
	  380.f,
	  450.f,
	  520.f
	};
	for (float y : yPositions) {
		for (float x : xPositions) {
			Coin newCoin(coinTexture);
			Vector2f coinTopLeft(x, y);
			FloatRect coinBounds(coinTopLeft.x, coinTopLeft.y, coinSize, coinSize);
			if (!map.innerSquareBounds.intersects(coinBounds)) {
				newCoin.setPosition(coinTopLeft.x, coinTopLeft.y);
				coins.push_back(newCoin);
			}
		}
	}
	Texture fruitTexture;
	if (!fruitTexture.loadFromFile("cherryyy.png")) {
		cerr << "Failed to load cheryyy texture for level 1!" << endl;
		return EXIT_FAILURE;
	}

	float desiredFruitSize = 170.f;
	vector<Fruit> fruits;
	float FruitScale = desiredFruitSize / fruitTexture.getSize().x;
	float totalFruitWidth = 3.f * desiredFruitSize;
	vector<Vector2f> fruitPositions = {
	  Vector2f(170.f, 250.f),
	  Vector2f(230.f, 250.f),
	  Vector2f(290.f, 250.f),
	};
	for (auto& pos : fruitPositions) {
		Fruit fruit(fruitTexture);
		fruit.getSprite().setScale(FruitScale, FruitScale);
		fruit.setPosition(pos.x, pos.y);
		fruits.push_back(fruit);
	}

	SoundBuffer eatBuffer;
	if (!eatBuffer.loadFromFile("eat.wav")) {
		cerr << "Failed to load eat sound!" << endl;
		return EXIT_FAILURE;
	}

	Sound eatSound;
	eatSound.setBuffer(eatBuffer);
	SoundBuffer eatFruitBuffer;
	if (!eatFruitBuffer.loadFromFile("pacman_eatfruit.wav")) {
		cerr << "Failed to load eat cherry sound!" << endl;
		return EXIT_FAILURE;
	}
	Sound eatFruitSound;
	eatFruitSound.setBuffer(eatFruitBuffer);
	Font font;
	if (!font.loadFromFile("ArcadeClassic.ttf")) {
		cerr << "Failed to load font!" << endl;
		return EXIT_FAILURE;
	}
	SoundBuffer deathBuffer;
	if (!deathBuffer.loadFromFile("pacmanDeath.wav")) {
		cerr << "Failed to load death sound!" << endl;
		return EXIT_FAILURE;
	}

	Sound deathSound;
	deathSound.setBuffer(deathBuffer);

	Text scoreText, lifeText, levelText;
	scoreText.setFont(font);
	lifeText.setFont(font);
	levelText.setFont(font);

	scoreText.setCharacterSize(24);
	lifeText.setCharacterSize(24);
	levelText.setCharacterSize(24);

	scoreText.setFillColor(Color::White);
	lifeText.setFillColor(Color::White);
	levelText.setFillColor(Color::White);

	scoreText.setPosition(window.getSize().x - 150.f, 10.f);
	lifeText.setPosition(30.f, 10.f);
	levelText.setPosition((window.getSize().x / 2.f) - 50.f, 10.f);

	int lives = 3;
	lifeText.setString("Lives: " + to_string(lives));
	int score = 0;
	scoreText.setString("Score: " + to_string(score));

	// Set initial level text
	levelText.setString("Level: " + to_string(currentLevel.number));

	Texture ghostTexture;
	if (!ghostTexture.loadFromFile("Ghosts_Spritesheet.png")) {
		cerr << "Failed to load ghosts spritesheet!" << endl;
		return EXIT_FAILURE;
	}
	float ghostSize = 60.f;
	float ghostScale = ghostSize / (ghostTexture.getSize().x / 4);
	vector<Ghost> ghosts{
	  Ghost(ghostTexture, confinedGhostStartPosition, {0, -1}, 50, true),
	  Ghost(ghostTexture, {100.0f, 100.0f}, {0, -1}, 70.0f, false),
	  Ghost(ghostTexture, {300.0f, 100.0f}, {0, -1}, 80.0f, false),
	  Ghost(ghostTexture, {400.0f, 100.0f}, {0, -1}, 80.0f, false),
	};
	for (size_t i = 0; i < ghosts.size(); ++i) {
		ghosts[i].getSprite().setScale(ghostScale, ghostScale);
		ghosts[i].getSprite().setTextureRect(IntRect(i * (ghostTexture.getSize().x / 4), 0, ghostTexture.getSize().x / 4, ghostTexture.getSize().y));
	}

	Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setCharacterSize(40);
	gameOverText.setFillColor(Color::Red);
	gameOverText.setString("Game Over");
	gameOverText.setPosition(window.getSize().x / 2.f - gameOverText.getGlobalBounds().width / 2.f,
		window.getSize().y / 2.f - gameOverText.getGlobalBounds().height / 2.f);
	gameOverText.setStyle(Text::Bold);

	bool gameOver = false;
	Clock clock;
	Vector2f bottomRightCorner(window.getSize().x, window.getSize().y);
	Vector2f bottomLeftCorner(0, window.getSize().y);
	Vector2f bottomRightSquare(map.innerSquareBounds.left + map.innerSquareBounds.width,
		map.innerSquareBounds.top + map.innerSquareBounds.height);
	Vector2f startPositionInSquare = bottomRightSquare - Vector2f(50.f, 50.f);
	Ghost chasingGhost(ghostTexture, { 100.0f, 100.0f }, { 0, -1 }, 80.0f);
	chasingGhost.setTargetCorner(pacmanStartPosition);
	Ghost bottomRightGhost(ghostTexture, { 300.0f, 100.0f }, { 0, -1 }, 80.0f);
	bottomRightGhost.setTargetCorner(bottomRightCorner);
	Ghost bottomLeftGhost(ghostTexture, { 400.0f, 100.0f }, { 0, -1 }, 80.0f);
	bottomLeftGhost.setTargetCorner(bottomLeftCorner);
	Ghost confinedGhost(ghostTexture, startPositionInSquare, { 0, -1 }, 80.0f, true);
	confinedGhost.setTargetCorner(bottomRightSquare);
	const float invulnerabilityTime = 2.0f;
	float invulnerabilityTimer = 0.0f;
	bool isInvulnerable = false;
	// Define the radius for effective touching detection
	float pacmanEffectiveRadius = pacmanRadius / 2.0f; // Adjust as needed
	float ghostEffectiveRadius = ghostSize / 2.0f; // Adjust as needed based on the Ghost sprite

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

		float deltaTime = clock.restart().asSeconds();
		window.clear(Color::Black);
		bool anyChase = false;
		// Ghost chasing logic based on current level
		int chasingGhostCount = 0;
		for (auto& ghost : ghosts) {
			if (chasingGhostCount < currentLevel.chasingGhosts) {
				ghost.setChasing(true);
				chasingGhostCount++;
			}
			else {
				ghost.setChasing(false);
			}
			ghost.update(deltaTime, window, pacman);
			window.draw(ghost.getSprite());
		}
		if (!gameOver)
		{
			if (isInvulnerable) {
				invulnerabilityTimer -= deltaTime;
				if (invulnerabilityTimer <= 0.0f) {
					isInvulnerable = false;
				}
			}
			pacman.update(deltaTime, map);
			//Modify collision detection between ghosts and Pacman
			for (auto& ghost : ghosts) {
				if (properTouch(pacman.getPosition(), ghost.getPosition(), pacmanEffectiveRadius, ghostEffectiveRadius)) {
					if (!isInvulnerable) {
						--lives;
						deathSound.play();
						lifeText.setString("Lives: " + to_string(lives));
						if (lives <= 0) {
							gameOver = true;
							break;
						}
						else {
							isInvulnerable = true;
							invulnerabilityTimer = invulnerabilityTime;

							pacman.setPosition(pacmanStartPosition);
						}
						for (auto& g : ghosts) {
							if (g.isChasing()) {
								g.setPosition(redGhostStartPosition);
							}
							else if (g.isConfined()) {
								g.setPosition(confinedGhostStartPosition);
							}
							// ... set positions for other ghosts as needed
						}
						break;
					}
				}
			}
			for (auto it = coins.begin(); it != coins.end();)
			{
				if (pacman.getBoundingBox().intersects(it->getGlobalBounds()))
				{
					eatSound.play();
					it = coins.erase(it);
					score += 100;
					scoreText.setString("Score: " + to_string(score));
				}
				else
				{
					++it;
				}
			}
			// Fruit collection
			for (auto it = fruits.begin(); it != fruits.end();) {
				if (pacman.getBoundingBox().intersects(it->getGlobalBounds())) {
					eatFruitSound.play();
					it = fruits.erase(it);
					score += currentLevel.fruitScore; // Score based on level
					scoreText.setString("Score: " + to_string(score));
				}
				else {
					++it;
				}
			}
			// Inside level progression logic
			if (coins.empty() && fruits.empty()) {
				fruits.clear(); // Clear existing fruits
				Texture newFruitTexture; // Local texture for new fruits
				if (currentLevel.number == 1) {
					currentLevel = level2;
					fruits.clear(); // Clear existing fruits

					if (!fruitTexture.loadFromFile("apple.png")) {
						cerr << "Failed to load mango texture for level 2!" << endl;
						return EXIT_FAILURE;
					}


					for (const Vector2f& pos : currentLevel.fruitPositions) {
						Fruit newFruit(fruitTexture);
						newFruit.getSprite().setScale(FruitScale, FruitScale);
						newFruit.setPosition(pos.x - fruitTexture.getSize().x * FruitScale / 2, pos.y - fruitTexture.getSize().y * FruitScale / 2);
						fruits.push_back(newFruit);
					}
					resetGameForNextLevel(pacman, ghosts, coins, map,
						coinTexture, pacmanStartPosition,
						redGhostStartPosition, confinedGhostStartPosition,
						coinSize, currentLevel.coinPositions);
				}
				else if (currentLevel.number == 2) {
					currentLevel = level3;
					fruits.clear(); // Clear existing fruits

					if (!fruitTexture.loadFromFile("mango.png")) {
						cerr << "Failed to load mango texture for level 3!" << endl;
						return EXIT_FAILURE;
					}
					else {
						cout << "Loaded mango texture for level 3 successfully." << endl;
					}
					for (const Vector2f& pos : currentLevel.fruitPositions) {
						Fruit newFruit(fruitTexture);
						newFruit.getSprite().setScale(FruitScale, FruitScale);
						newFruit.setPosition(pos.x - fruitTexture.getSize().x * FruitScale / 2, pos.y - fruitTexture.getSize().y * FruitScale / 2);
						fruits.push_back(newFruit);
					}
					resetGameForNextLevel(pacman, ghosts, coins, map,
						coinTexture, pacmanStartPosition,
						redGhostStartPosition, confinedGhostStartPosition,
						coinSize, currentLevel.coinPositions);
				}
				else if (currentLevel.number == 3) {
					gameOverText.setString("You Won!");
					gameOverText.setFillColor(Color::Green);
					window.draw(gameOverText);
					window.display();
					this_thread::sleep_for(chrono::seconds(3)); // Use std::this_thread::sleep_for
				}
				levelText.setString("Level: " + to_string(currentLevel.number));
			}
			scoreText.setString("Score: " + to_string(score));
			lifeText.setString("Lives: " + to_string(lives));
		}
		map.draw(window);
		window.draw(pacman);
		for (const auto& coin : coins) {
			coin.draw(window);
		}
		for (const auto& fruit : fruits) {
			fruit.draw(window);
		}
		window.draw(scoreText);
		window.draw(lifeText);
		window.draw(levelText);
		if (gameOver) {
			window.draw(gameOverText);
		}
		window.display();

	}

	return 0;
}


void resetGameForNextLevel(Pacman& pacman, vector<Ghost>& ghosts, vector<Coin>& coins,
	Map& map, Texture& coinTexture,
	const Vector2f& pacmanStartPosition, const Vector2f& redGhostStartPosition,
	const Vector2f& confinedGhostStartPosition,
	const float coinSize, const vector<Vector2f>& levelCoinPositions) {


	// Clear existing coins and fruits
	coins.clear();



	vector<float> xPositions = {
	  35.f, 85.f, 135.f, 185.f,
	  240.f, 290.f, 340.f, 390.f,
	  450.f, 500.f, 550.f, 600.f,
	  650.f, 700.f, 750.f
	};

	vector<float> yPositions = {
	  50.f,
	  100.f,
	  170.f,
	  240.f,
	  310.f,
	  380.f,
	  450.f,
	  520.f
	};
	for (float y : yPositions) {
		for (float x : xPositions) {
			Coin newCoin(coinTexture);
			Vector2f coinTopLeft(x, y);
			FloatRect coinBounds(coinTopLeft.x, coinTopLeft.y, coinSize, coinSize);
			if (!map.innerSquareBounds.intersects(coinBounds)) {
				newCoin.setPosition(coinTopLeft.x, coinTopLeft.y);
				coins.push_back(newCoin);
			}
		}
	}

	for (const Vector2f& pos : levelCoinPositions) {
		Coin newCoin(coinTexture);
		newCoin.setPosition(pos.x - coinSize / 2, pos.y - coinSize / 2);
		coins.push_back(newCoin);
	}



	// Reset Pacman and ghosts to their initial positions
	pacman.setPosition(pacmanStartPosition);
	for (auto& ghost : ghosts) {
		ghost.setPosition(ghost.isConfined() ? confinedGhostStartPosition : redGhostStartPosition);
	}

	// Update the map color to the current level color
	map.border.setOutlineColor(currentLevel.mapColor);

}