
#include <gba_console.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <maxmod.h>
#include "soundbank.h"
#include "soundbank_bin.h"
#include "fontdata.h"
#include "defines.h"
#include <stdio.h>
#include <stdlib.h>



/**********************************************************************************************************************
												FUNCTION PROTOTYPES
**********************************************************************************************************************/
void ScreenInit();		// Sets up the display control regiter and BG layers
void PaletteSetup();	// Sets up the palette data
void SpriteSetup();		// Sets up the sprite data
void FontSetup();		// Sets up the font data
void DoMenu();			// Menu Screen
void DoInstructions();	// Instructions Screen
void DoGameOver();		// Game Over Screen
void DoGameWin();		// Game Win Screen
void DoBackground();	// fills the background with stars
extern void clearscreen (unsigned short *BG0MapBase);   // clears the screen in assembly
void DrawOBJ(unsigned short *OBJ, int YCoord, int hide, int XCoord, int size, int tile, int palette);	//draws the OBJ
int EnemyCollision(int x1, int y1, int x2, int y2);		//collision detection player/enemy
int CapsuleCollision(int x1, int y1, int x2, int y2);	//collision detection player/capsule

/***********************************************************************************************************************
													GLOBAL VARIABLES
***********************************************************************************************************************/
unsigned short *Player			= OBJ_HOLDER1;
unsigned short *Enemy1			= OBJ_HOLDER2;
unsigned short *Enemy2			= OBJ_HOLDER3;
unsigned short *RedCapsule		= OBJ_HOLDER4;
unsigned short *OrangeCapsule	= OBJ_HOLDER5;
unsigned short *YellowCapsule	= OBJ_HOLDER6;
unsigned short *GreenCapsule	= OBJ_HOLDER7;
unsigned short *BlueCapsule		= OBJ_HOLDER8;
unsigned short *PurpleCapsule	= OBJ_HOLDER9;
unsigned short *BG0MapBase		= (unsigned short*) 0x06004000;		//text background
unsigned short *BG1MapBase		= (unsigned short*) 0x06004800;		//stars background 1
unsigned short *BG2MapBase		= (unsigned short*) 0x06005000;		//stars background 2
unsigned short *BG1ScrollX		= (unsigned short*) 0x04000014;		//Background 1 X scrolling
unsigned short *BG2ScrollX		= (unsigned short*) 0x04000018;		//Background 2 X scrolling

unsigned short heldKey;			//key that is being currently held down
unsigned short Key;				//key that has been pressed and let go
int PlayerXCoord		= 1;
int PlayerYCoord		= 70;
int Enemy1XCoord		= 0;
int Enemy1YCoord		= 0;
int Enemy2XCoord		= 0;
int Enemy2YCoord		= 0;
int CapsuleXCoord		= 0;
int CapsuleYCoord		= 0;
int ActiveTile = 1;				// for animation


int scrollit;		//the iterator for scrolling


enum States {Menu = 0, Instructions, Game, GameOver, Win} GameState;		//enum for the game states

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
	//---------------------------------------------------------------------------------


	ScreenInit();
	mmInitDefault( (mm_addr)soundbank_bin, 8 ); 
	PaletteSetup();
	SpriteSetup();
	FontSetup();
	DoBackground();


	int XScrollVal1 = 0;
	int XScrollVal2 = 0;
	int CapsuleCount =0;
	
	mmStart(MOD_MODULE2, MM_PLAY_LOOP);
	// main loop
	while (1) 
	{	
		VBlankIntrWait();
		mmFrame();
		/*********Scrolling*********/
		scrollit++;					
		if (scrollit == 2)
		{
			XScrollVal2++;
			scrollit = 0;
		}
		XScrollVal1++;

		*BG1ScrollX = XScrollVal1;
		*BG2ScrollX = XScrollVal2;
		/****************************/

		scanKeys();
		heldKey = keysHeld();

		Key = keysUp();

		if (GameState == Menu)
		{
			/***********************Hides OBJs**************************/
			DrawOBJ(Player,PlayerYCoord,1,PlayerXCoord,1,ActiveTile,15);
			DrawOBJ(Enemy1,Enemy1YCoord,1,Enemy1XCoord,1,9,15);
			DrawOBJ(Enemy2,Enemy2YCoord,1,Enemy2XCoord,1,9,15);
			DrawOBJ(RedCapsule,CapsuleYCoord,1,CapsuleXCoord,0,13,14);
			DrawOBJ(OrangeCapsule,CapsuleYCoord,1,CapsuleXCoord,0,14,14);
			DrawOBJ(YellowCapsule,CapsuleYCoord,1,CapsuleXCoord,0,15,14);
			DrawOBJ(GreenCapsule,CapsuleYCoord,1,CapsuleXCoord,0,16,14);
			DrawOBJ(BlueCapsule,CapsuleYCoord,1,CapsuleXCoord,0,17,14);
			DrawOBJ(PurpleCapsule,CapsuleYCoord,1,CapsuleXCoord,0,18,14);
			/***********************************************************/
			DoMenu();
			if (Key & KEY_START)
			{
				GameState = Instructions;
				clearscreen(BG0MapBase);
			}

		}
		else 
			if (GameState == Instructions)
			{
				DoInstructions();
				if (Key & KEY_START)
				{
					GameState = Game;
					clearscreen(BG0MapBase);
				}
			}
			else 
				if (GameState == Game)
				{

					/*****************Update Player***************/
					if ((heldKey & KEY_DOWN) && (PlayerYCoord < 145))
					{
						PlayerYCoord++;
						if (ActiveTile == 1)
						{
							ActiveTile = 5;
						}
						else
						{
							ActiveTile = 1;
						}

					}

					if ((heldKey & KEY_UP) && (PlayerYCoord > 1))
					{
						PlayerYCoord--;
						if (ActiveTile == 1)
						{
							ActiveTile = 5;
						}
						else
						{
							ActiveTile = 1;
						}

					}

					/**********************Update Enemy********************/
					if (Enemy1XCoord == 0)
					{
						Enemy1XCoord = 250;
						Enemy1YCoord = rand()%145;
					}
					else
					{
						Enemy1XCoord--;
					}

					if (Enemy2XCoord == 0)
					{
						Enemy2XCoord = 230;
						Enemy2YCoord = rand()%145;
					}
					else
					{
						Enemy2XCoord--;
					}

					if (Enemy1YCoord == Enemy2YCoord)  
					{
						Enemy1YCoord =rand()%145;
					}

					/************************Update Capsule**********************/
					if (CapsuleXCoord < -8)
					{
						CapsuleXCoord = 260;
						CapsuleYCoord = rand()%145;
					}
					else 
					{
						CapsuleXCoord--;
					}

					/************************Check for Collision with Capsule*************************/
					if (CapsuleCollision(PlayerXCoord,PlayerYCoord,CapsuleXCoord,CapsuleYCoord) == 1)
					{
						mmJingle( MOD_CATMEOW );
						CapsuleCount ++;
						CapsuleXCoord = 260;
					}

					/**********************Draw Player**************************/
					DrawOBJ(Player,PlayerYCoord,0,PlayerXCoord,1,ActiveTile,15);
					/**********************Draw Enemy***************************/
					DrawOBJ(Enemy1,Enemy1YCoord,0,Enemy1XCoord,1,9,15);
					DrawOBJ(Enemy2,Enemy2YCoord,0,Enemy2XCoord,1,9,15);
					/*******************************Draw Capsule**************************************/
					switch (CapsuleCount)
					{
					case 0 :
						DrawOBJ(RedCapsule,CapsuleYCoord,0,CapsuleXCoord,0,13,14);		//Draw Red Capsule
						break;
					case 1 :
						DrawOBJ(RedCapsule,CapsuleYCoord,1,CapsuleXCoord,0,13,14);		//Hide Red Capsule
						
						DrawOBJ(OrangeCapsule,CapsuleYCoord,0,CapsuleXCoord,0,14,14);	//Draw Orange Capsule
						break;
					case 2 :
						DrawOBJ(OrangeCapsule,CapsuleYCoord,1,CapsuleXCoord,0,14,14);	//Hide Orange Capsule
						
						DrawOBJ(YellowCapsule,CapsuleYCoord,0,CapsuleXCoord,0,15,14);	//Draw Yellow Capsule
						break;
					case 3 :
						DrawOBJ(YellowCapsule,CapsuleYCoord,1,CapsuleXCoord,0,15,14);	//Hide Yellow Capsule
						
						DrawOBJ(GreenCapsule,CapsuleYCoord,0,CapsuleXCoord,0,16,14);	//Draw Green Capsule
						break;
					case 4 :
						DrawOBJ(GreenCapsule,CapsuleYCoord,1,CapsuleXCoord,0,16,14);	//Hide Green Capsule
						
						DrawOBJ(BlueCapsule,CapsuleYCoord,0,CapsuleXCoord,0,17,14);		//Draw Blue Capsule
						break;
					case 5 :
						DrawOBJ(BlueCapsule,CapsuleYCoord,1,CapsuleXCoord,0,17,14);		//Hide Blue Capsule
						
						DrawOBJ(PurpleCapsule,CapsuleYCoord,0,CapsuleXCoord,0,18,14);	//Draw Purple Capsule
						break;
					case 6 :
						DrawOBJ(PurpleCapsule,CapsuleYCoord,1,CapsuleXCoord,0,18,14);	//Hide Purple Capsule
						CapsuleCount = 0;												//Reset
						CapsuleXCoord = 0;												//Reset
						GameState = Win;
						break;
					}
					/***********************Check for Collision with Enemy 1****************************/
						if (EnemyCollision(PlayerXCoord,PlayerYCoord,Enemy1XCoord,Enemy1YCoord) == 1)
						{
							GameState = GameOver;
							/***********************Resets Position*********************/
							Enemy1XCoord = 0;
							Enemy2XCoord = 0;

						}
					/************************Check for Collision with Enemy2*****************************/
						if (EnemyCollision(PlayerXCoord,PlayerYCoord,Enemy2XCoord,Enemy2YCoord) == 1)
						{
							GameState = GameOver;
							/***********************Resets Position*********************/
							Enemy1XCoord = 0;
							Enemy2XCoord = 0;

						}

					}
					else
						if (GameState == GameOver)
						{
							DoGameOver();
							if (Key & KEY_START)
							{
								GameState= Menu;
								clearscreen(BG0MapBase);
							}
						}
						else
							if (GameState == Win)
							{
								DoGameWin();
								if (Key & KEY_START)
								{
									GameState= Menu;
									clearscreen(BG0MapBase);
								}
							}
						
				}
	}

void ScreenInit()
{
	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	irqInit();
	irqSet( IRQ_VBLANK, mmVBlank );
	irqEnable(IRQ_VBLANK);

	//setting up the display control register and bg layers
	DISPCNT = (0 << 0) | (1 << 6) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12);  //MODE 0|1-DIMENSIONAL OBJ DATA|BG0 ON|BG1 ON|BG2 ON|BG3 ON|OBJs ON
	BG0CNT  = (0 << 0) | (0 << 2) | (0 << 7) | (8 << 8)| (0 << 14);		//PRIORITY 0|CHARACTER BASE BLOCK 0|16 PALETTES OF 16 COLOURS|SCREEN BASE BLOCK 8|256*256 PIXELS
	BG1CNT  = (1 << 0) | (0 << 2) | (0 << 7) | (9 << 8)| (0 << 14);		//PRIORITY 1|CHARACTER BASE BLOCK 0|16 PALETTES OF 16 COLOURS|SCREEN BASE BLOCK 9|256*256 PIXELS
	BG2CNT  = (2 << 0) | (0 << 2) | (0 << 7) | (10 << 8) | (0 << 14);	//PRIORITY 2|CHARACTER BASE BLOCK 0|16 PALETTES OF 16 COLOURS|SCREEN BASE BLOCK 10|256*256 PIXELS	
	BG3CNT  = (3 << 0) | (0 << 2) | (0 << 7) | (11 << 8) | (0 << 14);	//PRIORITY 3|CHARACTER BASE BLOCK 0|16 PALETTES OF 16 COLOURS|SCREEN BASE BLOCK 11|256*256 PIXELS

}



void PaletteSetup()
{
	// Background Colour 
	unsigned short *BGPalette0 = (unsigned short*) 0x05000000;
	BGPalette0[0]=		RGB5( 0, 0, 9);

	// Rainbow Palette - sets first color in palettes 9-15 
	unsigned short *RainbowPal = (unsigned short*) 0x05000000;
	RainbowPal[(14*16)+1] = RGB5(31, 0, 0);  //red
	RainbowPal[(13*16)+1] = RGB5(31,19, 0);	 //orange
	RainbowPal[(12*16)+1] = RGB5(31,31, 0);	 //yellow
	RainbowPal[(11*16)+1] = RGB5( 0,31, 0);  //green
	RainbowPal[(10*16)+1] = RGB5( 0, 0,31);  //blue
	RainbowPal[( 9*16)+1] = RGB5(29, 0,31);  //purple

	// BG Palette 16 configuration
	unsigned short *BGPalette15= (unsigned short*) 0x050001E0;
	BGPalette15[0]=		RGB5( 0, 0, 9);
	BGPalette15[1]=		RGB5(28,28,31);
	BGPalette15[2]=		RGB5(31,27, 0);
	BGPalette15[3]=		RGB5(29,18,10);
	BGPalette15[4]=		RGB5(22,22,22);
	BGPalette15[5]=		RGB5(31, 0,14);
	BGPalette15[6]=		RGB5(18,12,31);
	

	// OBJ palette 16 configuration
	unsigned short *OBJPalette15 = (unsigned short*) 0x050003E0;

	OBJPalette15[0]=	RGB5( 0, 0, 9);
	OBJPalette15[1]=	RGB5(29,12,31);
	OBJPalette15[2]=	RGB5( 0, 0, 0);
	OBJPalette15[3]=	RGB5(31, 0,14);
	OBJPalette15[4]=	RGB5(22,22,22);
	OBJPalette15[5]=	RGB5(29,18,10);
	OBJPalette15[6]=	RGB5(31,31,31);
	OBJPalette15[7]=	RGB5( 0, 0,31);
	OBJPalette15[8]=	RGB5(11, 0,16);
	OBJPalette15[9]=	RGB5(24,31,31);
	OBJPalette15[10]=	RGB5( 0, 0,16);
	

	//Capsule palette 15 configuration
	unsigned short*CAPPalette14	= (unsigned short*) 0x050003c0;
	CAPPalette14[0]=	RGB5( 0, 0, 9);
	CAPPalette14[1]=	RGB5( 0, 0, 0);		//Black for outline
	CAPPalette14[2]=	RGB5(16, 0, 0);		//Red Capsule
	CAPPalette14[3]=	RGB5(31, 0, 0);		//Red Capsule
	CAPPalette14[4]=	RGB5(26,15, 0);		//Orange Capsule
	CAPPalette14[5]=	RGB5(31,19, 0);		//Orange Capsule
	CAPPalette14[6]=	RGB5(21,20, 0);		//Yellow Capsule
	CAPPalette14[7]=	RGB5(31,31, 0);		//Yellow Capsule
	CAPPalette14[8]=	RGB5( 0,16, 0);		//Green Capsule
	CAPPalette14[9]=	RGB5( 0,31, 0);		//Green Capsule
	CAPPalette14[10]=	RGB5( 0, 0,16);		//Blue Capsule
	CAPPalette14[11]=	RGB5( 0, 0,31);		//Blue Capsule
	CAPPalette14[12]=	RGB5(11, 0,16);		//Purple Capsule
	CAPPalette14[13]=	RGB5(29, 0,31);		//Purple Capsule


}


void SpriteSetup()
{
	/******************************************************************************************************************
	Player Sprite Data
	******************************************************************************************************************/
	// Player sprite data for frame 1
	unsigned int *PlayerSprite1 = (unsigned int*) 0x06010020;
	PlayerSprite1[0]= (0 << 0) | (0 << 4) | (0 << 8) | (0 << 12) | (0 << 16) | (0 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite1[1]= (0 << 0) | (0 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (2 << 20) | (2 << 24) | (2 << 28);
	PlayerSprite1[2]= (0 << 0) | (2 << 4) | (5 << 8) | (5 << 12) | (5 << 16) | (5 << 20) | (5 << 24) | (5 << 28);
	PlayerSprite1[3]= (2 << 0) | (5 << 4) | (5 << 8) | (1 << 12) | (3 << 16) | (1 << 20) | (1 << 24) | (1 << 28);
	PlayerSprite1[4]= (2 << 0) | (5 << 4) | (1 << 8) | (3 << 12) | (1 << 16) | (1 << 20) | (3 << 24) | (1 << 28);
	PlayerSprite1[5]= (2 << 0) | (5 << 4) | (1 << 8) | (1 << 12) | (1 << 16) | (1 << 20) | (1 << 24) | (3 << 28);
	PlayerSprite1[6]= (2 << 0) | (5 << 4) | (1 << 8) | (3 << 12) | (1 << 16) | (3 << 20) | (1 << 24) | (2 << 28);
	PlayerSprite1[7]= (4 << 0) | (2 << 4) | (1 << 8) | (1 << 12) | (1 << 16) | (1 << 20) | (3 << 24) | (2 << 28);

	unsigned int *PlayerSprite2 = (unsigned int*) 0x06010040;
	PlayerSprite2[0]= (0 << 0) | (0 << 4) | (0 << 8) | (0 << 12) | (0 << 16) | (0 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite2[1]= (2 << 0) | (2 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (0 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite2[2]= (5 << 0) | (5 << 4) | (5 << 8) | (5 << 12) | (5 << 16) | (2 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite2[3]= (1 << 0) | (2 << 4) | (2 << 8) | (1 << 12) | (2 << 16) | (2 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite2[4]= (2 << 0) | (4 << 4) | (4 << 8) | (2 << 12) | (4 << 16) | (4 << 20) | (2 << 24) | (0 << 28);
	PlayerSprite2[5]= (2 << 0) | (4 << 4) | (4 << 8) | (2 << 12) | (4 << 16) | (4 << 20) | (2 << 24) | (0 << 28);
	PlayerSprite2[6]= (4 << 0) | (4 << 4) | (4 << 8) | (4 << 12) | (4 << 16) | (4 << 20) | (4 << 24) | (2 << 28);
	PlayerSprite2[7]= (4 << 0) | (6 << 4) | (2 << 8) | (4 << 12) | (6 << 16) | (2 << 20) | (4 << 24) | (2 << 28);

	unsigned int *PlayerSprite3 = (unsigned int*) 0x06010060;
	PlayerSprite3[0]= (4 << 0) | (4 << 4) | (2 << 8) | (1 << 12) | (1 << 16) | (3 << 20) | (1 << 24) | (2 << 28);
	PlayerSprite3[1]= (2 << 0) | (4 << 4) | (4 << 8) | (2 << 12) | (1 << 16) | (1 << 20) | (1 << 24) | (2 << 28);
	PlayerSprite3[2]= (2 << 0) | (2 << 4) | (2 << 8) | (1 << 12) | (1 << 16) | (1 << 20) | (1 << 24) | (2 << 28);
	PlayerSprite3[3]= (2 << 0) | (5 << 4) | (5 << 8) | (1 << 12) | (1 << 16) | (3 << 20) | (1 << 24) | (1 << 28);
	PlayerSprite3[4]= (0 << 0) | (2 << 4) | (5 << 8) | (5 << 12) | (5 << 16) | (5 << 20) | (5 << 24) | (5 << 28);
	PlayerSprite3[5]= (2 << 0) | (4 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (2 << 20) | (2 << 24) | (2 << 28);
	PlayerSprite3[6]= (4 << 0) | (2 << 4) | (0 << 8) | (2 << 12) | (4 << 16) | (2 << 20) | (0 << 24) | (2 << 28);
	PlayerSprite3[7]= (2 << 0) | (2 << 4) | (0 << 8) | (2 << 12) | (2 << 16) | (2 << 20) | (0 << 24) | (2 << 28);

	unsigned int *PlayerSprite4 = (unsigned int*) 0x06010080;
	PlayerSprite4[0]= (4 << 0) | (2 << 4) | (2 << 8) | (4 << 12) | (2 << 16) | (2 << 20) | (4 << 24) | (2 << 28);
	PlayerSprite4[1]= (3 << 0) | (4 << 4) | (4 << 8) | (2 << 12) | (4 << 16) | (4 << 20) | (3 << 24) | (2 << 28);
	PlayerSprite4[2]= (4 << 0) | (4 << 4) | (2 << 8) | (4 << 12) | (4 << 16) | (4 << 20) | (4 << 24) | (2 << 28);
	PlayerSprite4[3]= (2 << 0) | (4 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (4 << 20) | (2 << 24) | (0 << 28);
	PlayerSprite4[4]= (5 << 0) | (2 << 4) | (4 << 8) | (4 << 12) | (4 << 16) | (2 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite4[5]= (2 << 0) | (2 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (2 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite4[6]= (4 << 0) | (2 << 4) | (0 << 8) | (2 << 12) | (4 << 16) | (2 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite4[7]= (2 << 0) | (2 << 4) | (0 << 8) | (2 << 12) | (2 << 16) | (2 << 20) | (0 << 24) | (0 << 28);

	// Player sprite data for frame 2
	unsigned int *PlayerSprite5 = (unsigned int*) 0x060100a0;
	PlayerSprite5[0]= (0 << 0) | (0 << 4) | (0 << 8) | (0 << 12) | (0 << 16) | (0 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite5[1]= (0 << 0) | (0 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (2 << 20) | (2 << 24) | (2 << 28);
	PlayerSprite5[2]= (0 << 0) | (2 << 4) | (5 << 8) | (5 << 12) | (5 << 16) | (5 << 20) | (5 << 24) | (5 << 28);
	PlayerSprite5[3]= (2 << 0) | (5 << 4) | (5 << 8) | (1 << 12) | (3 << 16) | (1 << 20) | (1 << 24) | (1 << 28);
	PlayerSprite5[4]= (2 << 0) | (5 << 4) | (1 << 8) | (3 << 12) | (1 << 16) | (1 << 20) | (3 << 24) | (1 << 28);
	PlayerSprite5[5]= (2 << 0) | (5 << 4) | (2 << 8) | (2 << 12) | (1 << 16) | (1 << 20) | (1 << 24) | (2 << 28);
	PlayerSprite5[6]= (2 << 0) | (2 << 4) | (4 << 8) | (4 << 12) | (2 << 16) | (3 << 20) | (1 << 24) | (2 << 28);
	PlayerSprite5[7]= (4 << 0) | (4 << 4) | (4 << 8) | (2 << 12) | (1 << 16) | (1 << 20) | (3 << 24) | (2 << 28);

	unsigned int *PlayerSprite6 = (unsigned int*) 0x060100c0;
	PlayerSprite6[0]= (0 << 0) | (0 << 4) | (0 << 8) | (0 << 12) | (0 << 16) | (0 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite6[1]= (2 << 0) | (2 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (0 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite6[2]= (5 << 0) | (2 << 4) | (2 << 8) | (5 << 12) | (2 << 16) | (2 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite6[3]= (2 << 0) | (4 << 4) | (4 << 8) | (2 << 12) | (4 << 16) | (4 << 20) | (2 << 24) | (0 << 28);
	PlayerSprite6[4]= (2 << 0) | (4 << 4) | (4 << 8) | (2 << 12) | (4 << 16) | (4 << 20) | (2 << 24) | (0 << 28);
	PlayerSprite6[5]= (4 << 0) | (4 << 4) | (4 << 8) | (4 << 12) | (4 << 16) | (4 << 20) | (4 << 24) | (2 << 28);
	PlayerSprite6[6]= (4 << 0) | (6 << 4) | (2 << 8) | (4 << 12) | (6 << 16) | (2 << 20) | (4 << 24) | (2 << 28);
	PlayerSprite6[7]= (4 << 0) | (2 << 4) | (2 << 8) | (4 << 12) | (2 << 16) | (2 << 20) | (4 << 24) | (2 << 28);

	unsigned int *PlayerSprite7 = (unsigned int*) 0x060100e0;
	PlayerSprite7[0]= (4 << 0) | (4 << 4) | (2 << 8) | (3 << 12) | (1 << 16) | (3 << 20) | (1 << 24) | (2 << 28);
	PlayerSprite7[1]= (2 << 0) | (2 << 4) | (1 << 8) | (1 << 12) | (1 << 16) | (1 << 20) | (1 << 24) | (2 << 28);
	PlayerSprite7[2]= (2 << 0) | (5 << 4) | (1 << 8) | (3 << 12) | (1 << 16) | (1 << 20) | (1 << 24) | (1 << 28);
	PlayerSprite7[3]= (2 << 0) | (5 << 4) | (5 << 8) | (1 << 12) | (1 << 16) | (3 << 20) | (1 << 24) | (3 << 28);
	PlayerSprite7[4]= (0 << 0) | (2 << 4) | (5 << 8) | (5 << 12) | (5 << 16) | (5 << 20) | (5 << 24) | (5 << 28);
	PlayerSprite7[5]= (0 << 0) | (2 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (2 << 20) | (2 << 24) | (2 << 28);
	PlayerSprite7[6]= (0 << 0) | (2 << 4) | (4 << 8) | (2 << 12) | (0 << 16) | (2 << 20) | (4 << 24) | (2 << 28);
	PlayerSprite7[7]= (0 << 0) | (2 << 4) | (2 << 8) | (2 << 12) | (0 << 16) | (2 << 20) | (2 << 24) | (2 << 28);

	unsigned int *PlayerSprite8 = (unsigned int*) 0x06010100;
	PlayerSprite8[0]= (3 << 0) | (4 << 4) | (4 << 8) | (2 << 12) | (4 << 16) | (4 << 20) | (3 << 24) | (2 << 28);
	PlayerSprite8[1]= (4 << 0) | (4 << 4) | (2 << 8) | (4 << 12) | (4 << 16) | (4 << 20) | (4 << 24) | (2 << 28);
	PlayerSprite8[2]= (2 << 0) | (4 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (4 << 20) | (2 << 24) | (0 << 28);
	PlayerSprite8[3]= (1 << 0) | (2 << 4) | (4 << 8) | (4 << 12) | (4 << 16) | (2 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite8[4]= (5 << 0) | (5 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (2 << 20) | (0 << 24) | (0 << 28);
	PlayerSprite8[5]= (2 << 0) | (2 << 4) | (2 << 8) | (2 << 12) | (2 << 16) | (4 << 20) | (2 << 24) | (0 << 28);
	PlayerSprite8[6]= (0 << 0) | (2 << 4) | (4 << 8) | (2 << 12) | (0 << 16) | (2 << 20) | (4 << 24) | (2 << 28);
	PlayerSprite8[7]= (0 << 0) | (2 << 4) | (2 << 8) | (2 << 12) | (0 << 16) | (0 << 20) | (2 << 24) | (2 << 28);

	/*******************************************************************************************************************
	Enemy Sprite Data
	*******************************************************************************************************************/
	unsigned int *EnemySprite1 = (unsigned int*) 0x06010120;
	EnemySprite1[0]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite1[1]= ( 0 << 0) | ( 0 << 4) | ( 2 << 8) | ( 2 << 12) | ( 2 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite1[2]= ( 0 << 0) | ( 2 << 4) | ( 2 << 8) | ( 8 << 12) | ( 2 << 16) | ( 2 << 20) | ( 2 << 24) | ( 2 << 28);
	EnemySprite1[3]= ( 2 << 0) | ( 7 << 4) | ( 2 << 8) | ( 7 << 12) | ( 2 << 16) | ( 7 << 20) | ( 7 << 24) | ( 7 << 28);
	EnemySprite1[4]= ( 2 << 0) | ( 7 << 4) | ( 7 << 8) | ( 7 << 12) | ( 7 << 16) | (10 << 20) | ( 7 << 24) | (10 << 28);
	EnemySprite1[5]= ( 2 << 0) | ( 8 << 4) | (10 << 8) | ( 2 << 12) | ( 2 << 16) | (10 << 20) | ( 7 << 24) | (10 << 28);
	EnemySprite1[6]= ( 2 << 0) | ( 8 << 4) | (10 << 8) | ( 2 << 12) | ( 6 << 16) | ( 2 << 20) | ( 7 << 24) | ( 2 << 28);
	EnemySprite1[7]= ( 0 << 0) | ( 2 << 4) | ( 7 << 8) | (10 << 12) | ( 2 << 16) | (10 << 20) | ( 7 << 24) | (10 << 28);

	unsigned int *EnemySprite2 = (unsigned int*) 0x06010140;
	EnemySprite2[0]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite2[1]= ( 2 << 0) | ( 2 << 4) | ( 2 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite2[2]= ( 2 << 0) | ( 8 << 4) | ( 2 << 8) | ( 2 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite2[3]= ( 2 << 0) | ( 7 << 4) | ( 2 << 8) | ( 8 << 12) | ( 2 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite2[4]= ( 7 << 0) | ( 7 << 4) | ( 7 << 8) | ( 8 << 12) | ( 2 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite2[5]= ( 2 << 0) | ( 2 << 4) | (10 << 8) | ( 7 << 12) | ( 2 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite2[6]= ( 6 << 0) | ( 2 << 4) | (10 << 8) | ( 7 << 12) | ( 2 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite2[7]= ( 2 << 0) | (10 << 4) | ( 7 << 8) | ( 2 << 12) | ( 2 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);

	unsigned int *EnemySprite3 = (unsigned int*) 0x06010160;
	EnemySprite3[0]= ( 0 << 0) | ( 0 << 4) | ( 2 << 8) | ( 7 << 12) | ( 7 << 16) | ( 7 << 20) | ( 2 << 24) | ( 7 << 28);
	EnemySprite3[1]= ( 0 << 0) | ( 0 << 4) | ( 2 << 8) | ( 2 << 12) | ( 7 << 16) | ( 2 << 20) | ( 2 << 24) | ( 2 << 28);
	EnemySprite3[2]= ( 0 << 0) | ( 2 << 4) | ( 7 << 8) | ( 2 << 12) | ( 9 << 16) | ( 2 << 20) | ( 7 << 24) | ( 2 << 28);
	EnemySprite3[3]= ( 0 << 0) | ( 2 << 4) | ( 7 << 8) | ( 2 << 12) | ( 6 << 16) | ( 2 << 20) | ( 7 << 24) | ( 2 << 28);
	EnemySprite3[4]= ( 2 << 0) | (10 << 4) | ( 7 << 8) | (10 << 12) | ( 2 << 16) | (10 << 20) | (10 << 24) | (10 << 28);
	EnemySprite3[5]= ( 2 << 0) | (10 << 4) | (10 << 8) | ( 7 << 12) | (10 << 16) | ( 7 << 20) | ( 7 << 24) | (10 << 28);
	EnemySprite3[6]= ( 0 << 0) | ( 2 << 4) | (10 << 8) | (10 << 12) | ( 8 << 16) | ( 8 << 20) | ( 8 << 24) | ( 8 << 28);
	EnemySprite3[7]= ( 0 << 0) | ( 0 << 4) | ( 2 << 8) | ( 2 << 12) | ( 2 << 16) | ( 2 << 20) | ( 2 << 24) | ( 2 << 28);

	unsigned int *EnemySprite4 = (unsigned int*) 0x06010180;
	EnemySprite4[0]= ( 7 << 0) | ( 7 << 4) | ( 2 << 8) | ( 0 << 12) | ( 0 << 16) | ( 2 << 20) | ( 2 << 24) | ( 0 << 28);
	EnemySprite4[1]= ( 7 << 0) | ( 2 << 4) | ( 2 << 8) | ( 0 << 12) | ( 2 << 16) | ( 8 << 20) | (10 << 24) | ( 2 << 28);
	EnemySprite4[2]= ( 9 << 0) | ( 2 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 2 << 20) | ( 8 << 24) | ( 2 << 28);
	EnemySprite4[3]= ( 6 << 0) | ( 2 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 2 << 20) | ( 7 << 24) | ( 2 << 28);
	EnemySprite4[4]= ( 2 << 0) | ( 7 << 4) | ( 2 << 8) | ( 2 << 12) | ( 2 << 16) | ( 7 << 20) | ( 7 << 24) | ( 2 << 28);
	EnemySprite4[5]= ( 7 << 0) | ( 7 << 4) | ( 7 << 8) | (10 << 12) | (10 << 16) | ( 7 << 20) | ( 2 << 24) | ( 0 << 28);
	EnemySprite4[6]= ( 7 << 0) | (10 << 4) | (10 << 8) | (10 << 12) | ( 2 << 16) | ( 2 << 20) | ( 0 << 24) | ( 0 << 28);
	EnemySprite4[7]= ( 2 << 0) | ( 2 << 4) | ( 2 << 8) | ( 2 << 12) | ( 2 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);

	/***********************************************************************************************************************
	Capsule Sprite Data
	***********************************************************************************************************************/
	// Red Capsule sprite data
	unsigned int *RedCapsuleSprite = (unsigned int*) 0x060101a0;
	RedCapsuleSprite[0]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	RedCapsuleSprite[1]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	RedCapsuleSprite[2]= ( 0 << 0) | ( 1 << 4) | ( 3 << 8) | ( 3 << 12) | ( 3 << 16) | ( 3 << 20) | ( 1 << 24) | ( 0 << 28);
	RedCapsuleSprite[3]= ( 1 << 0) | ( 3 << 4) | ( 2 << 8) | ( 3 << 12) | ( 3 << 16) | ( 2 << 20) | ( 3 << 24) | ( 1 << 28);
	RedCapsuleSprite[4]= ( 1 << 0) | ( 3 << 4) | ( 2 << 8) | ( 3 << 12) | ( 3 << 16) | ( 2 << 20) | ( 3 << 24) | ( 1 << 28);
	RedCapsuleSprite[5]= ( 0 << 0) | ( 1 << 4) | ( 3 << 8) | ( 3 << 12) | ( 3 << 16) | ( 3 << 20) | ( 1 << 24) | ( 0 << 28);
	RedCapsuleSprite[6]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	RedCapsuleSprite[7]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);

	// Orange Capsule sprite data
	unsigned int *OrangeCapsuleSprite = (unsigned int*) 0x060101c0;
	OrangeCapsuleSprite[0]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	OrangeCapsuleSprite[1]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	OrangeCapsuleSprite[2]= ( 0 << 0) | ( 1 << 4) | ( 5 << 8) | ( 5 << 12) | ( 5 << 16) | ( 5 << 20) | ( 1 << 24) | ( 0 << 28);
	OrangeCapsuleSprite[3]= ( 1 << 0) | ( 5 << 4) | ( 4 << 8) | ( 5 << 12) | ( 5 << 16) | ( 4 << 20) | ( 5 << 24) | ( 1 << 28);
	OrangeCapsuleSprite[4]= ( 1 << 0) | ( 5 << 4) | ( 4 << 8) | ( 5 << 12) | ( 5 << 16) | ( 4 << 20) | ( 5 << 24) | ( 1 << 28);
	OrangeCapsuleSprite[5]= ( 0 << 0) | ( 1 << 4) | ( 5 << 8) | ( 5 << 12) | ( 5 << 16) | ( 5 << 20) | ( 1 << 24) | ( 0 << 28);
	OrangeCapsuleSprite[6]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	OrangeCapsuleSprite[7]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);

	// Yellow Capsule sprite data
	unsigned int *YellowCapsuleSprite = (unsigned int*) 0x060101e0;
	YellowCapsuleSprite[0]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	YellowCapsuleSprite[1]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	YellowCapsuleSprite[2]= ( 0 << 0) | ( 1 << 4) | ( 7 << 8) | ( 7 << 12) | ( 7 << 16) | ( 7 << 20) | ( 1 << 24) | ( 0 << 28);
	YellowCapsuleSprite[3]= ( 1 << 0) | ( 7 << 4) | ( 6 << 8) | ( 7 << 12) | ( 7 << 16) | ( 6 << 20) | ( 7 << 24) | ( 1 << 28);
	YellowCapsuleSprite[4]= ( 1 << 0) | ( 7 << 4) | ( 6 << 8) | ( 7 << 12) | ( 7 << 16) | ( 6 << 20) | ( 7 << 24) | ( 1 << 28);
	YellowCapsuleSprite[5]= ( 0 << 0) | ( 1 << 4) | ( 7 << 8) | ( 7 << 12) | ( 7 << 16) | ( 7 << 20) | ( 1 << 24) | ( 0 << 28);
	YellowCapsuleSprite[6]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	YellowCapsuleSprite[7]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);

	// Green Capsule sprite data
	unsigned int *GreenCapsuleSprite = (unsigned int*) 0x06010200;
	GreenCapsuleSprite[0]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	GreenCapsuleSprite[1]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	GreenCapsuleSprite[2]= ( 0 << 0) | ( 1 << 4) | ( 9 << 8) | ( 9 << 12) | ( 9 << 16) | ( 9 << 20) | ( 1 << 24) | ( 0 << 28);
	GreenCapsuleSprite[3]= ( 1 << 0) | ( 9 << 4) | ( 8 << 8) | ( 9 << 12) | ( 9 << 16) | ( 8 << 20) | ( 9 << 24) | ( 1 << 28);
	GreenCapsuleSprite[4]= ( 1 << 0) | ( 9 << 4) | ( 8 << 8) | ( 9 << 12) | ( 9 << 16) | ( 8 << 20) | ( 9 << 24) | ( 1 << 28);
	GreenCapsuleSprite[5]= ( 0 << 0) | ( 1 << 4) | ( 9 << 8) | ( 9 << 12) | ( 9 << 16) | ( 9 << 20) | ( 1 << 24) | ( 0 << 28);
	GreenCapsuleSprite[6]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	GreenCapsuleSprite[7]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);

	// Blue Capsule sprite data
	unsigned int *BlueCapsuleSprite = (unsigned int*) 0x06010220;
	BlueCapsuleSprite[0]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	BlueCapsuleSprite[1]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	BlueCapsuleSprite[2]= ( 0 << 0) | ( 1 << 4) | (11 << 8) | (11 << 12) | (11 << 16) | (11 << 20) | ( 1 << 24) | ( 0 << 28);
	BlueCapsuleSprite[3]= ( 1 << 0) | (11 << 4) | (10 << 8) | (11 << 12) | (11 << 16) | (10 << 20) | (11 << 24) | ( 1 << 28);
	BlueCapsuleSprite[4]= ( 1 << 0) | (11 << 4) | (10 << 8) | (11 << 12) | (11 << 16) | (10 << 20) | (11 << 24) | ( 1 << 28);
	BlueCapsuleSprite[5]= ( 0 << 0) | ( 1 << 4) | (11 << 8) | (11 << 12) | (11 << 16) | (11 << 20) | ( 1 << 24) | ( 0 << 28);
	BlueCapsuleSprite[6]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	BlueCapsuleSprite[7]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);

	// Purple Capsule sprite data
	unsigned int *PurpleCapsuleSprite = (unsigned int*) 0x06010240;
	PurpleCapsuleSprite[0]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);
	PurpleCapsuleSprite[1]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	PurpleCapsuleSprite[2]= ( 0 << 0) | ( 1 << 4) | (13 << 8) | (13 << 12) | (13 << 16) | (13 << 20) | ( 1 << 24) | ( 0 << 28);
	PurpleCapsuleSprite[3]= ( 1 << 0) | (13 << 4) | (12 << 8) | (13 << 12) | (13 << 16) | (12 << 20) | (13 << 24) | ( 1 << 28);
	PurpleCapsuleSprite[4]= ( 1 << 0) | (13 << 4) | (12 << 8) | (13 << 12) | (13 << 16) | (12 << 20) | (13 << 24) | ( 1 << 28);
	PurpleCapsuleSprite[5]= ( 0 << 0) | ( 1 << 4) | (13 << 8) | (13 << 12) | (13 << 16) | (13 << 20) | ( 1 << 24) | ( 0 << 28);
	PurpleCapsuleSprite[6]= ( 0 << 0) | ( 0 << 4) | ( 1 << 8) | ( 1 << 12) | ( 1 << 16) | ( 1 << 20) | ( 0 << 24) | ( 0 << 28);
	PurpleCapsuleSprite[7]= ( 0 << 0) | ( 0 << 4) | ( 0 << 8) | ( 0 << 12) | ( 0 << 16) | ( 0 << 20) | ( 0 << 24) | ( 0 << 28);

}

void FontSetup()
{
	// Custom Font!
	unsigned int *FontData = (unsigned int*) 0x6000000;
	int it;

	for ( it = 0; it < 2050; it++)
	{
		FontData[it] = fontData[it];
	}

}

void DoMenu()
{
	// N FOR NYAN
	BG0MapBase[(1*32)+10] = (46 << 0) | (15 << 12);		//N
	BG0MapBase[(1*32)+12] = (38 << 0) | (15 << 12);		//F
	BG0MapBase[(1*32)+13] = (47 << 0) | (15 << 12);		//O
	BG0MapBase[(1*32)+14] = (50 << 0) | (15 << 12);		//R
	BG0MapBase[(1*32)+16] = (46 << 0) | (15 << 12);		//N
	BG0MapBase[(1*32)+17] = (57 << 0) | (15 << 12);		//Y
	BG0MapBase[(1*32)+18] = (33 << 0) | (15 << 12);		//A
	BG0MapBase[(1*32)+19] = (46 << 0) | (15 << 12);		//N

	// Press Enter to "Start"
	BG0MapBase[(9*32)+9] = (80 << 0) | (15 << 12);		//p
	BG0MapBase[(9*32)+10] = (82 << 0) | (15 << 12);		//r
	BG0MapBase[(9*32)+11] = (69 << 0) | (15 << 12);		//e
	BG0MapBase[(9*32)+12] = (83 << 0) | (15 << 12);		//s
	BG0MapBase[(9*32)+13] = (83 << 0) | (15 << 12);		//s

	BG0MapBase[(9*32)+15] = (37 << 0) | (15 << 12);		//E
	BG0MapBase[(9*32)+16] = (46 << 0) | (15 << 12);		//N
	BG0MapBase[(9*32)+17] = (52 << 0) | (15 << 12);		//T
	BG0MapBase[(9*32)+18] = (37 << 0) | (15 << 12);		//E
	BG0MapBase[(9*32)+19] = (50 << 0) | (15 << 12);		//R

	BG0MapBase[(10*32)+13]= (84 << 0) | (15 << 12);		//t
	BG0MapBase[(10*32)+14]= (79 << 0) | (15 << 12);		//o

	BG0MapBase[(11*32)+11]= ( 2 << 0) | (15 << 12);		//"
	BG0MapBase[(11*32)+12]= (51 << 0) | (15 << 12);		//S
	BG0MapBase[(11*32)+13]= (52 << 0) | (15 << 12);		//T
	BG0MapBase[(11*32)+14]= (33 << 0) | (15 << 12);		//A
	BG0MapBase[(11*32)+15]= (50 << 0) | (15 << 12);		//R
	BG0MapBase[(11*32)+16]= (52 << 0) | (15 << 12);		//T
	BG0MapBase[(11*32)+17]= ( 2 << 0) | (15 << 12);		//"

}

void DoInstructions()
{
	// Collect all the capsules and dodge monsters to win
	BG0MapBase[(4*32)+1]= (35 << 0) | (15 << 12); //C
	BG0MapBase[(4*32)+2]= (79 << 0) | (15 << 12); //o
	BG0MapBase[(4*32)+3]= (76 << 0) | (15 << 12); //l
	BG0MapBase[(4*32)+4]= (76 << 0) | (15 << 12); //l
	BG0MapBase[(4*32)+5]= (69 << 0) | (15 << 12); //e
	BG0MapBase[(4*32)+6]= (67 << 0) | (15 << 12); //c
	BG0MapBase[(4*32)+7]= (84 << 0) | (15 << 12); //t

	BG0MapBase[(4*32)+9]=  (65 << 0) | (15 << 12); //a
	BG0MapBase[(4*32)+10]= (76 << 0) | (15 << 12); //l
	BG0MapBase[(4*32)+11]= (76 << 0) | (15 << 12); //l

	BG0MapBase[(4*32)+13]= (84 << 0) | (15 << 12); //t
	BG0MapBase[(4*32)+14]= (72 << 0) | (15 << 12); //h
	BG0MapBase[(4*32)+15]= (69 << 0) | (15 << 12); //e

	BG0MapBase[(5*32)+1]= (67 << 0) | (15 << 12); //c
	BG0MapBase[(5*32)+2]= (65 << 0) | (15 << 12); //a
	BG0MapBase[(5*32)+3]= (80 << 0) | (15 << 12); //p
	BG0MapBase[(5*32)+4]= (83 << 0) | (15 << 12); //s
	BG0MapBase[(5*32)+5]= (85 << 0) | (15 << 12); //u
	BG0MapBase[(5*32)+6]= (76 << 0) | (15 << 12); //l
	BG0MapBase[(5*32)+7]= (69 << 0) | (15 << 12); //e
	BG0MapBase[(5*32)+8]= (83 << 0) | (15 << 12); //s

	BG0MapBase[(5*32)+10]=  (65 << 0) | (15 << 12); //a
	BG0MapBase[(5*32)+11]=  (78 << 0) | (15 << 12); //n
	BG0MapBase[(5*32)+12]=  (68 << 0) | (15 << 12); //d

	BG0MapBase[(6*32)+1]= (68 << 0) | (15 << 12); //d
	BG0MapBase[(6*32)+2]= (79 << 0) | (15 << 12); //o
	BG0MapBase[(6*32)+3]= (68 << 0) | (15 << 12); //d
	BG0MapBase[(6*32)+4]= (71 << 0) | (15 << 12); //g
	BG0MapBase[(6*32)+5]= (69 << 0) | (15 << 12); //e

	BG0MapBase[(6*32)+7]=  (77 << 0) | (15 << 12); //m
	BG0MapBase[(6*32)+8]=  (79 << 0) | (15 << 12); //o
	BG0MapBase[(6*32)+9]=  (78 << 0) | (15 << 12); //n
	BG0MapBase[(6*32)+10]= (83 << 0) | (15 << 12); //s
	BG0MapBase[(6*32)+11]= (84 << 0) | (15 << 12); //t
	BG0MapBase[(6*32)+12]= (69 << 0) | (15 << 12); //e
	BG0MapBase[(6*32)+13]= (82 << 0) | (15 << 12); //r
	BG0MapBase[(6*32)+14]= (83 << 0) | (15 << 12); //s

	BG0MapBase[(7*32)+1]= (84 << 0) | (15 << 12); //t
	BG0MapBase[(7*32)+2]= (79 << 0) | (15 << 12); //o

	BG0MapBase[(7*32)+5]= (55 << 0) | (15 << 12); //W
	BG0MapBase[(7*32)+6]= (41 << 0) | (15 << 12); //I
	BG0MapBase[(7*32)+7]= (46 << 0) | (15 << 12); //N

}

void DoGameOver()
{
	// GAME OVER
	BG0MapBase[(9*32)+11]= (39 << 0) | (15 << 12); //G
	BG0MapBase[(9*32)+12]= (33 << 0) | (15 << 12); //A
	BG0MapBase[(9*32)+13]= (45 << 0) | (15 << 12); //M
	BG0MapBase[(9*32)+14]= (37 << 0) | (15 << 12); //E

	BG0MapBase[(9*32)+16]= (47 << 0) | (15 << 12); //O
	BG0MapBase[(9*32)+17]= (54 << 0) | (15 << 12); //V
	BG0MapBase[(9*32)+18]= (37 << 0) | (15 << 12); //E
	BG0MapBase[(9*32)+19]= (50 << 0) | (15 << 12); //R
}

void DoGameWin()
{
	// YOU WIN
	BG0MapBase[(9*32)+12]= (57 << 0) | (14 << 12); //Y
	BG0MapBase[(9*32)+13]= (47 << 0) | (13 << 12); //O
	BG0MapBase[(9*32)+14]= (53 << 0) | (12 << 12); //U
	
	BG0MapBase[(9*32)+16]= (55 << 0) | (11 << 12); //W
	BG0MapBase[(9*32)+17]= (41 << 0) | (10 << 12); //I
	BG0MapBase[(9*32)+18]= (46 << 0) | ( 9 << 12); //N
}


void DoBackground()
{
	int i;
	// background 1 with stars
	for ( i=0; i< 5; i++)
	{
		BG1MapBase[((rand()%20)*32)+(rand()%30)]= (99 << 0) | (15 << 12);
	}
	// background 2 with stars
	for ( i=0; i< 5; i++)
	{
		BG2MapBase[((rand()%20)*32)+(rand()%30)]= (100 << 0) | (15 << 12);
	}
}


void DrawOBJ(unsigned short *OBJ, int YCoord, int hide, int XCoord, int size, int tile, int palette)
{
	OBJ[0] = (YCoord & 255) | (hide << 9);		// Y coordinate | Show / Hide OBJ
	OBJ[1] = (XCoord & 255) | (size << 14);		// X coordinate | OBJ size
	OBJ[2] = (tile << 0)	| (palette << 12);	// tile number  | palette number
	OBJ[3] = 0;
}

int EnemyCollision(int x1, int y1, int x2, int y2)
{
	if (((x1 + 16) > x2) && (x1 < (16 + x2)))
	{
		if (((y1 + 16) > y2) && (y1 < (16 + y2)))
		{
			return 1;
		}
		else
		{
			return 0;
		}

	}
	else
	{
		return 0;
	}
}

int CapsuleCollision(int x1, int y1, int x2, int y2)
{
	if (((x1 + 16) > x2) && (x1 < (8 + x2)))
	{
		if (((y1 + 16) > y2) && (y1 < (8 + y2)))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}