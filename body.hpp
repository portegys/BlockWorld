//***************************************************************************//
//* File Name: body.hpp                                                     *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 11/19/04                                                     *//
//* File Desc: Class declaration and implementation details                 *//
//*            representing an object body.                                 *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifndef __BODY_HPP__
#define __BODY_HPP__

#include <GL/gl.h>
#include <list>
#include "spacial.hpp"

// Transforms.
class BodyTransform
{
public:

	// Constructor.
	BodyTransform()
	{
		ox = oy = oz = 0.0;
		tx = ty = tz = 0.0;
		sx = sy = sz = 1.0;
		rx = ry = rz = 0.0;
		spacial = new cSpacial();
	}

	// Destructor.
	~BodyTransform()
	{
		delete spacial;
	}

	// Offset translation.
	GLfloat ox,oy,oz;

	// Translation.
	GLfloat tx,ty,tz;

	// Scale.
	GLfloat sx,sy,sz;

	// Rotation state: rx=pitch, ry=yaw, rz=roll
	GLfloat rx,ry,rz;
	cSpacial *spacial;

	// Rotation methods.
	void setPitch(GLfloat n)
	{
		spacial->qcalc->clear();
		rx = spacial->pitch = 0.0;
		addPitch(n);
	}
	void setYaw(GLfloat n)
	{
		spacial->qcalc->clear();
		ry = spacial->yaw = 0.0;
		addYaw(n);
	}
	void setRoll(GLfloat n)
	{
		spacial->qcalc->clear();
		rz = spacial->roll = 0.0;
		addRoll(n);
	}
	void addPitch(GLfloat n)
	{
		spacial->pitch = -n;
		spacial->update();
		spacial->pitch = 0.0;
		rx += n;
	}
	void addYaw(GLfloat n)
	{
		spacial->yaw = n;
		spacial->update();
		spacial->yaw = 0.0;
		ry += n;
	}
	void addRoll(GLfloat n)
	{
		spacial->roll = n;
		spacial->update();
		spacial->roll = 0.0;
		rz += n;
	}
};

// Body part.
class BodyPart
{
public:

	// Body transform.
	BodyTransform transform;

	// Subparts.
	std::list<BodyPart *> subparts;
};
#endif
