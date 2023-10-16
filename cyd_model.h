#ifndef __CYD_MODEL_H__
#define __CYD_MODEL_H__

// Number of Cyd components.
#define CYD_NUM_COMPONENTS 18

// Cyd display list indexes.
#define CYD_UPPER_RIGHT_LEG 0
#define CYD_HEAD 1
#define CYD_UPPER_RIGHT_ARM 2
#define CYD_UPPER_LEFT_ARM 3
#define CYD_LOWER_LEFT_ARM 4
#define CYD_LOWER_RIGHT_ARM 5
#define CYD_RIGHT_KNEE 6
#define CYD_UPPER_LEFT_LEG 7
#define CYD_LOWER_LEFT_LEG 8
#define CYD_LEFT_KNEE 9
#define CYD_LOWER_RIGHT_LEG 10
#define CYD_LEFT_EYE 11
#define CYD_RIGHT_EYE 12
#define CYD_RIGHT_HAND 13
#define CYD_LEFT_HAND 14
#define CYD_RIGHT_FOOT 15
#define CYD_LEFT_FOOT 16
#define CYD_TORSO 17

// Display lists.
extern int CydDisplays[CYD_NUM_COMPONENTS];

// Component bounds.
struct CydBound
{
	GLfloat min[3],max[3];
};

extern struct CydBound CydBounds[CYD_NUM_COMPONENTS];

// Build Cyd model.
void buildCydModel();

// Draw Cyd model component.
void drawCydComponent(int component);

#endif
