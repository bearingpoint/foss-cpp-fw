/*
 * enttypes.h
 *
 *  Created on: Apr 15, 2015
 *      Author: bogdan
 */

#ifndef ENTITIES_ENTTYPES_H_
#define ENTITIES_ENTTYPES_H_

namespace EntityTypes {
enum EType : unsigned {
	BUG					= 1,
	GAMETE				= 2,
	WALL				= 3,
	FOOD_DISPENSER		= 4,
	FOOD_CHUNK			= 5,
	LABEL				= 6,

	CAMERA_CTRL			= 7,
	PATH_CONTROLLER		= 8,
	BOX					= 9,
	SIGNAL_VIEWER		= 10,
	GIZMO				= 11,
};
} // namespace

#endif /* ENTITIES_ENTTYPES_H_ */

