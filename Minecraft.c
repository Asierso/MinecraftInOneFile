#include <stdio.h>
#include <stdbool.h>
#include <windows.h> 
#include <time.h> 
#include <pthread.h>

//
// Main vars defined
//

//Window buffer matrix
char WINDOW_BUFFER[126][49];
//UI layer buffer matrix
char UI_BUFFER[126][49];

//Color buffer matrix
int WINDOW_BUFFER_FCOLOR[126][49]; //Fore
int WINDOW_BUFFER_BCOLOR[126][49]; //Back
//UI layer color buffer matrix
int UI_BUFFER_FCOLOR[126][49]; //Fore
int UI_BUFFER_BCOLOR[126][49]; //Back

//Layer buffer matrix
int WINDOW_BUFFER_PHYSIC[126][49];//Physics Layer
int UI_BUFFER_PHYSIC[126][49];//Physics Layer
//Buffer size
char WINDOW_BUFFER_W = sizeof(WINDOW_BUFFER) / sizeof(WINDOW_BUFFER[0]);
char WINDOW_BUFFER_H = sizeof(WINDOW_BUFFER[0]) / sizeof(WINDOW_BUFFER[0][0]);

//Light map
char LIGHT_MAP[21] = "@BR*#$PX0woIcv:+!~\".,";
char SPECIAL_MAP[21] = "¬‚ƒ‰ ÍÀÎŒÓœÔ‘Ù÷ˆ€˚‹¸";

//Controls
int CURSOR[2] = {0,0};
char KEY;
//Cubes
int CUBE_DIM[2] = {9,4};

//UI load memory cells
int UI_MEMORY[4] = {0,0,0,0};
//Getch enabled var
bool ENABLE_GETCH = false;
int KEY_MODE = 0;

//Temporal key array
char KEY_ARRAY[20];
int KEY_ARRAY_ITERATOR = 0;

//Color map
enum Colors { 
 BLACK = 0,
 BLUE = 1,
 GREEN = 2,
 CYAN = 3,
 RED = 4,
 MAGENTA = 5,
 BROWN = 6,
 LGREY = 7,
 DGREY = 8,
 LBLUE = 9,
 LGREEN = 10,
 LCYAN = 11,
 LRED = 12,
 LMAGENTA = 13,
 YELLOW = 14,
 WHITE = 15
};

//Orientations
enum Orientation
{
	HORIZONTAL = 0,
	VERTICAL = 1
};

//Threads
pthread_t PHYSIC_PROC; //Physics thread
pthread_t UPDATE_PROC; //Update / Key getcher / Drawer thread

//
// Referenced methods (by a thread or by another method)
//

//Physics renderer (Called in a thread)
void *physics()
{
	int x,y,i,j;
	while(1)
	{
		for(i = 0;i<WINDOW_BUFFER_H;i+=CUBE_DIM[1]) //Handle y
		{
			for(j = 0;j<WINDOW_BUFFER_W;j+=CUBE_DIM[0]) //Handle x 
			{
				//Sand blocks physics
				if(WINDOW_BUFFER_PHYSIC[j][i] == 11 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 0 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1)
				{
					drawBlock(j,i,0,false);
					drawBlock(j,i+=CUBE_DIM[1],11,false);
				}
				if(WINDOW_BUFFER_PHYSIC[j][i] == 11) //Fall on over fluid
					if(WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 4 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1)
					{
						drawBlock(j,i,0,false);
						drawBlock(j,i+=CUBE_DIM[1],11,false);
					}	
					else if(WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 10 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1)
					{
						drawBlock(j,i,0,false);
						drawBlock(j,i+=CUBE_DIM[1],11,false);
					}
						
				//Water blocks physics
				if(WINDOW_BUFFER_PHYSIC[j][i] == 4 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 0 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1) //Down expand
					drawBlock(j,i+=CUBE_DIM[1],4,false);
				else if(WINDOW_BUFFER_PHYSIC[j][i] == 4 && WINDOW_BUFFER_PHYSIC[j+CUBE_DIM[0]+1][i] == 0 && j+CUBE_DIM[0] < WINDOW_BUFFER_W-1 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] != 4) //Right expand
					drawBlock(j+=CUBE_DIM[0],i,4,false);
				
				if(WINDOW_BUFFER_PHYSIC[j][i] == 4 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 0 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1) {
				}
				else
					if(WINDOW_BUFFER_PHYSIC[j][i] == 4 && WINDOW_BUFFER_PHYSIC[j-CUBE_DIM[1]-1][i] == 0 && j > 0 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] != 4) //Left expand
						drawBlock(j-=CUBE_DIM[0],i,4,false);
				//Lava blocks physics
				if(WINDOW_BUFFER_PHYSIC[j][i] == 10 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 0 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1) //Down expand
					drawBlock(j,i+=CUBE_DIM[1],10,false);
				else if(WINDOW_BUFFER_PHYSIC[j][i] == 10 && WINDOW_BUFFER_PHYSIC[j+CUBE_DIM[0]+1][i] == 0 && j+CUBE_DIM[0] < WINDOW_BUFFER_W-1 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] != 10) //Right expand
					drawBlock(j+=CUBE_DIM[0],i,10,false);
				if(WINDOW_BUFFER_PHYSIC[j][i] == 10 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 0 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1) {
				}
				else
					if(WINDOW_BUFFER_PHYSIC[j][i] == 10 && WINDOW_BUFFER_PHYSIC[j-CUBE_DIM[1]-1][i] == 0 && j > 0 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] != 10) //Left expand
						drawBlock(j-=CUBE_DIM[0],i,10,false);
				//Lava and water join (lateral)
				if(WINDOW_BUFFER_PHYSIC[j][i] == 10 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] != 0)
					if(WINDOW_BUFFER_PHYSIC[j+CUBE_DIM[0]+1][i] == 4)
						drawBlock(j,i,8,false);
					else if(WINDOW_BUFFER_PHYSIC[j-CUBE_DIM[1]-1][i] == 4)
						drawBlock(j,i,8,false);
				if(WINDOW_BUFFER_PHYSIC[j][i] == 10 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 4 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1)
					drawBlock(j,i+=CUBE_DIM[1],8,false);
				if(WINDOW_BUFFER_PHYSIC[j][i] == 4 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 10 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1)
					drawBlock(j,i+=CUBE_DIM[1],8,false);
				if(WINDOW_BUFFER_PHYSIC[j][i] == 10 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 16 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1) //Snowy grass dry
					drawBlock(j,i+=CUBE_DIM[1],2,false);
				//Cobblestone block physics
				if(WINDOW_BUFFER_PHYSIC[j][i] == 8 && WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 1 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1) //Mossy conversion
					drawBlock(j,i,9,false);
				//Diamond, redstone and gold blocks falling physyics
				if(WINDOW_BUFFER_PHYSIC[j][i] == 17 || WINDOW_BUFFER_PHYSIC[j][i] == 18 || WINDOW_BUFFER_PHYSIC[j][i] == 19)
				{
					int physId = WINDOW_BUFFER_PHYSIC[j][i];
					if(WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 0 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1)
					{
						drawBlock(j,i,0,false);
						drawBlock(j,i+=CUBE_DIM[1],physId,false);
					}
					//Fall over fluid
					if(WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 4 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1)
					{
						drawBlock(j,i,0,false);
						drawBlock(j,i+=CUBE_DIM[1],physId,false);
					}	
					else if(WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 10 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1)
					{
						drawBlock(j,i,0,false);
						drawBlock(j,i+=CUBE_DIM[1],physId,false);
					}
				}
				//leaves and flowers physics
				if(WINDOW_BUFFER_PHYSIC[j][i] == 20 || WINDOW_BUFFER_PHYSIC[j][i] == 6)
				{
					if(WINDOW_BUFFER_PHYSIC[j][i+CUBE_DIM[1]+1] == 10 && i+CUBE_DIM[1] < WINDOW_BUFFER_H-1) //Bottom
						drawBlock(j,i,0,false);
					if(WINDOW_BUFFER_PHYSIC[j][i-1] == 10 && i-CUBE_DIM[1] > 0) //Top
						drawBlock(j,i,0,false);
					if(WINDOW_BUFFER_PHYSIC[j+CUBE_DIM[0]+1][i] == 10 && j+CUBE_DIM[0] < WINDOW_BUFFER_W-1) //Right
						drawBlock(j,i,0,false);
					if(WINDOW_BUFFER_PHYSIC[j-1][i] == 10 && j-CUBE_DIM[0] > 0) //Left
						drawBlock(j,i,0,false); 
				}
			}
		}
		//Physics delay (450ms)
		delay(450);
	}
}

//
// Start and update methods
//

//Main load function
void start()
{
	drawMenu(1);
	char arr[20] = "default";
	load(arr);
}

//Main loop function (logical code goes here)
void *update()
{
	int i;
	if(ENABLE_GETCH) KEY = getch();  
	if(KEY_MODE ==	0)
		switch(KEY)
	{
		//Cursor movement
		case 'W': 
		case 'w': //Move cursor up
			if(CURSOR[1] > 0 && UI_MEMORY[0] == 0)
				CURSOR[1]-=CUBE_DIM[1];
			drawCursor();
			break;
		case 'S':
		case 's': //Move cursor down
			if(CURSOR[1] < WINDOW_BUFFER_H-CUBE_DIM[1]-3 && UI_MEMORY[0] == 0)
				CURSOR[1]+=CUBE_DIM[1];
			drawCursor();
			break;
		case 'A':
		case 'a': //Move cursor left
			if(CURSOR[0] > 0 && UI_MEMORY[0] == 0)
				CURSOR[0]-=CUBE_DIM[0];
			drawCursor();
			break;
		case 'D':
		case 'd': //Move cursor right
			if(CURSOR[0] < WINDOW_BUFFER_W-CUBE_DIM[0]-2 && UI_MEMORY[0] == 0)
				CURSOR[0]+=CUBE_DIM[0];
			drawCursor();
			break;
		//UI
		case 'E':
		case 'e': //Show menu 0 (Inventory)
			drawMenu(0);
			break;
		case 'Q':
		case 'q': //Show menu 1 (Credits)
			drawMenu(1);
			break;
		case 'T':
		case 'G':
		case 'g': //Save world
			drawMenu(2);
			break;
		case 'L':
		case 'l': //Load world
			drawMenu(3);
			break;
		case 'R':
		case 'r': //Reset screen
			drawMatrix(0,0,WINDOW_BUFFER_W,WINDOW_BUFFER_H,NULL,BLACK,BLACK,0,false);
			break;
		//Block placement
		case '0': //Air
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],0,false);
			break;
		case '1': //Grass
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],1,false);
			break;
		case '2': //Dirt
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],2,false);
			break;
		case '3': //Bricks
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],3,false);
			break;
		case '4': //Water
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],4,false);
			break;
		case '5': //Planks
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],5,false);
			break;
		case '6': //Leaves
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],6,false);
			break;
		case '7': //Wood
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],7,false);
			break;
		case '8': //Cobblestone
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],8,false);
			break;
		case '9': //Mossy Cobblestone
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],9,false);
			break;
		case 'b':
		case 'B': //Lava
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],10,false);
			break;
		case 'c':
		case 'C': //Sand
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],11,false);
			break;
		case 'z':
		case 'Z': //Red wool
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],12,false);
			break;
		case 'x':
		case 'X': //Green wool
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],13,false);
			break;
		case 'v':
		case 'V': //Blue wool
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],14,false);
			break;
		case 'n':
		case 'N': //White wool
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],15,false);
			break;
		case 'm':
		case 'M': //Snowy grass
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],16,false);
			break;
		case 'h':
		case 'H': //Diamond
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],17,false);
			break;
		case 'j':
		case 'J': //Redstone
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],18,false);
			break;
		case 'k':
		case 'K': //Gold
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],19,false);
			break;
		case 'p':
		case 'P': //Flower leaves
			if(UI_MEMORY[0] == 0) drawBlock(CURSOR[0],CURSOR[1],20,false);
			break;
	}
	
	if(KEY_MODE == 1)
	{
		if(KEY == '.')
		{
			KEY_ARRAY_ITERATOR = 0;
			for(i = 0;i<4;i++)
			{
				if(UI_MEMORY[i] == 1)
					drawMenu(i);
			}
			KEY_MODE = 0;
			for(i = 0;i<20;i++)
				KEY_ARRAY[i] = NULL;
		}
		else
		{
			if(KEY_ARRAY_ITERATOR > 0)
				KEY_ARRAY[KEY_ARRAY_ITERATOR-1] = KEY;
			drawMenu(4);
			KEY_ARRAY_ITERATOR++;
		}
	}
	
	//Enable getch from first lock
	if(!ENABLE_GETCH) ENABLE_GETCH=true;
} 

//Engine startup
void main()
{
	start();
	//Define 2 threads (physics and update)
	pthread_create(&PHYSIC_PROC,NULL,&physics,NULL); //Exec physics
	//Engine loop
	while(1)
	{
		//Hide cursor
		int fontSize[2] = {1,1};
		HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO info;
		info.dwSize = 100;
		info.bVisible = FALSE;
		SetConsoleCursorInfo(consoleHandle, &info);
		SetConsoleTitle("Minecraft en C");
		//Engine actions
		pthread_create(&UPDATE_PROC,NULL,&update,NULL); //Exec update
		renderBuffer(); //Render buffer
	}
}

//
// Matrix overriders methods
//

//Override buffer with a char matrix
void drawMatrix(int x,int y,int w,int h,char chr,int bg,int fg,int physicId,bool useUIBuffer)
{
	//Default values in case of empty
	if(bg == NULL) bg = 0;
	if(fg == NULL) fg = 15;
	if(physicId == NULL) physicId = 0;
	if(useUIBuffer == NULL) useUIBuffer = false;
	//Write in buffer with double for
	int i, j;
	for(i = 0;i<w;i++) //Handle x
	{
		for(j = 0;j<h;j++) //Handle y
		{	
			if(useUIBuffer)
			{
				UI_BUFFER[x+i][y+j] = chr;
				UI_BUFFER_FCOLOR[x+i][y+j] = fg;
				UI_BUFFER_BCOLOR[x+i][y+j] = bg;
				UI_BUFFER_PHYSIC[x+i][y+j] = physicId;	
			}
			else
			{
				WINDOW_BUFFER[x+i][y+j] = chr;
				WINDOW_BUFFER_FCOLOR[x+i][y+j] = fg;
				WINDOW_BUFFER_BCOLOR[x+i][y+j] = bg;
				WINDOW_BUFFER_PHYSIC[x+i][y+j] = physicId;	
			}
		}
	}
}

//Override buffer with a 1D char array (text)
void drawText(int x,int y,char text[],int bg,int fg,int dir,int physicId,bool useUIBuffer)
{
	//Default values in case of empty
	if(dir == NULL) dir = 0;
	if(bg == NULL) bg = 0;
	if(fg == NULL) fg = 15;
	if(physicId == NULL) physicId = 0;
	if(useUIBuffer == NULL) useUIBuffer = false;
	//Write in buffer
	int i;
	for(i = 0;i<strlen(text);i++) //Handle text array
	{	
		if(useUIBuffer)
			switch(dir)
			{
				case 0: //Horizontal
					UI_BUFFER[x+i][y] = text[i];
					UI_BUFFER_FCOLOR[x+i][y] = fg;
					UI_BUFFER_BCOLOR[x+i][y] = bg;
					UI_BUFFER_PHYSIC[x+i][y] = physicId;
					break;
				case 1: //Vertical
					UI_BUFFER[x][y+i] = text[i];
					UI_BUFFER_FCOLOR[x][y+i] = fg;
					UI_BUFFER_BCOLOR[x][y+i] = bg;
					UI_BUFFER_PHYSIC[x][y+i] = physicId;
					break;
			}
		else
			switch(dir)
			{
				case 0: //Horizontal
					WINDOW_BUFFER[x+i][y] = text[i];
					WINDOW_BUFFER_FCOLOR[x+i][y] = fg;
					WINDOW_BUFFER_BCOLOR[x+i][y] = bg;
					WINDOW_BUFFER_PHYSIC[x+i][y] = physicId;
					break;
				case 1: //Vertical
					WINDOW_BUFFER[x][y+i] = text[i];
					WINDOW_BUFFER_FCOLOR[x][y+i] = fg;
					WINDOW_BUFFER_BCOLOR[x][y+i] = bg;
					WINDOW_BUFFER_PHYSIC[x][y+i] = physicId;
					break;
			}
	}
}

//Draw cursor buffer
void drawCursor()
{
	int i, j;
	for(i = 0;i<WINDOW_BUFFER_W;i++) //Handle x 
	{
		for(j = 0;j<WINDOW_BUFFER_H;j++) //Handle y
		{	
			if(UI_BUFFER_PHYSIC[i][j] == -1)
			{
				UI_BUFFER[i][j] = NULL;
				UI_BUFFER_FCOLOR[i][j] = NULL;
				UI_BUFFER_BCOLOR[i][j] = NULL;
				UI_BUFFER_PHYSIC[i][j] = NULL;
			}
		}
	}
	if(UI_MEMORY[0] == 0 && UI_MEMORY[1] == 0 && UI_MEMORY[2] == 0 && UI_MEMORY[3] == 0)
	{
		drawMatrix(CURSOR[0],CURSOR[1],CUBE_DIM[0],CUBE_DIM[1],' ',GREEN,GREEN,-1,true);
	}
}

//Render buffer matrix in screen
void renderBuffer()
{
	int i, j;
	//Render window and ui buffer
	for(i = 0;i<WINDOW_BUFFER_W;i++) //Handle x 
	{
		for(j = 0;j<WINDOW_BUFFER_H;j++) //Handle y
		{	
			gotoxy(i,j); //Cursor pos
			renderColor(WINDOW_BUFFER_BCOLOR[i][j],WINDOW_BUFFER_FCOLOR[i][j]); //Color
			printf("%c",WINDOW_BUFFER[i][j]); //Write
			//printf("%d",WINDOW_BUFFER_PHYSIC[i][j]); //Write Physics Layer (Only debug)
			
			if(UI_BUFFER[i][j] != NULL) // UI buffer
			{
				gotoxy(i,j); //Cursor pos
				renderColor(UI_BUFFER_BCOLOR[i][j],UI_BUFFER_FCOLOR[i][j]); //Color
				printf("%c",UI_BUFFER[i][j]); //Write
			}
		}
	}
	
}

//
// Methods about time control, and screen write
//

//Color render
void renderColor(int background, int text){
	HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);
	int ncolor = text + (background * 16);
	SetConsoleTextAttribute(Console, ncolor);
}

//Drawer Method
void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);		
}

//Time delay
void delay(int ms)
{
    clock_t start_time = clock();
  	//Looping clock
    while (clock() < start_time + ms)
        ;
}

//
// Menu region methods
//
void drawMenu(int menuId)
{
	int dx,dy,w,h,i,j,k;
	char text[25] = "Tecla ";
	char keys[25] = "0123456789BCZXVNMHJKP";
	switch(menuId)
	{
		case 0: //Inventory
			dx = WINDOW_BUFFER_W/4+5;
			dy = WINDOW_BUFFER_H/4-5;
			w = 54;
			h = 35;
			j=1;
			if(UI_MEMORY[0]==0 && UI_MEMORY[1]==0 && UI_MEMORY[2]==0 && UI_MEMORY[3]==0)
			{
				drawMatrix(dx,dy,w,1,205,BLACK,WHITE,0,true);
				drawMatrix(dx,dy+h-1,w,1,205,BLACK,WHITE,0,true);
				drawMatrix(dx,dy,1,h,186,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy,1,h,186,BLACK,WHITE,0,true);
				drawMatrix(dx,dy,1,1,201,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy,1,1,187,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy+h-1,1,1,188,BLACK,WHITE,0,true);
				drawMatrix(dx,dy+h-1,1,1,200,BLACK,WHITE,0,true);
				drawMatrix(dx+1,dy+1,w-2,h-2,' ',BLACK,BLACK,0,true);
				drawText(dx+2,dy+1,"Inventario",BLACK,WHITE,HORIZONTAL,1,true);
				drawText(dx+(w/2)-4,dy+h-2,"Pagina 1/1",BLACK,WHITE,HORIZONTAL,1,true);
				UI_MEMORY[0] = 1;
				for(k = 3;k<43;k+=13)
				{
					for(i = 3;i<33;i+= 6)
					{
						text[strlen("Tecla ")] = keys[j];
						drawBlock(dx+k,dy+i,j,true);
						drawText(dx+k,dy+i+4,text,BLACK,WHITE,HORIZONTAL,1,true);
						j++;
					}
				}
			}	
			else if(UI_MEMORY[1]==0 && UI_MEMORY[2]==0 && UI_MEMORY[3]==0)
			{
				drawMatrix(dx,dy,w,h,NULL,NULL,NULL,-1,true);
				UI_MEMORY[0] = 0;
			}
			break;
		case 1: //Credits
			dx = WINDOW_BUFFER_W/4;
			dy = WINDOW_BUFFER_H/4+2;
			w = 60;
			h = 20;
			j=1;
			if(UI_MEMORY[1]==0 && UI_MEMORY[0]==0 && UI_MEMORY[2]==0 && UI_MEMORY[3]==0)
			{
				drawMatrix(dx,dy,w,1,205,BLACK,WHITE,0,true);
				drawMatrix(dx,dy+h-1,w,1,205,BLACK,WHITE,0,true);
				drawMatrix(dx,dy,1,h,186,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy,1,h,186,BLACK,WHITE,0,true);
				drawMatrix(dx,dy,1,1,201,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy,1,1,187,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy+h-1,1,1,188,BLACK,WHITE,0,true);
				drawMatrix(dx,dy+h-1,1,1,200,BLACK,WHITE,0,true);
				drawMatrix(dx+1,dy+1,w-2,h-2,' ',BLACK,BLACK,0,true);
				drawText(dx+2,dy+1,"Bienvenido a Minecraft 2D en C",BLACK,WHITE,HORIZONTAL,1,true);
				drawText(dx+2,dy+2,"Creado por Asierso (@asiersob)",BLACK,RED,HORIZONTAL,1,true);
				drawText(dx+3,dy+4,"-Establezca la resolucion de la terminal a 126x51",BLACK,WHITE,HORIZONTAL,1,true);
				drawText(dx+3,dy+5," y la fuente a 10pt para evitar bugs visuales",BLACK,WHITE,HORIZONTAL,1,true);
				drawText(dx+2,dy+7,"Controles del juego",BLACK,WHITE,HORIZONTAL,1,true);
				drawText(dx+3,dy+9,"Moverse: W,A,S,D",BLACK,YELLOW,HORIZONTAL,1,true);
				drawText(dx+3,dy+10,"Inventario: E",BLACK,YELLOW,HORIZONTAL,1,true);
				drawText(dx+3,dy+11,"Creditos: Q",BLACK,YELLOW,HORIZONTAL,1,true);
				drawText(dx+3,dy+12,"Guardar: G",BLACK,YELLOW,HORIZONTAL,1,true);
				drawText(dx+3,dy+13,"Cargar: L",BLACK,YELLOW,HORIZONTAL,1,true);
				drawText(dx+3,dy+14,"Confirmar dialogo: .",BLACK,YELLOW,HORIZONTAL,1,true);
				drawText(dx+3,dy+15,"Resetear mundo: R",BLACK,YELLOW,HORIZONTAL,1,true);
				drawText(dx+3,dy+h-2,"Pulsa Q para cerrar esta ventana",BLACK,WHITE,HORIZONTAL,1,true);
				UI_MEMORY[1] = 1;
			}
			else if(UI_MEMORY[0]==0 && UI_MEMORY[2]==0 && UI_MEMORY[3]==0)
			{
				drawMatrix(dx,dy,w,h,NULL,NULL,NULL,-1,true);
				UI_MEMORY[1] = 0;
			}
			break;
		case 2: //Save world
			dx = WINDOW_BUFFER_W/4;
			dy = WINDOW_BUFFER_H/4+9;
			w = 60;
			h = 5;
			j=1;
			if(UI_MEMORY[2]==0 && UI_MEMORY[1]==0 && UI_MEMORY[0]==0 && UI_MEMORY[3]==0)
			{
				drawMatrix(dx,dy,w,1,205,BLACK,WHITE,0,true);
				drawMatrix(dx,dy+h-1,w,1,205,BLACK,WHITE,0,true);
				drawMatrix(dx,dy,1,h,186,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy,1,h,186,BLACK,WHITE,0,true);
				drawMatrix(dx,dy,1,1,201,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy,1,1,187,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy+h-1,1,1,188,BLACK,WHITE,0,true);
				drawMatrix(dx,dy+h-1,1,1,200,BLACK,WHITE,0,true);
				drawMatrix(dx+1,dy+1,w-2,h-2,' ',BLACK,BLACK,0,true);
				drawText(dx+2,dy+1,"Guardar",BLACK,WHITE,HORIZONTAL,1,true);
				drawText(dx+2,dy+3,"Nombre del mundo:",BLACK,WHITE,HORIZONTAL,1,true);
				UI_MEMORY[2] = 1;
				KEY_MODE = 1;
			}	
			else if(UI_MEMORY[1]==0 && UI_MEMORY[0]==0 && UI_MEMORY[3]==0)
			{
				drawMatrix(dx,dy,w,h,NULL,NULL,NULL,-1,true);
				save(KEY_ARRAY);
				UI_MEMORY[2] = 0;
				KEY_MODE = 0;
			}
			break;
		case 3: //Load world
			dx = WINDOW_BUFFER_W/4;
			dy = WINDOW_BUFFER_H/4+9;
			w = 60;
			h = 5;
			j=1;
			if(UI_MEMORY[3]==0 && UI_MEMORY[1]==0 && UI_MEMORY[0]==0 && UI_MEMORY[2]==0)
			{
				drawMatrix(dx,dy,w,1,205,BLACK,WHITE,0,true);
				drawMatrix(dx,dy+h-1,w,1,205,BLACK,WHITE,0,true);
				drawMatrix(dx,dy,1,h,186,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy,1,h,186,BLACK,WHITE,0,true);
				drawMatrix(dx,dy,1,1,201,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy,1,1,187,BLACK,WHITE,0,true);
				drawMatrix(dx+w-1,dy+h-1,1,1,188,BLACK,WHITE,0,true);
				drawMatrix(dx,dy+h-1,1,1,200,BLACK,WHITE,0,true);
				drawMatrix(dx+1,dy+1,w-2,h-2,' ',BLACK,BLACK,0,true);
				drawText(dx+2,dy+1,"Cargar",BLACK,WHITE,HORIZONTAL,1,true);
				drawText(dx+2,dy+3,"Nombre del mundo:",BLACK,WHITE,HORIZONTAL,1,true);
				UI_MEMORY[3] = 1;
				KEY_MODE = 1;
			}	
			else if(UI_MEMORY[1]==0 && UI_MEMORY[0]==0 && UI_MEMORY[2]==0 && UI_MEMORY[3]==1)
			{
				drawMatrix(dx,dy,w,h,NULL,NULL,NULL,-1,true);
				load(KEY_ARRAY);
				UI_MEMORY[3] = 0;
				KEY_MODE = 0;
			}	
			break;
		case 4: //Save-Load world name display
			dx = WINDOW_BUFFER_W/4;
			dy = WINDOW_BUFFER_H/4+9;
			w = 60;
			h = 5;
			if(UI_MEMORY[2]==1 || UI_MEMORY[3] == 1)
				drawText(dx+3+strlen("Nombre del mundo:"),dy+3,KEY_ARRAY,BLACK,WHITE,HORIZONTAL,1,true);	
			break;
	}
}

//
// Block drawer
//
void drawBlock(int x,int y,int blockId,bool useUIBuffer)
{
	//220,223
	//Default values in case of empty
	if(useUIBuffer == NULL) useUIBuffer = false;
	switch(blockId)
	{
		case 0: //Air
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],' ',BLACK,BLACK,0,useUIBuffer);
			break;
		case 1: //Grass
			drawMatrix(x,y,CUBE_DIM[0],1,252,LGREEN,GREEN,1,useUIBuffer);
			drawMatrix(x,y+1,CUBE_DIM[0],3,176,BROWN,LRED,1,useUIBuffer);
			break;
		case 2: //Dirt
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],176,BROWN,LRED,2,useUIBuffer);
			break;
		case 3: //Bricks
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],' ',LRED,RED,3,useUIBuffer);
			drawMatrix(x,y,2,CUBE_DIM[1],223,LRED,RED,3,useUIBuffer);
			drawMatrix(x+2,y,2,CUBE_DIM[1],220,LRED,RED,3,useUIBuffer);
			drawMatrix(x+4,y,2,CUBE_DIM[1],223,LRED,RED,3,useUIBuffer);
			drawMatrix(x+6,y,2,CUBE_DIM[1],220,LRED,RED,3,useUIBuffer);
			drawMatrix(x+8,y,1,CUBE_DIM[1],223,LRED,RED,3,useUIBuffer);
			break;
		case 4: //Water
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],177,BLUE,LBLUE,4,useUIBuffer);
			break;
		case 5: //Planks
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],254,YELLOW,BROWN,5,useUIBuffer);
			break;
		case 6: //Leaves
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],LIGHT_MAP[4],LGREEN,GREEN,6,useUIBuffer);
			break;
		case 7: //Wood
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],179,BROWN,LRED,7,useUIBuffer);
			break;
		case 8: //Cobblestone
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],LIGHT_MAP[4],DGREY,LGREY,8,useUIBuffer);
			break;
		case 9: //Mossy Cobblestone
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],LIGHT_MAP[0],DGREY,GREEN,9,useUIBuffer);
			break;
		case 10: //Lava
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],LIGHT_MAP[17],RED,YELLOW,10,useUIBuffer);
			break;
		case 11: //Sand
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],' ',YELLOW,BROWN,11,useUIBuffer);
			drawMatrix(x+1,y+3,1,1,254,YELLOW,BROWN,11,useUIBuffer);
			drawMatrix(x+5,y+1,1,1,254,YELLOW,BROWN,11,useUIBuffer);
			drawMatrix(x+3,y+2,1,1,254,YELLOW,BROWN,11,useUIBuffer);
			drawMatrix(x+5,y+3,1,1,254,YELLOW,BROWN,11,useUIBuffer);
			drawMatrix(x+3,y,1,1,254,YELLOW,BROWN,11,useUIBuffer);
			drawMatrix(x+1,y+1,1,1,254,YELLOW,BROWN,11,useUIBuffer);
			drawMatrix(x+7,y+2,1,1,254,YELLOW,BROWN,11,useUIBuffer);
			drawMatrix(x+7,y,1,1,254,YELLOW,BROWN,11,useUIBuffer);
			break;
		case 12: //Red wool
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],176,RED,MAGENTA,11,useUIBuffer);
			break;
		case 13: //Green wool
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],176,LGREEN,GREEN,11,useUIBuffer);
			break;
		case 14: //Blue wool
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],176,CYAN,BLUE,11,useUIBuffer);
			break;
		case 15: //Blue wool
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],176,WHITE,LGREY,11,useUIBuffer);
			break;
		case 16: //Snowy grass
			drawMatrix(x,y,CUBE_DIM[0],1,252,WHITE,LGREY,16,useUIBuffer);
			drawMatrix(x,y+1,CUBE_DIM[0],3,176,BROWN,LRED,16,useUIBuffer);
			break;
		case 17: //Diamond
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],LIGHT_MAP[4],CYAN,BLUE,17,useUIBuffer);
			drawMatrix(x+2,y+1,CUBE_DIM[0]-4,CUBE_DIM[1]-2,LIGHT_MAP[0],BLUE,BLUE,17,useUIBuffer);
			break;
		case 18: //Redstone
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],LIGHT_MAP[4],LRED,RED,18,useUIBuffer);
			drawMatrix(x+2,y+1,CUBE_DIM[0]-4,CUBE_DIM[1]-2,LIGHT_MAP[0],RED,RED,18,useUIBuffer);
			break;
		case 19: //Gold
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],LIGHT_MAP[4],YELLOW,BROWN,19,useUIBuffer);
			drawMatrix(x+2,y+1,CUBE_DIM[0]-4,CUBE_DIM[1]-2,LIGHT_MAP[0],BROWN,BROWN,19,useUIBuffer);
			break;
		case 20: //Flower leaves
			drawMatrix(x,y,CUBE_DIM[0],CUBE_DIM[1],252,GREEN,LGREY,20,useUIBuffer);
			break;
	}
}

//
// World manager
//
void save(char fname[20]) //Save world
{
	int i,j;
	//Concat extension to name
	strcat(fname, ".mcw");
	//Save window buffer
	FILE *file = fopen(fname,"w");
	//Parse buffer and save it by lines
	for(i = 0;i<WINDOW_BUFFER_H;i+=CUBE_DIM[1]) //Handle y
	{
		for(j = 0;j<WINDOW_BUFFER_W;j+=CUBE_DIM[0]) //Handle x 
		{
			fprintf(file,"%d;",WINDOW_BUFFER_PHYSIC[j][i]);
		}
		fprintf(file,"\n");
	}
	fclose(file);
}

void load(char fname[20]) //Load world
{
	int i,x,y;
	i = 0;
	x = 0;
	y = 0;
	//Concat extension to name
	strcat(fname, ".mcw");
	//Load window buffer from file
	FILE *file = fopen(fname,"r");
	char buffer[29];
	drawMatrix(0,0,WINDOW_BUFFER_W,WINDOW_BUFFER_H,NULL,BLACK,BLACK,0,false);
	//Parse loaded data by lines and process it
	while (fgets(buffer, 29, file))
    {
        strtok(buffer, "\n");
        char *ptr = strtok(buffer, ";");
        x=0;
        i++;
		while (ptr != NULL)
		{
			int a = ptr;
			if(atoi(ptr)>0)drawBlock(x,y,atoi(ptr),false);
			ptr = strtok(NULL, ";");
			x+=CUBE_DIM[0];
		}
		if(i==2)
		{
			y+=CUBE_DIM[1];	
			i=0;
		}
    }
	fclose(file);
}
//Programmed by Asierso
