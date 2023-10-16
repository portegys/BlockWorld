/*
 * Cyd takes on the Block World using ODE physics.
 */

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>
#include "glut.h"
#include "cyd.hpp"
#include "frameRate.hpp"
#include <list>
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

// Cyd.
Cyd cyd;

// Cyd movement.
#define ANGULAR_DELTA_SCALE 1.0   // Scales body rotation rate.
#define LINEAR_DELTA_SCALE 0.01   // Scales body movement rate.
#define LINEAR_SPEED_SCALE 1.0    // Scales how hard Cyd collides with objects.
GLfloat MovementRate = 3.0;

// Cyd animations.
#define CYD_LEGS_WALKING 0
#define CYD_ARMS_WALKING 1
#define CYD_ARMS_PICKUP 2
#define CYD_ARMS_DOWN 3

// Object dimension range.
#define MAX_DIMENSION 1.5
#define DIMENSION_QUANTUM 0.01

// light vector. LIGHTZ is implicitly 1
#define LIGHTX (1.0f)
#define LIGHTY (0.4f)

// select correct drawing functions

#ifdef dDOUBLE
#define dsDrawBox dsDrawBoxD
#define dsDrawSphere dsDrawSphereD
#define dsDrawCylinder dsDrawCylinderD
#define dsDrawCappedCylinder dsDrawCappedCylinderD
#endif


// some constants

#define NUM 500			// max number of objects
#define DENSITY (5.0)	// density of all objects
#define GPB 10			// maximum number of geometries per body
#define MAX_CONTACTS 4	// maximum number of contact points per body

// dynamics and collision objects
#define GEOM_BOX 0
#define GEOM_SPHERE 1
#define GEOM_CYLINDER 2
struct MyObject {
  dBodyID body;			// the body
  dGeomID geom[GPB];	// geometries representing this body
  int geomType[GPB];	// type of geometry, box, sphere, or cylinder
  dReal dimensions[GPB][3];
  GLfloat color[3];
  bool selected;
};

static int num=0;		// number of objects in simulation
static int nextobj=0;	// next object to recycle if num==NUM
static dWorldID world;
static dSpaceID space;
static MyObject obj[NUM];
static GLfloat objColors[NUM][3];
static dJointGroupID contactgroup;
static int show_aabb = 0;	// show geom AABBs?
static int show_contacts = 0;	// show contact points?
static int random_pos = 1;	// drop objects from random position?
static int write_world = 0;

// Cyd/object interaction variables.
enum { COLLISION, SELECTION } InteractionMode = COLLISION;
dGeomID HeldObject = 0;

// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
  int i,j;

  // get the categories for the geoms.
  unsigned long c1 = dGeomGetCategoryBits(o1);
  unsigned long c2 = dGeomGetCategoryBits(o2);

  // body belongs to Cyd?
  bool cyd1 = false;
  bool cyd2 = false;
  if (c1 >= CYD_UPPER_RIGHT_LEG && c1 <= CYD_TORSO) cyd1 = true;
  if (c2 >= CYD_UPPER_RIGHT_LEG && c2 <= CYD_TORSO) cyd2 = true;

  // Cyd bounding blocks do not collide.
  if (cyd1 && cyd2) return;

  // exit without doing anything if the two bodies are connected by a joint
  dBodyID b1 = dGeomGetBody(o1);
  dBodyID b2 = dGeomGetBody(o2);
  if (b1 && b2 && dAreConnectedExcluding (b1,b2,dJointTypeContact)) return;

  dContact contact[MAX_CONTACTS];   // up to MAX_CONTACTS contacts per box-box
  for (i=0; i<MAX_CONTACTS; i++) {
    contact[i].surface.mode = dContactBounce | dContactSoftCFM;
    contact[i].surface.mu = dInfinity;
    contact[i].surface.mu2 = 0;
    contact[i].surface.bounce = 0.1;
    contact[i].surface.bounce_vel = 0.1;
    contact[i].surface.soft_cfm = 0.01;
  }
  if (int numc = dCollide (o1,o2,MAX_CONTACTS,&contact[0].geom,
			   sizeof(dContact))) {
    dMatrix3 RI;
    dRSetIdentity (RI);
    const dReal ss[3] = {0.02,0.02,0.02};
    for (i=0; i<numc; i++) {

	  // Selection mode.
	  if (InteractionMode == SELECTION && (cyd1 || cyd2))
	  {
		  if (HeldObject != 0) continue;
		  if (b2 != 0 && (c1 == CYD_LEFT_HAND || c1 == CYD_RIGHT_HAND))
		  {
			for (j = 0; j < num; j++)
			{
				if (obj[j].body == b2) obj[j].selected = true;
			}
		  }
		  if (b1 != 0 && (c2 == CYD_LEFT_HAND || c2 == CYD_RIGHT_HAND))
		  {
			for (j = 0; j < num; j++)
			{
				if (obj[j].body == b1) obj[j].selected = true;
			}
		  }
		  continue;
	  }
	  if (o1 != HeldObject && o2 != HeldObject)
	  {
		dJointID c = dJointCreateContact (world,contactgroup,contact+i);
		dJointAttach (c,b1,b2);
		if (show_contacts) dsDrawBox (contact[i].geom.pos,RI,ss);
	  }
    }
  }
}

// Show help.
void showHelp()
{
  printf ("To show this menu, press ?.\n");
  printf ("To rotate camera, drag left mouse button.\n");
  printf ("To move camera, drag right mouse button.\n");
  printf ("Cyd commands:\n");
  printf ("To move forward, press j.\n");
  printf ("To move backward, press k.\n");
  printf ("To turn left, press h.\n");
  printf ("To turn right, press l.\n");
  printf ("To move up, press u.\n");
  printf ("To move down, press m.\n");
  printf ("To move faster, press w.\n");
  printf ("To move slower, press q.\n");
  printf ("To change object interaction mode, press space.\n");
  printf ("  Modes:\n");
  printf ("   collision.\n");
  printf ("   selection.\n");
  printf ("To pickup an object, press [.\n");
  printf ("To drop an object, press ].\n");
  printf ("To fuse the selected objects, press f.\n");
  printf ("To destroy the selected objects, press z.\n");
  printf ("To select next body part, press n.\n");
  printf ("To rotate body part, press h, l, and arrow keys.\n");
  printf ("To show bounding boxes, press v.\n");
  printf ("Block world commands:\n");
  printf ("To drop another object, press:\n");
  printf ("   b for box.\n");
  printf ("   s for sphere.\n");
  printf ("   c for cylinder.\n");
  printf ("   x for a composite object.\n");
  printf ("To disable the selected objects, press d.\n");
  printf ("To enable the selected objects, press e.\n");
  printf ("To toggle showing the geom AABBs, press a.\n");
  printf ("To toggle showing the contact points, press t.\n");
  printf ("To toggle dropping from random position/orientation, press r.\n");
  printf ("To save the current state to 'state.dif', press 1.\n");
}

// start simulation - set viewpoint

static void start()
{
  static float xyz[3] = {2.1640f,-1.3079f,1.7600f};
  static float hpr[3] = {125.5000f,-17.0000f,0.0000f};
  dsSetViewpoint (xyz,hpr);
  showHelp();
  for (int i = 0; i < NUM; i++)
  {
	  for (int j = 0; j < 3; j++)
	  {
		objColors[i][j] = dRandReal();
	  }
  }
}


char locase (char c)
{
  if (c >= 'A' && c <= 'Z') return c - ('a'-'A');
  else return c;
}

// pick up object.
void pickupObject()
{
	int i,j,k;
	dGeomID bestGeom = 0;
	dGeomID g;
	int bestIndex;
	GLfloat rightHandPosition[3],leftHandPosition[3],tmpPos[3];
	Vector targetPosition,bestPosition,testPos;
	GLfloat bestDist,testDist;
	const dReal *p;
	dReal pos[3];
	bool multObj;

	if (HeldObject != 0) return;
	if (InteractionMode != SELECTION) return;

	// Get target (optimal) object position.
	for (i = 0; i < 3; i++)
	{
		tmpPos[i] = 
			((CydBounds[CYD_RIGHT_HAND].max[i] + CydBounds[CYD_RIGHT_HAND].min[i]) / 2.0) +
			((CydBounds[CYD_LEFT_HAND].max[i] + CydBounds[CYD_LEFT_HAND].min[i]) / 2.0);
		tmpPos[i] /= 2.0;
	}
	i = cyd.getPartFromComponent(CYD_RIGHT_HAND);
	cyd.transform.spacial->transformPoint(tmpPos, cyd.bodyParts[i].xmatrix);
	targetPosition.x = tmpPos[0];
	targetPosition.y = tmpPos[1];
	targetPosition.z = tmpPos[2];

	// Find closest object.
	for (i = 0; i < num; i++)
	{
		if (obj[i].selected)
		{
			multObj = false;
			for (j = 0; j < GPB && obj[i].geom[j] != 0; j++) {}
			if (j > 1) multObj = true;
			for (j = 0; j < GPB && obj[i].geom[j] != 0; j++)
			{
				p = dGeomGetPosition(obj[i].geom[j]);
				for (k = 0; k < 3; k++) pos[k] = p[k];
				if (multObj) {
					g = dGeomTransformGetGeom(obj[i].geom[j]);
					p = dGeomGetPosition(g);
					for (k = 0; k < 3; k++) pos[k] += p[k];
				}
				g = obj[i].geom[j];
				if (bestGeom == 0)
				{
					bestGeom = g;
					bestIndex = i;
					bestPosition.x = pos[0];
					bestPosition.y = pos[1];
					bestPosition.z = pos[2];
					bestDist = targetPosition.Distance(bestPosition);
				} else {
					testPos.x = pos[0];
					testPos.y = pos[1];
					testPos.z = pos[2];
					testDist = targetPosition.Distance(testPos);
					if (testDist < bestDist)
					{
						bestGeom = g;
						bestIndex = i;
						bestPosition = testPos;
						bestDist = testDist;
					}
				}
			}
		}
	}
	if (bestGeom != 0)
	{
		HeldObject = bestGeom;
		dBodyDisable(obj[bestIndex].body);
		cyd.bodyParts[Cyd::TORSO].transform.setPitch(0.0);
	}
}

// Carry object.
void carryObject(dGeomID heldObject)
{
	int i,j,part,index;
	dMatrix3 rotation;
	GLfloat angle,ax,ay,az;
	GLfloat matrix[4][4],quat[4];
	double cosa,sina;
	GLfloat objectPosition[3];
	dBodyID heldBody;

	// Validate object.
	if (HeldObject == 0) return;
	heldBody = dGeomGetBody(HeldObject);
	for (index = 0; index < num; index++)
	{
		if (obj[index].body == heldBody) break;
	}
	if (index == num)
	{
		HeldObject = 0;
		return;
	}

	// Orient object according to right hand.
	part = cyd.getPartFromComponent(CYD_RIGHT_HAND);
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			matrix[i][j] = cyd.bodyParts[part].xmatrix[(i*4)+j];
		}
	}
	cyd.bodyParts[part].transform.spacial->qcalc->build_quat(quat,matrix);
	cosa  = quat[3];
	angle = acos(cosa) * 2.0;
	sina = sqrt(1.0 - (cosa * cosa));
	if (fabs(sina) < 0.0005) sina = 1.0;
	ax = quat[0] / sina;
	ay = quat[1] / sina;
	az = quat[2] / sina;
	dRFromAxisAndAngle(rotation, ax, ay, az, -angle);
	dBodySetRotation(heldBody, rotation);

	// Position object between hands.
	for (i = 0; i < 3; i++)
	{
		objectPosition[i] = 
			((CydBounds[CYD_RIGHT_HAND].max[i] + CydBounds[CYD_RIGHT_HAND].min[i]) / 2.0) +
			((CydBounds[CYD_LEFT_HAND].max[i] + CydBounds[CYD_LEFT_HAND].min[i]) / 2.0);
		objectPosition[i] /= 2.0;
	}
	part = cyd.getPartFromComponent(CYD_RIGHT_HAND);
	cyd.transform.spacial->transformPoint(objectPosition, cyd.bodyParts[part].xmatrix);
	dBodySetPosition(heldBody, objectPosition[0], objectPosition[1], objectPosition[2]);
}

// drop held object.
void dropObject()
{
	if (HeldObject != 0)
	{
		dBodyEnable(dGeomGetBody(HeldObject));
		cyd.bodyParts[Cyd::TORSO].transform.setPitch(45.0);
	}
	HeldObject = 0;
}

// Current body part.
int CurrentPart = CYD_NUM_BODY_PARTS;

// Keyboard input.
void
keyInput(unsigned char key, int x, int y)
{
  int i;

  switch(key)
    {
	case ' ':
		if (HeldObject != 0) break;
		if (InteractionMode == COLLISION)
		{
			InteractionMode = SELECTION;
			cyd.showHands = true;
			cyd.animations[CYD_ARMS_WALKING]->unloop();
			cyd.animations[CYD_ARMS_WALKING]->stop();
			cyd.animations[CYD_ARMS_DOWN]->unloop();
			cyd.animations[CYD_ARMS_DOWN]->stop();
			cyd.animations[CYD_ARMS_PICKUP]->start();
		} else {
			InteractionMode = COLLISION;
			for (i = 0; i < num; i++)
			{
				obj[i].selected = false;
			}
			cyd.showHands = false;
			cyd.animations[CYD_ARMS_WALKING]->unloop();
			cyd.animations[CYD_ARMS_WALKING]->stop();
			cyd.animations[CYD_ARMS_PICKUP]->unloop();
			cyd.animations[CYD_ARMS_PICKUP]->stop();
			cyd.animations[CYD_ARMS_DOWN]->start();
		}
		break;
    case '[':
		pickupObject();
		break;
	case ']':
		dropObject();
		break;
    case 'n':
		CurrentPart = (CurrentPart + 1) % (CYD_NUM_BODY_PARTS + 1);
		switch(CurrentPart)
		{
		case Cyd::TORSO: printf("Part: Torso"); break;
		case Cyd::HEAD:	printf("Part: Head"); break;
		case Cyd::UPPER_RIGHT_ARM:	printf("Part: Upper left arm"); break;
		case Cyd::LOWER_RIGHT_ARM:	printf("Part: Lower left arm"); break;
		case Cyd::UPPER_RIGHT_LEG:	printf("Part: Upper left leg"); break;
		case Cyd::LOWER_RIGHT_LEG:	printf("Part: Lower left leg"); break;
		case Cyd::UPPER_LEFT_ARM:	printf("Part: Upper right arm"); break;
		case Cyd::LOWER_LEFT_ARM:	printf("Part: Lower right arm"); break;
		case Cyd::UPPER_LEFT_LEG:	printf("Part: Upper right leg"); break;
		case Cyd::LOWER_LEFT_LEG:	printf("Part: Lower right leg"); break;
		default: printf("Part: Cyd");
		}
		printf("\n");
		break;
	case 'q':
		MovementRate -= (MovementRate * 0.1);
		if (MovementRate < 0.01) MovementRate = 0.0;
		printf("Movement rate=%f\n", MovementRate);
		break;
	case 'w':
		MovementRate += (MovementRate * 0.1);
		if (MovementRate <= 0.0) MovementRate = 0.01;
		printf("Movement rate=%f\n", MovementRate);
		break;
	case 'v': cyd.showBoxes = !cyd.showBoxes; break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		int i = key - '0';
		if (cyd.animations.size() > i)
		{
			if (!cyd.animations[i]->active)
			{
				cyd.animations[i]->loop();
				cyd.animations[i]->start();
			} else {
				cyd.animations[i]->unloop();
			}
		}
		break;
    }
}

// Special keyboard input.
void
specialKeyInput(int key, int x, int y)
{
  GLfloat direction[3];

  switch(CurrentPart)
    {
  case Cyd::TORSO:
		switch(key)
		{
		case GLUT_KEY_LEFT: cyd.bodyParts[CurrentPart].transform.addRoll(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_RIGHT: cyd.bodyParts[CurrentPart].transform.addRoll(ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_UP: cyd.bodyParts[CurrentPart].transform.addPitch(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_DOWN: cyd.bodyParts[CurrentPart].transform.addPitch(ANGULAR_DELTA_SCALE * MovementRate); break;
		case 'h': cyd.bodyParts[CurrentPart].transform.addYaw(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case 'l': cyd.bodyParts[CurrentPart].transform.addYaw(ANGULAR_DELTA_SCALE * MovementRate); break;
		}
      break;
	case Cyd::HEAD:
      switch(key)
		{
		case GLUT_KEY_LEFT: cyd.bodyParts[CurrentPart].transform.addYaw(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_RIGHT: cyd.bodyParts[CurrentPart].transform.addYaw(ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_UP: cyd.bodyParts[CurrentPart].transform.rx -= ANGULAR_DELTA_SCALE * MovementRate; break;
		case GLUT_KEY_DOWN: cyd.bodyParts[CurrentPart].transform.rx += ANGULAR_DELTA_SCALE * MovementRate; break;
		}
      break;
    case Cyd::UPPER_RIGHT_ARM:
    case Cyd::UPPER_RIGHT_LEG:
    case Cyd::UPPER_LEFT_ARM:
    case Cyd::UPPER_LEFT_LEG:
      switch(key)
		{
		case GLUT_KEY_LEFT: cyd.bodyParts[CurrentPart].transform.addRoll(ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_RIGHT: cyd.bodyParts[CurrentPart].transform.addRoll(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_UP: cyd.bodyParts[CurrentPart].transform.addPitch(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_DOWN: cyd.bodyParts[CurrentPart].transform.addPitch(ANGULAR_DELTA_SCALE * MovementRate); break;
		case 'h': cyd.bodyParts[CurrentPart].transform.addYaw(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case 'l': cyd.bodyParts[CurrentPart].transform.addYaw(ANGULAR_DELTA_SCALE * MovementRate); break;
		}
      break;
    case Cyd::LOWER_RIGHT_ARM:
    case Cyd::LOWER_LEFT_ARM:
      switch(key)
		{
		case GLUT_KEY_LEFT: cyd.bodyParts[CurrentPart].transform.addRoll(ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_RIGHT: cyd.bodyParts[CurrentPart].transform.addRoll(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_UP: cyd.bodyParts[CurrentPart].transform.addPitch(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_DOWN: cyd.bodyParts[CurrentPart].transform.addPitch(ANGULAR_DELTA_SCALE * MovementRate); break;
		case 'h': cyd.bodyParts[CurrentPart].transform.addYaw(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case 'l': cyd.bodyParts[CurrentPart].transform.addYaw(ANGULAR_DELTA_SCALE * MovementRate); break;
		}
      break;
    case Cyd::LOWER_RIGHT_LEG:
    case Cyd::LOWER_LEFT_LEG:
      switch(key)
		{
		case GLUT_KEY_UP: cyd.bodyParts[CurrentPart].transform.addPitch(ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_DOWN: cyd.bodyParts[CurrentPart].transform.addPitch(-ANGULAR_DELTA_SCALE * MovementRate); break;
	  }
      break;
	default:
      switch(key)
		{
		case GLUT_KEY_LEFT: cyd.transform.addRoll(ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_RIGHT: cyd.transform.addRoll(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_UP: cyd.transform.addPitch(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case GLUT_KEY_DOWN: cyd.transform.addPitch(ANGULAR_DELTA_SCALE * MovementRate); break;
		case 'h': cyd.transform.addYaw(-ANGULAR_DELTA_SCALE * MovementRate); break;
		case 'j':
			cyd.transform.spacial->getUp(direction);
			cyd.transform.tx -= direction[0] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.transform.ty -= direction[1] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.transform.tz -= direction[2] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.speed = MovementRate * LINEAR_SPEED_SCALE;
			if (!cyd.animations[CYD_LEGS_WALKING]->active)
			{
				cyd.animations[CYD_LEGS_WALKING]->loop();
				cyd.animations[CYD_LEGS_WALKING]->start();
			}
			if (InteractionMode == COLLISION && 
				!cyd.animations[CYD_ARMS_DOWN]->active &&
				!cyd.animations[CYD_ARMS_WALKING]->active)
			{
				cyd.animations[CYD_ARMS_WALKING]->loop();
				cyd.animations[CYD_ARMS_WALKING]->start();
			}
			break;
		case 'k':
			cyd.transform.spacial->getUp(direction);
			cyd.transform.tx += direction[0] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.transform.ty += direction[1] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.transform.tz += direction[2] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.speed = -MovementRate * LINEAR_SPEED_SCALE;
			if (!cyd.animations[CYD_LEGS_WALKING]->active)
			{
				cyd.animations[CYD_LEGS_WALKING]->loop();
				cyd.animations[CYD_LEGS_WALKING]->start();
			}
			if (InteractionMode == COLLISION && 
				!cyd.animations[CYD_ARMS_DOWN]->active &&
				!cyd.animations[CYD_ARMS_WALKING]->active)
			{
				cyd.animations[CYD_ARMS_WALKING]->loop();
				cyd.animations[CYD_ARMS_WALKING]->start();
			}
			break;
		case 'l': cyd.transform.addYaw(ANGULAR_DELTA_SCALE * MovementRate); break;
		case 'u':
			cyd.transform.spacial->getForward(direction);
			cyd.transform.tx += direction[0] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.transform.ty += direction[1] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.transform.tz += direction[2] * LINEAR_DELTA_SCALE * MovementRate;
			break;
		case 'm':
			cyd.transform.spacial->getForward(direction);
			cyd.transform.tx -= direction[0] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.transform.ty -= direction[1] * LINEAR_DELTA_SCALE * MovementRate;
			cyd.transform.tz -= direction[2] * LINEAR_DELTA_SCALE * MovementRate;
			break;
		}
		break;
    }
}

// destroy all selected objects.

static void destroySelected()
{
	int i,j,k,p;
 
	bool done = false;
	while (!done) {
	  done = true;
	  for (i = 0; i < num; i++) {
		if (!obj[i].selected) continue;
		done = false;
		dBodyDestroy (obj[i].body);
		for (k=0; k < GPB; k++) {
		  if (obj[i].geom[k]) dGeomDestroy (obj[i].geom[k]);
		}
		memset (&obj[i],0,sizeof(obj[i]));
		obj[i].selected = false;
		for (i = i+1; i < num; i++)
		{
			obj[i-1].body = obj[i].body;
			for (k=0; k < GPB; k++) {
				if (obj[i].geom[k]) {
					obj[i-1].geom[k] = obj[i].geom[k];
					obj[i-1].geomType[k] = obj[i].geomType[k];
					obj[i-1].dimensions[k][0] = obj[i].dimensions[k][0];
					obj[i-1].dimensions[k][1] = obj[i].dimensions[k][1];
					obj[i-1].dimensions[k][2] = obj[i].dimensions[k][2];
				}
			}
			obj[i-1].selected = obj[i].selected;
			memset (&obj[i],0,sizeof(obj[i]));
			obj[i].selected = false;
		}
		num--;
		break;
	  }
	}
}

// called when a key pressed

static void command (int cmd)
{
  size_t i;
  int j,k,p;
  dReal sides[3];
  dMass m;

  if (cmd == '?') { showHelp(); return; }

  cmd = locase (cmd);

  if (cmd == 'n' || cmd == 'q' || cmd == 'w' ||
	  cmd == 'v' || cmd == ' ' || cmd == '[' || cmd == ']')
  {
	keyInput((unsigned char)cmd, 0, 0);
	return;
  }

  if (cmd == 'b' || cmd == 's' || cmd == 'c' || cmd == 'x'
      /* || cmd == 'l' */) {
    if (num < NUM) {
      i = num;
      num++;
    }
    else {
      i = nextobj;
      nextobj++;
      if (nextobj >= num) nextobj = 0;

      // destroy the body and geoms for slot i
      dBodyDestroy (obj[i].body);
      for (k=0; k < GPB; k++) {
	if (obj[i].geom[k]) dGeomDestroy (obj[i].geom[k]);
      }
      memset (&obj[i],0,sizeof(obj[i]));
    }

    obj[i].body = dBodyCreate (world);
	dBodySetAutoDisableFlag (obj[i].body, 0);
	obj[i].selected = false;
    for (k=0; k<3; k++)
	{
		sides[k] = dRandReal()*MAX_DIMENSION+DIMENSION_QUANTUM;
		sides[k] =
			(float)((int)(sides[k] / (float)DIMENSION_QUANTUM)) *
			(float)DIMENSION_QUANTUM;
	}

    dMatrix3 R;
    if (random_pos) {
      dBodySetPosition (obj[i].body,
			dRandReal()*5-1,dRandReal()*5-1,dRandReal()+2);
      dRFromAxisAndAngle (R,dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
			  dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
    }
    else {
      dReal maxheight = 0;
      for (k=0; k<num; k++) {
	const dReal *pos = dBodyGetPosition (obj[k].body);
	if (pos[2] > maxheight) maxheight = pos[2];
      }
      dBodySetPosition (obj[i].body, 0,0,maxheight+1);
      dRFromAxisAndAngle (R,0,0,1,dRandReal()*10.0-5.0);
    }
    dBodySetRotation (obj[i].body,R);
    dBodySetData (obj[i].body,(void*) i);

    if (cmd == 'b') {
      dMassSetBox (&m,DENSITY,sides[0],sides[1],sides[2]);
      obj[i].geom[0] = dCreateBox (space,sides[0],sides[1],sides[2]);
	  obj[i].geomType[0] = GEOM_BOX;
      for (k=0; k<3; k++) obj[i].dimensions[0][k] = sides[k];
    }
    else if (cmd == 'c') {
      sides[0] *= 0.5;
      dMassSetCappedCylinder (&m,DENSITY,3,sides[0],sides[1]);
      obj[i].geom[0] = dCreateCCylinder (space,sides[0],sides[1]);
	  obj[i].geomType[0] = GEOM_CYLINDER;
      for (k=0; k<3; k++) obj[i].dimensions[0][k] = sides[k];
    }
/*
    // cylinder option not yet implemented
    else if (cmd == 'l') {
      sides[1] *= 0.5;
      dMassSetCappedCylinder (&m,DENSITY,3,sides[0],sides[1]);
      obj[i].geom[0] = dCreateCylinder (space,sides[0],sides[1]);
    }
*/
    else if (cmd == 's') {
      sides[0] *= 0.5;
      dMassSetSphere (&m,DENSITY,sides[0]);
      obj[i].geom[0] = dCreateSphere (space,sides[0]);
	  obj[i].geomType[0] = GEOM_SPHERE;
      for (k=0; k<3; k++) obj[i].dimensions[0][k] = sides[k];
    }
    else if (cmd == 'x') {
	  dMass m2;
      dGeomID g2[GPB];		// encapsulated geometries
      dReal dpos[GPB][3];	// delta-positions for encapsulated geometries
	  int type[GPB];
	  dReal dims[GPB][3];

      // start accumulating masses for the encapsulated geometries
      dMassSetZero (&m);

      // set random delta positions
      for (j=0; j<GPB; j++) {
		for (k=0; k<3; k++) dpos[j][k] = dRandReal()*0.3-0.15;
      }

      for (k=0; k<GPB; k++) {
		obj[i].geom[k] = dCreateGeomTransform (space);
		dGeomTransformSetCleanup (obj[i].geom[k],1);
		if ((k%3)==0) {
		  dReal radius = dRandReal()*0.25+0.05;
		  g2[k] = dCreateSphere (0,radius);
		  dMassSetSphere (&m2,DENSITY,radius);
		  type[k] = GEOM_SPHERE;
		  dims[k][0] = radius;
		}
		else if ((k%3)==1) {
		  g2[k] = dCreateBox (0,sides[0],sides[1],sides[2]);
		  dMassSetBox (&m2,DENSITY,sides[0],sides[1],sides[2]);
		  type[k] = GEOM_BOX;
		  for (p=0; p<3; p++) dims[k][p] = sides[p];
		}
		else {
		  dReal radius = dRandReal()*0.1+0.05;
		  dReal length = dRandReal()*1.0+0.1;
		  g2[k] = dCreateCCylinder (0,radius,length);
		  dMassSetCappedCylinder (&m2,DENSITY,3,radius,length);
		  type[k] = GEOM_CYLINDER;
		  dims[k][0] = radius;
		  dims[k][1] = length;
		}
		dGeomTransformSetGeom (obj[i].geom[k],g2[k]);
		obj[i].geomType[k] = type[k];
		for (p=0; p<3; p++) obj[i].dimensions[k][p] = dims[k][p];

		// set the transformation (adjust the mass too)
		dGeomSetPosition (g2[k],dpos[k][0],dpos[k][1],dpos[k][2]);
		dMassTranslate (&m2,dpos[k][0],dpos[k][1],dpos[k][2]);
		dMatrix3 Rtx;
		dRFromAxisAndAngle (Rtx,dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
					dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
		dGeomSetRotation (g2[k],Rtx);
		dMassRotate (&m2,Rtx);

		// add to the total mass
		dMassAdd (&m,&m2);
      }

      // move all encapsulated objects so that the center of mass is (0,0,0)
      for (k=0; k<GPB; k++) {
		dGeomSetPosition (g2[k],
				  dpos[k][0]-m.c[0],
				  dpos[k][1]-m.c[1],
				  dpos[k][2]-m.c[2]);
      }
      dMassTranslate (&m,-m.c[0],-m.c[1],-m.c[2]);
    }

    for (k=0; k < GPB; k++) {
      if (obj[i].geom[k]) dGeomSetBody (obj[i].geom[k],obj[i].body);
    }

    dBodySetMass (obj[i].body,&m);
  }

  if (cmd == 'd') {
	  if (HeldObject == 0) {
		  for (k = 0; k < num; k++) {
			if (obj[k].selected) dBodyDisable (obj[k].body);
		  }
	  }
  }
  else if (cmd == 'e') {
	  if (HeldObject == 0) {
		  for (k = 0; k < num; k++) {
			if (obj[k].selected) dBodyEnable (obj[k].body);
		  }
	  }
  }
  else if (cmd == 'a') {
    show_aabb ^= 1;
  }
  else if (cmd == 't') {
    show_contacts ^= 1;
  }
  else if (cmd == 'r') {
    random_pos ^= 1;
  }
  else if (cmd == '1') {
    write_world = 1;
  }
  else if (cmd == 'f') {
    // fuse the selected bodies and geoms.
	int numg;
	dMass m2[GPB];
    dGeomID g2[GPB];	// encapsulated geometries
    dReal dpos[GPB][3];	// delta-positions for encapsulated geometries
	dQuaternion quats[GPB];
	int type[GPB];
	dReal dims[GPB][3];
	bool multObj;
	dGeomID g;
	const dReal *pos;
	dReal ctr[3];

	// drop held object.
	dropObject();

	// can fuse be done?
	for (i = j = p = 0; i < num; i++) {
	  if (obj[i].selected) {
		p++;
		for (k=0; k < GPB; k++, j++) {
		  if (obj[i].geom[k] == 0) break;
		}
	  }
	}
	if (p < 2 || j == 0) return;
	if (j > GPB) {
	  printf("Cannot fuse: maximum geoms (%d) exceeded\n", GPB);
	  return;
	}

    // copy the geometry information.
	for (i = numg = 0; i < num; i++) {
	  if (obj[i].selected) {
	    multObj = false;
		for (k=0; k < GPB; k++, j++) {
		  if (obj[i].geom[k] == 0) break;
		}
		if (k > 1) multObj = true;
		for (k=0; k < GPB; k++, j++) {
		  if (obj[i].geom[k] == 0) break;
		  type[numg] = obj[i].geomType[k];
		  for (p=0; p<3; p++) dims[numg][p] = obj[i].dimensions[k][p];
		  if (multObj) {
			  g = dGeomTransformGetGeom(obj[i].geom[k]);
			  pos = dGeomGetPosition(obj[i].geom[k]);
			  for (p=0; p<3; p++) dpos[numg][p] = pos[p];
		      pos = dGeomGetPosition(g);
			  for (p=0; p<3; p++) dpos[numg][p] += pos[p];
		  } else {
			  g = obj[i].geom[k];
			  pos = dGeomGetPosition(g);
			  for (p=0; p<3; p++) dpos[numg][p] = pos[p];
		  }
		  dGeomGetQuaternion(obj[i].geom[k], quats[numg]);
		  numg++;
		}
	  }
	}

	// destroy the selected objects.
	destroySelected();

	// create new body.
	i = num;
	num++;
    memset (&obj[i],0,sizeof(obj[i]));
    obj[i].body = dBodyCreate (world);
    dBodySetData (obj[i].body,(void*) i);
	dBodySetAutoDisableFlag (obj[i].body, 0);
	obj[i].selected = false;

    // start accumulating masses for the encapsulated geometries
    dMassSetZero (&m);

    for (k=0; k<numg; k++) {
		obj[i].geom[k] = dCreateGeomTransform (space);
		dGeomTransformSetCleanup (obj[i].geom[k],1);
		switch(type[k]) {
		case GEOM_SPHERE:
		  g2[k] = dCreateSphere (0,dims[k][0]);
		  dMassSetSphere (&m2[k],DENSITY,dims[k][0]);
		  break;
		case GEOM_BOX:
		  g2[k] = dCreateBox (0,dims[k][0],dims[k][1],dims[k][2]);
		  dMassSetBox (&m2[k],DENSITY,dims[k][0],dims[k][1],dims[k][2]);
		  break;
		case GEOM_CYLINDER:
		  g2[k] = dCreateCCylinder (0,dims[k][0],dims[k][1]);
		  dMassSetCappedCylinder (&m2[k],DENSITY,3,dims[k][0],dims[k][1]);
		  break;
		}
		dGeomTransformSetGeom (obj[i].geom[k],g2[k]);
		obj[i].geomType[k] = type[k];
		for (p=0; p<3; p++) obj[i].dimensions[k][p] = dims[k][p];

		// set the transformation (adjust the mass too)
		dGeomSetPosition (g2[k],dpos[k][0],dpos[k][1],dpos[k][2]);
		dMatrix3 Rtx;
		dRfromQ(Rtx, quats[k]);
		dGeomSetRotation (g2[k],Rtx);
		dMassRotate (&m2[k],Rtx);
		dMassTranslate (&m2[k],dpos[k][0],dpos[k][1],dpos[k][2]);

		// add to the total mass
		dMassAdd (&m,&m2[k]);
    }

    // move all encapsulated objects so that the center of mass is (0,0,0)
   for (k=0; k<numg; k++) {
		dGeomSetPosition (g2[k],
				  dpos[k][0]-m.c[0],
				  dpos[k][1]-m.c[1],
				  dpos[k][2]-m.c[2]);
    }
	ctr[0] = m.c[0];   // save body position
	ctr[1] = m.c[1];
	ctr[2] = m.c[2];
    dMassTranslate (&m,-m.c[0],-m.c[1],-m.c[2]);

    for (k=0; k < GPB; k++) {
      if (obj[i].geom[k]) dGeomSetBody (obj[i].geom[k],obj[i].body);
    }

	// set body mass and position
    dBodySetMass (obj[i].body,&m);
	dBodySetPosition(obj[i].body,ctr[0],ctr[1],ctr[2]);
  }
  else if (cmd == 'z') {
	dropObject();
	destroySelected();
  }
}

// draw a geom

void drawGeom (dGeomID g, const dReal *pos, const dReal *R, int show_aabb)
{
  int i;

  if (!g) return;
  if (!pos) pos = dGeomGetPosition (g);
  if (!R) R = dGeomGetRotation (g);

  int type = dGeomGetClass (g);
  if (type == dBoxClass) {
    dVector3 sides;
    dGeomBoxGetLengths (g,sides);
    dsDrawBox (pos,R,sides);
  }
  else if (type == dSphereClass) {
    dsDrawSphere (pos,R,dGeomSphereGetRadius (g));
  }
  else if (type == dCCylinderClass) {
    dReal radius,length;
    dGeomCCylinderGetParams (g,&radius,&length);
    dsDrawCappedCylinder (pos,R,length,radius);
  }
/*
  // cylinder option not yet implemented
  else if (type == dCylinderClass) {
    dReal radius,length;
    dGeomCylinderGetParams (g,&radius,&length);
    dsDrawCylinder (pos,R,length,radius);
  }
*/
  else if (type == dGeomTransformClass) {
    dGeomID g2 = dGeomTransformGetGeom (g);
    const dReal *pos2 = dGeomGetPosition (g2);
    const dReal *R2 = dGeomGetRotation (g2);
    dVector3 actual_pos;
    dMatrix3 actual_R;
    dMULTIPLY0_331 (actual_pos,R,pos2);
    actual_pos[0] += pos[0];
    actual_pos[1] += pos[1];
    actual_pos[2] += pos[2];
    dMULTIPLY0_333 (actual_R,R,R2);
    drawGeom (g2,actual_pos,actual_R,0);
  }

  if (show_aabb) {
    // draw the bounding box for this geom
    dReal aabb[6];
    dGeomGetAABB (g,aabb);
    dVector3 bbpos;
    for (i=0; i<3; i++) bbpos[i] = 0.5*(aabb[i*2] + aabb[i*2+1]);
    dVector3 bbsides;
    for (i=0; i<3; i++) bbsides[i] = aabb[i*2+1] - aabb[i*2];
    dMatrix3 RI;
    dRSetIdentity (RI);
    dsSetColorAlpha (1,0,0,0.5);
    dsDrawBox (bbpos,RI,bbsides);
  }
}

// Frame-rate independent movement.
#define TARGET_FRAME_RATE 70.0
class FrameRate frameRate(TARGET_FRAME_RATE);

// simulation loop

static void simLoop (int pause)
{
  dsSetColor (0,0,2);
  dSpaceCollide (space,0,&nearCallback);
  if (!pause) dWorldQuickStep (world,0.05);

  if (write_world) {
    FILE *f = fopen ("state.dif","wt");
    if (f) {
      dWorldExportDIF (world,f,"X");
      fclose (f);
    }
    write_world = 0;
  }

  // remove all contact joints
  dJointGroupEmpty (contactgroup);

  dsSetColor (1,1,0);
  dsSetTexture (DS_WOOD);
  for (int i=0; i<num; i++) {
    for (int j=0; j < GPB; j++) {
      if (obj[i].selected) {
		dsSetColor (objColors[i][0],objColors[i][1],objColors[i][2]);
      }
      else if (! dBodyIsEnabled (obj[i].body)) {
		dsSetColor (1,0.8,0);
      }
      else {
		dsSetColor (1,1,0);
      }
      drawGeom (obj[i].geom[j],0,0,show_aabb);
    }
  }

  // Update animations.
  for (int i = 0; i < cyd.animations.size(); i++)
  {
	switch(i)
	{
	case CYD_LEGS_WALKING:
		cyd.animations[i]->run(frameRate.speedFactor * MovementRate);
		break;
	case CYD_ARMS_WALKING:
		cyd.animations[i]->run(frameRate.speedFactor * MovementRate);
		break;
	default:
		cyd.animations[i]->run(frameRate.speedFactor);
		break;
	}
  }

  // Update and get world transforms.
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  cyd.update();
  glPopMatrix();

  // Draw Cyd.
  GLfloat position[3];
  position[0] = 0.0;
  position[1] = 0.0;
  position[2] = 0.0;
  cyd.transform.spacial->transformPoint(position, cyd.xmatrix);
  if (position[2] >= 0.5)
  {
	cyd.draw(LIGHTX, LIGHTY);
  } else if (position[2] >= -0.5)
  {
    cyd.draw();
  }

  // Update object carrying variables.
  if (HeldObject != 0) carryObject(HeldObject);

  // Set frame-rate independence speed factor.
  frameRate.update();

  // Catch Cyd movement commands.
	if(GetAsyncKeyState(VK_UP)) specialKeyInput(GLUT_KEY_UP, 0, 0);
	if(GetAsyncKeyState(VK_DOWN)) specialKeyInput(GLUT_KEY_DOWN, 0, 0);
	if(GetAsyncKeyState(VK_RIGHT)) specialKeyInput(GLUT_KEY_RIGHT, 0, 0);
	if(GetAsyncKeyState(VK_LEFT)) specialKeyInput(GLUT_KEY_LEFT, 0, 0);
	if(GetAsyncKeyState('H')) specialKeyInput('h', 0, 0);
	if(GetAsyncKeyState('J')) specialKeyInput('j', 0, 0);
	if(GetAsyncKeyState('K')) specialKeyInput('k', 0, 0);
	if (!GetAsyncKeyState('J') && !GetAsyncKeyState('K'))
	{
		cyd.speed = 0.0;
		if (cyd.animations[CYD_LEGS_WALKING]->active)
		{
			cyd.animations[CYD_LEGS_WALKING]->unloop();
			cyd.animations[CYD_LEGS_WALKING]->stop();
			cyd.bodyParts[Cyd::UPPER_RIGHT_LEG].transform.setPitch(0.0);
			cyd.bodyParts[Cyd::LOWER_RIGHT_LEG].transform.setPitch(0.0);
			cyd.bodyParts[Cyd::UPPER_LEFT_LEG].transform.setPitch(0.0);
			cyd.bodyParts[Cyd::LOWER_LEFT_LEG].transform.setPitch(0.0);
		}
		if (InteractionMode == COLLISION && cyd.animations[CYD_ARMS_WALKING]->active)
		{
			cyd.animations[CYD_ARMS_WALKING]->unloop();
			cyd.animations[CYD_ARMS_WALKING]->stop();
			cyd.bodyParts[Cyd::UPPER_RIGHT_ARM].transform.setPitch(0.0);
			cyd.bodyParts[Cyd::LOWER_RIGHT_ARM].transform.setPitch(0.0);
			cyd.bodyParts[Cyd::UPPER_LEFT_ARM].transform.setPitch(0.0);
			cyd.bodyParts[Cyd::LOWER_LEFT_ARM].transform.setPitch(0.0);
		}
	}
	if(GetAsyncKeyState('L')) specialKeyInput('l', 0, 0);
	if(GetAsyncKeyState('U')) specialKeyInput('u', 0, 0);
	if(GetAsyncKeyState('M')) specialKeyInput('m', 0, 0);
}

int main (int argc, char **argv)
{
  // setup pointers to drawstuff callback functions
  dsFunctions fn;
  fn.version = DS_VERSION;
  fn.start = &start;
  fn.step = &simLoop;
  fn.command = &command;
  fn.stop = 0;
  fn.path_to_textures = "drawstuff/textures";

  // create world

  world = dWorldCreate();
  space = dHashSpaceCreate (0);
  contactgroup = dJointGroupCreate (0);
  dWorldSetGravity (world,0,0,-0.5);
  dWorldSetCFM (world,1e-5);
  dWorldSetAutoDisableFlag (world,1);
  dWorldSetContactMaxCorrectingVel (world,0.1);
  dWorldSetContactSurfaceLayer (world,0.001);
  dCreatePlane (space,0,0,1,0);
  memset (obj,0,sizeof(obj));

  // Initialize Cyd.
  cyd.init(world, space);

  // run simulation
  dsSimulationLoop (argc,argv,352,288,&fn);

  dJointGroupDestroy (contactgroup);
  dSpaceDestroy (space);
  dWorldDestroy (world);

  return 0;
}
