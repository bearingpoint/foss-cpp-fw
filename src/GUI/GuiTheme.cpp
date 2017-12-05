/*
 * GuiTheme.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include <boglfw/GUI/GuiTheme.h>
#include <boglfw/GUI/DefaultTheme.h>

std::shared_ptr<GuiTheme> GuiTheme::activeTheme_ = std::make_shared<DefaultTheme>();
