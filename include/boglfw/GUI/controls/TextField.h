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
	enum Type {
		TEXT,
		NUMBER
	};

	TextField(Type type=TEXT, std::string initialText="");
	virtual ~TextField();

	Type type() const { return type_; }
	void setType(Type type);

	std::string text() const;
	void setText(std::string const& text);

	// these two are valid only for NUMBER type textField:
	float value() const;
	void setValue(float val);

	virtual bool keyDown(int keyCode) override;
	virtual bool keyChar(char c) override;
	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) override;

	// triggered whenever a character is added or removed from the field (even for NUMBER types)
	Event<void()> onChanged;
	// triggered when user presses RETURN key within the field
	Event<void()> onTrigger;

protected:
	static constexpr int maxTextbufferSize = 512;
	char textBuffer_[maxTextbufferSize];
	int bufPos_=0;
	int bufSize_=0;
	Type type_;
};

#endif /* GUI_CONTROLS_TEXTFIELD_H_ */
