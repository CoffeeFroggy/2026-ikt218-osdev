#ifndef SNAKEAPP_SNAKE_H
#define SNAKEAPP_SNAKE_H

#define BOARD_SIZE 25
#define SNAKE_MAX_LENGTH (BOARD_SIZE * BOARD_SIZE)

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum CollisionType {
    NONE,
    FOOD,
    SELF,
    WALL
};

struct SnakeSegment {
    int x;
    int y;
};

struct Snake {
    int length;
    enum Direction direction;
    struct SnakeSegment body[SNAKE_MAX_LENGTH];
};

struct Food {
    int x;
    int y;
};

void InitializeBoard(void);
void InitializeSnake(struct Snake* snake);
void InitializeFood(struct Food* food);
struct SnakeSegment MoveSnake(struct Snake* snake);
void SpawnFood(struct Snake* snake, struct Food* food);
void AddSegment(struct Snake* snake, int x, int y);
enum CollisionType CheckCollision(struct Snake* snake, struct Food* food);
void DrawBoard(struct Snake* snake, struct Food* food);
void PlayGame(void);

#endif