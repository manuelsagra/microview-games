#include <MicroView.h>		// include MicroView library
#include <EEPROM.h>

#define P1_BUTTON A0
#define P1_PADDLE A2
#define MAX_PADDLE 1000
#define MIN_PADDLE 589
#define MAX_STEP 10
#define PADDLE_SIZE 6 // Height = 2 * Size + 1
#define TOTAL_PADDLE_SIZE (2 * PADDLE_SIZE)
#define HEIGHT 48
#define WIDTH 64
#define MAX_X (WIDTH - 1 - TOTAL_PADDLE_SIZE)
#define FRAMES_FLASH 25
#define FRAMES_GAME_OVER 200
#define DELAY 20
#define LIVES 3
#define LEVELS 4
#define EXTRA_LIFE 100
#define BRICK_LINES 4
#define BRICK_WIDTH 7
#define BRICK_LINE_WIDTH 8
#define BRICK_LINE_START 10
#define BRICK_LINE_SEPARATION 4

#define LEVEL_1 {{1, 1, 1, 1, 1, 1, 1, 1},\
				 {1, 1, 1, 1, 1, 1, 1, 1},\
				 {1, 1, 1, 1, 1, 1, 1, 1},\
				 {1, 1, 1, 1, 1, 1, 1, 1}}

#define LEVEL_2 {{1, 0, 1, 0, 1, 0, 1, 0},\
				 {0, 1, 0, 1, 0, 1, 0, 1},\
				 {1, 0, 1, 0, 1, 0, 1, 0},\
				 {1, 1, 1, 1, 1, 1, 1, 1}}

#define LEVEL_3 {{1, 0, 0, 1, 0, 1, 0, 1},\
				 {1, 1, 1, 1, 0, 0, 0, 1},\
				 {1, 0, 0, 1, 0, 1, 0, 0},\
				 {1, 0, 0, 1, 0, 1, 0, 1}}

#define LEVEL_4 {{1, 1, 1, 1, 1, 1, 1, 1},\
				 {1, 0, 0, 1, 1, 0, 0, 1},\
				 {1, 0, 1, 0, 0, 1, 0, 1},\
				 {1, 1, 1, 1, 1, 1, 1, 1}}

const int LEVEL_BRICKS[LEVELS][BRICK_LINES][BRICK_LINE_WIDTH] = 
	{LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4};

enum Status {
	TITLE, ANIMATE_TITLE, PLAYING, WAITING, GAME_OVER, WAIT_TITLE
};

struct Ball {
	float x;
	float y;
	float dx;
	float dy;
};

int frames;
bool flash;
int oldPaddleRead = 0;
int paddleX;
unsigned int score;
unsigned int high;
int lives;
int level;
int bricks[BRICK_LINES][BRICK_LINE_WIDTH];
float dx[PADDLE_SIZE];
float dy[PADDLE_SIZE];
Status status;
Ball ball;

void setup() {
	pinMode(P1_BUTTON, INPUT_PULLUP);

	// Bounce vectors
	for (int i = 0; i < PADDLE_SIZE; i++) {
		float angle = (M_PI_2 * (float) i / (float) PADDLE_SIZE) + (M_PI_2 / (float) (PADDLE_SIZE << 1));
		dx[i] = cos(angle);
		dy[i] = sin(angle);
	}

	uView.begin();
	reset();
}

void reset() {
	score = 0;
	status = TITLE;
	high = eepromIntRead();
	lives = LIVES;
	level = 0;
	frames = 0;
	initBricks();
}

void resetBall() {
	ball.x = WIDTH / 2;
	ball.y = HEIGHT - 2;
	ball.dx = 0.7071;
	ball.dy = -0.7071;
}

void loop() {
	switch (status) {
		case TITLE:
			title();
			break;

		case ANIMATE_TITLE:
			animateTitle();
			break;

		case PLAYING:
		case WAITING:
			playing();
			break;

		case GAME_OVER:
			gameOver();
			break;

		case WAIT_TITLE:
			waitTitle();
			break;
	}
}

void title() {
	uView.clear(PAGE);

	// Title
	uView.setCursor(18, 0);
	uView.print("Micro");
	uView.setCursor(8, 8);
	uView.print("BreakOut");

	// High score
	uView.setCursor(0, 40);
	uView.print("TOP ");
	uView.setCursor(24, 40);
	uView.print(high);
	uView.display();

	status = ANIMATE_TITLE;
}

void animateTitle() {
	// Animate "PRESS ANY BUTTON" text
	if (frames == FRAMES_FLASH) {
		String press = "PRESS";
		String button = "BUTTON";
		if (flash) {
			press = "         ";
			button = "      ";
		}
		uView.setCursor(18, 20);
		uView.print(press);
		uView.setCursor(14, 29);
		uView.print(button);
		uView.display();

		flash = !flash;
		frames = -1;
	}
	frames++;

	// Check button
	if (digitalRead(P1_BUTTON) == LOW) {
		frames = 0;
		status = WAITING;
	}

	delay(DELAY);
}

void gameOver() {
	// Game Over with black rectangle background
	uView.setColor(WHITE);
	uView.rectFill(15, 15, 33, 25);

	uView.setColor(BLACK);	
	uView.setCursor(20, 20);
	uView.print("GAME");

	uView.setCursor(20, 28);
	uView.print("OVER");
	uView.setColor(WHITE);

	uView.display();

	// New high score?
	if (score > high) {
		high = score;
		eepromIntWrite(high);
	}

	frames = 0;

	status = WAIT_TITLE;
}

void waitTitle() {
	frames++;
	if (frames == FRAMES_GAME_OVER || digitalRead(P1_BUTTON) == LOW) {
		reset();
	}
	delay(DELAY);
}

void playing() {
	uView.clear(PAGE);

	drawPaddle();
	drawBricks();
	if (status == WAITING) {
		if (digitalRead(P1_BUTTON) == LOW) {
			status = PLAYING;
			resetBall();
			ball.x = paddleX + PADDLE_SIZE;
		}
	} else {
		updateBall();
	}	

	// Score & Lives
	uView.setCursor(0, 0);
	uView.print(score);
	uView.setCursor(58, 0);
	uView.print(lives);

	uView.display();

	delay(DELAY);
}

void drawPaddle() {
	int x = (float) (paddleRead() / 100.0) * (float) MAX_X;
	paddleX = x;
	uView.line(x, HEIGHT -1, x + TOTAL_PADDLE_SIZE, HEIGHT - 1);
}

void drawBricks() {
	int l = BRICK_LINE_START;
	for (int y = 0; y < BRICK_LINES; y++) {
		for (int x = 0; x < BRICK_LINE_WIDTH; x++) {
			if (bricks[y][x]) {
				int px = x << 3;
				uView.line(px, l, px + BRICK_WIDTH, l);
			}
		}
		l += BRICK_LINE_SEPARATION;
	}
}

void updateBall() {
	ball.x += ball.dx;
	ball.y += ball.dy;

	// Sides
	if (ball.x <= 0) {
		ball.x = 0;
		ball.dx = -ball.dx;
	} else if (ball.x >= WIDTH - 1) {
		ball.x = WIDTH - 1;
		ball.dx = -ball.dx;
	}

	// Bottom & Top
	if (ball.y >= HEIGHT - 1) {
		if (ball.x >= paddleX && ball.x < (paddleX + TOTAL_PADDLE_SIZE)) {
			int hit = ball.x - paddleX;
			if (hit < PADDLE_SIZE) {
				ball.dx = -dx[hit];
				ball.dy = -dy[hit];				
			} else {
				ball.dx = dx[TOTAL_PADDLE_SIZE - 1 - hit];
				ball.dy = -dy[TOTAL_PADDLE_SIZE - 1 - hit];
			}
		} else {
			lives--;
			status = (lives > 0) ? WAITING : GAME_OVER;
		}
	} else if (ball.y <= 0) {
		ball.y = 0;
		ball.dy = -ball.dy;
	}

	// Brick collision
	if (ball.y <= (BRICK_LINE_START + BRICK_LINE_SEPARATION * BRICK_LINES) && ball.y >= BRICK_LINE_START) {
		int brickY = (ball.y - BRICK_LINE_START) / BRICK_LINE_SEPARATION;
		int brickX = (int) ball.x >> 3;
		if (bricks[brickY][brickX]) {
			bricks[brickY][brickX] = 0;
			score++;
			if (score % EXTRA_LIFE == 0) {
				lives++;
			}
			if (isLevelCompleted()) {
				level++;
				level %= LEVELS;
				initBricks();
				resetBall();
				status = WAITING;
			}
			ball.dy = -ball.dy;
		}
	}

	uView.pixel(ball.x, ball.y);
}

int paddleRead() {	// 0..100
	int read = (float) (analogRead(P1_PADDLE) - MIN_PADDLE) * 100 / (float) (MAX_PADDLE - MIN_PADDLE);
	
	// Limit movement to prevent jerkiness in defective potentiometers
	if (read > oldPaddleRead + MAX_STEP) {
		read = oldPaddleRead + MAX_STEP;
	} else if (read < oldPaddleRead - MAX_STEP) {
		read = oldPaddleRead - MAX_STEP;
	}

	if (read < 0) {
		read = 0;
	} else if (read > MAX_PADDLE) {
		read = MAX_PADDLE;
	}
	oldPaddleRead = read;

	return read;
}

bool isLevelCompleted() {
	for (int y = 0; y < BRICK_LINES; y++) {
		for (int x = 0; x < BRICK_LINE_WIDTH; x++) {
			if (bricks[y][x]) {
				return false;
			}
		}
	}
	return true;
}

void initBricks() {
	for (int y = 0; y < BRICK_LINES; y++) {
		for (int x = 0; x < BRICK_LINE_WIDTH; x++) {
			bricks[y][x] = LEVEL_BRICKS[level][y][x];
		}
	}
}

void eepromIntWrite(int value) {
	EEPROM.write(2, value & 0xFF);
	EEPROM.write(3, value >> 8);
}

int eepromIntRead() {
	return EEPROM.read(2) | (EEPROM.read(3) << 8);
}