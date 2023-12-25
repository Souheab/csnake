#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define GAME_WIN_HEIGHT 15
#define GAME_WIN_WIDTH 35
#define MAIN_WIN_HEIGHT GAME_WIN_HEIGHT + 3
#define MAIN_WIN_WIDTH GAME_WIN_WIDTH + 2
#define TICK_TIME_MS 150

typedef struct {
  int y, x;
} Point;

typedef enum { LEFT, RIGHT, UP, DOWN } Direction;

typedef struct {
  Point segments[GAME_WIN_HEIGHT * GAME_WIN_WIDTH];
  Direction direction;
  int length;
} Snake;

int score = 0;
int game_over_flag = 0;
Snake snake = {{
                   [0] = {GAME_WIN_HEIGHT / 2, GAME_WIN_WIDTH / 2 + 2},
                   [1] = {GAME_WIN_HEIGHT / 2, GAME_WIN_WIDTH / 2 + 1},
                   [2] = {GAME_WIN_HEIGHT / 2, GAME_WIN_WIDTH / 2},
               },
               RIGHT,
               3};
Point pellet = {10, 10};

void init_curses() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  nodelay(stdscr, TRUE);
  timeout(TICK_TIME_MS);
  keypad(stdscr, TRUE);
}

void redraw_display() {
  clear();
  int screen_height, screen_width;
  getmaxyx(stdscr, screen_height, screen_width);

  int game_win_start_y = (screen_height - GAME_WIN_HEIGHT) / 2;
  int game_win_start_x = (screen_width - GAME_WIN_WIDTH) / 2;
  int main_win_start_y = game_win_start_y - 1;
  int main_win_start_x = game_win_start_x - 1;

  WINDOW *main_win = newwin(MAIN_WIN_HEIGHT, MAIN_WIN_WIDTH, main_win_start_y,
                            main_win_start_x);
  WINDOW *game_win = newwin(GAME_WIN_HEIGHT, GAME_WIN_WIDTH, game_win_start_y,
                            game_win_start_x);
  box(game_win, 0, 0);
  refresh();
  mvwprintw(main_win, MAIN_WIN_HEIGHT - 2, 1, "%s %d", "Score: ", score);
  mvwaddch(game_win, pellet.y, pellet.x, ACS_DIAMOND);
  wrefresh(main_win);

  for (int i = 0; snake.segments[i].y != 0 || snake.segments[i].x != 0; i++) {
    mvwaddch(game_win, snake.segments[i].y, snake.segments[i].x, ACS_BLOCK);
  }

  wrefresh(game_win);
}

Point get_random_point_value() {
  srand(time(NULL));
  int y = rand() % (GAME_WIN_HEIGHT - 2) + 1;
  int x = rand() % (GAME_WIN_WIDTH - 2) + 1;

  return (Point){y, x};
}

void sigwinch_handler(int signum) {
  endwin();
  refresh();
  redraw_display();
}

void input_handler(int input_ch) {
  switch (input_ch) {
  case 'q':
  case 'Q':
    endwin();
    exit(0);
    break;
  case KEY_UP:
  case 'K':
  case 'k':
    if (snake.direction != DOWN) {
      snake.direction = UP;
    }
    break;
  case KEY_DOWN:
  case 'J':
  case 'j':
    if (snake.direction != UP) {
      snake.direction = DOWN;
    }
    break;
  case KEY_LEFT:
  case 'H':
  case 'h':
    if (snake.direction != RIGHT) {
      snake.direction = LEFT;
    }
    break;
  case KEY_RIGHT:
  case 'L':
  case 'l':
    if (snake.direction != LEFT) {
      snake.direction = RIGHT;
    }
    break;
  }
}

void snake_collision_handler() {
  Point head = snake.segments[0];

  if (head.y == pellet.y && head.x == pellet.x) {
    pellet = get_random_point_value();
    score++;

    Point tail = snake.segments[snake.length - 1];
    Point tail_parent = snake.segments[snake.length - 2];
    Direction new_tail_direction;

    if (tail.x == tail_parent.x && tail.y > tail_parent.y) {
      new_tail_direction = UP;
    } else if (tail.x == tail_parent.x && tail.y < tail_parent.y) {
      new_tail_direction = DOWN;
    } else if (tail.x > tail_parent.x && tail.y == tail_parent.y) {
      new_tail_direction = LEFT;
    } else if (tail.x < tail_parent.x && tail.y == tail_parent.y) {
      new_tail_direction = RIGHT;
    }

    switch (new_tail_direction) {
    case LEFT:
      tail = (Point){tail.y, tail.x + 1};
      break;
    case RIGHT:
      tail = (Point){tail.y, tail.x - 1};
      break;
    case UP:
      tail = (Point){tail.y - 1, tail.x};
      break;
    case DOWN:
      tail = (Point){tail.y + 1, tail.x + 1};
      break;
    }

    snake.segments[snake.length] = tail;

    snake.length++;
  }

  else if (head.x < 1 || head.x > GAME_WIN_WIDTH - 2 || head.y < 1 ||
           head.y > GAME_WIN_HEIGHT - 2) {
    game_over_flag = 1;
  }

  else {
    for (int i = 1; i < snake.length; i++) {
      Point segment = snake.segments[i];
      if (head.x == segment.x && head.y == segment.y) {
        game_over_flag = 1;
      }
    }
  }
}

void move_snake() {
  Point head = snake.segments[0];

  switch (snake.direction) {
  case LEFT:
    head.x--;
    break;
  case RIGHT:
    head.x++;
    break;
  case UP:
    head.y--;
    break;
  case DOWN:
    head.y++;
    break;
  }

  for (int i = snake.length - 1; i > 0; i--) {
    snake.segments[i] = snake.segments[i - 1];
  }
  snake.segments[0] = head;
}

void game_over_handler() {
  if (game_over_flag) {
    endwin();
    printf("Game Over!\n");
    exit(0);
  }
}

int main() {
  init_curses();
  redraw_display();
  signal(SIGWINCH, sigwinch_handler);

  while (1) {
    input_handler(getch());
    move_snake();
    snake_collision_handler();
    game_over_handler();
    redraw_display();
    usleep(TICK_TIME_MS * 1000);
  }

  return 0;
}
