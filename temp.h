#include <avr/io.h>
#include <stdio.h>

#include "Bits.h"
#include "matrix.h"
#include "timer.h"
#include "ADC.h"
#include "queue.h"
#include "task_scheduler.h"
#include "pair.h"
#include "io.c"
#include "sevenseg.h"

#define MASK_1 0xB4BCD35C
#define MASK_2 0x7A5BC2E3

const int Up = 1;
const int Down = 2;
const int Left = 3;
const int Right = 4;

unsigned int StartGame;
unsigned int EndGame;
unsigned int Paused;
unsigned int CollisionSelf;
unsigned int CollisionFood;
unsigned int LFSR_1;
unsigned int LFSR_2;

const unsigned char offset = 48;
unsigned long int GCD;
unsigned long int TempGCD;

struct Snake
{
	struct Queue *body;
	int Direction;
	struct Pair head;
	struct Pair foodindex;
} snake;

void InitSnake(void)
{
	snake.Direction = Right;
	snake.body = CreateQueue();
	Push(snake.body, 1, 1);
	snake.head.x = 1;
	snake.head.y = 1;
	snake.foodindex.x = 1;
	snake.foodindex.y = 0;
}

int ShiftLFSR(unsigned int *LFSR, unsigned int Mask)
{
	int Feedback;
	
	Feedback = *LFSR & 1;
	*LFSR >>= 1;
	if (Feedback == 1)
	{
		*LFSR ^= Mask;
	}
	
	return *LFSR;
}

void LFSRInit(void)
{
	LFSR_1 = 0x23456789;
	LFSR_2 = 0xABCDE;
}

int Random(void)
{
	ShiftLFSR(&LFSR_1, MASK_1);
	
	return (ShiftLFSR(&LFSR_1, MASK_1) ^ ShiftLFSR(&LFSR_2, MASK_2)) & 0xFFFF;
}

void DisplayFood(void)
{
	matrix_write(snake.foodindex.x, snake.foodindex.y);
}

void DisplaySnake(void)
{
	matrix_clear();
	
	unsigned char Rows[8];
	
	for (int i = 0; i < 8; ++i)
	{
		Rows[i] = 0x00;
	}
	
	struct Node *n = snake.body->front;
	
	while (n != NULL)
	{
		Rows[n->x - 1] |= n->y;
		
		n = n->next;
	}
	
	DisplayFood();
	
	struct Node *m = snake.body->front;
	
	while (m != NULL)
	{
		if (snake.foodindex.x == m->x)
		{
			Rows[m->x - 1] |= snake.foodindex.y;
			matrix_write(m->x, Rows[m->x - 1]);
		}
		else
		{
			matrix_write(m->x, Rows[m->x - 1]);
		}
		
		m = m->next;
	}
}

int GetDirection(void)
{
	if (adc_read(0) <= 600 && adc_read(1) <= 100)
	{
		return Up;
	}
	else if (adc_read(0) <= 600 && adc_read(1) >= 800)
	{
		return Down;
	}
	else if (adc_read(0) <= 100 && adc_read(1) <= 600)
	{
		return Left;
	}
	else if (adc_read(0) >= 800 && adc_read(1) <= 600)
	{
		return Right;
	}
	
	return snake.Direction;
}

int WallCollision(void)
{
	if (snake.head.y <= 1 && snake.Direction == Up)
	{
		return 1;
	}
	else if (snake.head.y >= 128 && snake.Direction == Down)
	{
		return 1;
	}
	else if (snake.head.x <= 1 && snake.Direction == Left)
	{
		return 1;
	}
	else if (snake.head.x >= 8 && snake.Direction == Right)
	{
		return 1;
	}
	
	return 0;
}

int SelfCollision(void)
{
	if (Size(snake.body) <= 1)
	{
		return 0;
	}
	
	for (struct Node *i = snake.body->front; i != snake.body->back; i = i->next)
	{
		if (i->x == snake.head.x && i->y == snake.head.y)
		{
			return 1;
		}
	}
	
	return 0;
}

int FoodCollision(void)
{
	if (snake.foodindex.x == snake.head.x && snake.foodindex.y == snake.head.y)
	{
		return 1;
	}
	
	return 0;
}

void CreateFood(void)
{
	unsigned int size = 64 - Size(snake.body);
	unsigned char Cols[64];
	unsigned char Rows[64];
	unsigned char FreeCols[size];
	unsigned char FreeRows[size];
	unsigned char OccupiedCols[64];
	unsigned char OccupiedRows[64];
	
	int x = 0;
	
	for (int i = 1; i < 256; i *= 2)
	{
		for (int j = 1; j < 9; ++j)
		{
			if (x > 63)
			{
				break;
			}
			Cols[x] = j;
			Rows[x] = i;
			OccupiedCols[x] = 0;
			OccupiedRows[x] = 0;
			++x;
		}
	}
	
	struct Node *n = snake.body->front;
	
	while (n != NULL)
	{
		for (int i = 0; i < 64; ++i)
		{
			if (OccupiedCols[i] == n->x && OccupiedRows[i] == n->y)
			{
				OccupiedCols[i] = 1;
				OccupiedRows[i] = 1;
			}
		}
		
		n = n->next;
	}
	
	int y = 0;
	int z = 0;
	for (int i = 0; i < 64; ++i)
	{
		if (!OccupiedCols[i] && y < size)
		{
			FreeCols[y] = Cols[y];
			++y;
		}
		if (!OccupiedRows[i] && z < size)
		{
			FreeRows[z] = Rows[z];
			++z;
		}
	}
	
	int index = Random() % size - 1;
	snake.foodindex.x = FreeCols[index];
	snake.foodindex.y = FreeRows[index];
}

enum GameState { GameInit, GameMainMenu, GameMenuDisplay, GameDifficultyMenu, GameButtonRelease, GameStart, GameDisplayScore, GamePaused, GameEnd, GameEndMessage, GameReset };

int Game(int state)
{
	static unsigned char *Menu[] = { "Snake Game", "New Game", "Difficulty" };
	static unsigned char *Difficulty[] = { "Easy", "Medium", "Hard", "Back" };
	static unsigned int i;
	static unsigned int j;
	static unsigned int Options;
	static int Score;
	static unsigned int Unpaused;
	unsigned char OnesBuffer[1];
	unsigned char TensBuffer[2];
	static unsigned int CursorCount = 8;
	
	unsigned char Button = ~PINA & 0x1C;
	
	switch (state)
	{
		case GameInit:
		i = 0;
		j = 0;
		Options = 0;
		StartGame = 0;
		Score = 0;
		Paused = 0;
		GCD = TempGCD;
		Unpaused = 0;
		EndGame = 0;
		state = GameMenuDisplay;
		break;
		
		case GameMainMenu:
		if (Button == 0x04)
		{
			++i;
			if (i > 2)
			{
				i = 0;
			}
			
			state = GameMenuDisplay;
		}
		else if (Button == 0x08)
		{
			if (i == 1)
			{
				state = GameButtonRelease;
				break;
			}
			else if (i == 2)
			{
				Options = 1;
				state = GameMenuDisplay;
			}
			else
			{
				state = GameMainMenu;
			}
		}
		else
		{
			state = GameMainMenu;
		}
		break;
		
		case GameMenuDisplay:
		if (i == 2 && Options)
		{
			LCD_ClearScreen();
			LCD_DisplayString(1, Difficulty[j]);
			state = GameDifficultyMenu;
		}
		else
		{
			LCD_ClearScreen();
			LCD_DisplayString(1, Menu[i]);
			state = GameMainMenu;
		}
		break;
		
		case GameDifficultyMenu:
		if (Button == 0x04)
		{
			++j;
			if (j > 3)
			{
				j = 0;
			}
			
			state = GameMenuDisplay;
		}
		else if (Button == 0x08)
		{
			if (j == 0)
			{
				GCD = 100;
				Options = 0;
				state = GameMenuDisplay;
			}
			else if (j == 1)
			{
				GCD = 50;
				Options = 0;
				state = GameMenuDisplay;
			}
			else if (j == 2)
			{
				GCD = 25;
				Options = 0;
				state = GameMenuDisplay;
			}
			else
			{
				Options = 0;
				state = GameMenuDisplay;
			}
			
			j = 0;
		}
		else
		{
			state = GameDifficultyMenu;
		}
		break;
		
		case GameButtonRelease:
		if (!Button && !Paused)
		{
			LCD_ClearScreen();
			LCD_DisplayString(1, "Score: ");
			LCD_Cursor(CursorCount);
			StartGame = 1;
			state = GameDisplayScore;
		}
		else if (!Button && Paused && !Unpaused)
		{
			state = GamePaused;
		}
		else if (!Button && Unpaused)
		{
			Paused = 0;
			state = GameDisplayScore;
		}
		else
		{
			state = GameButtonRelease;
		}
		break;
		
		case GameStart:
		if (Button == 0x08)
		{
			Paused = 1;
			Unpaused = 0;
			LCD_DisplayString(1, "Paused");
			state = GameButtonRelease;
		}
		else if (Button == 0x10)
		{
			state = GameReset;
		}
		else
		{
			if (CollisionFood)
			{
				++Score;
				state = GameDisplayScore;
			}
			else if (CollisionSelf)
			{
				StartGame = 0;
				EndGame = 1;
				state = GameEndMessage;
			}
			else
			{
				state = GameStart;
			}
		}
		break;
		
		case GameDisplayScore:
		CursorCount = 8;
		LCD_Cursor(CursorCount);
		if (Score >= 10)
		{
			itoa(Score, TensBuffer, 10);
			for (unsigned int i = 0; i < 2; ++i)
			{
				LCD_WriteData(TensBuffer[i]);
				LCD_Cursor(++CursorCount);
			}
		}
		else
		{
			itoa(Score, OnesBuffer, 10);
			LCD_WriteData(OnesBuffer[0]);
		}
		if (Score >= 64)
		{
			state = GameEndMessage;
		}
		else
		{
			state = GameStart;
		}
		break;
		
		case GamePaused:
		if (Button == 0x08)
		{
			LCD_ClearScreen();
			LCD_DisplayString(1, "Score: ");
			Unpaused = 1;
			state = GameButtonRelease;
		}
		else if (Button == 0x10)
		{
			state = GameReset;
		}
		else
		{
			state = GamePaused;
		}
		break;
		
		case GameEnd:
		if (Button != 0x10)
		{
			state = GameEnd;
		}
		else
		{
			state = GameInit;
		}
		break;
		
		case GameEndMessage:
		if (Score >= 64)
		{
			LCD_DisplayString(1, "Congratulations!You Win!");
		}
		else
		{
			LCD_DisplayString(1, "You lose!       Nice try!");
		}
		state = GameEnd;
		break;
		
		case GameReset:
		StartGame = 0;
		state = GameInit;
		break;
		
		default:
		state = GameInit;
		break;
	}
	
	return state;
}

enum MoveState { MoveInit, MoveWait, MoveDisplace };

int Move(int state)
{
	switch (state)
	{
		case MoveInit:
		if (StartGame)
		{
			InitSnake();
			state = MoveWait;
		}
		else
		{
			state = MoveInit;
		}
		break;
		
		case MoveWait:
		if (StartGame && !Paused)
		{
			if (GetDirection() == Up && snake.Direction != Down)
			{
				snake.Direction = Up;
			}
			else if (GetDirection() == Down && snake.Direction != Up)
			{
				snake.Direction = Down;
			}
			else if (GetDirection() == Left && snake.Direction != Right)
			{
				snake.Direction = Left;
			}
			else if (GetDirection() == Right && snake.Direction != Left)
			{
				snake.Direction = Right;
			}
			
			state = MoveDisplace;
		}
		else
		{
			state = MoveWait;
		}
		if (!StartGame)
		{
			state = MoveInit;
			break;
		}
		break;
		
		case MoveDisplace:
		if (!WallCollision())
		{
			if (snake.Direction == Up)
			{
				snake.head.y >>= 1;
				
				Pop(snake.body);
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Down)
			{
				snake.head.y <<= 1;
				
				Pop(snake.body);
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Left)
			{
				--snake.head.x;
				
				Pop(snake.body);
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Right)
			{
				++snake.head.x;
				
				Pop(snake.body);
				Push(snake.body, snake.head.x, snake.head.y);
			}
		}
		else if (WallCollision())
		{
			if (snake.Direction == Up)
			{
				snake.head.y = 128;
				
				Pop(snake.body);
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Down)
			{
				snake.head.y = 1;
				
				Pop(snake.body);
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Left)
			{
				snake.head.x = 8;
				
				Pop(snake.body);
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Right)
			{
				snake.head.x = 1;
				
				Pop(snake.body);
				Push(snake.body, snake.head.x, snake.head.y);
			}
		}
		
		state = MoveWait;
		break;
		
		default:
		state = MoveInit;
		break;
	}
	
	return state;
}

enum CollisionState { CollisionInit, CollisionCheck };

int Collision(int state)
{
	switch (state)
	{
		case CollisionInit:
		CollisionSelf = 0;
		CollisionFood = 0;
		
		if (StartGame)
		{
			state = CollisionCheck;
		}
		break;
		
		case CollisionCheck:
		if (StartGame && !Paused)
		{
			if (SelfCollision())
			{
				CollisionSelf = 1;
			}
			
			state = CollisionCheck;
		}
		else
		{
			state = CollisionCheck;
		}
		if (!StartGame)
		{
			state = CollisionInit;
			break;
		}
		break;
		
		default:
		state = CollisionInit;
		break;
	}
	
	return state;
}

enum FoodState { FoodInit, FoodWait, FoodGrowSnake } foodstate;

int Food(int state)
{
	switch (state)
	{
		case FoodInit:
		if (StartGame)
		{
			CreateFood();
			state = FoodWait;
		}
		else
		{
			state = FoodInit;
		}
		break;
		
		case FoodWait:
		if (StartGame && !Paused)
		{
			if (FoodCollision())
			{
				CollisionFood = 1;
				state = FoodGrowSnake;
			}
			else
			{
				state = FoodWait;
			}
		}
		else
		{
			state = FoodWait;
		}
		if (!StartGame)
		{
			state = FoodInit;
		}
		break;
		
		case FoodGrowSnake:
		if (!WallCollision())
		{
			if (snake.Direction == Up)
			{
				snake.head.y >>= 1;
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Down)
			{
				snake.head.y <<= 1;
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Left)
			{
				--snake.head.x;
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Right)
			{
				++snake.head.x;
				Push(snake.body, snake.head.x, snake.head.y);
			}
		}
		else
		{
			if (snake.Direction == Up)
			{
				snake.head.y = 128;
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Down)
			{
				snake.head.y = 1;
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Left)
			{
				snake.head.x = 8;
				Push(snake.body, snake.head.x, snake.head.y);
			}
			else if (snake.Direction == Right)
			{
				snake.head.x = 1;
				Push(snake.body, snake.head.x, snake.head.y);
			}
		}
		
		CollisionFood = 0;
		CreateFood();
		state = FoodWait;
		break;
		
		default:
		state = FoodInit;
		break;
	}
	
	return state;
}

enum DisplayState { DisplayInit, DisplaySnakeBody, DisplayMenu };

int Display(int state)
{
	switch (state)
	{
		case DisplayInit:
		matrix_clear();
		if (StartGame)
		{
			state = DisplaySnakeBody;
		}
		else
		{
			state = DisplayInit;
		}
		break;
		
		case DisplaySnakeBody:
		if (StartGame)
		{
			DisplaySnake();
			state = DisplaySnakeBody;
		}
		else
		{
			state = DisplayInit;
		}
		break;
		
		default:
		break;
	}
	
	return state;
}

int main(void)
{
	DDRA = 0x00;
	PORTA = 0xFF;
	DDRC = 0xFF;
	PORTC = 0x00;
	DDRD = 0xFF;
	PORTD = 0x00;

	matrix_init();
	adc_init();
	LCD_init();
	LFSRInit();
	
	unsigned long int GameStateCalc = 100;
	unsigned long int MoveStateCalc = 100;
	unsigned long int CollisionStateCalc = 100;
	unsigned long int FoodStateCalc = 100;
	unsigned long int DisplayStateCalc = 100;
	
	unsigned long int tmpGCD = 1;
	int GCDArray[] = { GameStateCalc, MoveStateCalc, CollisionStateCalc, FoodStateCalc, DisplayStateCalc };
	tmpGCD = findGCD(GCDArray, 5);
	GCD = tmpGCD;
	
	unsigned long int GameStatePeriod = GameStateCalc / GCD;
	unsigned long int MoveStatePeriod = MoveStateCalc / GCD;
	unsigned long int CollisionStatePeriod = CollisionStateCalc / GCD;
	unsigned long int FoodStatePeriod = FoodStateCalc / GCD;
	unsigned long int DisplayStatePeriod = DisplayStateCalc / GCD;
	
	static task gamestate, movestate, collisionstate, foodstate, displaystate;
	task *tasks[] = { &gamestate, &movestate, &collisionstate, &foodstate, &displaystate };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task *);

	gamestate.state = GameInit;
	gamestate.period = GameStatePeriod;
	gamestate.elapsedTime = GameStatePeriod;
	gamestate.TickFct = &Game;

	movestate.state = MoveInit;
	movestate.period = MoveStatePeriod;
	movestate.elapsedTime = MoveStatePeriod;
	movestate.TickFct = &Move;
	
	collisionstate.state = CollisionInit;
	collisionstate.period = CollisionStatePeriod;
	collisionstate.elapsedTime = CollisionStatePeriod;
	collisionstate.TickFct = &Collision;
	
	foodstate.state = FoodInit;
	foodstate.period = FoodStatePeriod;
	foodstate.elapsedTime = FoodStatePeriod;
	foodstate.TickFct = &Food;
	
	displaystate.state = DisplayInit;
	displaystate.period = DisplayStatePeriod;
	displaystate.elapsedTime = DisplayStatePeriod;
	displaystate.TickFct = &Display;
	
	TimerOn();
	TempGCD = GCD;

	while(1)
	{
		TimerSet(GCD);
		for (unsigned short i = 0; i < numTasks; ++i)
		{
			// Task is ready to tick
			if (tasks[i]->elapsedTime == tasks[i]->period)
			{
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			
			++tasks[i]->elapsedTime;
		}
		
		while (!TimerFlag);
		TimerFlag = 0;
	}
	
	return 0;
}