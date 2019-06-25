/*
 * TextField.h
 *
 *  Created on: Mar 29, 2015
 *      Author: alexandra
 */

#ifndef GUI_CONTROLS_TEXTFIELD_H_
#define GUI_CONTROLS_TEXTFIELD_H_

#include <boglfw/GUI/GuiBasicElement.h>
#include <boglfw/utils/Event.h>
#include <string>

class TextField : public GuiBasicElement {
public:
	TextField(std::string initialText);
	virtual ~TextField();

	std::string getText() const;
	void setText(std::string const& text);

	virtual bool keyDown(int keyCode) override;
	virtual bool keyChar(char c) override;
	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) override;

	Event<void()> onTextChanged;
	Event<void()> onTrigger;

protected:
	static constexpr int maxTextbufferSize = 512;
	char textBuffer_[maxTextbufferSize];
	int bufPos_=0;
	int bufSize_=0;
};

#endif /* GUI_CONTROLS_TEXTFIELD_H_ */
