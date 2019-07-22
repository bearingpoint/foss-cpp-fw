#pragma once

#include <boglfw/GUI/Layout.h>

class GridLayout : public Layout {
public:

	enum FlowDirection {
		ROW,
		COLUMN
	};

	GridLayout() {}
	~GridLayout() {}

	FlowDirection flowDirection() const { return flowDir_; }
	void setFlowDirection(FlowDirection);

	unsigned elementsPerLine() const { return elemsPerLine_; }
	// set the number of elements per "line";
	// the meaning of line depends on flowDirection - thus for a ROW flow, line means row,
	// and for a COLUMN flow, line means column;
	// there will me at most [count] elements on each line, the rest will go to the next lines
	void setElementsPerLine(unsigned count);

	FlexCoordPair cellPadding() const { return cellPadding_; }
	// sets the padding within each cell;
	// [padding.x] is used for both left and right, and
	// [padding.y] is used for both top and bottom
	void setCellPadding(FlexCoordPair padding);

	virtual void update(elementIterator first, elementIterator end, glm::vec2 clientSize) override;

private:
	FlowDirection flowDir_ = ROW;
	unsigned elemsPerLine_ = 3;
	FlexCoordPair cellPadding_;
};
