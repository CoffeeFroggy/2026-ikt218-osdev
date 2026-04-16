#include <snakeApp/snake.h>

char board[BOARD_SIZE][BOARD_SIZE][3];
struct Snake snake;
struct Food food;
enum CollisionType collisionType;

void InitializeBoard(void) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j][0] = ' ';
            board[i][j][1] = ' ';
            board[i][j][2] = '\0';
        }
    }
}

void InitializeSnake(struct Snake* snake) {
    snake->length = 1;
    snake->direction = LEFT;
    snake->body[0].x = BOARD_SIZE / 2;
    snake->body[0].y = BOARD_SIZE / 2;
}

void InitializeFood(struct Food* food) {
    food->x = (BOARD_SIZE / 2) - 3;
    food->y = (BOARD_SIZE / 2) - 3;
}

struct SnakeSegment MoveSnake(struct Snake* snake) {
    int x = snake->body[0].x;
    int y = snake->body[0].y;

    switch (snake->direction) {
        case UP: {
            snake->body[0].y--;
            for (int i = 1; i < snake->length; i++) {
                int tempX = snake->body[i].x;
                int tempY = snake->body[i].y;
                snake->body[i].x = x;
                snake->body[i].y = y;
                x = tempX;
                y = tempY;
            }
            break;
        }
        case DOWN: {
            snake->body[0].y++;
            for (int i = 1; i < snake->length; i++) {
                int tempX = snake->body[i].x;
                int tempY = snake->body[i].y;
                snake->body[i].x = x;
                snake->body[i].y = y;
                x = tempX;
                y = tempY;
            }
            break;
        }
        case LEFT: {
            snake->body[0].x--;
            for (int i = 1; i < snake->length; i++) {
                int tempX = snake->body[i].x;
                int tempY = snake->body[i].y;
                snake->body[i].x = x;
                snake->body[i].y = y;
                x = tempX;
                y = tempY;
            }
            break;
        }
        case RIGHT: {
            snake->body[0].x++;
            for (int i = 1; i < snake->length; i++) {
                int tempX = snake->body[i].x;
                int tempY = snake->body[i].y;
                snake->body[i].x = x;
                snake->body[i].y = y;
                x = tempX;
                y = tempY;
            }
            break;
        }
    }
    return (struct SnakeSegment){ x, y };
}

void SpawnFood(struct Snake* snake, struct Food* food) {
    int x, y;
    int occupied;

    if (snake->length == SNAKE_MAX_LENGTH) {
        return;
    }

    do {
        x = rand() % BOARD_SIZE;
        y = rand() % BOARD_SIZE;

        occupied = 0;

        for (int i = 0; i < snake->length; i++) {
            if (snake->body[i].x == x && snake->body[i].y == y) {
                occupied = 1;
                break;
            }
        }
    } while (occupied);

    food->x = x;
    food->y = y;
}

void AddSegment(struct Snake* snake, int x, int y) {
    if (snake->length < SNAKE_MAX_LENGTH) {
        snake->body[snake->length].x = x;
        snake->body[snake->length].y = y;
        snake->length++;
    }
}

enum CollisionType CheckCollision(struct Snake* snake, struct Food* food) {
    if (snake->body[0].x == food->x && snake->body[0].y == food->y) {
        return FOOD;
    } else if (snake->body[0].x < 0 || snake->body[0].x >= BOARD_SIZE || snake->body[0].y < 0 || snake->body[0].y >= BOARD_SIZE) {
        return WALL;
    } else {
        for (int i = 1; i < snake->length; i++) {
            if (snake->body[0].x == snake->body[i].x && snake->body[0].y == snake->body[i].y) {
                return SELF;
            }
        }
        return NONE;
    }
}

void DrawBoard(struct Snake* snake, struct Food* food) {
    InitializeBoard();

    if (snake->direction == UP) {
        board[snake->body[0].y][snake->body[0].x][0] = '^';
        board[snake->body[0].y][snake->body[0].x][1] = '^';
    } else if (snake->direction == DOWN) {
        board[snake->body[0].y][snake->body[0].x][0] = 'v';
        board[snake->body[0].y][snake->body[0].x][1] = 'v';
    } else if (snake->direction == LEFT) {
        board[snake->body[0].y][snake->body[0].x][0] = '<';
        board[snake->body[0].y][snake->body[0].x][1] = '<';
    } else if (snake->direction == RIGHT) {
        board[snake->body[0].y][snake->body[0].x][0] = '>';
        board[snake->body[0].y][snake->body[0].x][1] = '>';
    }

    for (int i = 1; i < snake->length; i++) {
        board[snake->body[i].y][snake->body[i].x][0] = '[';
        board[snake->body[i].y][snake->body[i].x][1] = ']';
    }

    board[food->y][food->x][0] = '{';
    board[food->y][food->x][1] = '}';

    for (int i = 0; i < BOARD_SIZE + 2; i++) {
        printf("##");
    }
    printf("\n");

    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("##");
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%s", board[i][j]);
        }
        printf("##\n");
    }

    for (int i = 0; i < BOARD_SIZE + 2; i++) {
        printf("##");
    }
    printf("\n");
}

void PlayGame(void) {
    InitializeSnake(&snake);
    InitializeFood(&food);

    while(1) {
        if (snake.length == SNAKE_MAX_LENGTH) {
            break;
        }

        struct SnakeSegment tail = MoveSnake(&snake);

        collisionType = CheckCollision(&snake, &food);
        if (collisionType == FOOD) {
            AddSegment(&snake, tail.x, tail.y);
            SpawnFood(&snake, &food);
        } else if (collisionType == WALL) {
            InitializeSnake(&snake);
            InitializeFood(&food);
        } else if (collisionType == SELF) {
            InitializeSnake(&snake);
            InitializeFood(&food);
        }

        collisionType = NONE;

        DrawBoard(&snake, &food);
    }

    printf("You win!\n");
}