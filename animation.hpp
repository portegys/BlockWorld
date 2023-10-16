//***************************************************************************//
//* File Name: animation.hpp                                                *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 11/19/04                                                     *//
//* File Desc: Class declaration and implementation details                 *//
//*            representing an object animation.                            *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifndef __ANIMATION_HPP__
#define __ANIMATION_HPP__

#include <GL/gl.h>
#include <list>
#include "body.hpp"

// Body part animation.
class AnimPart
{
public:

	// Body part to animate.
	BodyPart *part;

	// Target translation and speeds.
	GLfloat tx,ty,tz;
	GLfloat txs, tys, tzs;

	// Target rotation and speeds.
	GLfloat rx,ry,rz;
	GLfloat rxs, rys, rzs;

	AnimPart()
	{
		tx = ty = tz = 0.0;
		txs = tys = tzs = 0.0;
		rx = ry = rz = 0.0;
		rxs = rys = rzs = 0.0;
	}

	// Transform part: return true if transforms done.
	bool run(float speedFactor)
	{
		GLfloat delta;
		GLfloat forward[3];
		GLfloat speed;
		bool done = true;

		// Translations.
		part->transform.spacial->getForward(forward);
		delta = tx - part->transform.tx;
		speed = speedFactor * txs;
		if (speed < 0.0) speed = 0.0;
		if (txs > 0.0 && fabs(delta) > 0.0)
		{
			done = false;
			if (delta > 0.0 && delta > speed)
			{
				delta = speed;
			} else if (delta < 0.0 && -delta > speed)
			{
					delta = -speed;
			}
			part->transform.tx += forward[0] + delta;
		}
		delta = ty - part->transform.ty;
		speed = speedFactor * tys;
		if (speed < 0.0) speed = 0.0;
		if (tys > 0.0 && fabs(delta) > 0.0)
		{
			done = false;
			if (delta > 0.0 && delta > speed)
			{
				delta = speed;
			} else if (delta < 0.0 && -delta > speed)
			{
					delta = -speed;
			}
			part->transform.ty += forward[1] + delta;
		}
		delta = tz - part->transform.tz;
		speed = speedFactor * tzs;
		if (speed < 0.0) speed = 0.0;
		if (tzs > 0.0 && fabs(delta) > 0.0)
		{
			done = false;
			if (delta > 0.0 && delta > speed)
			{
				delta = speed;
			} else if (delta < 0.0 && -delta > speed)
			{
					delta = -speed;
			}
			part->transform.tz += forward[2] + delta;
		}

		// Pitch.
		delta = rx - part->transform.rx;
		speed = speedFactor * rxs;
		if (speed < 0.0) speed = 0.0;
		if (rxs > 0.0 && fabs(delta) > 0.0)
		{
			done = false;
			if (delta > 0.0 && delta > speed)
			{
				delta = speed;
			} else if (delta < 0.0 && -delta > speed)
			{
					delta = -speed;
			}
			part->transform.addPitch(delta);
		}

		// Yaw.
		delta = ry - part->transform.ry;
		speed = speedFactor * rys;
		if (speed < 0.0) speed = 0.0;
		if (rys > 0.0 && fabs(delta) > 0.0)
		{
			done = false;
			if (delta > 0.0 && delta > speed)
			{
				delta = speed;
			} else if (delta < 0.0 && -delta > speed)
			{
					delta = -speed;
			}
			part->transform.addYaw(delta);
		}

		// Roll.
		delta = rz - part->transform.rz;
		speed = speedFactor * rzs;
		if (speed < 0.0) speed = 0.0;
		if (rzs > 0.0 && fabs(delta) > 0.0)
		{
			done = false;
			if (delta > 0.0 && delta > speed)
			{
				delta = speed;
			} else if (delta < 0.0 && -delta > speed)
			{
					delta = -speed;
			}
			part->transform.addRoll(delta);
		}

		return done;
	}
};

// Cluster of concurrent body part animations.
class AnimCluster
{
public:

	std::list<AnimPart *> cluster;

	// Run parts: return true if all done.
	bool run(float speedFactor)
	{
		bool done;
		AnimPart *part;
		std::list<AnimPart *>::iterator partItr;

		done = true;
		for (partItr = cluster.begin();
			partItr != cluster.end(); partItr++)
		{
			part = *partItr;
			if (!part->run(speedFactor)) done = false;
		}
		return done;
	}
};

// Animation sequence.
class Animation
{
public:

	// Sequence of clusters.
	std::list<AnimCluster *> sequence;
	std::list<AnimCluster *>::iterator seqItr;

	// Animation is active?
	bool active;

	// Animation is looped?
	bool looped;

	Animation()
	{
		active = looped = false;
	}

	void reset()
	{
		seqItr = sequence.begin();

	}

	void start()
	{
		reset();
		active = true;
	}

	void stop()
	{
		reset();
		active = false;
	}

	void loop()
	{
		looped = true;
	}

	void unloop()
	{
		looped = false;
	}

	// Run cluster sequence: return true if done.
	bool run(float speedFactor)
	{
		AnimCluster *cluster;

		if (!active) return true;

		cluster = *seqItr;
		if (cluster->run(speedFactor))
		{
			seqItr++;
		}

		if (seqItr == sequence.end())
		{
			if (looped)
			{
				reset();
				return false;
			} else {
				stop();
				return true;
			}
		} else {
			return false;
		}
	}
};
#endif
