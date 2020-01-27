# ESnake
ESnake is a two-dimensional game where the player must maneuver a line that grows in length after consuming food. The goal of the game is to survive as long as possible by not colliding the body of the snake with itself. The snake is controlled using a joystick and the food is spawned using a pseudo random number generator to generate the indices for the location in the food array. The contains a menu that allows the player to select up to four different difficulties ranging from “Easy” to “Impossible”, with each difficulty increasing the speed of the snake.When the game starts music will be played from a speaker which can be changed by pressing a button.  Each time a food is consumed, the score increments on the LCD display.


## User Guide

### Rules:
1.	When the player collides the head of the snake with the snake’s body, the player will lose. 
2.	When the player collides with a wall they will come back out on the other side of the wall.
3.	When the score reaches 63, the player wins and a win message will appear.
### Controls:
1.	There are four directions, “Up”, “Down”, “Left”, and “Right” which are controlled using a joystick.
2.	The game can be reset by pressing the leftmost button on the board.
3.	The game can be paused by pressing the middle button on the board.
4.	The music the game is playing game be changed by pressing the rightmost button on the board.
5.	After the game ends the player must press the reset button to exit the message screen.
## Technologies and Components
### Technologies:
1.	Atmel Studios 7 - used to write the code used in the game.
2.	Analog-to-Digital Converter - used to convert the analog signal given by the joystick and convert to values that allowed the direction of the joystick to be determined.
3.	Pulse Width Modulation - used to pulse the voltage supplied to the speaker to generate audio signals at specific frequency.
4.	Linear Feedback Shift Register - used to implement the randomness in the game. 
### Components:
1.	ATmega 1284 - mirocontroller used to connect everything together.
2.	Sainsmart Joystick - used to control the movement of the snake.
3.	1602A LCD Display - used to display the menu and the score in the game.
4.	Maxwell 7219 Shift Register - used in conjunction with the LED matrix to display the snake.
5.	Sainsmart LED Matrix - used to display the food and the snake.
6.	5611AS 7 Segment Display - used to display the current difficulty of the game.
7.	SN74HC595N Shift Register - used in conjunction with the 7 Segment Display to display the current difficulty of the game.
