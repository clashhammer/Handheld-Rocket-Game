/*
   ===============
   ROCKET SCHOLAR 
   ===============
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// =========================================================
// TFT
// =========================================================
#define TFT_CS   10
#define TFT_DC    8
#define TFT_RST   9

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// =========================================================
// Pins
// =========================================================
#define BTN_UP    2
#define BTN_DOWN  3
#define BTN_FIRE  4
#define BUZZER    7

// =========================================================
// Screen
// =========================================================
#define SCREEN_W 160
#define SCREEN_H 128

// =========================================================
// Game constants
// =========================================================
#define ROCKET_X        8
#define ROCKET_W       18
#define ROCKET_H       12

#define BULLET_W        6
#define BULLET_H        2

#define MAX_METEORS     3
#define MAX_BULLETS     3

#define FIRE_COOLDOWN  180
#define HUD_H          14

// =========================================================
// Colors
// =========================================================
#define COL_BG          ST77XX_BLACK
#define COL_TEXT        ST77XX_WHITE
#define COL_TITLE       ST77XX_YELLOW
#define COL_ROCKET      ST77XX_CYAN
#define COL_WING        ST77XX_BLUE
#define COL_FIRE1       ST77XX_YELLOW
#define COL_FIRE2       ST77XX_RED
#define COL_BULLET      ST77XX_GREEN
#define COL_HEART       ST77XX_RED
#define COL_METEOR      0xFD20
#define COL_CRATER      0xC800
#define COL_HUDLINE     ST77XX_WHITE
#define COL_SUBJECT     ST77XX_YELLOW

// =========================================================
// Subjects
// =========================================================
const char* subjects[] = {
  "MATH", "BIOLOGY", "GEOGRAPHY", "CHEMISTRY",
  "HISTORY", "PHYSICS", "ART", "LITERATURE"
};
#define SUBJECT_COUNT 8

// =========================================================
// Structures
// =========================================================
struct Meteor {
  int x;
  int y;
  int speed;
  bool active;
  uint8_t size;
  uint8_t subjectIdx;
};

struct Bullet {
  int x;
  int y;
  bool active;
};

// =========================================================
// Globals
// =========================================================
Meteor meteors[MAX_METEORS];
Bullet bullets[MAX_BULLETS];

int rocketY;
int oldRocketY;

int score;
int lives;
int oldScore;
int oldLives;
int oldFrontMeteorIdx = -2;

bool gameStarted = false;
bool gameOver = false;

bool startScreenDrawn = false;
bool gameOverScreenDrawn = false;
bool startBlinkState = false;
bool gameOverBlinkState = false;

uint32_t lastMeteorSpawn = 0;
uint32_t lastFireTime = 0;
uint32_t lastFrame = 0;

uint16_t frameDelay = 68;

// old positions for partial redraw
int oldBulletX[MAX_BULLETS];
int oldBulletY[MAX_BULLETS];
bool oldBulletActive[MAX_BULLETS];

int oldMeteorX[MAX_METEORS];
int oldMeteorY[MAX_METEORS];
uint8_t oldMeteorSize[MAX_METEORS];
bool oldMeteorActive[MAX_METEORS];

// =========================================================
// Sound
// =========================================================
void beep(int freq, int dur) {
  tone(BUZZER, freq, dur);
}

void playShotSound()    { beep(1200, 20); }
void playExplodeSound() { beep(280, 60); }
void playHitSound()     { beep(180, 120); }

void playStartSound() {
  beep(523, 50); delay(60);
  beep(659, 50); delay(60);
  beep(784, 80);
}

// =========================================================
// Utility
// =========================================================
int getFrontMeteorIndex() {
  int idx = -1;
  int bestX = 10000;

  for (int i = 0; i < MAX_METEORS; i++) {
    if (!meteors[i].active) continue;
    if (meteors[i].x < bestX) {
      bestX = meteors[i].x;
      idx = i;
    }
  }
  return idx;
}

void saveOldState() {
  oldRocketY = rocketY;

  for (int i = 0; i < MAX_BULLETS; i++) {
    oldBulletX[i] = bullets[i].x;
    oldBulletY[i] = bullets[i].y;
    oldBulletActive[i] = bullets[i].active;
  }

  for (int i = 0; i < MAX_METEORS; i++) {
    oldMeteorX[i] = meteors[i].x;
    oldMeteorY[i] = meteors[i].y;
    oldMeteorSize[i] = meteors[i].size;
    oldMeteorActive[i] = meteors[i].active;
  }
}

void clearBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    bullets[i].active = false;
    oldBulletActive[i] = false;
    oldBulletX[i] = 0;
    oldBulletY[i] = 0;
  }
}

void clearMeteors() {
  for (int i = 0; i < MAX_METEORS; i++) {
    meteors[i].active = false;
    oldMeteorActive[i] = false;
    oldMeteorX[i] = 0;
    oldMeteorY[i] = 0;
    oldMeteorSize[i] = 0;
  }
}

// =========================================================
// Drawing helpers
// =========================================================

// Horizontal rocket pointing RIGHT
void drawRocket(int x, int y) {
  tft.fillRoundRect(x + 3, y + 3, 10, 6, 2, COL_ROCKET);
  tft.fillTriangle(x + 13, y + 3, x + 13, y + 9, x + 18, y + 6, COL_ROCKET);
  tft.fillTriangle(x + 5, y + 3, x + 2, y,      x + 6, y + 3, COL_WING);
  tft.fillTriangle(x + 5, y + 9, x + 2, y + 12, x + 6, y + 9, COL_WING);

  if ((millis() / 100) % 2 == 0) {
    tft.fillTriangle(x + 3, y + 4, x + 3, y + 8, x,     y + 6, COL_FIRE1);
  } else {
    tft.fillTriangle(x + 3, y + 4, x + 3, y + 8, x + 1, y + 6, COL_FIRE2);
  }
}

void eraseRocket(int x, int y) {
  tft.fillRect(x, y, 20, 14, COL_BG);
}

void drawMeteor(const Meteor& m) {
  tft.fillCircle(m.x, m.y, m.size, COL_METEOR);
  tft.drawPixel(m.x - 2, m.y - 1, COL_CRATER);
  tft.drawPixel(m.x + 2, m.y,     COL_CRATER);
  tft.drawPixel(m.x,     m.y + 2, COL_CRATER);
}

// tighter erase = less flicker
void eraseMeteorBox(int x, int y, int size) {
  int left = x - size - 1;
  int top  = y - size - 1;
  int w    = size * 2 + 2;
  int h    = size * 2 + 2;

  if (left < 0) left = 0;
  if (top < HUD_H) top = HUD_H;
  if (left + w > SCREEN_W) w = SCREEN_W - left;
  if (top + h > SCREEN_H) h = SCREEN_H - top;

  if (w > 0 && h > 0) {
    tft.fillRect(left, top, w, h, COL_BG);
  }
}

void drawBullet(const Bullet& b) {
  tft.fillRect(b.x, b.y, BULLET_W, BULLET_H, COL_BULLET);
}

void eraseBullet(int x, int y) {
  tft.fillRect(x, y, BULLET_W + 1, BULLET_H + 1, COL_BG);
}

void drawHeart(int x, int y) {
  tft.drawPixel(x + 1, y, COL_HEART);
  tft.drawPixel(x + 3, y, COL_HEART);
  tft.drawPixel(x, y + 1, COL_HEART);
  tft.drawPixel(x + 4, y + 1, COL_HEART);
  tft.fillRect(x, y + 2, 5, 2, COL_HEART);
  tft.drawPixel(x + 1, y + 4, COL_HEART);
  tft.drawPixel(x + 3, y + 4, COL_HEART);
  tft.drawPixel(x + 2, y + 5, COL_HEART);
}

void drawHUD() {
  tft.fillRect(0, 0, SCREEN_W, HUD_H, COL_BG);
  tft.drawFastHLine(0, HUD_H - 1, SCREEN_W, COL_HUDLINE);

  for (int i = 0; i < lives; i++) {
    drawHeart(4 + i * 10, 4);
  }

  int frontIdx = getFrontMeteorIndex();

  tft.setTextSize(1);

  tft.setTextColor(COL_SUBJECT, COL_BG);
  tft.setCursor(38, 3);
  if (frontIdx >= 0) {
    tft.print(subjects[meteors[frontIdx].subjectIdx]);
  } else {
    tft.print("READY        ");
  }

  tft.setTextColor(COL_TEXT, COL_BG);
  tft.setCursor(SCREEN_W - 46, 3);
  tft.print("SC:");
  tft.print(score);
  tft.print("   ");
}

void updateHUDIfNeeded() {
  int frontIdx = getFrontMeteorIndex();

  bool hudChanged = false;

  if (score != oldScore || lives != oldLives) {
    hudChanged = true;
  } else {
    if (frontIdx != oldFrontMeteorIdx) {
      hudChanged = true;
    } else if (frontIdx >= 0 && oldFrontMeteorIdx >= 0) {
      if (meteors[frontIdx].subjectIdx != meteors[oldFrontMeteorIdx].subjectIdx) {
        hudChanged = true;
      }
    }
  }

  if (hudChanged) {
    drawHUD();
    oldScore = score;
    oldLives = lives;
    oldFrontMeteorIdx = frontIdx;
  }
}

// =========================================================
// Menu screens
// =========================================================
void drawStartScreenFull() {
  tft.fillScreen(COL_BG);

  tft.setTextSize(2);
  tft.setTextColor(COL_TITLE, COL_BG);
  tft.setCursor(22, 18);
  tft.print("ROCKET");

  tft.setCursor(18, 40);
  tft.print("SCHOLAR");

  tft.drawFastHLine(18, 62, 124, COL_TEXT);

  drawRocket(68, 74);

  tft.setTextSize(1);
  tft.setTextColor(COL_TEXT, COL_BG);
  tft.setCursor(20, 98);
  tft.print("Destroy the meteors!");

  startScreenDrawn = true;
}

void updateStartScreenBlink() {
  bool blinkNow = ((millis() / 500) % 2 == 0);
  if (blinkNow == startBlinkState) return;

  startBlinkState = blinkNow;

  tft.fillRect(35, 112, 90, 12, COL_BG);

  if (blinkNow) {
    tft.setTextSize(1);
    tft.setTextColor(COL_TEXT, COL_BG);
    tft.setCursor(40, 116);
    tft.print("Press FIRE");
  }
}

void drawGameOverFull() {
  tft.fillScreen(COL_BG);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_RED, COL_BG);
  tft.setCursor(18, 20);
  tft.print("GAME OVER");

  tft.setTextSize(1);
  tft.setTextColor(COL_TEXT, COL_BG);
  tft.setCursor(48, 52);
  tft.print("Score: ");
  tft.print(score);

  tft.setCursor(30, 72);
  if (score >= 30)      tft.print("Excellent (6)");
  else if (score >= 20) tft.print("Very good (5)");
  else if (score >= 10) tft.print("Good (4)");
  else if (score >= 5)  tft.print("Average (3)");
  else                  tft.print("Weak (2)");

  gameOverScreenDrawn = true;
}

void updateGameOverBlink() {
  bool blinkNow = ((millis() / 600) % 2 == 0);
  if (blinkNow == gameOverBlinkState) return;

  gameOverBlinkState = blinkNow;

  tft.fillRect(20, 100, 120, 12, COL_BG);

  if (blinkNow) {
    tft.setTextSize(1);
    tft.setTextColor(COL_TEXT, COL_BG);
    tft.setCursor(24, 104);
    tft.print("Press FIRE again");
  }
}

// =========================================================
// Game state
// =========================================================
void resetGame() {
  rocketY = SCREEN_H / 2 - ROCKET_H / 2;
  oldRocketY = rocketY;

  score = 0;
  lives = 3;
  oldScore = -1;
  oldLives = -1;
  oldFrontMeteorIdx = -2;
  gameOver = false;

  frameDelay = 72;
  lastMeteorSpawn = 0;
  lastFireTime = 0;

  startScreenDrawn = false;
  gameOverScreenDrawn = false;
  startBlinkState = false;
  gameOverBlinkState = false;

  clearBullets();
  clearMeteors();

  tft.fillScreen(COL_BG);
  drawHUD();
  drawRocket(ROCKET_X, rocketY);

  saveOldState();
  oldScore = score;
  oldLives = lives;
  oldFrontMeteorIdx = getFrontMeteorIndex();

  playStartSound();
}

void spawnMeteor() {
  uint32_t now = millis();
  uint32_t interval = max(750UL, 1800UL - (uint32_t)(score * 10));

  if (now - lastMeteorSpawn < interval) return;

  for (int i = 0; i < MAX_METEORS; i++) {
    if (!meteors[i].active) {
      meteors[i].x = SCREEN_W + 10;
      meteors[i].y = random(HUD_H + 12, SCREEN_H - 10);
      meteors[i].speed = random(2, 4) + (score / 18);
      if (meteors[i].speed > 4) meteors[i].speed = 4;
      meteors[i].size = random(8, 11);
      meteors[i].subjectIdx = random(0, SUBJECT_COUNT);
      meteors[i].active = true;
      lastMeteorSpawn = now;
      return;
    }
  }
}

void fireBullet() {
  uint32_t now = millis();
  if (now - lastFireTime < FIRE_COOLDOWN) return;

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x = ROCKET_X + ROCKET_W + 1;
      bullets[i].y = rocketY + (ROCKET_H / 2);
      bullets[i].active = true;
      lastFireTime = now;
      playShotSound();
      return;
    }
  }
}

bool bulletHitsMeteor(const Bullet& b, const Meteor& m) {
  return (b.x + BULLET_W >= m.x - m.size) &&
         (b.x <= m.x + m.size) &&
         (b.y + BULLET_H >= m.y - m.size) &&
         (b.y <= m.y + m.size);
}

bool meteorHitsRocket(const Meteor& m) {
  return (m.x - m.size <= ROCKET_X + ROCKET_W) &&
         (m.y + m.size >= rocketY) &&
         (m.y - m.size <= rocketY + ROCKET_H);
}

// =========================================================
// Partial redraw game frame
// =========================================================
void drawGameFrame() {
  eraseRocket(ROCKET_X, oldRocketY);

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (oldBulletActive[i]) {
      eraseBullet(oldBulletX[i], oldBulletY[i]);
    }
  }

  for (int i = 0; i < MAX_METEORS; i++) {
    if (oldMeteorActive[i]) {
      eraseMeteorBox(oldMeteorX[i], oldMeteorY[i], oldMeteorSize[i]);
    }
  }

  drawRocket(ROCKET_X, rocketY);

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      drawBullet(bullets[i]);
    }
  }

  for (int i = 0; i < MAX_METEORS; i++) {
    if (meteors[i].active) {
      drawMeteor(meteors[i]);
    }
  }

  updateHUDIfNeeded();
  saveOldState();
}

// =========================================================
// Update
// =========================================================
void updateGame() {
  if (!digitalRead(BTN_UP)) {
    rocketY -= 4;
    if (rocketY < HUD_H + 1) rocketY = HUD_H + 1;
  }

  if (!digitalRead(BTN_DOWN)) {
    rocketY += 4;
    if (rocketY > SCREEN_H - ROCKET_H - 1) rocketY = SCREEN_H - ROCKET_H - 1;
  }

  if (!digitalRead(BTN_FIRE)) {
    fireBullet();
  }

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;

    bullets[i].x += 8;
    if (bullets[i].x > SCREEN_W) {
      bullets[i].active = false;
    }
  }

  spawnMeteor();

  for (int i = 0; i < MAX_METEORS; i++) {
    if (!meteors[i].active) continue;

    meteors[i].x -= meteors[i].speed;

    if (meteors[i].x + meteors[i].size < 0) {
      meteors[i].active = false;
      lives--;
      playHitSound();
      if (lives <= 0) gameOver = true;
      continue;
    }

    if (meteorHitsRocket(meteors[i])) {
      meteors[i].active = false;
      lives--;
      playHitSound();
      if (lives <= 0) gameOver = true;
      continue;
    }

    for (int j = 0; j < MAX_BULLETS; j++) {
      if (!bullets[j].active) continue;

      if (bulletHitsMeteor(bullets[j], meteors[i])) {
        bullets[j].active = false;
        meteors[i].active = false;
        score++;
        frameDelay = max(58, 72 - score / 10);
        playExplodeSound();
        break;
      }
    }
  }
}

// =========================================================
// Setup / Loop
// =========================================================
void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_FIRE, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  randomSeed(analogRead(A0));

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.invertDisplay(false);
  tft.fillScreen(COL_BG);

  drawStartScreenFull();
}

void loop() {
  uint32_t now = millis();
  if (now - lastFrame < frameDelay) return;
  lastFrame = now;

  if (!gameStarted) {
    if (!startScreenDrawn) {
      drawStartScreenFull();
    }
    updateStartScreenBlink();

    if (!digitalRead(BTN_FIRE)) {
      delay(180);
      gameStarted = true;
      gameOver = false;
      startScreenDrawn = false;
      gameOverScreenDrawn = false;
      resetGame();
    }
    return;
  }

  if (gameOver) {
    if (!gameOverScreenDrawn) {
      drawGameOverFull();
    }
    updateGameOverBlink();

    if (!digitalRead(BTN_FIRE)) {
      delay(180);
      gameStarted = true;
      gameOver = false;
      startScreenDrawn = false;
      gameOverScreenDrawn = false;
      resetGame();
    }
    return;
  }

  updateGame();
  drawGameFrame();
}