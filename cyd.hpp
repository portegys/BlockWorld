//***************************************************************************//
//* File Name: cyd.hpp                                                      *//
//* Author: Tom Portegys portegys@ilstu.edu                                 *//
//* Date Made: 12/03/04                                                     *//
//* File Desc: The Cyd android.                                             *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//
#ifndef __CYD_HPP__
#define __CYD_HPP__

#include <ode/ode.h>
#include <list>
#include <vector>
#include "body.hpp"
#include "animation.hpp"
#include "cyd_model.h"

// Density of Cyd's body.
#define CYD_DENSITY (5.0)

class Cyd
{
public:

	// Cyd transform.
	BodyTransform transform;
	GLfloat xmatrix[16];

	// Speed.
	GLfloat speed;

	// Body parts.
	#define CYD_NUM_BODY_PARTS 10
	typedef enum
	{
		TORSO=0, HEAD=1,
		UPPER_RIGHT_ARM=2, LOWER_RIGHT_ARM=3,
		UPPER_RIGHT_LEG=4, LOWER_RIGHT_LEG=5,
		UPPER_LEFT_ARM=6, LOWER_LEFT_ARM=7,
		UPPER_LEFT_LEG=8, LOWER_LEFT_LEG=9
	} CYD_BODY_PART;

	class CydBodyPart : public BodyPart
	{
	public:

		// Body part.
		int part;

		// Body part transform matrix.
		GLfloat xmatrix[16];

		// Get transform.
		void getTransform()
		{
			CydBodyPart *subpart;
			std::list<BodyPart *>::iterator listItr;

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();

			// Transform this part.
			switch(part)
			{
			case TORSO: getTorsoTransform(); break;
			case HEAD:	getHeadTransform(); break;
			case UPPER_RIGHT_ARM:	getUpperRightArmTransform(); break;
			case LOWER_RIGHT_ARM:	getLowerRightArmTransform(); break;
			case UPPER_RIGHT_LEG:	getUpperRightLegTransform(); break;
			case LOWER_RIGHT_LEG:	getLowerRightLegTransform(); break;
			case UPPER_LEFT_ARM:	getUpperLeftArmTransform(); break;
			case LOWER_LEFT_ARM:	getLowerLeftArmTransform(); break;
			case UPPER_LEFT_LEG:	getUpperLeftLegTransform(); break;
			case LOWER_LEFT_LEG:	getLowerLeftLegTransform(); break;
			}

			// Transform sub-parts.
			for (listItr = subparts.begin();
				listItr != subparts.end(); listItr++)
			{
				subpart = (CydBodyPart *)*listItr;

				// Remove torso rotation from leg transform.
				if (subpart->part == UPPER_RIGHT_LEG)
				{
					glPopMatrix();
				}
				glPushMatrix();
				subpart->getTransform();
				glPopMatrix();
			}
			glPopMatrix();
		}

		// Draw.
		void draw()
		{
			CydBodyPart *subpart;
			std::list<BodyPart *>::iterator listItr;

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();

			// Draw this part.
			switch(part)
			{
			case TORSO: drawTorso(); break;
			case HEAD:	drawHead(); break;
			case UPPER_RIGHT_ARM:	drawUpperRightArm(); break;
			case LOWER_RIGHT_ARM:	drawLowerRightArm(); break;
			case UPPER_RIGHT_LEG:	drawUpperRightLeg(); break;
			case LOWER_RIGHT_LEG:	drawLowerRightLeg(); break;
			case UPPER_LEFT_ARM:	drawUpperLeftArm(); break;
			case LOWER_LEFT_ARM:	drawLowerLeftArm(); break;
			case UPPER_LEFT_LEG:	drawUpperLeftLeg(); break;
			case LOWER_LEFT_LEG:	drawLowerLeftLeg(); break;
			}

			glPopMatrix();

			// Draw sub-parts.
			for (listItr = subparts.begin();
				listItr != subparts.end(); listItr++)
			{
				subpart = (CydBodyPart *)*listItr;
				subpart->draw();
			}
		}

		// Specialized transform and drawing functions.
		void getTorsoTransform()
		{
			GLfloat t[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);

			// Separate torso rotation to pop before legs drawn.
			glPushMatrix();

			glTranslatef(0.0, 0.0,
				CydBounds[CYD_TORSO].min[2]);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(0.0, 0.0,
				-CydBounds[CYD_TORSO].min[2]);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawTorso()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_TORSO]);
#else
			drawCydComponent(CYD_TORSO);
#endif
		}

		void getHeadTransform()
		{
			GLfloat t[3],a[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);
			for (int i = 0; i < 3; i++)
			{
				a[i] = (CydBounds[CYD_UPPER_RIGHT_ARM].max[i] +
					CydBounds[CYD_UPPER_RIGHT_ARM].min[i]) / 2.0;
			}
			glTranslatef(0.0, a[1], 0.0);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(0.0, -a[1], 0.0);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawHead()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_HEAD]);
			glCallList(CydDisplays[CYD_LEFT_EYE]);
			glCallList(CydDisplays[CYD_RIGHT_EYE]);
#else
			drawCydComponent(CYD_HEAD);
			drawCydComponent(CYD_LEFT_EYE);
			drawCydComponent(CYD_RIGHT_EYE);
#endif
		}

		void getUpperRightArmTransform()
		{
			GLfloat t[3],a[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);
			for (int i = 0; i < 3; i++)
			{
				a[i] = (CydBounds[CYD_UPPER_RIGHT_ARM].max[i] +
					CydBounds[CYD_UPPER_RIGHT_ARM].min[i]) / 2.0;
			}
			glTranslatef(a[0] * 0.9, a[1],
				CydBounds[CYD_UPPER_RIGHT_ARM].max[2] * 0.9);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(-a[0] * 0.9, -a[1],
				-CydBounds[CYD_UPPER_RIGHT_ARM].max[2] * 0.9);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawUpperRightArm()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_UPPER_RIGHT_ARM]);
#else
			drawCydComponent(CYD_UPPER_RIGHT_ARM);
#endif
		}

		void getLowerRightArmTransform()
		{
			GLfloat t[3],a[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);
			for (int i = 0; i < 3; i++)
			{
				a[i] = (CydBounds[CYD_LOWER_RIGHT_ARM].max[i] +
					CydBounds[CYD_LOWER_RIGHT_ARM].min[i]) / 2.0;
			}
			glTranslatef(a[0] * 0.9, a[1],
				CydBounds[CYD_LOWER_RIGHT_ARM].max[2] * 0.85);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(-a[0] * 0.9, -a[1],
				-CydBounds[CYD_LOWER_RIGHT_ARM].max[2] * 0.85);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawLowerRightArm()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_LOWER_RIGHT_ARM]);
			glCallList(CydDisplays[CYD_RIGHT_HAND]);
#else
			drawCydComponent(CYD_LOWER_RIGHT_ARM);
			drawCydComponent(CYD_RIGHT_HAND);
#endif
		}

		void getUpperRightLegTransform()
		{
			GLfloat t[3],a[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);
			for (int i = 0; i < 3; i++)
			{
				a[i] = (CydBounds[CYD_UPPER_RIGHT_LEG].max[i] +
					CydBounds[CYD_UPPER_RIGHT_LEG].min[i]) / 2.0;
			}
			glTranslatef(a[0] * 0.9, a[1],
				CydBounds[CYD_UPPER_RIGHT_LEG].max[2] * 0.8);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(-a[0] * 0.9, -a[1],
				-CydBounds[CYD_UPPER_RIGHT_LEG].max[2] * 0.8);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawUpperRightLeg()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_UPPER_RIGHT_LEG]);
			glCallList(CydDisplays[CYD_RIGHT_KNEE]);
#else
			drawCydComponent(CYD_UPPER_RIGHT_LEG);
			drawCydComponent(CYD_RIGHT_KNEE);
#endif
		}

		void getLowerRightLegTransform()
		{
			GLfloat t[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);
			glTranslatef(0.0, CydBounds[CYD_LOWER_RIGHT_LEG].max[1] * 0.5,
				CydBounds[CYD_LOWER_RIGHT_LEG].max[2] * 1.1);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(0.0, -CydBounds[CYD_LOWER_RIGHT_LEG].max[1] * 0.5,
				-CydBounds[CYD_LOWER_RIGHT_LEG].max[2] * 1.1);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawLowerRightLeg()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_LOWER_RIGHT_LEG]);
			glCallList(CydDisplays[CYD_RIGHT_FOOT]);
#else
			drawCydComponent(CYD_LOWER_RIGHT_LEG);
			drawCydComponent(CYD_RIGHT_FOOT);
#endif
		}

		void getUpperLeftArmTransform()
		{
			GLfloat t[3],a[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);
			for (int i = 0; i < 3; i++)
			{
				a[i] = (CydBounds[CYD_UPPER_LEFT_ARM].max[i] +
					CydBounds[CYD_UPPER_LEFT_ARM].min[i]) / 2.0;
			}
			glTranslatef(a[0] * 0.9, a[1],
				CydBounds[CYD_UPPER_LEFT_ARM].max[2] * 0.9);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(-a[0] * 0.9, -a[1],
				-CydBounds[CYD_UPPER_LEFT_ARM].max[2] * 0.9);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawUpperLeftArm()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_UPPER_LEFT_ARM]);
#else
			drawCydComponent(CYD_UPPER_LEFT_ARM);
#endif
		}

		void getLowerLeftArmTransform()
		{
			GLfloat t[3],a[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);
			for (int i = 0; i < 3; i++)
			{
				a[i] = (CydBounds[CYD_LOWER_LEFT_ARM].max[i] +
					CydBounds[CYD_LOWER_LEFT_ARM].min[i]) / 2.0;
			}
			glTranslatef(a[0] * 0.9, a[1],
				CydBounds[CYD_LOWER_LEFT_ARM].max[2] * 0.85);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(-a[0] * 0.9, -a[1],
				-CydBounds[CYD_LOWER_LEFT_ARM].max[2] * 0.85);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawLowerLeftArm()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_LOWER_LEFT_ARM]);
			glCallList(CydDisplays[CYD_LEFT_HAND]);
#else
			drawCydComponent(CYD_LOWER_LEFT_ARM);
			drawCydComponent(CYD_LEFT_HAND);
#endif
		}

		void getUpperLeftLegTransform()
		{
			GLfloat t[3],a[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);
			for (int i = 0; i < 3; i++)
			{
				a[i] = (CydBounds[CYD_UPPER_LEFT_LEG].max[i] +
					CydBounds[CYD_UPPER_LEFT_LEG].min[i]) / 2.0;
			}
			glTranslatef(a[0] * 0.9, a[1],
				CydBounds[CYD_UPPER_LEFT_LEG].max[2] * 0.8);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(-a[0] * 0.9, -a[1],
				-CydBounds[CYD_UPPER_LEFT_LEG].max[2] * 0.8);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawUpperLeftLeg()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_UPPER_LEFT_LEG]);
			glCallList(CydDisplays[CYD_LEFT_KNEE]);
#else
			drawCydComponent(CYD_UPPER_LEFT_LEG);
			drawCydComponent(CYD_LEFT_KNEE);
#endif
		}

		void getLowerLeftLegTransform()
		{
			GLfloat t[3];
			cSpacial *spacial;

			t[0] = transform.ox + transform.tx;
			t[1] = transform.oy + transform.ty;
			t[2] = transform.oz + transform.tz;
			glTranslatef(t[0], t[1], t[2]);
			glTranslatef(0.0, CydBounds[CYD_LOWER_LEFT_LEG].max[1] * 0.5,
				CydBounds[CYD_LOWER_LEFT_LEG].max[2] * 1.1);
			spacial = transform.spacial;
			glMultMatrixf(&spacial->rotmatrix[0][0]);
			glTranslatef(0.0, -CydBounds[CYD_LOWER_LEFT_LEG].max[1] * 0.5,
				-CydBounds[CYD_LOWER_LEFT_LEG].max[2] * 1.1);
			glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);
		}

		void drawLowerLeftLeg()
		{
			glMultMatrixf(xmatrix);
#ifdef CYD_DRAW_USING_DISPLAY
			glCallList(CydDisplays[CYD_LOWER_LEFT_LEG]);
			glCallList(CydDisplays[CYD_LEFT_FOOT]);
#else
			drawCydComponent(CYD_LOWER_LEFT_LEG);
			drawCydComponent(CYD_LEFT_FOOT);
#endif
		}
	};
	CydBodyPart bodyParts[CYD_NUM_BODY_PARTS];

	// Bounding boxes for ODE collision detection/response.
	struct BoundingBox {
	  dBodyID body;
	  dGeomID geom;
	};
	struct BoundingBox boundingBoxes[CYD_NUM_COMPONENTS];

	// ODE world and space.
	dWorldID world;
	dSpaceID space;

	// Show bounding boxes?
	bool showBoxes;
	bool showHands;

	// Animations.
	std::vector<Animation *> animations;

	// Initialize.
	void init(dWorldID world, dSpaceID space)
	{
		// Raise Cyd to "ground" level.
		transform.oz = 0.5;

		// Build Cyd model.
		buildCydModel();

		// Build body parts.
		bodyParts[TORSO].part = TORSO;
		bodyParts[TORSO].subparts.push_back(&bodyParts[UPPER_RIGHT_ARM]);
		bodyParts[TORSO].subparts.push_back(&bodyParts[UPPER_LEFT_ARM]);
		bodyParts[TORSO].subparts.push_back(&bodyParts[HEAD]);
		bodyParts[TORSO].subparts.push_back(&bodyParts[UPPER_RIGHT_LEG]);
		bodyParts[TORSO].subparts.push_back(&bodyParts[UPPER_LEFT_LEG]);
		bodyParts[HEAD].part = HEAD;
		bodyParts[UPPER_RIGHT_ARM].part = UPPER_RIGHT_ARM;
		bodyParts[UPPER_RIGHT_ARM].subparts.push_back(&bodyParts[LOWER_RIGHT_ARM]);
		bodyParts[LOWER_RIGHT_ARM].part = LOWER_RIGHT_ARM;
		bodyParts[UPPER_LEFT_ARM].part = UPPER_LEFT_ARM;
		bodyParts[UPPER_LEFT_ARM].subparts.push_back(&bodyParts[LOWER_LEFT_ARM]);
		bodyParts[LOWER_LEFT_ARM].part = LOWER_LEFT_ARM;
		bodyParts[UPPER_RIGHT_LEG].part = UPPER_RIGHT_LEG;
		bodyParts[UPPER_RIGHT_LEG].subparts.push_back(&bodyParts[LOWER_RIGHT_LEG]);
		bodyParts[LOWER_RIGHT_LEG].part = LOWER_RIGHT_LEG;
		bodyParts[UPPER_LEFT_LEG].part = UPPER_LEFT_LEG;
		bodyParts[UPPER_LEFT_LEG].subparts.push_back(&bodyParts[LOWER_LEFT_LEG]);
		bodyParts[LOWER_LEFT_LEG].part = LOWER_LEFT_LEG;

		// Create bounding boxes.
		int i,j;
		dReal sides[3],position[3];
		dMass m;

		this->world = world;
		this->space = space;

		showBoxes = showHands = false;
		for (i = 0; i < CYD_NUM_COMPONENTS; i++)
		{
			boundingBoxes[i].body = dBodyCreate(world);
			for (j = 0; j < 3; j++)
			{
				sides[j] = CydBounds[i].max[j] - CydBounds[i].min[j];
				position[j] = (CydBounds[i].max[j] + CydBounds[i].min[j]) / 2.0;
			}
			dMassSetBox(&m,CYD_DENSITY,sides[0],sides[1],sides[2]);
			dBodySetMass(boundingBoxes[i].body,&m);
			boundingBoxes[i].geom = dCreateBox(space,sides[0],sides[1],sides[2]);
			dGeomSetCategoryBits(boundingBoxes[i].geom, (unsigned long)i);
			dGeomSetBody(boundingBoxes[i].geom,boundingBoxes[i].body);
			dGeomSetPosition(boundingBoxes[i].geom,position[0],position[1],position[2]);
		}

		// Create animations:
		animations.resize(4);

		// Lower body walking.
		Animation *animation = new Animation();
		animations[0] = animation;
		AnimCluster *cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		AnimPart *part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_LEG];
		part->rx = -20.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_LEG];
		part->rx = 20.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_LEFT_LEG];
		part->rx = 40.0;
		part->rxs = 2.0;
		cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_LEG];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_LEG];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_LEFT_LEG];
		part->rx = 0.0;
		part->rxs = 2.0;
		cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_LEG];
		part->rx = 20.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_RIGHT_LEG];
		part->rx = 40.0;
		part->rxs = 2.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_LEG];
		part->rx = -20.0;
		part->rxs = 1.0;
		cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_LEG];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_RIGHT_LEG];
		part->rx = 0.0;
		part->rxs = 2.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_LEG];
		part->rx = 0.0;
		part->rxs = 1.0;

		// Upper body walking.
		animation = new Animation();
		animations[1] = animation;
		cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_ARM];
		part->rx = 20.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_ARM];
		part->rx = -20.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_LEFT_ARM];
		part->rx = -40.0;
		part->rxs = 2.0;
		cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_LEFT_ARM];
		part->rx = 0.0;
		part->rxs = 2.0;
		cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_ARM];
		part->rx = -20.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_RIGHT_ARM];
		part->rx = -40.0;
		part->rxs = 2.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_ARM];
		part->rx = 20.0;
		part->rxs = 1.0;
		cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_RIGHT_ARM];
		part->rx = 0.0;
		part->rxs = 2.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;

		// Arms to pickup position.
		animation = new Animation();
		animations[2] = animation;
		cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_ARM];
		part->rx = -90.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_RIGHT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_ARM];
		part->rx = -90.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_LEFT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[TORSO];
		part->rx = 45.0;
		part->rxs = 1.0;

		// Arms to sides position.
		animation = new Animation();
		animations[3] = animation;
		cluster = new AnimCluster();
		animation->sequence.push_back(cluster);
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_RIGHT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_RIGHT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[UPPER_LEFT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[LOWER_LEFT_ARM];
		part->rx = 0.0;
		part->rxs = 1.0;
		part = new AnimPart();
		cluster->cluster.push_back(part);
		part->part = &bodyParts[TORSO];
		part->rx = 0.0;
		part->rxs = 1.0;
	}

	// Update.
	void update()
	{
		int i,j,p,q;
		dMatrix3 rotation;
		GLfloat angle,ax,ay,az;
		GLfloat matrix[4][4],quat[4];
		double cosa,sina;
		GLfloat position[3],forward[3];
		dReal velocity[3],angularVelocity[3];

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		// Get Cyd transform.
		glTranslatef(transform.ox + transform.tx,
			transform.oy + transform.ty,
			transform.oz + transform.tz);
		glMultMatrixf(&transform.spacial->rotmatrix[0][0]);
		glScalef(transform.sx,transform.sy, transform.sz);
		glGetFloatv(GL_MODELVIEW_MATRIX, xmatrix);

		// Recursive body part transforms.
		bodyParts[TORSO].getTransform();

		glPopMatrix();

		// Transform bounding boxes.
		transform.spacial->getUp(forward);
		for (i = 0; i < 3; i++) velocity[i] = -forward[i] * speed;
		for (i = 0; i < 3; i++) angularVelocity[i] = 0.0;
		for (i = 0; i < CYD_NUM_COMPONENTS; i++)
		{
			dBodySetLinearVel (boundingBoxes[i].body,
				velocity[0], velocity[1], velocity[2]);
			dBodySetAngularVel(boundingBoxes[i].body,
				angularVelocity[0], angularVelocity[1], angularVelocity[2]);
			for (j = 0; j < 3; j++)
			{
				position[j] = (CydBounds[i].max[j] + CydBounds[i].min[j]) / 2.0;
			}

			j = getPartFromComponent(i);
			for (p = 0; p < 4; p++)
			{
				for (q = 0; q < 4; q++)
				{
					matrix[p][q] = bodyParts[j].xmatrix[(p*4)+q];
				}
			}
			bodyParts[j].transform.spacial->qcalc->build_quat(quat,matrix);
			cosa  = quat[3];
			angle = acos(cosa) * 2.0;
			sina = sqrt(1.0 - (cosa * cosa));
			if (fabs(sina) < 0.0005) sina = 1.0;
			ax = quat[0] / sina;
			ay = quat[1] / sina;
			az = quat[2] / sina;
			dRFromAxisAndAngle(rotation, ax, ay, az, -angle);
			transform.spacial->transformPoint(position, bodyParts[j].xmatrix);
			dBodySetRotation(boundingBoxes[i].body, rotation);
			dBodySetPosition(boundingBoxes[i].body,position[0],position[1],position[2]);
		}
	}

	// Get body part for given component.
	int getPartFromComponent(int component)
	{
		int part;

		switch(component)
		{
			  case CYD_UPPER_RIGHT_LEG:
				  part = UPPER_RIGHT_LEG;
				  break;
			  case CYD_HEAD:
				  part = HEAD;
				  break;
			  case CYD_UPPER_RIGHT_ARM:
				  part = UPPER_RIGHT_ARM;
				  break;
			  case CYD_UPPER_LEFT_ARM:
				  part = UPPER_LEFT_ARM;
				  break;
			  case CYD_LOWER_LEFT_ARM:
				  part = LOWER_LEFT_ARM;
				  break;
			  case CYD_LOWER_RIGHT_ARM:
				  part = LOWER_RIGHT_ARM;
				  break;
			  case CYD_RIGHT_KNEE:
				  part = UPPER_RIGHT_LEG;
				  break;
			  case CYD_UPPER_LEFT_LEG:
				  part = UPPER_LEFT_LEG;
				  break;
			  case CYD_LOWER_LEFT_LEG:
				  part = LOWER_LEFT_LEG;
				  break;
			  case CYD_LEFT_KNEE:
				  part = UPPER_LEFT_LEG;
				  break;
			  case CYD_LOWER_RIGHT_LEG:
				  part = LOWER_RIGHT_LEG;
				  break;
			  case CYD_LEFT_EYE:
				  part = HEAD;
				  break;
			  case CYD_RIGHT_EYE:
				  part = HEAD;
				  break;
			  case CYD_RIGHT_HAND:
				  part = LOWER_RIGHT_ARM;
				  break;
			  case CYD_LEFT_HAND:
				  part = LOWER_LEFT_ARM;
				  break;
			  case CYD_RIGHT_FOOT:
				  part = LOWER_RIGHT_LEG;
				  break;
			  case CYD_LEFT_FOOT:
				  part = LOWER_LEFT_LEG;
				  break;
			  case CYD_TORSO:
				  part = TORSO;
				  break;
		}
		return part;
	}

	// Draw Cyd with shadow.
	void draw(GLfloat lightx, GLfloat lighty)
	{
		glMatrixMode(GL_MODELVIEW);

		// Draw shadow.
		glDisable (GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0, 0.0, 0.0, 0.3);
		for (int i = 0; i < CYD_NUM_COMPONENTS; i++)
		{
			glPushMatrix();
			glDepthRange (0,0.9999);
			setShadowTransform(lightx, lighty);

			// Draw precise shadow - too slow!
//			bodyParts[TORSO].draw();

			// Draw bounding box shadow.
			glPushMatrix();
			glMultMatrixf(bodyParts[getPartFromComponent(i)].xmatrix);
			drawBoundingBox(i, true);
			glPopMatrix();
			glPopMatrix();
		}

		// Draw Cyd.
		draw();
	}

	// Draw Cyd.
	void draw()
	{
		glMatrixMode(GL_MODELVIEW);

		// Set smooth texture mapping and lighting.
		glShadeModel(GL_SMOOTH);
		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable(GL_LIGHTING);
		glDisable (GL_CULL_FACE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE ,1);

		// Recursive draw.
		bodyParts[TORSO].draw();

		// Draw bounding boxes.
		if (showBoxes || showHands)
		{
			glDisable (GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);
		    glEnable (GL_BLEND);
		    glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(1.0, 0.0, 1.0, 0.5);
			for (int i = 0; i < CYD_NUM_COMPONENTS; i++)
			{
				if (showBoxes || i == CYD_RIGHT_HAND || i == CYD_LEFT_HAND)
				{
					glPushMatrix();
					glMultMatrixf(bodyParts[getPartFromComponent(i)].xmatrix);
					drawBoundingBox(i, true);
					glPopMatrix();
				}
			}
		}
	}

	// Draw bounding box.
	void drawBoundingBox(int component, bool solid)
	{
		GLfloat xmin = CydBounds[component].min[0];
		GLfloat xmax = CydBounds[component].max[0];
		GLfloat ymin = CydBounds[component].min[1];
		GLfloat ymax = CydBounds[component].max[1];
		GLfloat zmin = CydBounds[component].min[2];
		GLfloat zmax = CydBounds[component].max[2];

		if (solid)
		{
		  // sides
		  glBegin (GL_TRIANGLE_STRIP);
		  glVertex3f (xmin,ymin,zmin);
		  glVertex3f (xmin,ymin,zmax);
		  glVertex3f (xmin,ymax,zmin);
		  glVertex3f (xmin,ymax,zmax);
		  glVertex3f (xmax,ymax,zmin);
		  glVertex3f (xmax,ymax,zmax);
		  glVertex3f (xmax,ymin,zmin);
		  glVertex3f (xmax,ymin,zmax);
		  glVertex3f (xmin,ymin,zmin);
		  glVertex3f (xmin,ymin,zmax);
		  glEnd();

		  // top face
		  glBegin (GL_TRIANGLE_FAN);
		  glVertex3f (xmin,ymin,zmax);
		  glVertex3f (xmax,ymin,zmax);
		  glVertex3f (xmax,ymax,zmax);
		  glVertex3f (xmin,ymax,zmax);
		  glEnd();

		  // bottom face
		  glBegin (GL_TRIANGLE_FAN);
		  glVertex3f (xmin,ymin,zmin);
		  glVertex3f (xmin,ymax,zmin);
		  glVertex3f (xmax,ymax,zmin);
		  glVertex3f (xmax,ymin,zmin);
		  glEnd();

		} else {

			glBegin(GL_LINE_LOOP);
				glVertex3f(xmin, ymin, zmax);
				glVertex3f(xmin, ymax, zmax);
				glVertex3f(xmax, ymax, zmax);
				glVertex3f(xmax, ymin, zmax);
			glEnd();
			glBegin(GL_LINE_LOOP);
				glVertex3f(xmin, ymin, zmin);
				glVertex3f(xmin, ymax, zmin);
				glVertex3f(xmax, ymax, zmin);
				glVertex3f(xmax, ymin, zmin);
			glEnd();
			glBegin(GL_LINES);
				glVertex3f(xmin, ymin, zmax);
				glVertex3f(xmin, ymin, zmin);
				glVertex3f(xmin, ymax, zmax);
				glVertex3f(xmin, ymax, zmin);
				glVertex3f(xmax, ymax, zmax);
				glVertex3f(xmax, ymax, zmin);
				glVertex3f(xmax, ymin, zmax);
				glVertex3f(xmax, ymin, zmin);
			glEnd();
		}
	}

	// Set shadow projection transform.
	void setShadowTransform(GLfloat lightx, GLfloat lighty)
	{
	  GLfloat matrix[16];
	  for (int i=0; i<16; i++) matrix[i] = 0;
	  matrix[0]=1;
	  matrix[5]=1;
	  matrix[8]=-lightx;
	  matrix[9]=-lighty;
	  matrix[15]=1;
	  glMultMatrixf (matrix);
	}
};
#endif
