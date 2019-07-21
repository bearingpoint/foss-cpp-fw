#pragma once

#include <boglfw/GUI/Layout.h>
#include <boglfw/utils/FlexibleCoordinate.h>

class SplitLayout : public Layout {
public:

	enum SplitDirection {
		HORIZONTAL,
		VERTICAL,
	};

	SplitLayout();
	~SplitLayout() {}

	// sets the layout to use for the first sub-space
	void setFirstSub(std::shared_ptr<Layout> first);
	// sets the layout to use for the second sub-space
	void setSecondSub(std::shared_ptr<Layout> second);

	void setDirection(SplitDirection dir);
	SplitDirection direction() const { return direction_; }

	// the physical position where the layout is split, epxressed from the top or left edge (depending on direction);
	void setSplitPosition(FlexCoord coord);
	FlexCoord splitPosition() const { return splitOffset_; }

	// how many child elements will be allocated to the first sub-space; all the rest go to the second subspace
	void setSplitCount(unsigned count);
	unsigned splitCount() { return splitPoint_; }

	virtual void update(elementIterator first, elementIterator end, glm::vec2 clientSize) override;

protected:
	void setOwner(GuiContainerElement *pOwner) override;

private:
	SplitDirection direction_ = HORIZONTAL;
	FlexCoord splitOffset_ = {50, FlexCoord::PERCENT}; // the size of the first sub-space relative to the full size of the layout
	unsigned splitPoint_ = 1; // how many elements are pushed into the first subspace (all the rest go into the second)

	std::shared_ptr<Layout> first_;
	std::shared_ptr<Layout> second_;
};
