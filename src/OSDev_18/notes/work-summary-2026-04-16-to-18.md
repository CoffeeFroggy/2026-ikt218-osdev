I developed a simple application framework inside my OS by creating a menu system that lets the user switch between the music player and Snake. I implemented a basic terminal input method using keyboard interrupts so the user can select an application from the menu and return to it after use. This demonstrates integration between screen output, keyboard handling, and overall OS control flow.

For the Snake application, I designed a full game loop with its own game state, including the snake, food, score, and board representation. I used dynamic memory allocation to create and destroy the game state, which shows practical use of memory management in my OS. The game uses PIT/timing to control movement speed and game pacing, while keyboard input is used in real time to change direction, restart, or quit.

I also integrated sound through the PC speaker to give feedback for events such as eating food, dying, and winning. Overall, this task shows integration of multiple OS components, real-time system programming, practical application of operating system concepts, and creative problem-solving through building an interactive menu-driven game environment.

Finally, output is printed to the screen using the existing `TerminalWrite...()` infrastructure.
