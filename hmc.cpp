#include "iGraphics.h"
#include "iSound.h"
#include <time.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <cmath> 

using namespace std;

struct PlayerScore {
    string name;
    int score;
    bool operator<(const PlayerScore& other) const {
        return score > other.score;
    }
};

vector<PlayerScore> highScores;
const string HIGHSCORE_FILE = "highscores.txt";
const int MAX_HIGHSCORES = 5;

char playerName[50] = "";
int playerNameIndex = 0;
bool inputName = false;
int finalScore = 0;
float obstacleSpeed = 2;
int health = 3;
int coinsCollected = 0;

char hab[10][13] = {"hab001_1.png","hab002_1.png","hab003_1.png","hab004_1.png","hab005_1.png",
                    "hab006_1.png","hab007_1.png","hab008_1.png","hab009_1.png","hab010_1.png"};
char obs[3][10] = {"obs1.png","obs2.png","obs3.png"};
char cloud[7][13] = {"cloud1.png","cloud2.png","cloud3.png","cloud4.png","cloud5.png","cloud6.png","cloud7.png"};
char menu[5][15] = {"menu_1.png","menu_2.png","menu_3.png","menu_4.png","menu_5.png"};
char menuBig[5][20] = {"menu_1_big.png","menu_2_big.png","menu_3_big.png","menu_4_big.png","menu_5_big.png"};
char bird1files[9][14]={"tile001.png","tile002.png","tile003.png","tile004.png",
                        "tile005.png","tile006.png","tile007.png","tile008.png","tile009.png"};   
char bird2files[6][10]={"bb1.png","bb2.png","bb3.png","bb4.png","bb5.png","bb6.png"}; 
char bird3files[10][15]={"bbb1.png","bbb2.png","bbb3.png","bbb4.png","bbb5.png",
                        "bbb6.png","bbb7.png","bbb8.png","bbb9.png","bbb10.png"};

bool hoverMenu[5] = {false};
int menuX[5] = {350, 850, 100, 610, 1120};
int menuY[5] = {220, 200, 20, 20, 20};
int menuW = 200, menuH = 200;


Image menuBigImg[5], cloudImg[7], skyImg, heartbreak, menuImg[5], menuImghab, back;
Image habimgfrm[10], obsimgfrm[3],bird1frm[9],bird2frm[6],bird3frm[10];
Image evilBalloonImg;

Sprite evilBalloon;
Sprite habimg[10], obsimg[3],bird1,bird2,bird3;


int bird1idx=0;
int bird1x=1400,bird1y=600;
int bird1speed=25;
bool bird1active=true;

int bird2idx=0;
int bird2x=1400,bird2y=100;
int bird2speed=20;
bool bird2active=true;

int bird3idx=0;
int bird3x=1400,bird3y=100;
int bird3speed=22;
bool bird3active=true;

int evilX = 1400, evilY = 750;
int evilSpeedX = -25;
int evilSpeedY = -2;
int evilDirection = 1; // -1 = left, 1 = right
int evilTimer = 0;
int evilSpawnTick = 0;
bool evilActive = true;
int evilCooldown = 0;
int evilStepCounter = 0;

int gamestate = 0;
int score = 0, scoretick = 0;
int bi = 0;
int obsx[3], obsy[3], obsi[3];
int x = 700, y = 70;
int easy;
bool isObsActive[3] = {true, true, true};
bool showHeartbreak = false;
int heartbreakCounter = 0;

double c1x=200,c2x=350,c3x=500,c4x=800,c5x=600,c6x=920,c7x=1225;
double c1y=114,c2y=229,c3y=343,c4y=457,c5y=500,c6y=686,c7y=800;

void moveBird1() {
    if (!bird1active) return;

    bird1x += bird1speed;
    if (bird1x >1500){ bird1x = -100; // Reset to right side after going off screen
    bird1y = 200+rand() % 300; // Randomize vertical position
    }

    iSetSpritePosition(&bird1, bird1x, bird1y);

    bird1idx = (bird1idx + 1) % 9;
    iChangeSpriteFrames(&bird1, bird1frm + bird1idx, 1);
     if (evilActive) {
        evilX += evilSpeedX * evilDirection;
            evilY -= 6;

        // Create zigzag every few steps
        if(evilX<20 || evilX>1800)
        {
            evilDirection*=-1;
        }

        iSetSpritePosition(&evilBalloon, evilX, evilY);

        // Offscreen? deactivate and start cooldown
        if (evilX < -100 || evilY < -100) {
            evilActive = false;
            evilCooldown = 0;
            evilStepCounter = 0;
        }
    } else {
        evilCooldown++;
        if (evilCooldown >= 15) {
            evilActive = true;
            evilX = 1400;
            evilY = 750;
            evilDirection = -1;
            evilStepCounter = 0;
            iSetSpritePosition(&evilBalloon, evilX, evilY);
        }
    }
}
void moveBird2() {
    if (!bird2active) return;

    bird2x += bird2speed;
    if (bird2x >1500) {bird2x = -100; // Reset to right side after going off screen
    bird2y = 250 +rand() % 300; // Randomize vertical position
    }
    iSetSpritePosition(&bird2, bird2x, bird2y);

    bird2idx = (bird2idx + 1) % 6;
    iChangeSpriteFrames(&bird2, bird2frm + bird2idx, 1);
}
void moveBird3() {
    if (!bird3active) return;

    bird3x -= bird3speed;   
    if (bird3x<-100) {
        bird3x = 1500; // Reset to right side after going off screen
        bird3y = rand() % 100; // Randomize vertical position
    }
    iSetSpritePosition(&bird3, bird3x, bird3y); 
    bird3idx = (bird3idx + 1) % 10; // Cycle through bird3 frames
    iChangeSpriteFrames(&bird3, bird3frm + bird3idx, 1);
}
struct Coin {
    int x, y;
    int currentFrame; // Index for the rotating coin image (0-13)
    bool active;      // Is this coin currently on screen and interactable?
};

char coinFiles[14][15]; // Stores filenames like "coin1.png", "coin2.png", ...
Image coinFrames[14];  // Stores the loaded image data for each coin frame
vector<Coin> coins;    // A dynamic array to manage all coins
int coinWidth = 20;    // Will be set after loading the first coin image
int coinHeight = 20;   // Will be set after loading the first coin image
const int MAX_ACTIVE_COINS = 4;
int coinSpeed = 2; // Changed to int and initialized. THIS WILL BE DYNAMIC!
const int MIN_COIN_SEPARATION = 250;

// --- Hot Air Balloon Dimensions for collision ---
int habWidth = 0;
int habHeight = 0;

// --- Back Button Position Constants ---
const int BACK_BUTTON_X = 1300;
const int BACK_BUTTON_Y = 740;

// --- Image Declarations ---
Image healthIcon; // Declared globally
Image coinCollectIcon; // Declare coin collection icon

// --- Constants for Health Icon Display ---
const int HEALTH_ICON_START_X = 10;  // Starting X position for the first health icon
const int HEALTH_ICON_Y = 715;       // Y position for all health icons
const int HEALTH_ICON_GAP = 5; 
void scoreupdate() {
    scoretick++;
    if(scoretick % 9 == 0) {
        score++;
        // Increase obstacle speed by 1 when score is a multiple of 10
        if (score > 0 && score % 10 == 0) {
            obstacleSpeed += 1; // Increment by integer 1
            coinSpeed = obstacleSpeed; // Make coin speed match obstacle speed
        }
    }
}

void loadHighScores() {
    highScores.clear();
    ifstream file(HIGHSCORE_FILE);
    if (file.is_open()) {
        string name; int score;
        while (file >> name >> score) {
            highScores.push_back({name, score});
        }
        file.close();
        sort(highScores.begin(), highScores.end());
    }
}

void saveHighScores() {
    ofstream file(HIGHSCORE_FILE);
    for (int i = 0; i < min((int)highScores.size(), MAX_HIGHSCORES); ++i) {
        file << highScores[i].name << " " << highScores[i].score << endl;
    }
    file.close();
}

void addHighScore(string name, int score) {
    for (char &c : name) if (c == ' ') c = '_';
    highScores.push_back({name, score});
    sort(highScores.begin(), highScores.end());
    if (highScores.size() > MAX_HIGHSCORES)
        highScores.resize(MAX_HIGHSCORES);
    saveHighScores();
    loadHighScores(); 
}

void move() {
    if (isSpecialKeyPressed(GLUT_KEY_RIGHT)) x += 15;
    if (isSpecialKeyPressed(GLUT_KEY_LEFT)) x -= 15;

     if (isSpecialKeyPressed(GLUT_KEY_UP))    y += 15;
    if (isSpecialKeyPressed(GLUT_KEY_DOWN))  y -= 15;
    if (x > 1360) x = -40;
    if (x < -40) x = 1360;
     if (y > 760) y = 760;
    if (y < 0)   y = 0;
    iSetSpritePosition(&habimg[bi], x, y);
}
void obstacle() {
    for (int i = 0; i < 3; i++) {
        if (isObsActive[i]) {
            obsy[i] -= obstacleSpeed;
            if (obsy[i] < -150) {
                obsy[i] = 800;
                obsx[i] = rand() % 1300;
                obsi[i] = rand() % 3;
                iChangeSpriteFrames(&obsimg[i], &obsimgfrm[obsi[i]], 1);
            }
            iSetSpritePosition(&obsimg[i], obsx[i], obsy[i]);
        } else {
            obsy[i] -= obstacleSpeed;
            if (obsy[i] < -150) {
                isObsActive[i] = true;
                obsy[i] = 800;
                obsx[i] = rand() % 1300;
                obsi[i] = rand() % 3;
                iChangeSpriteFrames(&obsimg[i], &obsimgfrm[obsi[i]], 1);
            }
            iSetSpritePosition(&obsimg[i], obsx[i], obsy[i]);
        }
    }
}

void incmnt() {
    bi = (bi + 1) % 10;
    iChangeSpriteFrames(&habimg[bi], &habimgfrm[bi], 1);
    iSetSpritePosition(&habimg[bi], x, y);
}

void cldx() {
    double* cx[] = {&c1x, &c2x, &c3x, &c4x, &c5x, &c6x, &c7x};
    double speed[] = {.2, -.1, -.3, .5, .25, -.3, .8};
    for (int i = 0; i < 7; i++) {
        *cx[i] += speed[i];
               if (speed[i] > 0 && *cx[i] > 1450) *cx[i] = -150;
        
        else if (speed[i] < 0 && *cx[i] < -150) *cx[i] = 1450;
        }
    }

void cldy() {
    double* cy[] = {&c1y, &c2y, &c3y, &c4y, &c5y, &c6y, &c7y};
    for (int i = 0; i < 7; i++) {
        *cy[i] -= 2;
        if (*cy[i] < -50) *cy[i] = 800;
    }
}
void coinAnimate() {
    for (size_t i = 0; i < coins.size(); ++i) {
        if (coins[i].active) {
            coins[i].currentFrame = (coins[i].currentFrame + 1) % 14; // Cycle through 14 frames
        }
    }
}

// Checks if a potential coin position (bounding box) overlaps with any active obstacle
bool checkCoinObstacleCollision(int coinLeft, int coinRight, int coinBottom, int coinTop) {
    for (int i = 0; i < 3; ++i) {
        if (isObsActive[i]) {
            // Get current bounding box of the obstacle
            int obsLeft = obsx[i];
            int obsRight = obsx[i] + obsimgfrm[obsi[i]].width;
            int obsBottom = obsy[i];
            int obsTop = obsy[i] + obsimgfrm[obsi[i]].height;

            // Check for Axis-Aligned Bounding Box (AABB) overlap
            if (coinLeft < obsRight && coinRight > obsLeft &&
                coinBottom < obsTop && coinTop > obsBottom) {
                return true; // Collision found
            }
        }
    }
    return false; // No collision with any active obstacle
}

// Spawns a coin at a random position, avoiding active obstacles AND other coins
void spawnCoin() {
    // Find an inactive coin slot in our vector, or add a new one if space allows
    int coinIdx = -1;
    for (size_t i = 0; i < coins.size(); ++i) {
        if (!coins[i].active) {
            coinIdx = i;
            break;
        }
    }
    if (coinIdx == -1 && coins.size() < MAX_ACTIVE_COINS) {
        coins.push_back({}); // Add a new empty Coin struct
        coinIdx = coins.size() - 1;
    } else if (coinIdx == -1) { // No inactive coins and already at max capacity
        return; // Cannot spawn more coins right now
    }

    Coin& newCoin = coins[coinIdx]; // Get a reference to the coin we're about to use

    bool positionFound = false;
    const int MAX_SPAWN_RETRIES = 50; // Increased retries for better chance of finding a spot
    for (int retry = 0; retry < MAX_SPAWN_RETRIES; ++retry) {
        // Generate potential X within screen bounds, considering coin width
        int potentialX = rand() % (1400 - coinWidth);
        // Generate potential Y above the screen, staggered, to give coins time to separate
        int potentialY = 800 + (rand() % 400);

        int currentCoinLeft = potentialX;
        int currentCoinRight = potentialX + coinWidth;
        int currentCoinBottom = potentialY;
        int currentCoinTop = potentialY + coinHeight;

        // Check for collision with obstacles first
        if (checkCoinObstacleCollision(currentCoinLeft, currentCoinRight, currentCoinBottom, currentCoinTop)) {
            continue; // Retry if it collides with an obstacle
        }

        // --- NEW: Check for collision/proximity with other active coins ---
        bool overlapsWithOtherCoins = false;
        for (size_t i = 0; i < coins.size(); ++i) {
            if (coins[i].active && i != (size_t)coinIdx) {
                // Calculate distance between centers. Using squared distance for performance.
                double distSq = pow((potentialX + coinWidth / 2.0) - (coins[i].x + coinWidth / 2.0), 2) +
                                 pow((potentialY + coinHeight / 2.0) - (coins[i].y + coinHeight / 2.0), 2);

                // Compare with squared MIN_COIN_SEPARATION
                if (distSq < pow(MIN_COIN_SEPARATION, 2)) {
                    overlapsWithOtherCoins = true;
                    break;
                }
            }
        }

        if (overlapsWithOtherCoins) {
            continue; // Retry if it overlaps or is too close to another coin
        }
        // --- END NEW COIN CHECK ---

        // If no obstacle or coin overlap, this position is good
        newCoin.x = potentialX;
        newCoin.y = potentialY;
        newCoin.currentFrame = rand() % 14;
        newCoin.active = true;
        positionFound = true;
        break;
    }

    if (!positionFound) {
        newCoin.active = false; // If no clear spot found after retries, keep it inactive
    }
}

// Moves active coins downwards and handles off-screen coins; also triggers spawning
void coinMovement() {
    int activeCoinCount = 0;
    for (size_t i = 0; i < coins.size(); ++i) {
        if (coins[i].active) {
            coins[i].y -= coinSpeed; // Move coin downwards using the dynamic speed

            if (coins[i].y < -coinHeight) { // Coin went completely off-screen
                coins[i].active = false; // Deactivate it
            }
            activeCoinCount++; // Count active coins
        }
    }

    // Try to spawn a new coin if there are fewer than MAX_ACTIVE_COINS,
    // and a random chance (e.g., 5% per call) allows it.
    if (activeCoinCount < MAX_ACTIVE_COINS && (rand() % 100) < 5) {
        spawnCoin();
    }
}


void iDraw() {
    iClear();

    // Draw background and clouds
    iShowLoadedImage(0, 0, &skyImg);
    iShowLoadedImage(c1x, c1y, &cloudImg[0]);
    iShowLoadedImage(c2x, c2y, &cloudImg[1]);
    iShowLoadedImage(c3x, c3y, &cloudImg[2]);
    iShowLoadedImage(c4x, c4y, &cloudImg[3]);
    iShowLoadedImage(c5x, c5y, &cloudImg[4]);
    iShowLoadedImage(c6x, c6y, &cloudImg[5]);
    iShowLoadedImage(c7x, c7y, &cloudImg[6]);

    if (gamestate == 0) {
        // Main menu
        iShowLoadedImage(600, 530, &menuImghab);
        for (int i = 0; i < 5; i++) {
            if (hoverMenu[i]) 
                iShowLoadedImage(menuX[i] - 10, menuY[i] - 10, &menuBigImg[i]);
            else 
                iShowLoadedImage(menuX[i], menuY[i], &menuImg[i]);
        }
    }
    else if (gamestate == 1) {
        // Gameplay
        iShowSprite(&habimg[bi]);
        for (int i = 0; i < 3; i++) {
            if (isObsActive[i]) {
                iShowSprite(&obsimg[i]);
                if (iCheckCollision(&habimg[bi], &obsimg[i])) {
                    showHeartbreak = true;
                    heartbreakCounter = 60;
                    iPauseSound(easy);
                    iPlaySound("wrong.wav", false, 80);
                    health--;
                    isObsActive[i] = false;

                    if (health <= 0) {
                        finalScore = score;
                        addHighScore(playerName, finalScore);
                        gamestate = 6;  // Switch to Game Over screen
                    } else {
                        iResumeSound(easy);
                    }
                }
            }
        }
                if (iCheckCollision(&habimg[bi], &bird1)) {
                    bird1x= -100; // Reset bird position
                    showHeartbreak = true;
                    heartbreakCounter = 60;
                    iPauseSound(easy);
                    iPlaySound("wrong.wav", false, 80);
                    health--;

                    if (health <= 0) {
                        finalScore = score;
                        addHighScore(playerName, finalScore);
                        gamestate = 6;  // Switch to Game Over screen
                    } else {
                        iResumeSound(easy);
                    }
                }
                if(iCheckCollision(&habimg[bi], &bird2)) {
                    bird2x= -100; // Reset bird position
                    showHeartbreak = true;
                    heartbreakCounter = 60;
                    iPauseSound(easy);
                    iPlaySound("wrong.wav", false, 80);
                    health--;

                    if (health <= 0) {
                        finalScore = score;
                        addHighScore(playerName, finalScore);
                        gamestate = 6;  // Switch to Game Over screen
                    } else {
                        iResumeSound(easy);
                    }
                }
                if(iCheckCollision(&habimg[bi], &bird3)) {
                    bird3x= -100; // Reset bird position
                    showHeartbreak = true;
                    heartbreakCounter = 60;
                    iPauseSound(easy);
                    iPlaySound("wrong.wav", false, 80);
                    health--;
                    if(health <= 0) {
                        finalScore = score;
                        addHighScore(playerName, finalScore);
                        gamestate = 6;  // Switch to Game Over screen
                    } else {
                        iResumeSound(easy);
                    }
                    
                }

        int habLeft = x;
        int habRight = x + habWidth;
        int habBottom = y;
        int habTop = y + habHeight;
                
        for (size_t i = 0; i < coins.size(); ++i) {
            if (coins[i].active) {
                // Draw the current rotation frame of the coin
                iShowLoadedImage(coins[i].x, coins[i].y, &coinFrames[coins[i].currentFrame]);

                // Player-Coin Collision (AABB check)
                int coinLeft = coins[i].x;
                int coinRight = coins[i].x + coinWidth;
                int coinBottom = coins[i].y;
                int coinTop = coins[i].y + coinHeight;

                if (habLeft < coinRight && habRight > coinLeft &&
                    habBottom < coinTop && habTop > coinBottom) {
                    // Collision with coin!
                    score += 10; // Increase score for collecting a coin
                    coinsCollected++; // Increment coinsCollected
                    coins[i].active = false; // Deactivate the collected coin
                    iPlaySound("coin.wav", false, 80); // Play coin collection sound (requires coin.wav)
                }
            }
        }

        if (showHeartbreak) {
            iShowLoadedImage(700, 400, &heartbreak);
            if (--heartbreakCounter <= 0) 
                showHeartbreak = false;
        }

        char str[30];
        sprintf(str, "Score: %d", score);
        iText(10, 760, str, GLUT_BITMAP_HELVETICA_18);
        sprintf(str, "Speed: %d", obstacleSpeed);
        iText(10, 740, str, GLUT_BITMAP_HELVETICA_18);
       for (int i = 0; i < health; ++i) {
            int currentIconX = HEALTH_ICON_START_X + (healthIcon.width + HEALTH_ICON_GAP) * i;
            iShowLoadedImage(currentIconX, HEALTH_ICON_Y, &healthIcon);
        }
 // Display Coin Icon and Coins Collected Value
        iShowLoadedImage(10, 680, &coinCollectIcon); // Position the coin icon below health
        char coinCountStr[30]; // Buffer for coins collected number
        sprintf(coinCountStr, "%d", coinsCollected); // Just the number
        iText(10 + coinCollectIcon.width + 5, 685, coinCountStr, GLUT_BITMAP_HELVETICA_18); // Position number next to icon

        // Show Back button in gameplay
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &back);
    
        if (bird1active) {
    iShowSprite(&bird1);
    }
    if( bird2active) {
    iShowSprite(&bird2);
    }
    if( bird3active) {
    iShowSprite(&bird3); // <-- Add this line to show bird3
    }
    if (evilActive) {
    iShowSprite(&evilBalloon);

    if (iCheckCollision(&habimg[bi], &evilBalloon)) {
        // On collision
        evilActive = false;
        evilX = 1400;
        evilY = 750;
        showHeartbreak = true;
        heartbreakCounter = 60;
        iPauseSound(easy);
        iPlaySound("wrong.wav", false, 80);
        health--;

        if (health <= 0) {
            finalScore = score;
            addHighScore(playerName, finalScore);
            gamestate = 6;  // Game over
        } else {
            iResumeSound(easy);
        }
    }
}

}
    else if (gamestate == 3) {
        // Name input screen
        iSetColor(5, 5, 255);
         int enterNameX = (1400 - 200) / 2; // Adjusted for visual centering on a 1400px width
        iText(enterNameX, 400, "Enter Your Name:", GLUT_BITMAP_TIMES_ROMAN_24);

        // Player name input display - adjust for centering
        // This is a basic estimation. For more precise centering,
        // you'd typically need to measure the string's pixel width.
        int playerNameDisplayX = (1400 - (playerNameIndex * 12)) / 2; // Rough estimate: 12px per char
        iText(playerNameDisplayX, 370, playerName, GLUT_BITMAP_HELVETICA_18);

        // Centering "Press ENTER to start the game."
        // Estimated x for centering 'Press ENTER to start the game.'
        
        iText(560, 300, "Press ENTER to start the game.", GLUT_BITMAP_HELVETICA_18);

        // Show Back button
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &back);
}
    else if (gamestate == 4) {
        // About screen
        iShowImage(0, 0, "aboutus.PNG");
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &back);
    }
    else if (gamestate == 6) {
       char gameOverStr[] = "GAME OVER";
        char finalScoreStr[50];
        sprintf(finalScoreStr, "FINAL SCORE: %d", finalScore);
        char collectedCoinsGameOverStr[50];
        sprintf(collectedCoinsGameOverStr, "COINS COLLECTED: %d", coinsCollected);

        // Calculate center positions for text
        int screenWidth = 1400; // Assuming your window width is 1400
        int screenHeight = 800; // Assuming your window height is 800

        // Game Over Text
        iSetColor(255, 0, 0); // Red color for Game Over
        // Approximate center based on visual testing for GLUT_BITMAP_TIMES_ROMAN_24
        int gameOverX = (screenWidth - 150) / 2;
        int gameOverY = screenHeight / 2 + 50; // Above center
        iText(gameOverX, gameOverY, gameOverStr, GLUT_BITMAP_TIMES_ROMAN_24);

        // Final Score Text
        iSetColor(0, 0, 255); // Blue color for Final Score
        int finalScoreX = (screenWidth - 250) / 2; // Approximate center
        int finalScoreY = screenHeight / 2; // Center
        iText(finalScoreX, finalScoreY, finalScoreStr, GLUT_BITMAP_TIMES_ROMAN_24);

        // Coins Collected Text (also blue), displayed below Final Score
        int coinsCollectedX = (screenWidth - 300) / 2;
        int coinsCollectedY = screenHeight / 2 - 50; // Below Final Score
        iText(coinsCollectedX, coinsCollectedY, collectedCoinsGameOverStr, GLUT_BITMAP_TIMES_ROMAN_24);

        // Show Back button to return to main menu
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &back);
    }
    else if (gamestate == 2) {
    iSetColor(255, 255, 255); // Default to White for "High Scores" title

        // Center "High Scores" title
int highScoreTitleTextWidth = 150; // Estimated width for "High Scores" with HELVETICA_18
        int highScoreTitleX = (1400 - highScoreTitleTextWidth) / 2;
        iText(highScoreTitleX, 700, "High Scores", GLUT_BITMAP_HELVETICA_18);

        int startY = 650; // Starting Y for the first score entry
        for (size_t i = 0; i < highScores.size(); i++) {
            char scoreLine[100];
            string displayName = highScores[i].name;
            for(char &c : displayName) if (c == '_') c = ' '; // Replace underscores back to spaces for display

            sprintf(scoreLine, "%d. %s - %d", (int)(i+1), displayName.c_str(), highScores[i].score);

            // Set color based on rank
            if (i == 0) { // First place
                iSetColor(255, 0, 0); // Red
            } else if (i == 1) { // Second place
                iSetColor(0, 255, 0); // Green
            } else if (i == 2) { // Third place
                iSetColor(0, 0, 255); // Blue
            } else { // Rest of the players
                iSetColor(255, 255, 255); // White
            }

            // Estimate the width of the dynamic score line
            // This is an approximation. A common character width for GLUT_BITMAP_HELVETICA_18 is around 10-11 pixels.
            // Adjust estimatedLineLength based on your longest possible score line (e.g., "5. LongPlayerName - 99999" could be around 25-30 characters).
            int estimatedLineLength = 30; // Max estimated characters in a score line
            int charWidthEstimate = 10;   // Estimated average pixel width per character for GLUT_BITMAP_HELVETICA_18
            int scoreLinePixelWidth = estimatedLineLength * charWidthEstimate;

            // Calculate centered X position for the current score line
            int centeredScoreLineX = (1400 - scoreLinePixelWidth) / 2;

            iText(centeredScoreLineX, startY - i * 30, scoreLine, GLUT_BITMAP_HELVETICA_18);
        }
        // Back button to return to main menu
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &back);
}

    // Show Back button on other screens except main menu and game over (which has own back button)
    if (gamestate > 0 && gamestate <= 5 && gamestate != 6) {
        iShowLoadedImage(420, 740, &back);
    }
}
void iMouseMove(int mx, int my)
{
    // place your codes here
     for (int i = 0; i < 5; i++)
    {
        hoverMenu[i] = (mx >= menuX[i] && mx <= menuX[i] + menuW &&
                        my >= menuY[i] && my <= menuY[i] + menuH);
    }
}
void iMouseDrag(int mx, int my)
{
    // place your codes here
}
void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if(gamestate== 0) 
        {
            if(mx >= 350 && mx <= (350+190) && my >=220 && my <= (220+268)) // Start Game
            {
               gamestate = 3;
               inputName = true;
              playerName[0] = '\0';
               playerNameIndex = 0;
                iResumeSound(easy);
            }
            else if(mx >= 850 && mx <=(850+190) && my >=200 && my <=(200+268)) // high
            {
                gamestate=2;
                // Show instructions (not implemented)
            }
            else if(mx >= 100 && mx <= (100+190)&& my >= 20 && my <= (20+268)) // help
            {
                 
            }
            else if(mx >= 610 && mx <= (610+190) && my >= 20 && my <=288) // About
            {
                gamestate = 4;
            }
            else if(mx >=1120 && mx <= (1120+190)&& my >= 20 && my <=288 ) // exit
            {
                exit(0);
            }

        }
        if (gamestate >= 1 && gamestate <= 6)
    {
            if(mx >= 420 && mx <= 500 && my >= 740 && my <= 800) // Back button
            {
                gamestate = 0;
                iPauseSound(easy);
                iResumeSound(easy);
            }
        }
            
    }
}
void iMouseWheel(int dir, int mx, int my)
{
    // place your code here
}
void iKeyboard(unsigned char key)
{
    if (gamestate == 3 && inputName) {
    if (key == '\b') { // backspace
        if (playerNameIndex > 0) {
            playerNameIndex--;
            playerName[playerNameIndex] = '\0';
        }
    } else if (key == '\r' || key == '\n') { // enter
        inputName = false;
        gamestate = 1;
        easy = iPlaySound("Easy.wav", true, 80);

        // Reset game values
        score = 0;
        scoretick = 0;
        health = 3;
        obstacleSpeed = 2.0;
        coinsCollected = 0;
        coinSpeed = obstacleSpeed;

        for (int i = 0; i < 3; i++) {
            obsy[i] = 800 + i * 267;
            obsx[i] = rand() % 1300;
            obsi[i] = rand() % 3;
            isObsActive[i] = true;
            iChangeSpriteFrames(&obsimg[i], &obsimgfrm[obsi[i]], 1);
            iSetSpritePosition(&obsimg[i], obsx[i], obsy[i]);
        }
        for (size_t i = 0; i < coins.size(); ++i) {
                coins[i].active = false; // Deactivate all existing coins
            }
            // Spawn initial set of coins to populate the screen (up to MAX_ACTIVE_COINS)
            for (int i = 0; i < MAX_ACTIVE_COINS; ++i) {
                spawnCoin();
            }


    } else if (playerNameIndex < sizeof(playerName) - 1) {
        if (isalnum(key) || key == ' ') {
            playerName[playerNameIndex++] = key;
            playerName[playerNameIndex] = '\0';
        }
    }
}

}
void iSpecialKeyboard(unsigned char key)
{
   // start moving left
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    srand(time(0));
     for(int i = 0; i < 14; ++i) {
        sprintf(coinFiles[i], "coin%d.png", i + 1); // Generates "coin1.png", "coin2.png", etc.
    }
    for(int i = 0; i < 14; ++i) {
        iLoadImage(&coinFrames[i], coinFiles[i]);
        // Resize each coin frame to your desired dimensions (e.g., 30x30 pixels for collected coins)
        iResizeImage(&coinFrames[i], 30, 30); // These are the coins that move on screen
    }
    // Get coin dimensions from the first loaded image (now it will be the resized dimension)
    if (14 > 0) {
        coinWidth = coinFrames[0].width;
        coinHeight = coinFrames[0].height;
    }

    // Load balloon images first
    for (int j = 0; j < 10; j++) {
        iLoadImage(&habimgfrm[j], hab[j]);
    }


    for (int i = 0; i < 9; i++) {
    iLoadImage(&bird1frm[i], bird1files[i]);
}
for(int i=0; i < 6; i++) {
    iLoadImage(&bird2frm[i], bird2files[i]);
}
for(int i=0; i < 10; i++) {
    iLoadImage(&bird3frm[i], bird3files[i]);
}
iInitSprite(&bird1);
iInitSprite(&bird2);
iInitSprite(&bird3);
iChangeSpriteFrames(&bird1, bird1frm, 9);
iSetSpritePosition(&bird1, bird1x, bird1y);
iChangeSpriteFrames(&bird2, bird2frm, 6);
iSetSpritePosition(&bird2, bird2x, bird2y);
iChangeSpriteFrames(&bird3, bird3frm, 10);
iSetSpritePosition(&bird3, bird3x, bird3y);
iLoadImage(&evilBalloonImg, "evilballoon.png");
iInitSprite(&evilBalloon);
iChangeSpriteFrames(&evilBalloon, &evilBalloonImg, 1);
iSetSpritePosition(&evilBalloon, evilX, evilY);

iSetTimer(100, moveBird1);
iSetTimer(100, moveBird2);
iSetTimer(100, moveBird3);
    // Initialize balloon sprites & set frames & position
    for (int i = 0; i < 10; i++) {
        iInitSprite(&habimg[i]);
        iChangeSpriteFrames(&habimg[i], &habimgfrm[i], 1);
        iSetSpritePosition(&habimg[i], x, y);
    }
    loadHighScores(); 

    // Load obstacle images
    for (int i = 0; i < 3; i++) {
        iLoadImage(&obsimgfrm[i], obs[i]);
    }

    // Initialize obstacle sprites & set frames & position
    for (int i = 0; i < 3; i++) {
        iInitSprite(&obsimg[i]);
        obsi[i] = rand() % 3; // Random obstacle image index
        iChangeSpriteFrames(&obsimg[i], &obsimgfrm[obsi[i]], 1);
        obsx[i] = rand() % 1300;
        obsy[i] = 800 + i * 267;
        iSetSpritePosition(&obsimg[i], obsx[i], obsy[i]);
    }

    // Load other images (clouds, menus, backgrounds)
    for (int i = 0; i < 7; i++) {
        iLoadImage(&cloudImg[i], cloud[i]);
    }
    for (int i = 0; i < 5; i++) {
        iLoadImage(&menuImg[i], menu[i]);
        iLoadImage(&menuBigImg[i], menuBig[i]);
    }
    iLoadImage(&menuImghab, "menuhab.png");
    iLoadImage(&skyImg, "sky.jpg");
    iLoadImage(&heartbreak, "heartbreak.png");
    iLoadImage(&back, "backbutton.png");
    iLoadImage(&healthIcon, "Healthicon.png");
    iResizeImage(&healthIcon, 30, 30);
    iLoadImage(&coinCollectIcon, "Coinicon.png"); // Load the Coinicon.png file (was .jpg)
    iResizeImage(&coinCollectIcon, 30, 30);
    // Setup timers
    iSetTimer(500, incmnt);
    iSetTimer(16, cldx);
    iSetTimer(16, cldy);
    iSetTimer(1, move);
    iSetTimer(16, obstacle);
    iSetTimer(33, scoreupdate);
    iSetTimer(50, coinAnimate); // Coin rotation animation (change frame every 50ms)
    iSetTimer(16, coinMovement);
    iInitializeSound();
    iInitialize(1400, 800, "Hot Air Balloon");
    return 0;
}
