#include "iGraphics.h" // iGraphics library for graphics
#include "iSound.h"    // iSound library for sound
#include <time.h>      // For time-based seeding of random numbers
#include <stdlib.h>    // For random number generation
#include <string>      // For string manipulation
#include <vector>      // For dynamic arrays (vector)
#include <algorithm>   // For sorting algorithms (sort)
#include <fstream>     // For file input/output (ifstream, ofstream)
#include <iostream>    // For console input/output (cin, cout)
#include <ctype.h>     // For character handling (isalnum)
#include <cmath>       // For mathematical functions like pow

using namespace std;

// --- Game Data ---
struct PlayerScore { // Struct to hold player name and score
    string name;     // Player's name
    int score;       // Player's score
    bool operator<(const PlayerScore &other) const { return score > other.score; } // Operator for sorting high scores
};
vector<PlayerScore> highScores;           // Vector to store high scores
const string HIGH_SCORES_FILE = "highscores.txt"; // File name for high scores
const int MAX_HIGH_SCORES = 10;           // Maximum number of high scores to keep

char playerName[50] = "";               // Player name input buffer
int playerNameIndex = 0;                  // Current index in playerName buffer
bool isInputtingName = false;             // Flag for name input state
int finalScore = 0;                       // Score at the end of the game
int obstacleSpeed = 2;                    // Speed of obstacles
int playerHealth = 3;                     // Player's current health
int coinsCollected = 0;                   // Number of coins collected

// Asset paths for various game elements
const char* hotAirBalloonFiles[] = {"hab001_1.png", "hab002_1.png", "hab003_1.png", "hab004_1.png", "hab005_1.png", "hab006_1.png", "hab007_1.png", "hab008_1.png", "hab009_1.png", "hab010_1.png"};
const char* obstacleFiles[] = {"obs1.png", "obs2.png", "obs3.png"};
const char* cloudFiles[] = {"cloud1.png", "cloud2.png", "cloud3.png", "cloud4.png", "cloud5.png", "cloud6.png", "cloud7.png"};
const char* menuButtonFiles[] = {"menu_1.png", "menu_2.png", "menu_3.png", "menu_4.png", "menu_5.png"};
const char* menuButtonBigFiles[] = {"menu_1_big.png", "menu_2_big.png", "menu_3_big.png", "menu_4_big.png", "menu_5_big.png"};

// Bird files for animation
char bird1files[9][14]={"tile001.png","tile002.png","tile003.png","tile004.png","tile005.png","tile006.png","tile007.png","tile008.png","tile009.png"}; 
char bird2files[6][10]={"bb1.png","bb2.png","bb3.png","bb4.png","bb5.png","bb6.png"}; 
char bird3files[10][15]={"bbb1.png","bbb2.png","bbb3.png","bbb4.png","bbb5.png","bbb6.png","bbb7.png","bbb8.png","bbb9.png","bbb10.png"};

bool isMenuHovered[5] = {false};     // Tracks if a menu button is hovered
int menuButtonX[] = {350, 850, 100, 610, 1120}; // X positions of menu buttons
int menuButtonY[] = {260, 240, 60, 60, 60};     // Y positions of menu buttons
int menuButtonWidth = 200, menuButtonHeight = 200; // Menu button dimensions

// Loaded Images and Sprites
Image menuButtonBigImage[5], cloudImage[7], skyImage, heartbreakImage, menuButtonImage[5], menuTitleImage, backButtonImage; // Various game images
Image hotAirBalloonFrames[10], obstacleFrames[3];   // Image frames for animation
Sprite hotAirBalloonSprites[10], obstacleSprites[3]; // Sprites for balloon and obstacles
Image evilBalloonImg;
Sprite evilBalloon;
// Bird images and sprites
Image bird1Frames[9], bird2Frames[6], bird3Frames[10]; // Bird image frames
Sprite bird1Sprite, bird2Sprite, bird3Sprite;       // Bird sprites

int currentGameState = 0;             // Current game state (0:menu, 1:play, 2:HS, 3:name, 4:about, 6:gameover)
int gameScore = 0, scoreUpdateTick = 0; // Current game score and update tick
int balloonFrameIndex = 0;            // Index for hot air balloon animation
int obstacleX[3], obstacleY[3], obstacleImageIndex[3]; // Obstacle positions and image indices
int playerX = 700, playerY = 70;      // Player balloon position
int backgroundMusicHandle;            // Handle for background music
bool isObstacleActive[3] = {true, true, true}; // Status of obstacles
bool showHeartbreakEffect = false;    // Flag to show heartbreak image
int heartbreakCounter = 0;            // Timer for heartbreak display

// Cloud parallax positions and speeds
double cloudX[] = {200, 350, 500, 800, 600, 920, 1225};
double cloudY[] = {114, 229, 343, 457, 500, 686, 800};
double cloudSpeed[] = {0.2, -0.1, -0.3, 0.5, 0.25, -0.3, 0.8};

// --- Coin Data ---
struct Coin { // Struct to represent a coin
    int x, y;    // Coin position
    int frame;   // Current animation frame
    bool active; // Active status
};
char coinFileNames[14][15];      // Filenames for coin animation frames
Image coinAnimationFrames[14];    // Image data for coin animation
vector<Coin> activeCoins;         // Vector of active coins
int coinWidth, coinHeight;        // Coin dimensions
const int MAX_ACTIVE_COINS = 4;   // Max number of active coins at once
int coinSpeed = 2;                // Speed of coins
const int MIN_COIN_SEPARATION = 250; // Minimum separation between coins

int hotAirBalloonWidth, hotAirBalloonHeight; // Player balloon dimensions

const int BACK_BUTTON_X = 1300, BACK_BUTTON_Y = 740; // Back button position

// UI Icons
Image healthIconImage, coinIconImage; // Images for health and coin icons

const int UI_ICON_START_X = 10, UI_ICON_Y = 715, UI_ICON_GAP = 5; // UI icon display parameters

int lastSpeedUpScore30 = 0, lastSpeedUpScore50 = 0; // Score trackers for speed adjustments

// --- Health Item Data ---
struct HealthItem { // Struct to represent a health item
    int x, y;    // Health item position
    bool active; // Active status
};
HealthItem currentHealthItem; // The single active health item
Image healthItemImage;       // Health item image
int healthItemWidth, healthItemHeight; // Health item dimensions
const int MAX_PLAYER_HEALTH = 3;         // Maximum player health
int healthItemSpawnDelay = 0;            // Timer for health item spawn
const int HEALTH_ITEM_SPAWN_INTERVAL = 3000; // Spawn interval in milliseconds

// --- Bird Data ---
int currentBird = 0;           // 0: no bird, 1-3: specific bird active
int birdX, birdY;              // Common position for the active bird
float birdSpeedX, birdSpeedY;  // Speed components for the active bird
int bird1FrameIndex = 0;       // Animation frame for bird1
int bird2FrameIndex = 0;       // Animation frame for bird2
int bird3FrameIndex = 0;       // Animation frame for bird3

int birdSpawnTimerTicks = 0;      // Counts ticks for bird spawning
int nextBirdSpawnTickTarget = 0;  // Target ticks for next bird spawn



int evilX = 1400, evilY = 750;
int evilSpeedX = -25;
int evilSpeedY = -2;
int evilDirection = 1; // -1 = left, 1 = right
int evilTimer = 0;
int evilSpawnTick = 0;
bool evilActive = true;
int evilCooldown = 0;
int evilStepCounter = 0;




// Updates game score and adjusts obstacle/coin speed based on score
void updateScore() {
    scoreUpdateTick++;
    if (scoreUpdateTick % 9 == 0) {
        gameScore++;
        int totalScore = gameScore + (coinsCollected * 10);
        if (obstacleSpeed < 10 && totalScore > 0 && totalScore % 30 == 0 && totalScore > lastSpeedUpScore30) {
            obstacleSpeed += 2; lastSpeedUpScore30 = totalScore;
        }
        if (obstacleSpeed >= 10 && totalScore > 0 && totalScore % 50 == 0 && totalScore > lastSpeedUpScore50) {
            obstacleSpeed += 1; lastSpeedUpScore50 = totalScore;
        }
        if (obstacleSpeed > 20) obstacleSpeed = 20;
        coinSpeed = obstacleSpeed;
    }
}

// Loads high scores from file
void loadHighScores() {
    highScores.clear(); ifstream file(HIGH_SCORES_FILE);
    if (file.is_open()) {
        string name; int s;
        while (file >> name >> s) highScores.push_back({name, s});
        file.close(); sort(highScores.begin(), highScores.end());
    }
}

// Saves high scores to file
void saveHighScores() {
    ofstream file(HIGH_SCORES_FILE);
    for (int i = 0; i < min((int)highScores.size(), MAX_HIGH_SCORES); ++i) file << highScores[i].name << " " << highScores[i].score << endl;
    file.close();
}

// Adds a new score to high scores list
void addHighScore(string name, int s) {
    for (char &c : name) if (c == ' ') c = '_';
    highScores.push_back({name, s}); sort(highScores.begin(), highScores.end());
    if (highScores.size() > MAX_HIGH_SCORES) highScores.resize(MAX_HIGH_SCORES);
    saveHighScores(); loadHighScores();
}

// Controls player movement based on arrow keys
void movePlayer() {
    if (isSpecialKeyPressed(GLUT_KEY_RIGHT)) playerX += 15;
    if (isSpecialKeyPressed(GLUT_KEY_LEFT)) playerX -= 15;
    if (isSpecialKeyPressed(GLUT_KEY_UP)) playerY += 15;
    if (isSpecialKeyPressed(GLUT_KEY_DOWN)) playerY -= 15;

    if (playerX + hotAirBalloonWidth > 1400) playerX = 0; // Wrap right
    if (playerX < 0) playerX = 1400 - hotAirBalloonWidth; // Wrap left
    if (playerY > 760) playerY = 760; // Max Y boundary
    if (playerY < 0) playerY = 0; // Min Y boundary

    iSetSpritePosition(&hotAirBalloonSprites[balloonFrameIndex], playerX, playerY);
}

// Manages obstacle movement and respawn
void manageObstacles() {
    for (int i = 0; i < 3; i++) {
        obstacleY[i] -= obstacleSpeed;
        if (obstacleY[i] < -obstacleFrames[obstacleImageIndex[i]].height) { // Off-screen check
            isObstacleActive[i] = true; obstacleY[i] = 800; // Reset Y
            obstacleX[i] = rand() % (1400 - obstacleFrames[obstacleImageIndex[i]].width); // Random X
            obstacleImageIndex[i] = rand() % 3; // Random image
            iChangeSpriteFrames(&obstacleSprites[i], &obstacleFrames[obstacleImageIndex[i]], 1);
        }
        iSetSpritePosition(&obstacleSprites[i], obstacleX[i], obstacleY[i]);
    }
}

// Animates the hot air balloon sprite
void animateBalloon() {
    balloonFrameIndex = (balloonFrameIndex + 1) % 10;
    iChangeSpriteFrames(&hotAirBalloonSprites[balloonFrameIndex], &hotAirBalloonFrames[balloonFrameIndex], 1);
    iSetSpritePosition(&hotAirBalloonSprites[balloonFrameIndex], playerX, playerY);
}

// Moves clouds horizontally for parallax effect
void moveCloudsX() {
    for (int i = 0; i < 7; i++) {
        cloudX[i] += cloudSpeed[i];
        if (cloudSpeed[i] > 0 && cloudX[i] > 1450) cloudX[i] = -150; // Wrap right
        else if (cloudSpeed[i] < 0 && cloudX[i] < -150) cloudX[i] = 1450; // Wrap left
    }
}

// Moves clouds vertically
void moveCloudsY() {
    for (int i = 0; i < 7; i++) {
        cloudY[i] -= 2; // Move downwards
        if (cloudY[i] < -50) cloudY[i] = 800; // Reset to top
    }
}

// Updates animation frames for active coins
void animateCoins() {
    for (size_t i = 0; i < activeCoins.size(); ++i) if (activeCoins[i].active) activeCoins[i].frame = (activeCoins[i].frame + 1) % 14;
}

// Checks for collision between a coin and obstacles
bool checkCoinObstacleCollision(int coinLeft, int coinRight, int coinBottom, int coinTop) {
    for (int i = 0; i < 3; ++i) { // Obstacles
        if (isObstacleActive[i]) {
            int obstacleLeft = obstacleX[i], obstacleRight = obstacleX[i] + obstacleFrames[obstacleImageIndex[i]].width;
            int obstacleBottom = obstacleY[i], obstacleTop = obstacleY[i] + obstacleFrames[obstacleImageIndex[i]].height;
            if (coinLeft < obstacleRight && coinRight > obstacleLeft && coinBottom < obstacleTop && coinTop > obstacleBottom) return true;
        }
    }
    return false;
}

// Spawns a new coin at a valid position
void spawnCoin() {
    int coinIndex = -1;
    for (size_t i = 0; i < activeCoins.size(); ++i) if (!activeCoins[i].active) { coinIndex = i; break; }
    if (coinIndex == -1 && activeCoins.size() < MAX_ACTIVE_COINS) { activeCoins.push_back({}); coinIndex = activeCoins.size() - 1; }
    else if (coinIndex == -1) return;

    Coin &newCoin = activeCoins[coinIndex];
    bool positionFound = false;
    for (int retry = 0; retry < 50; ++retry) {
        int posX = rand() % (1400 - coinWidth);
        int posY = 800 + (rand() % 400);
        int coinLeft = posX, coinRight = posX + coinWidth, coinBottom = posY, coinTop = posY + coinHeight;

        if (checkCoinObstacleCollision(coinLeft, coinRight, coinBottom, coinTop)) continue; // Avoid obstacles

        bool overlap = false;
        for (size_t i = 0; i < activeCoins.size(); ++i) {
            if (activeCoins[i].active && i != (size_t)coinIndex) {
                double distanceSq = pow((posX + coinWidth/2.0) - (activeCoins[i].x + coinWidth/2.0), 2) + pow((posY + coinHeight/2.0) - (activeCoins[i].y + coinHeight/2.0), 2);
                if (distanceSq < pow(MIN_COIN_SEPARATION, 2)) { overlap = true; break; }
            }
        }
        if (overlap) continue; // Avoid other coins
        
        newCoin.x = posX; newCoin.y = posY; newCoin.frame = rand() % 14; newCoin.active = true; positionFound = true; break;
    }
    if (!positionFound) newCoin.active = false;
}

// Manages coin movement and spawning
void manageCoinMovement() {
    int activeCoinCount = 0;
    for (size_t i = 0; i < activeCoins.size(); ++i) {
        if (activeCoins[i].active) {
            activeCoins[i].y -= coinSpeed;
            if (activeCoins[i].y < -coinHeight) activeCoins[i].active = false; // Off-screen
            activeCoinCount++;
        }
    }
    if (activeCoinCount < MAX_ACTIVE_COINS && (rand() % 100) < 5) spawnCoin();
}

// Checks for collision between a health item and obstacles, coins, or birds
bool checkHealthItemCollision(int itemLeft, int itemRight, int itemBottom, int itemTop) {
    for (int i = 0; i < 3; ++i) { // Obstacles
        if (isObstacleActive[i]) {
            int obstacleLeft = obstacleX[i], obstacleRight = obstacleX[i] + obstacleFrames[obstacleImageIndex[i]].width;
            int obstacleBottom = obstacleY[i], obstacleTop = obstacleY[i] + obstacleFrames[obstacleImageIndex[i]].height;
            if (itemLeft < obstacleRight && itemRight > obstacleLeft && itemBottom < obstacleTop && itemTop > obstacleBottom) return true;
        }
    }
    for (size_t i = 0; i < activeCoins.size(); ++i) { // Coins
        if (activeCoins[i].active) {
            int coinLeft = activeCoins[i].x, coinRight = activeCoins[i].x + coinWidth;
            int coinBottom = activeCoins[i].y, coinTop = activeCoins[i].y + coinHeight;
            if (itemLeft < coinRight && itemRight > coinLeft && itemBottom < coinTop && itemTop > coinBottom) return true;
        }
    }
    // Check collision with active bird
    Sprite* activeBirdSprite = nullptr; Image* activeBirdImage = nullptr;
    if (currentBird == 1) { activeBirdSprite = &bird1Sprite; activeBirdImage = &bird1Frames[bird1FrameIndex]; }
    else if (currentBird == 2) { activeBirdSprite = &bird2Sprite; activeBirdImage = &bird2Frames[bird2FrameIndex]; }
    else if (currentBird == 3) { activeBirdSprite = &bird3Sprite; activeBirdImage = &bird3Frames[bird3FrameIndex]; }

    if (activeBirdSprite && activeBirdImage && currentBird != 0) {
        if (itemLeft < activeBirdSprite->x + activeBirdImage->width && itemRight > activeBirdSprite->x &&
            itemBottom < activeBirdSprite->y + activeBirdImage->height && itemTop > activeBirdSprite->y) {
            return true;
        }
    }
    return false;
}

// Spawns a health item if not already active and player health is not full
void spawnHealthItem() {
    if (!currentHealthItem.active && playerHealth < MAX_PLAYER_HEALTH) {
        bool positionFound = false;
        for (int retry = 0; retry < 50; ++retry) {
            int posX = rand() % (1400 - healthItemWidth);
            int posY = 800 + (rand() % 400); // Above screen
            int itemLeft = posX, itemRight = posX + healthItemWidth, itemBottom = posY, itemTop = posY + healthItemHeight;
            if (!checkHealthItemCollision(itemLeft, itemRight, itemBottom, itemTop)) {
                currentHealthItem.x = posX; currentHealthItem.y = posY; currentHealthItem.active = true;
                positionFound = true; break;
            }
        }
        if (!positionFound) currentHealthItem.active = false;
    }
}

// Manages health item movement and spawning
void manageHealthItemMovement() {
    if (currentHealthItem.active) {
        currentHealthItem.y -= coinSpeed; // Health item moves at coin speed
        if (currentHealthItem.y < -healthItemHeight) {
            currentHealthItem.active = false; // Off-screen
            healthItemSpawnDelay = HEALTH_ITEM_SPAWN_INTERVAL; // Reset delay
        }
    } else {
        healthItemSpawnDelay -= 16; // Decrement timer
        if (healthItemSpawnDelay <= 0) {
            spawnHealthItem();
            healthItemSpawnDelay = HEALTH_ITEM_SPAWN_INTERVAL; // Reset delay
        }
    }
}

// Resets and spawns a new bird at a random position, aiming towards the player
void spawnNewBird() {
    currentBird = (rand() % 3) + 1; // Randomly choose bird 1, 2, or 3
    int targetX = playerX; int targetY = playerY; // Target is player's current position
    float travelTicks = (float)(rand() % 21 + 15); // 15 to 35 ticks for travel time (1.5 to 3.5 seconds)

    if (currentBird == 1 || currentBird == 2) { // Bird 1 and 2 spawn from left
        birdX = -(rand() % 100 + 50); birdY = rand() % 700 + 50;
        birdSpeedX = (targetX - birdX) / travelTicks; birdSpeedY = (targetY - birdY) / travelTicks;
    } else { // Bird 3 spawns from right
        birdX = 1400 + (rand() % 100 + 50); birdY = rand() % 700 + 50;
        birdSpeedX = (targetX - birdX) / travelTicks; birdSpeedY = (targetY - birdY) / travelTicks;
    }
    // Ensure a sufficient minimum speed
    if (abs(birdSpeedX) < 12) birdSpeedX = (birdSpeedX > 0 ? 12 : -12);
    if (abs(birdSpeedY) < 12) birdSpeedY = (birdSpeedY > 0 ? 12 : -12);

    birdSpawnTimerTicks = 0; // Reset spawn tick counter
    nextBirdSpawnTickTarget = rand() % 21 + 15; // New random interval for next bird
}

// Manages bird movement and spawning
void manageSpecialObstacles() {
    if (currentGameState != 1) return; // Only move special obstacles during gameplay

    // Bird movement and spawning
    if (currentBird == 0) { // No bird active, check for spawn
        birdSpawnTimerTicks++;
        if (birdSpawnTimerTicks >= nextBirdSpawnTickTarget) spawnNewBird();
    } else { // Bird is active, move it
        birdX += birdSpeedX; birdY += birdSpeedY;

        int currentBirdWidth = 0, currentBirdHeight = 0; // Get current bird's dimensions
        if (currentBird == 1) { currentBirdWidth = bird1Frames[bird1FrameIndex].width; currentBirdHeight = bird1Frames[bird1FrameIndex].height; }
        else if (currentBird == 2) { currentBirdWidth = bird2Frames[bird2FrameIndex].width; currentBirdHeight = bird2Frames[bird2FrameIndex].height; }
        else if (currentBird == 3) { currentBirdWidth = bird3Frames[bird3FrameIndex].width; currentBirdHeight = bird3Frames[bird3FrameIndex].height; }

        if (birdX < -currentBirdWidth || birdX > 1400 + currentBirdWidth || birdY < -currentBirdHeight || birdY > 800 + currentBirdHeight) {
            currentBird = 0; // Deactivate bird if off-screen
            birdSpawnTimerTicks = 0; nextBirdSpawnTickTarget = rand() % 21 + 15; return;
        }

        // Apply position to the correct sprite and animate
        if (currentBird == 1) { iSetSpritePosition(&bird1Sprite, birdX, birdY); bird1FrameIndex = (bird1FrameIndex + 1) % 9; iChangeSpriteFrames(&bird1Sprite, &bird1Frames[bird1FrameIndex], 1); }
        else if (currentBird == 2) { iSetSpritePosition(&bird2Sprite, birdX, birdY); bird2FrameIndex = (bird2FrameIndex + 1) % 6; iChangeSpriteFrames(&bird2Sprite, &bird2Frames[bird2FrameIndex], 1); }
        else if (currentBird == 3) { iSetSpritePosition(&bird3Sprite, birdX, birdY); bird3FrameIndex = (bird3FrameIndex + 1) % 10; iChangeSpriteFrames(&bird3Sprite, &bird3Frames[bird3FrameIndex], 1); }
    }
}
void iDraw() {
    iClear(); iShowLoadedImage(0, 0, &skyImage); // Background
    for (int i = 0; i < 7; i++) iShowLoadedImage(cloudX[i], cloudY[i], &cloudImage[i]); // Clouds

    if (currentGameState == 0) { // Main menu
        iShowLoadedImage(500, 530, &menuTitleImage);
        for (int i = 0; i < 5; i++) {
            if (isMenuHovered[i]) iShowLoadedImage(menuButtonX[i] - 10, menuButtonY[i] - 10, &menuButtonBigImage[i]);
            else iShowLoadedImage(menuButtonX[i], menuButtonY[i], &menuButtonImage[i]);
        }
    } else if (currentGameState == 1) { // Game play
        iShowSprite(&hotAirBalloonSprites[balloonFrameIndex]);
        int playerLeft = playerX, playerRight = playerX + hotAirBalloonWidth, playerBottom = playerY, playerTop = playerY + hotAirBalloonHeight; // Player bounds

        for (int i = 0; i < 3; i++) { // Obstacles
            if (isObstacleActive[i]) {
                iShowSprite(&obstacleSprites[i]);
                if (iCheckCollision(&hotAirBalloonSprites[balloonFrameIndex], &obstacleSprites[i])) { // Player-obstacle collision
                    showHeartbreakEffect = true; heartbreakCounter = 60;
                    iPauseSound(backgroundMusicHandle); iPlaySound("wrong.wav", false, 80);
                    playerHealth--; isObstacleActive[i] = false; // Deactivate obstacle
                    if (playerHealth <= 0) {
                        finalScore = gameScore; addHighScore(playerName, finalScore); currentGameState = 6; // Game Over
                    } else iResumeSound(backgroundMusicHandle);
                }
            }
        }

        // --- BIRD COLLISION LOGIC ---
        if (currentBird != 0) {
            Sprite* activeBirdSprite = nullptr;
            if (currentBird == 1) activeBirdSprite = &bird1Sprite;
            else if (currentBird == 2) activeBirdSprite = &bird2Sprite;
            else if (currentBird == 3) activeBirdSprite = &bird3Sprite;

            if (activeBirdSprite && iCheckCollision(&hotAirBalloonSprites[balloonFrameIndex], activeBirdSprite)) {
                currentBird = 0; // Deactivate the bird
                showHeartbreakEffect = true; heartbreakCounter = 60;
                iPauseSound(backgroundMusicHandle); iPlaySound("wrong.wav", false, 80);
                playerHealth--; // Reduce health
                if (playerHealth <= 0) {
                    finalScore = gameScore; addHighScore(playerName, finalScore); currentGameState = 6; // Game Over
                } else iResumeSound(backgroundMusicHandle);
            }
        }
        // --- END BIRD COLLISION LOGIC ---

        for (size_t i = 0; i < activeCoins.size(); ++i) { // Coins
            if (activeCoins[i].active) {
                iShowLoadedImage(activeCoins[i].x, activeCoins[i].y, &coinAnimationFrames[activeCoins[i].frame]);
                int coinLeft = activeCoins[i].x, coinRight = activeCoins[i].x + coinWidth, coinBottom = activeCoins[i].y, coinTop = activeCoins[i].y + coinHeight;
                if (playerLeft < coinRight && playerRight > coinLeft && playerBottom < coinTop && playerTop > coinBottom) { // Player-coin collision
                    gameScore += 10; coinsCollected++; activeCoins[i].active = false; iPlaySound("coin.wav", false, 80);
                }
            }
        }
        
        if (currentHealthItem.active) { // Health item
            iShowLoadedImage(currentHealthItem.x, currentHealthItem.y, &healthItemImage);
            int healthItemLeft = currentHealthItem.x, healthItemRight = currentHealthItem.x + healthItemWidth;
            int healthItemBottom = currentHealthItem.y, healthItemTop = currentHealthItem.y + healthItemHeight;
            if (playerLeft < healthItemRight && playerRight > healthItemLeft && playerBottom < healthItemTop && playerTop > healthItemBottom) { // Player-health item collision
                if (playerHealth < MAX_PLAYER_HEALTH) playerHealth++; // Increase health
                iPlaySound("health_pickup.wav", false, 80); currentHealthItem.active = false; healthItemSpawnDelay = HEALTH_ITEM_SPAWN_INTERVAL;
            }
        }

        if (showHeartbreakEffect) { // Heartbreak display
            iShowLoadedImage((1400 - heartbreakImage.width) / 2, (800 - heartbreakImage.height) / 2, &heartbreakImage);
            if (--heartbreakCounter <= 0) showHeartbreakEffect = false;
        }

        char scoreText[30]; // UI text
        sprintf(scoreText, "Score: %d", gameScore); iText(10, 770, scoreText, GLUT_BITMAP_HELVETICA_18);
        sprintf(scoreText, "Speed: %d", obstacleSpeed); iText(10, 750, scoreText, GLUT_BITMAP_HELVETICA_18);

        for (int i = 0; i < playerHealth; ++i) iShowLoadedImage(UI_ICON_START_X + (healthIconImage.width + UI_ICON_GAP) * i, UI_ICON_Y, &healthIconImage);
        iShowLoadedImage(10, 680, &coinIconImage);
        sprintf(scoreText, "%d", coinsCollected); iText(10 + coinIconImage.width + 5, 685, scoreText, GLUT_BITMAP_HELVETICA_18);
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &backButtonImage); // Back button

        if (currentBird == 1) iShowSprite(&bird1Sprite); // Only show the currently active bird
        else if (currentBird == 2) iShowSprite(&bird2Sprite);
        else if (currentBird == 3) iShowSprite(&bird3Sprite);

    } else if (currentGameState == 3) { // Name input
        iSetColor(0, 0, 255); iText(560, 400, "Enter Your Name:", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(680 - (playerNameIndex * 6), 370, playerName, GLUT_BITMAP_HELVETICA_18); // Centered name input
        iText(560, 300, "Press ENTER to start the game.", GLUT_BITMAP_HELVETICA_18);
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &backButtonImage);
    } else if (currentGameState == 4) { // About screen
        iShowImage(0, 0, "aboutus.PNG"); iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &backButtonImage);
    } else if (currentGameState == 6) { // Game Over
        iSetColor(255, 0, 0); iText(625, 450, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
        iSetColor(0, 0, 255);
        char scoreString[50]; sprintf(scoreString, "FINAL SCORE: %d", finalScore); iText(575, 400, scoreString, GLUT_BITMAP_TIMES_ROMAN_24);
        sprintf(scoreString, "COINS COLLECTED: %d", coinsCollected); iText(550, 350, scoreString, GLUT_BITMAP_TIMES_ROMAN_24);
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &backButtonImage);
    } else if (currentGameState == 2) { // High Scores
        iSetColor(255, 255, 255); iText(625, 700, "High Scores", GLUT_BITMAP_HELVETICA_18);
        int startY = 650;
        for (size_t i = 0; i < highScores.size(); i++) {
            char scoreLine[100]; string displayName = highScores[i].name;
            for (char &c : displayName) if (c == '_') c = ' '; // Display spaces
            sprintf(scoreLine, "%d. %s - %d", (int)(i + 1), displayName.c_str(), highScores[i].score);

            if (i == 0) iSetColor(255, 0, 0); // Red for 1st
            else if (i == 1) iSetColor(0, 255, 0); // Green for 2nd
            else if (i == 2) iSetColor(0, 0, 255); // Blue for 3rd
            else iSetColor(255, 255, 255); // White for rest

            iText(700 - (strlen(scoreLine) * 5), startY - i * 30, scoreLine, GLUT_BITMAP_HELVETICA_18); // Centered score
        }
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &backButtonImage);
    }
}

// Handles mouse movement for menu button hovering
void iMouseMove(int mouseX, int mouseY) {
    for (int i = 0; i < 5; i++) isMenuHovered[i] = (mouseX >= menuButtonX[i] && mouseX <= menuButtonX[i] + menuButtonWidth && mouseY >= menuButtonY[i] && mouseY <= menuButtonY[i] + menuButtonHeight);
}
void iMouseDrag(int mouseX, int mouseY) {} // Mouse drag function (empty)

// Handles mouse clicks
void iMouse(int button, int state, int mouseX, int mouseY) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (currentGameState == 0) { // Main Menu clicks
            if (mouseX >= 350 && mouseX <= 350 + menuButtonWidth && mouseY >= 260 && mouseY <= 260 + menuButtonHeight) { // Start Game
                currentGameState = 3; isInputtingName = true; playerName[0] = '\0'; playerNameIndex = 0;
            } else if (mouseX >= 850 && mouseX <= 850 + menuButtonWidth && mouseY >= 240 && mouseY <= 240 + menuButtonHeight) { // High Scores
                currentGameState = 2;
            } else if (mouseX >= 100 && mouseX <= 100 + menuButtonWidth && mouseY >= 60 && mouseY <= 60 + menuButtonHeight) { // Help
                // Implement help screen functionality if needed
            } else if (mouseX >= 610 && mouseX <= 610 + menuButtonWidth && mouseY >= 60 && mouseY <= 60 + menuButtonHeight) { // About
                currentGameState = 4;
            } else if (mouseX >= 1120 && mouseX <= 1120 + menuButtonWidth && mouseY >= 60 && mouseY <= 60 + menuButtonHeight) { // Exit
                exit(0);
            }
        }
        // Back button for all applicable states
        if ((currentGameState >= 1 && currentGameState <= 4) || currentGameState == 6) {
            if (mouseX >= BACK_BUTTON_X && mouseX <= BACK_BUTTON_X + backButtonImage.width && mouseY >= BACK_BUTTON_Y && mouseY <= BACK_BUTTON_Y + backButtonImage.height) {
                currentGameState = 0; iPauseSound(backgroundMusicHandle);
            }
        }
    }
}
void iMouseWheel(int direction, int mouseX, int mouseY) {} // Mouse wheel function (empty)

// Handles keyboard input
void iKeyboard(unsigned char key) {
    if (currentGameState == 3 && isInputtingName) { // Name input
        if (key == '\b') { // Backspace
            if (playerNameIndex > 0) playerName[--playerNameIndex] = '\0';
        } else if (key == '\r' || key == '\n') { // Enter
            isInputtingName = false; currentGameState = 1;
            backgroundMusicHandle = iPlaySound("Easy.wav", true, 80); // Start game music

            // Reset game state variables
            gameScore = 0; scoreUpdateTick = 0; playerHealth = 3; coinsCollected = 0;
            obstacleSpeed = 2; coinSpeed = obstacleSpeed;
            lastSpeedUpScore30 = 0; lastSpeedUpScore50 = 0; // Reset speed-up trackers

            for (int i = 0; i < 3; i++) { // Reset obstacles
                obstacleY[i] = 800 + i * 267; obstacleX[i] = rand() % (1400 - obstacleFrames[obstacleImageIndex[i]].width);
                obstacleImageIndex[i] = rand() % 3; isObstacleActive[i] = true;
                iChangeSpriteFrames(&obstacleSprites[i], &obstacleFrames[obstacleImageIndex[i]], 1); iSetSpritePosition(&obstacleSprites[i], obstacleX[i], obstacleY[i]);
            }
            for (size_t i = 0; i < activeCoins.size(); ++i) activeCoins[i].active = false; // Reset coins
            for (int i = 0; i < MAX_ACTIVE_COINS; ++i) spawnCoin(); // Initial coins

            currentHealthItem.active = false; healthItemSpawnDelay = HEALTH_ITEM_SPAWN_INTERVAL; // Reset health item

            currentBird = 0; birdSpawnTimerTicks = 0; nextBirdSpawnTickTarget = rand() % 21 + 15; // Reset bird

        } else if (playerNameIndex < (int)sizeof(playerName) - 1 && (isalnum(key) || key == ' ')) { // Add character to name
            playerName[playerNameIndex++] = key; playerName[playerNameIndex] = '\0';
        }
    }
}
void iSpecialKeyboard(unsigned char key) {} // Special keyboard function (empty)

int main(int argc, char *argv[]) {
    glutInit(&argc, argv); srand(time(0));

    // Load and resize coin frames
    for (int i = 0; i < 14; ++i) {
        sprintf(coinFileNames[i], "coin%d.png", i + 1);
        iLoadImage(&coinAnimationFrames[i], coinFileNames[i]);
        iResizeImage(&coinAnimationFrames[i], 30, 30);
    }
    coinWidth = coinAnimationFrames[0].width; coinHeight = coinAnimationFrames[0].height;

    // Load obstacle images
    for (int i = 0; i < 3; i++) iLoadImage(&obstacleFrames[i], obstacleFiles[i]);
    
    // Load balloon images
    for (int j = 0; j < 10; j++) iLoadImage(&hotAirBalloonFrames[j], hotAirBalloonFiles[j]);
    hotAirBalloonWidth = hotAirBalloonFrames[0].width; hotAirBalloonHeight = hotAirBalloonFrames[0].height;

    // Initialize balloon sprites
    for (int j = 0; j < 10; ++j) {
        iInitSprite(&hotAirBalloonSprites[j]);
        iChangeSpriteFrames(&hotAirBalloonSprites[j], &hotAirBalloonFrames[j], 1);
        iSetSpritePosition(&hotAirBalloonSprites[j], playerX, playerY);
    }

    loadHighScores();

    // Initialize obstacle sprites
    for (int i = 0; i < 3; i++) {
        iInitSprite(&obstacleSprites[i]); obstacleImageIndex[i] = rand() % 3;
        iChangeSpriteFrames(&obstacleSprites[i], &obstacleFrames[obstacleImageIndex[i]], 1);
        obstacleX[i] = rand() % (1400 - obstacleFrames[obstacleImageIndex[i]].width);
        obstacleY[i] = 800 + i * 267; // Staggered above screen
        iSetSpritePosition(&obstacleSprites[i], obstacleX[i], obstacleY[i]);
    }

    // Load cloud images
    for (int i = 0; i < 7; i++) iLoadImage(&cloudImage[i], cloudFiles[i]);

    // Load menu images
    for (int i = 0; i < 5; i++) {
        iLoadImage(&menuButtonImage[i], menuButtonFiles[i]);
        iLoadImage(&menuButtonBigImage[i], menuButtonBigFiles[i]);
    }
    iLoadImage(&menuTitleImage, "menuhab.png");
    iLoadImage(&skyImage, "sky.jpg");
    iLoadImage(&heartbreakImage, "heartbreak.png");
    iLoadImage(&backButtonImage, "backbutton.png");

    iLoadImage(&healthIconImage, "Healthicon.png"); iResizeImage(&healthIconImage, 30, 30);
    iLoadImage(&coinIconImage, "Coinicon.png"); iResizeImage(&coinIconImage, 30, 30);

    // Load & init health item
    iLoadImage(&healthItemImage, "livesprite.png"); iResizeImage(&healthItemImage, 30, 30);
    healthItemWidth = healthItemImage.width; healthItemHeight = healthItemImage.height;
    currentHealthItem.active = false; healthItemSpawnDelay = HEALTH_ITEM_SPAWN_INTERVAL;

    // Load bird images and resize
    for (int i = 0; i < 9; i++) { iLoadImage(&bird1Frames[i], bird1files[i]); iResizeImage(&bird1Frames[i], 75, 75); }
    for(int i=0; i < 6; i++) { iLoadImage(&bird2Frames[i], bird2files[i]); iResizeImage(&bird2Frames[i], 75, 75); }
    for(int i=0; i < 10; i++) { iLoadImage(&bird3Frames[i], bird3files[i]); iResizeImage(&bird3Frames[i], 75, 75); }

    // Initialize bird sprites (off-screen)
    iInitSprite(&bird1Sprite); iInitSprite(&bird2Sprite); iInitSprite(&bird3Sprite);
    iSetSpritePosition(&bird1Sprite, -200, -200); iSetSpritePosition(&bird2Sprite, -200, -200); iSetSpritePosition(&bird3Sprite, -200, -200);

    // Set timers for various game functions
    iSetTimer(500, animateBalloon);
    iSetTimer(16, moveCloudsX);
    iSetTimer(16, moveCloudsY);
    iSetTimer(1, movePlayer);
    iSetTimer(16, manageObstacles);
    iSetTimer(33, updateScore);
    iSetTimer(50, animateCoins);
    iSetTimer(16, manageCoinMovement);
    iSetTimer(16, manageHealthItemMovement);
    iSetTimer(100, manageSpecialObstacles);

    iInitializeSound(); iInitialize(1400, 800, "Hot Air Balloon"); // Init sound and window
    return 0;
}