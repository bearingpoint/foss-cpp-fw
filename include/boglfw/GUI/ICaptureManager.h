/*
 * ICaptureManager.h
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#ifndef GUI_ICAPTUREMANAGER_H_
#define GUI_ICAPTUREMANAGER_H_

class GuiBasicElement;

class ICaptureManager {
public:
	virtual ~ICaptureManager() {}

	virtual void setMouseCapture(GuiBasicElement* elementOrNull) = 0;
};

#endif /* GUI_ICAPTUREMANAGER_H_ */
