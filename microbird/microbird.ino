#include <MicroView.h>
#include <EEPROM.h>

#define BUTTON_PIN A0
#define HEIGHT 48
#define WIDTH 64
#define LINE 9
#define DELAY 20
#define DIFFICULTY 128 // 1..65535 (shorter = harder)
#define FRAMES_PER_MOVE 2
#define FRAMES_FLASH_TITLE 25
#define FRAMES_GAME_OVER 200
#define OBSTACLES 8
#define YMIN 5
#define APERTURE 20
#define SEPARATION 12
#define FLASHES 8

struct Obstacle {
	int x;
	int y;
};
enum Status {
	TITLE, ANIMATE_TITLE, PLAYING, GAME_OVER, WAIT_TITLE
};

int frames;
bool flash;
unsigned int score;
unsigned int high;
int y;
Obstacle obstacles[OBSTACLES];
int last;
Status status;

void setup() {
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	uView.begin();
	reset();
}

void reset() {
	y = HEIGHT / 2;
	flash = false;
	frames = 0;
	score = 0;
	high = eepromIntRead();
	status = TITLE;
	randomSeed(analogRead(0));
	for (int i = 0; i < OBSTACLES; i++) {
		obstacles[i].x = WIDTH + i * SEPARATION;
		obstacles[i].y = YMIN + random(HEIGHT - LINE + 1 - APERTURE - YMIN);
	}
	last = OBSTACLES - 1;
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
	
	// Top title
	uView.setCursor(6, 0);
	uView.print("MicroBird");

	// High score
	uView.setCursor(0, 40);
	uView.print("TOP ");
	uView.setCursor(24, 40);
	uView.print(high);

	uView.display();

	status = ANIMATE_TITLE;
}

void animateTitle() {
	// Animate "PRESS BUTTON" text
	if (frames == FRAMES_FLASH_TITLE) {
		String press = "PRESS";
		String button = "BUTTON";
		if (flash) {
			press = "     ";
			button = "      ";
		}
		uView.setCursor(17, 15);
		uView.print(press);
		uView.setCursor(14, 24);
		uView.print(button);
		uView.display();

		flash = !flash;
		frames = -1;
	}
	frames++;

	// Check button
	if (digitalRead(BUTTON_PIN) == LOW) {
		frames = 0;
		status = PLAYING;
	}

	delay(DELAY);
}

void gameOver() {
	// Flash
	for (int i = FLASHES; i > 0; i--) {
		uView.invert(true);
		delay(DELAY << 1);
		uView.invert(false);
		delay(DELAY << 1);
	}

	// Game Over with black rectangle background
	uView.setColor(BLACK);
	uView.rectFill(15, 15, 33, 25);

	uView.setColor(WHITE);	
	uView.setCursor(20, 20);
	uView.print("GAME");

	uView.setCursor(20, 28);
	uView.print("OVER");

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
	if (frames == FRAMES_GAME_OVER || digitalRead(BUTTON_PIN) == LOW) {
		reset();
	}
	delay(DELAY);
}

void playing() {
	// Update player
	if (frames == FRAMES_PER_MOVE) {
		frames = -1;
		if (digitalRead(BUTTON_PIN) == HIGH) {
			y ++;
		} else {
			y--;
		}
	}
	if (y <= LINE) {
		y = LINE + 1;
	} else if (y >= HEIGHT) {
		y = HEIGHT - 1;
	}
	frames++;

	// Show
	uView.clear(PAGE);
	uView.line(0, LINE, WIDTH, LINE);

	// Update obstacles
	for (int i = 0; i < OBSTACLES; i++) {
		obstacles[i].x--;
		if (obstacles[i].x <= -SEPARATION) {
			obstacles[i].x = obstacles[last].x + SEPARATION - (last > i ? 1 : 0);
			obstacles[i].y = YMIN + random(HEIGHT - LINE + 1 - APERTURE - YMIN);
			last = i;
			score++;
		}
		// Draw visible obstacles
		if (obstacles[i].x < WIDTH) {
			uView.rectFill(obstacles[i].x, LINE, SEPARATION, obstacles[i].y);
			uView.rectFill(obstacles[i].x, LINE + APERTURE + obstacles[i].y, SEPARATION, HEIGHT - (LINE + APERTURE + obstacles[i].y));
		}
		// Check collision
		if (obstacles[i].x <= 0 && (y < (LINE + obstacles[i].y) || y > (LINE + APERTURE + obstacles[i].y))) {
			status = GAME_OVER;
		}
	}

	uView.setCursor(0, 0);
	uView.print(score);
	uView.pixel(0, y);
	uView.display();

	// Make it harder...
	int df = (int) (DELAY * (1 - (float) score / DIFFICULTY));
	if (df < 0) {
		df = 0;
	}
	delay(df);
}

void eepromIntWrite(int value) {
	EEPROM.write(0, value & 0xFF);
	EEPROM.write(1, value >> 8);
}

int eepromIntRead() {
	return EEPROM.read(0) | (EEPROM.read(1) << 8);
}