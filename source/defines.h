#ifndef _DEFINES_H_
#define _DEFINES_H_

/***************************************************************************************************************
													DEFINES
***************************************************************************************************************/
#define DISPCNT (*(volatile unsigned short*) 0x04000000)    // Display Control Register
#define BG0CNT  (*(volatile unsigned short*) 0x04000008)	// Background0 Control
#define BG1CNT  (*(volatile unsigned short*) 0x0400000A)	// Background1 Control
#define BG2CNT  (*(volatile unsigned short*) 0x0400000C)	// Background2 Control
#define BG3CNT  (*(volatile unsigned short*) 0x0400000E)	// Background3 Control

#define OAM				(unsigned int*)		0x06010000		// Object Attribute Memory location
#define OBJ_HOLDER1		(unsigned short*)	0x07000000		// Object Holder for Player Memory location
#define OBJ_HOLDER2		(unsigned short*)	0x07000010		// Object Holder for Enemy 1 Memory location
#define OBJ_HOLDER3		(unsigned short*)	0x07000020		// Object Holder for Enemy 2 Memory location
#define OBJ_HOLDER4		(unsigned short*)	0x07000030		// Object Holder for Red Capsule memory location	
#define OBJ_HOLDER5		(unsigned short*)	0x07000040		// Object Holder for Orange Capsule memory location
#define OBJ_HOLDER6		(unsigned short*)	0x07000050		// Object Holder for Yellow Capsule memory location
#define OBJ_HOLDER7		(unsigned short*)	0x07000060		// Object Holder for Green Capsule memory location
#define OBJ_HOLDER8		(unsigned short*)	0x07000070		// Object Holder for Blue Capsule memory location
#define OBJ_HOLDER9		(unsigned short*)	0x07000080		// Object Holder for Purple Capsule memory location

#define KEY_A            1   //A button
#define KEY_B            2   //B button
#define KEY_SELECT       4   //Select button
#define KEY_START        8   //Start button
#define KEY_RIGHT        16  //Right button
#define KEY_LEFT         32  //Left button
#define KEY_UP           64  //Up button
#define KEY_DOWN         128 //Down button
#define KEY_RIGHTTRIGGER 256 //Right Trigger
#define KEY_LEFTTRIGGER  512 //Left Trigger

#define RGB5(r,g,b)	((r)|((g)<<5)|((b)<<10))   //macro for setting colour 

#endif //_DEFINES_H_