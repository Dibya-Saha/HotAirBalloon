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

// --- Existing Game Data Structures ---
struct PlayerScore
{
    string name;
    int score;
    bool operator<(const PlayerScore &other) const
    {
        return score > other.score;
    }
};

vector<PlayerScore> highScores;
const string HIGHSCORE_FILE = "highscores.txt";
const int MAX_HIGHSCORES = 10;

char playerName[50] = "";
int playerNameIndex = 0;
bool inputName = false;
int finalScore = 0;
int obstacleSpeed = 2; // Changed to int
int health = 3;        // Initial health value
int coinsCollected = 0;

char hab[10][13] = {"hab001_1.png", "hab002_1.png", "hab003_1.png", "hab004_1.png", "hab005_1.png",
                    "hab006_1.png", "hab007_1.png", "hab008_1.png", "hab009_1.png", "hab010_1.png"};
char obs[3][10] = {"obs1.png", "obs2.png", "obs3.png"};
char cloud[7][13] = {"cloud1.png", "cloud2.png", "cloud3.png", "cloud4.png", "cloud5.png", "cloud6.png", "cloud7.png"};
char menu[5][15] = {"menu_1.png", "menu_2.png", "menu_3.png", "menu_4.png", "menu_5.png"};
char menuBig[5][20] = {"menu_1_big.png", "menu_2_big.png", "menu_3_big.png", "menu_4_big.png", "menu_5_big.png"};

bool hoverMenu[5] = {false};
int menuX[5] = {350, 850, 100, 610, 1120};
int menuY[5] = {260, 240, 60, 60, 60};
int menuW = 200, menuH = 200;

Image menuBigImg[5], cloudImg[7], skyImg, heartbreak, menuImg[5], menuImghab, back;
Image habimgfrm[10], obsimgfrm[3];
Sprite habimg[10], obsimg[3];

int gamestate = 0;
int score = 0, scoretick = 0;
int bi = 0;                               // Current frame index for hot air balloon animation
int obsx[3], obsy[3], obsi[3];            // X, Y, and image index for obstacles
int x = 700, y = 70;                      // Player balloon position (for habimg[bi] sprite)
int easy;                                 // Sound handle for background music
bool isObsActive[3] = {true, true, true}; // Track if an obstacle is active for collision
bool showHeartbreak = false;
int heartbreakCounter = 0; // Timer for heartbreak image display

// Cloud positions for parallax effect
double c1x = 200, c2x = 350, c3x = 500, c4x = 800, c5x = 600, c6x = 920, c7x = 1225;
double c1y = 114, c2y = 229, c3y = 343, c4y = 457, c5y = 500, c6y = 686, c7y = 800;

// --- Coin Data Structures ---
struct Coin
{
    int x, y;
    int currentFrame; // Index for the rotating coin image (0-13)
    bool active;      // Is this coin currently on screen and interactable?
};

char coinFiles[14][15]; // Stores filenames like "coin1.png", "coin2.png", ...
Image coinFrames[14];   // Stores the loaded image data for each coin frame
vector<Coin> coins;     // A dynamic array to manage all coins
int coinWidth = 20;     // Will be set after loading the first coin image
int coinHeight = 20;    // Will be set after loading the first coin image
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
Image healthIcon;      // Declared globally
Image coinCollectIcon; // Declare coin collection icon

// --- Constants for Health Icon Display ---
const int HEALTH_ICON_START_X = 10; // Starting X position for the first health icon
const int HEALTH_ICON_Y = 715;      // Y position for all health icons
const int HEALTH_ICON_GAP = 5;      // Gap between health icons

// --- Add these two new global variables near your other global score/speed variables ---
int lastSpeedUpScore50 = 0;
int lastSpeedUpScore100 = 0;

void scoreupdate()
{
    scoretick++;
    if (scoretick % 9 == 0) // Score increments every 9 ticks
    {
        score++; // Continuous score increment

        // Calculate total score including collected coins
        int totalScore = score + (coinsCollected * 10);

        // First, handle the rule for speed < 10
        // Speed up every 50 totalScore points, but only if obstacleSpeed is less than 10
        // AND we haven't already sped up at this 50-point interval.
        if (obstacleSpeed < 10 && totalScore > 0 && totalScore % 30 == 0 && totalScore > lastSpeedUpScore50)
        {
            obstacleSpeed += 2;
            lastSpeedUpScore50 = totalScore; // Mark this score as the last point we sped up for the 50-interval rule
        }

        // Then, independently handle the rule for speed >= 10
        // Speed up every 100 totalScore points, but only if obstacleSpeed is 10 or more
        // AND we haven't already sped up at this 100-point interval.
        if (obstacleSpeed >= 10 && totalScore > 0 && totalScore % 50 == 0 && totalScore > lastSpeedUpScore100)
        {
            obstacleSpeed += 1;
            lastSpeedUpScore100 = totalScore; // Mark this score as the last point we sped up for the 100-interval rule
        }

        // Finally, apply the maximum speed limit
        if (obstacleSpeed > 20)
        {
            obstacleSpeed = 20;
        }

        // Keep coin speed in sync with obstacle speed
        coinSpeed = obstacleSpeed;
    }
}
void loadHighScores()
{
    highScores.clear();
    ifstream file(HIGHSCORE_FILE);
    if (file.is_open())
    {
        string name;
        int score;
        while (file >> name >> score)
        {
            highScores.push_back({name, score});
        }
        file.close();
        sort(highScores.begin(), highScores.end());
    }
}

void saveHighScores()
{
    ofstream file(HIGHSCORE_FILE);
    for (int i = 0; i < min((int)highScores.size(), MAX_HIGHSCORES); ++i)
    {
        file << highScores[i].name << " " << highScores[i].score << endl;
    }
    file.close();
}

void addHighScore(string name, int score)
{
    for (char &c : name)
        if (c == ' ')
            c = '_'; // Replace spaces with underscores for file storage
    highScores.push_back({name, score});
    sort(highScores.begin(), highScores.end());
    if (highScores.size() > MAX_HIGHSCORES)
        highScores.resize(MAX_HIGHSCORES); // Keep only the top scores
    saveHighScores();
    loadHighScores(); // Reload to ensure data integrity and sorted order
}

void move()
{
    if (isSpecialKeyPressed(GLUT_KEY_RIGHT))
        x += 15;
    if (isSpecialKeyPressed(GLUT_KEY_LEFT))
        x -= 15;

    
    if (x + habWidth > 1400)
    {
        x = 0; // Wrap it to appear immediately at the far left edge of the screen
    }
  
    if (x < 0)
    {
        x = 1400 - habWidth; // Wrap it to appear immediately at the far right edge of the screen
    }

    iSetSpritePosition(&habimg[bi], x, y);
}

void obstacle()
{
    for (int i = 0; i < 3; i++)
    {
        if (isObsActive[i])
        { // Only move active obstacles
            obsy[i] -= obstacleSpeed;
            if (obsy[i] < -obsimgfrm[obsi[i]].height)
            {                                                            // Check if obstacle is completely off-screen
                obsy[i] = 800;                                           // Reset to top
                obsx[i] = rand() % (1400 - obsimgfrm[obsi[i]].width);    // Random X within window width
                obsi[i] = rand() % 3;                                    // Random obstacle image
                iChangeSpriteFrames(&obsimg[i], &obsimgfrm[obsi[i]], 1); // Update sprite frame
            }
            iSetSpritePosition(&obsimg[i], obsx[i], obsy[i]); // Update sprite position
        }
        else
        {                             // Obstacle is inactive (e.g., just collided)
            obsy[i] -= obstacleSpeed; // Still move it off-screen
            if (obsy[i] < -obsimgfrm[obsi[i]].height)
            { // Once off-screen, reactivate and reset
                isObsActive[i] = true;
                obsy[i] = 800;
                obsx[i] = rand() % (1400 - obsimgfrm[obsi[i]].width);
                obsi[i] = rand() % 3;
                iChangeSpriteFrames(&obsimg[i], &obsimgfrm[obsi[i]], 1); // Update sprite frame
            }
            iSetSpritePosition(&obsimg[i], obsx[i], obsy[i]); // Update sprite position
        }
    }
}

void incmnt()
{                       // Hot air balloon animation
    bi = (bi + 1) % 10; // Cycle through 10 frames
 
    iChangeSpriteFrames(&habimg[bi], &habimgfrm[bi], 1);
    iSetSpritePosition(&habimg[bi], x, y);
}

void cldx()
{ // Cloud horizontal movement
    double *cx[] = {&c1x, &c2x, &c3x, &c4x, &c5x, &c6x, &c7x};
    double speed[] = {.2, -.1, -.3, .5, .25, -.3, .8}; // Different speeds for parallax
    for (int i = 0; i < 7; i++)
    {
        *cx[i] += speed[i];
        // Wrap clouds around horizontally
        if (speed[i] > 0 && *cx[i] > 1450)
            *cx[i] = -150;
        else if (speed[i] < 0 && *cx[i] < -150)
            *cx[i] = 1450;
    }
}

void cldy()
{ // Cloud vertical movement
    double *cy[] = {&c1y, &c2y, &c3y, &c4y, &c5y, &c6y, &c7y};
    for (int i = 0; i < 7; i++)
    {
        *cy[i] -= 2; // Move downwards
        if (*cy[i] < -50)
            *cy[i] = 800; // Reset to top
    }
}

// --- Coin Functions ---

// Updates the animation frame for all active coins
void coinAnimate()
{
    for (size_t i = 0; i < coins.size(); ++i)
    {
        if (coins[i].active)
        {
            coins[i].currentFrame = (coins[i].currentFrame + 1) % 14; // Cycle through 14 frames
        }
    }
}

// Checks if a potential coin position (bounding box) overlaps with any active obstacle
bool checkCoinObstacleCollision(int coinLeft, int coinRight, int coinBottom, int coinTop)
{
    for (int i = 0; i < 3; ++i)
    {
        if (isObsActive[i])
        {
            // Get current bounding box of the obstacle
            int obsLeft = obsx[i];
            int obsRight = obsx[i] + obsimgfrm[obsi[i]].width;
            int obsBottom = obsy[i];
            int obsTop = obsy[i] + obsimgfrm[obsi[i]].height;

            // Check for Axis-Aligned Bounding Box (AABB) overlap
            if (coinLeft < obsRight && coinRight > obsLeft &&
                coinBottom < obsTop && coinTop > obsBottom)
            {
                return true; // Collision found
            }
        }
    }
    return false; // No collision with any active obstacle
}

// Spawns a coin at a random position, avoiding active obstacles AND other coins
void spawnCoin()
{
    // Find an inactive coin slot in our vector, or add a new one if space allows
    int coinIdx = -1;
    for (size_t i = 0; i < coins.size(); ++i)
    {
        if (!coins[i].active)
        {
            coinIdx = i;
            break;
        }
    }
    if (coinIdx == -1 && coins.size() < MAX_ACTIVE_COINS)
    {
        coins.push_back({}); // Add a new empty Coin struct
        coinIdx = coins.size() - 1;
    }
    else if (coinIdx == -1)
    {           // No inactive coins and already at max capacity
        return; // Cannot spawn more coins right now
    }

    Coin &newCoin = coins[coinIdx]; // Get a reference to the coin we're about to use

    bool positionFound = false;
    const int MAX_SPAWN_RETRIES = 50; // Increased retries for better chance of finding a spot
    for (int retry = 0; retry < MAX_SPAWN_RETRIES; ++retry)
    {
        // Generate potential X within screen bounds, considering coin width
        int potentialX = rand() % (1400 - coinWidth);
        // Generate potential Y above the screen, staggered, to give coins time to separate
        int potentialY = 800 + (rand() % 400);

        int currentCoinLeft = potentialX;
        int currentCoinRight = potentialX + coinWidth;
        int currentCoinBottom = potentialY;
        int currentCoinTop = potentialY + coinHeight;

        // Check for collision with obstacles first
        if (checkCoinObstacleCollision(currentCoinLeft, currentCoinRight, currentCoinBottom, currentCoinTop))
        {
            continue; // Retry if it collides with an obstacle
        }

        // --- NEW: Check for collision/proximity with other active coins ---
        bool overlapsWithOtherCoins = false;
        for (size_t i = 0; i < coins.size(); ++i)
        {
            if (coins[i].active && i != (size_t)coinIdx)
            {
                // Calculate distance between centers. Using squared distance for performance.
                double distSq = pow((potentialX + coinWidth / 2.0) - (coins[i].x + coinWidth / 2.0), 2) +
                                pow((potentialY + coinHeight / 2.0) - (coins[i].y + coinHeight / 2.0), 2);

                // Compare with squared MIN_COIN_SEPARATION
                if (distSq < pow(MIN_COIN_SEPARATION, 2))
                {
                    overlapsWithOtherCoins = true;
                    break;
                }
            }
        }

        if (overlapsWithOtherCoins)
        {
            continue; // Retry if it overlaps or is too close to another coin
        }
        

        // If no obstacle or coin overlap, this position is good
        newCoin.x = potentialX;
        newCoin.y = potentialY;
        newCoin.currentFrame = rand() % 14;
        newCoin.active = true;
        positionFound = true;
        break;
    }

    if (!positionFound)
    {
        newCoin.active = false; // If no clear spot found after retries, keep it inactive
    }
}

// Moves active coins downwards and handles off-screen coins; also triggers spawning
void coinMovement()
{
    int activeCoinCount = 0;
    for (size_t i = 0; i < coins.size(); ++i)
    {
        if (coins[i].active)
        {
            coins[i].y -= coinSpeed; // Move coin downwards using the dynamic speed

            if (coins[i].y < -coinHeight)
            {                            // Coin went completely off-screen
                coins[i].active = false; // Deactivate it
            }
            activeCoinCount++; // Count active coins
        }
    }

 
    if (activeCoinCount < MAX_ACTIVE_COINS && (rand() % 100) < 5)
    {
        spawnCoin();
    }
}

void iDraw()
{
    iClear();

    // Draw background and clouds (always visible)
    iShowLoadedImage(0, 0, &skyImg);
    iShowLoadedImage(c1x, c1y, &cloudImg[0]);
    iShowLoadedImage(c2x, c2y, &cloudImg[1]);
    iShowLoadedImage(c3x, c3y, &cloudImg[2]);
    iShowLoadedImage(c4x, c4y, &cloudImg[3]);
    iShowLoadedImage(c5x, c5y, &cloudImg[4]);
    iShowLoadedImage(c6x, c6y, &cloudImg[5]);
    iShowLoadedImage(c7x, c7y, &cloudImg[6]);

    if (gamestate == 0)
    {
        // Main menu
        iShowLoadedImage(500, 530, &menuImghab);
        for (int i = 0; i < 5; i++)
        {
            if (hoverMenu[i])
                iShowLoadedImage(menuX[i] - 10, menuY[i] - 10, &menuBigImg[i]);
            else
                iShowLoadedImage(menuX[i], menuY[i], &menuImg[i]);
        }
    }
    else if (gamestate == 1)
    {
      
        iShowSprite(&habimg[bi]);

        // For collision, use the global x, y, habWidth, habHeight that track the balloon's position and size
        int habLeft = x;
        int habRight = x + habWidth;
        int habBottom = y;
        int habTop = y + habHeight;

        // Draw and handle obstacles
        for (int i = 0; i < 3; i++)
        {
            if (isObsActive[i])
            {
                iShowSprite(&obsimg[i]);

                // Player-Obstacle Collision using iCheckCollision for sprites
                if (iCheckCollision(&habimg[bi], &obsimg[i]))
                {
                    showHeartbreak = true;
                    heartbreakCounter = 60;             // Show heartbreak for 60 frames (approx 1 sec at 60 FPS)
                    iPauseSound(easy);                  // Pause background music
                    iPlaySound("wrong.wav", false, 80); // Play collision sound
                    health--;                           // Decrease health
                    isObsActive[i] = false;             // Deactivate this specific obstacle after collision

                    if (health <= 0)
                    {
                        finalScore = score;
                        addHighScore(playerName, finalScore);
                        gamestate = 6; // Switch to Game Over screen
                    }
                    else
                    {
                        iResumeSound(easy); // Resume background music if not game over
                    }
                }
            }
        }

        // --- Coin Drawing and Player-Coin Collision ---
        for (size_t i = 0; i < coins.size(); ++i)
        {
            if (coins[i].active)
            {
                // Draw the current rotation frame of the coin
                iShowLoadedImage(coins[i].x, coins[i].y, &coinFrames[coins[i].currentFrame]);

                // Player-Coin Collision (AABB check)
                int coinLeft = coins[i].x;
                int coinRight = coins[i].x + coinWidth;
                int coinBottom = coins[i].y;
                int coinTop = coins[i].y + coinHeight;

                if (habLeft < coinRight && habRight > coinLeft &&
                    habBottom < coinTop && habTop > coinBottom)
                {
                    // Collision with coin!
                    score += 10;                       // Increase score for collecting a coin
                    coinsCollected++;                  // Increment coinsCollected
                    coins[i].active = false;           // Deactivate the collected coin
                    iPlaySound("coin.wav", false, 80); // Play coin collection sound (requires coin.wav)
                }
            }
        }
    
        if (showHeartbreak)
        {
            // Calculate center position for heartbreak image
            int heartbreakX = (1400 - heartbreak.width) / 2; // Screen width 1400
            int heartbreakY = (800 - heartbreak.height) / 2; // Screen height 800
            iShowLoadedImage(heartbreakX, heartbreakY, &heartbreak);
            if (--heartbreakCounter <= 0) // Decrement counter, hide when 0 or less
                showHeartbreak = false;
        }

        // Display game stats
        char str[30];
        sprintf(str, "Score: %d", score);
        iText(10, 770, str, GLUT_BITMAP_HELVETICA_18);
        sprintf(str, "Speed: %d", obstacleSpeed); // Display as int
        iText(10, 750, str, GLUT_BITMAP_HELVETICA_18);

      
        for (int i = 0; i < health; ++i)
        {
            int currentIconX = HEALTH_ICON_START_X + (healthIcon.width + HEALTH_ICON_GAP) * i;
            iShowLoadedImage(currentIconX, HEALTH_ICON_Y, &healthIcon);
        }
      
        iShowLoadedImage(10, 680, &coinCollectIcon);                                        // Position the coin icon below health
        char coinCountStr[30];                                                              // Buffer for coins collected number
        sprintf(coinCountStr, "%d", coinsCollected);                                        // Just the number
        iText(10 + coinCollectIcon.width + 5, 685, coinCountStr, GLUT_BITMAP_HELVETICA_18); // Position number next to icon

   
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &back);
    }
    else if (gamestate == 3)
    {
        
        iSetColor(0, 0, 255); // Blue color

        int enterNameX = (1400 - 200) / 2; // Adjusted for visual centering on a 1400px width
        iText(enterNameX, 400, "Enter Your Name:", GLUT_BITMAP_TIMES_ROMAN_24);

        
        int playerNameDisplayX = (1400 - (playerNameIndex * 12)) / 2; // Rough estimate: 12px per char
        iText(playerNameDisplayX, 370, playerName, GLUT_BITMAP_HELVETICA_18);

    

        iText(560, 300, "Press ENTER to start the game.", GLUT_BITMAP_HELVETICA_18);

        // Show Back button
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &back);
    }
    else if (gamestate == 4)
    {
        // About screen
        iShowImage(0, 0, "aboutus.PNG"); // Ensure aboutus.PNG exists

        // Show Back button
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &back);
    }
    else if (gamestate == 6)
    {
        // Game Over screen
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
        iSetColor(0, 0, 255);                      // Blue color for Final Score
        int finalScoreX = (screenWidth - 250) / 2; // Approximate center
        int finalScoreY = screenHeight / 2;        // Center
        iText(finalScoreX, finalScoreY, finalScoreStr, GLUT_BITMAP_TIMES_ROMAN_24);

        // Coins Collected Text (also blue), displayed below Final Score
        int coinsCollectedX = (screenWidth - 300) / 2;
        int coinsCollectedY = screenHeight / 2 - 50; // Below Final Score
        iText(coinsCollectedX, coinsCollectedY, collectedCoinsGameOverStr, GLUT_BITMAP_TIMES_ROMAN_24);

      
        iShowLoadedImage(BACK_BUTTON_X, BACK_BUTTON_Y, &back);
    }
    else if (gamestate == 2)
    {
        // High Scores screen
        iSetColor(255, 255, 255); // Default to White for "High Scores" title

        // Center "High Scores" title
        int highScoreTitleTextWidth = 150; // Estimated width for "High Scores" with HELVETICA_18
        int highScoreTitleX = (1400 - highScoreTitleTextWidth) / 2;
        iText(highScoreTitleX, 700, "High Scores", GLUT_BITMAP_HELVETICA_18);

        int startY = 650; // Starting Y for the first score entry
        for (size_t i = 0; i < highScores.size(); i++)
        {
            char scoreLine[100];
            string displayName = highScores[i].name;
            for (char &c : displayName)
                if (c == '_')
                    c = ' '; // Replace underscores back to spaces for display

            sprintf(scoreLine, "%d. %s - %d", (int)(i + 1), displayName.c_str(), highScores[i].score);

            // Set color based on rank
            if (i == 0)
            {                         // First place
                iSetColor(255, 0, 0); // Red
            }
            else if (i == 1)
            {                         // Second place
                iSetColor(0, 255, 0); // Green
            }
            else if (i == 2)
            {                         // Third place
                iSetColor(0, 0, 255); // Blue
            }
            else
            {                             // Rest of the players
                iSetColor(255, 255, 255); // White
            }

          
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
}

/*
function iMouseMove() is called when the user moves the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouseMove(int mx, int my)
{
    // Handle menu button hover effects
    for (int i = 0; i < 5; i++)
    {
        hoverMenu[i] = (mx >= menuX[i] && mx <= menuX[i] + menuW &&
                        my >= menuY[i] && my <= menuY[i] + menuH);
    }
}

/*
function iMouseDrag() is called when the user presses and drags the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouseDrag(int mx, int my)
{
    // place your codes here
}

/*
function iMouse() is called when the user presses/releases the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (gamestate == 0) // Main Menu clicks
        {
            if (mx >= 350 && mx <= (350 + 190) && my >= 220 && my <= (220 + 268)) // Start Game button
            {
                gamestate = 3; // Switch to name input screen
                inputName = true;
                playerName[0] = '\0'; // Clear previous name
                playerNameIndex = 0;
            }
            else if (mx >= 850 && mx <= (850 + 190) && my >= 200 && my <= (200 + 268)) // High Scores button
            {
                gamestate = 2; // Switch to high scores screen
            }
            else if (mx >= 100 && mx <= (100 + 190) && my >= 20 && my <= (20 + 268)) // Help button (placeholder)
            {
                // No action defined for Help yet
            }
            else if (mx >= 610 && mx <= (610 + 190) && my >= 20 && my <= 288) // About button
            {
                gamestate = 4; // Switch to About screen
            }
            else if (mx >= 1120 && mx <= (1120 + 190) && my >= 20 && my <= 288) // Exit button
            {
                exit(0); // Terminate the program
            }
        }
        // --- Back button click handling ---
        // Apply this logic to ALL game states where the back button should be clickable.
        if (gamestate == 1 || gamestate == 2 || gamestate == 3 || gamestate == 4 || gamestate == 6)
        {
            // Use the consistent position and actual image dimensions for click detection
            if (mx >= BACK_BUTTON_X && mx <= BACK_BUTTON_X + back.width &&
                my >= BACK_BUTTON_Y && my <= BACK_BUTTON_Y + back.height)
            {
                gamestate = 0;     // Return to main menu
                iPauseSound(easy); // Pause game music if playing
            }
        }
    }
}

/*
function iMouseWheel() is called when the user scrolls the mouse wheel.
dir = 1 for up, -1 for down.
*/
void iMouseWheel(int dir, int mx, int my)
{
    // place your code here
}

void iKeyboard(unsigned char key)
{
    if (gamestate == 3 && inputName)
    { // Only allow input on name screen
        if (key == '\b')
        { // Backspace
            if (playerNameIndex > 0)
            {
                playerNameIndex--;
                playerName[playerNameIndex] = '\0'; // Remove last character
            }
        }
        else if (key == '\r' || key == '\n')
        { // Enter key to confirm name
            inputName = false;
            gamestate = 1;                           // Start the game
            easy = iPlaySound("Easy.wav", true, 80); // Start game background music

            // Reset game values for a fresh start
            score = 0;
            scoretick = 0;
            health = 3;
            coinsCollected = 0;        // Reset coinsCollected on new game
            obstacleSpeed = 2;         // Reset speed to initial integer value
            coinSpeed = obstacleSpeed; // Reset coin speed as well

            // Reset obstacles
            for (int i = 0; i < 3; i++)
            {
                obsy[i] = 800 + i * 267;                              // Place off-screen at top
                obsx[i] = rand() % (1400 - obsimgfrm[obsi[i]].width); // Random X
                obsi[i] = rand() % 3;
                isObsActive[i] = true;
                iChangeSpriteFrames(&obsimg[i], &obsimgfrm[obsi[i]], 1);
                iSetSpritePosition(&obsimg[i], obsx[i], obsy[i]);
            }

            // Reset and spawn initial coins
            for (size_t i = 0; i < coins.size(); ++i)
            {
                coins[i].active = false; // Deactivate all existing coins
            }
            // Spawn initial set of coins to populate the screen (up to MAX_ACTIVE_COINS)
            for (int i = 0; i < MAX_ACTIVE_COINS; ++i)
            {
                spawnCoin();
            }
        }
        else if (playerNameIndex < (int)sizeof(playerName) - 1)
        { // Add character if space available
            if (isalnum(key) || key == ' ')
            { // Only allow alphanumeric characters and space
                playerName[playerNameIndex++] = key;
                playerName[playerNameIndex] = '\0'; // Null-terminate the string
            }
        }
    }
}

void iSpecialKeyboard(unsigned char key)
{
    // Special keys like arrow keys are handled in `move()` which is called by a timer.
    // This function is present to satisfy iGraphics API but might not need specific code here
    // if movement is continuous based on `isSpecialKeyPressed`.
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    srand(time(0)); // Seed the random number generator using current time

    for (int i = 0; i < 14; ++i)
    {
        sprintf(coinFiles[i], "coin%d.png", i + 1); // Generates "coin1.png", "coin2.png", etc.
    }
    for (int i = 0; i < 14; ++i)
    {
        iLoadImage(&coinFrames[i], coinFiles[i]);
  
        iResizeImage(&coinFrames[i], 30, 30); // These are the coins that move on screen
    }
   
    if (14 > 0)
    {
        coinWidth = coinFrames[0].width;
        coinHeight = coinFrames[0].height;
    }
   
    for (int i = 0; i < 3; i++)
    {
        iLoadImage(&obsimgfrm[i], obs[i]);
    }


    for (int j = 0; j < 10; j++)
    {
        iLoadImage(&habimgfrm[j], hab[j]);
    }

    
    if (10 > 0)
    {
        habWidth = habimgfrm[0].width;
        habHeight = habimgfrm[0].height;
    }

    
    for (int j = 0; j < 10; ++j)
    {
        iInitSprite(&habimg[j]);
        iChangeSpriteFrames(&habimg[j], &habimgfrm[j], 1); // Set initial frame
        iSetSpritePosition(&habimg[j], x, y);              // Set initial position
    }

    loadHighScores(); // Load high scores at startup

  
    for (int i = 0; i < 3; i++)
    {
        iInitSprite(&obsimg[i]);
        obsi[i] = rand() % 3; // Randomly select an obstacle image
        iChangeSpriteFrames(&obsimg[i], &obsimgfrm[obsi[i]], 1);
        obsx[i] = rand() % (1400 - obsimgfrm[obsi[i]].width); // Random X within window width
        obsy[i] = 800 + i * 267;                              // Start above the screen, staggered
        iSetSpritePosition(&obsimg[i], obsx[i], obsy[i]);
    }

    
    for (int i = 0; i < 7; i++)
    {
        iLoadImage(&cloudImg[i], cloud[i]);
    }

   
    for (int i = 0; i < 5; i++)
    {
        iLoadImage(&menuImg[i], menu[i]);
        iLoadImage(&menuBigImg[i], menuBig[i]);
    }
    iLoadImage(&menuImghab, "menuhab.png");    // Menu title image
    iLoadImage(&skyImg, "sky.jpg");            // Background sky image
    iLoadImage(&heartbreak, "heartbreak.png"); // Collision feedback image
    iLoadImage(&back, "backbutton.png");       // Back button image


    iLoadImage(&healthIcon, "Healthicon.png");
    iResizeImage(&healthIcon, 30, 30); // Set the desired size for the health icon
  

   
    iLoadImage(&coinCollectIcon, "Coinicon.png"); // Load the Coinicon.png file (was .jpg)
    iResizeImage(&coinCollectIcon, 30, 30);       // Resize it to 30x30 pixels
   

   
    iSetTimer(500, incmnt);     // Balloon animation
    iSetTimer(16, cldx);        // Cloud horizontal movement
    iSetTimer(16, cldy);        // Cloud vertical movement
    iSetTimer(1, move);         // Player balloon movement (highly frequent for responsiveness)
    iSetTimer(16, obstacle);    // Obstacle movement and regeneration
    iSetTimer(33, scoreupdate); // Score update and speed increase

    // Coin timers
    iSetTimer(50, coinAnimate);  // Coin rotation animation (change frame every 50ms)
    iSetTimer(16, coinMovement); // Coin movement and spawning logic (move every 16ms)

    iInitializeSound(); // Initialize sound system

    
    iInitialize(1400, 800, "Hot Air Balloon");

    return 0;
}