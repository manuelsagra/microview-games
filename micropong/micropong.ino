#include <MicroView.h>		// include MicroView library

#define P1_BUTTON A0
#define P2_BUTTON A1
#define P1_PADDLE A2
#define P2_PADDLE A3
#define MAX_PADDLE 1000
#define MIN_PADDLE 589
#define MAX_STEP 10
#define PADDLE_SIZE 6 // Height = 2 * Size + 1
#define TOTAL_PADDLE_SIZE ((2 * PADDLE_SIZE) + 1)
#define HEIGHT 48
#define WIDTH 64
#define MAX_Y (HEIGHT - 1 - TOTAL_PADDLE_SIZE)
#define FRAMES_FLASH 25
#define DELAY 20

enum Status {
	TITLE, ANIMATE_TITLE, PLAYING, WAITING, GAME_OVER
};

struct Ball {
	float x;
	float y;
	float dx;
	float dy;
};

int frames;
bool flash;
int oldPaddleRead[] = {0, MAX_Y};
int paddleY[2];
int paddlePin[] = {P1_PADDLE, P2_PADDLE};
int paddlePosition[] = {0, WIDTH - 1};
int score[2];
float dx[PADDLE_SIZE];
float dy[PADDLE_SIZE];
Status status;
Ball ball;

void setup() {
	pinMode(P1_BUTTON, INPUT_PULLUP);
	pinMode(P2_BUTTON, INPUT_PULLUP);

	// Bounce vectors
	for (int i = 0; i < PADDLE_SIZE; i++) {
		float angle = (M_PI_2 * (float) i / (float) PADDLE_SIZE) + (M_PI_2 / (float) (PADDLE_SIZE << 1));
		dx[i] = sin(angle);
		dy[i] = cos(angle);
	}

	status = TITLE;

	uView.begin();	
}

void reset() {
	score[0] = 0;
	score[1] = 0;
	frames = 0;
	// Ball from middle left side, 45º
	ball.x = 0;
	ball.y = HEIGHT / 2;
	ball.dx = 0.7071;
	ball.dy = 0.7071;
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

		case WAITING:
			waiting();
			break;

		case GAME_OVER:
			gameOver();
			break;
	}
}

void title() {
	reset();

	uView.clear(PAGE);	
	uView.setCursor(6, 0);
	uView.print("MicroPong");
	uView.display();

	status = ANIMATE_TITLE;
}

void animateTitle() {
	// Animate "PRESS ANY BUTTON" text
	if (frames == FRAMES_FLASH) {
		String press = "PRESS ANY";
		String button = "BUTTON";
		if (flash) {
			press = "         ";
			button = "      ";
		}
		uView.setCursor(5, 20);
		uView.print(press);
		uView.setCursor(14, 29);
		uView.print(button);
		uView.display();

		flash = !flash;
		frames = -1;
	}
	frames++;

	// Check button
	if (digitalRead(P1_BUTTON) == LOW || digitalRead(P2_BUTTON) == LOW) {
		frames = 0;
		status = PLAYING;
	}

	delay(DELAY);
}

void waiting() {
	// Animate score
	if (frames == FRAMES_FLASH) {
		uView.setCursor(14, 0);
		if (flash && ball.x >= WIDTH - 1) {
			uView.print(" ");
		} else {
			uView.print(score[0]);
		}
		uView.setCursor(46, 0);
		if (flash && ball.x <= 0) {
			uView.print(" ");
		} else {
			uView.print(score[1]);
		}
		uView.display();

		flash = !flash;
		frames = -1;
	}
	frames++;

	// Check buttons to start again
	if ((ball.x <= 0 && digitalRead(P1_BUTTON) == LOW) || (ball.x >= WIDTH - 1 && digitalRead(P2_BUTTON) == LOW)) {
		if (score[0] == 9 || score[1] == 9) {
			status = TITLE;
		} else {
			status = PLAYING;
			ball.x = (ball.x <= 0) ? 1 : WIDTH - 2;
			ball.dx = -ball.dx;	
		}
	}	
	delay(DELAY);
}

void gameOver() {
	uView.setColor(WHITE);
	uView.rectFill(3, 10, 58, 30);

	uView.setColor(BLACK);
	uView.setCursor(9, 15);
	uView.print(score[0] == 9 ? "PLAYER 1" : "PLAYER 2");
	uView.setCursor(21, 28);
	uView.print("WINS");
	uView.setColor(WHITE);

	uView.display();

	status = WAITING;
}

void playing() {
	uView.clear(PAGE);

	drawPaddle(0);
	drawPaddle(1);

	updateBall();

	// Net & Score
	uView.line(WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1);
	uView.setCursor(14, 0);
	uView.print(score[0]);
	uView.setCursor(46, 0);
	uView.print(score[1]);

	uView.display();

	delay(DELAY);
}

void drawPaddle(int paddle) {
	int y = (float) (paddleRead(paddle) / 100.0) * (float) MAX_Y;
	paddleY[paddle] = y;
	uView.line(paddlePosition[paddle], y, paddlePosition[paddle], y + TOTAL_PADDLE_SIZE);
}

void updateBall() {
	ball.x += ball.dx;
	ball.y += ball.dy;

	// Left side
	if (ball.x <= 0) {
		if (ball.y >= paddleY[0] && ball.y < (paddleY[0] + TOTAL_PADDLE_SIZE)) {
			int hit = ball.y - paddleY[0];
			if (hit == PADDLE_SIZE) {
				ball.dx = 1;
				ball.dy = 0;
			} else if (hit < PADDLE_SIZE) {
				ball.dx = dx[hit];
				ball.dy = -dy[hit];
			} else {
				ball.dx = dx[TOTAL_PADDLE_SIZE - 1 - hit];
				ball.dy = dy[TOTAL_PADDLE_SIZE - 1 - hit];
			}
		} else {
			status = WAITING;
			score[1]++;
			checkScore();
		}
	// Right side
	} else if (ball.x >= WIDTH - 1) {
		if (ball.y >= paddleY[1] && ball.y < (paddleY[1] + TOTAL_PADDLE_SIZE)) {
			int hit = ball.y - paddleY[1];
			if (hit == PADDLE_SIZE) {
				ball.dx = -1;
				ball.dy = 0;
			} else if (hit < PADDLE_SIZE) {
				ball.dx = -dx[hit];
				ball.dy = -dy[hit];
			} else {
				ball.dx = -dx[TOTAL_PADDLE_SIZE - 1 - hit];
				ball.dy = dy[TOTAL_PADDLE_SIZE - 1 - hit];
			}
		} else {
			status = WAITING;
			score[0]++;
			checkScore();
		}
	}

	// Top & bottom
	if (ball.y <= 0) {
		ball.y = 0;
		ball.dy = -ball.dy;
	} else if (ball.y >= HEIGHT - 1) {
		ball.y = HEIGHT - 1;
		ball.dy = -ball.dy;
	}

	uView.pixel(ball.x, ball.y);
}

void checkScore() {
	if (score[0] == 9 || score[1] == 9) {
		status = GAME_OVER;
	}
}

int paddleRead(int paddle) {	// 0..100
	int read = (float) (analogRead(paddlePin[paddle]) - MIN_PADDLE) * 100 / (float) (MAX_PADDLE - MIN_PADDLE);
	
	// Limit movement to prevent jerkiness in defective potentiometers
	if (read > oldPaddleRead[paddle] + MAX_STEP) {
		read = oldPaddleRead[paddle] + MAX_STEP;
	} else if (read < oldPaddleRead[paddle] - MAX_STEP) {
		read = oldPaddleRead[paddle] - MAX_STEP;
	}

	if (read < 0) {
		read = 0;
	} else if (read > MAX_PADDLE) {
		read = MAX_PADDLE;
	}
	oldPaddleRead[paddle] = read;

	return read;
}