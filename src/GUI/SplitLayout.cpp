#include <boglfw/GUI/SplitLayout.h>
#include <boglfw/GUI/FillLayout.h>
#include <boglfw/utils/log.h>
#include <boglfw/math/math3D.h>

SplitLayout::SplitLayout()
	: first_(new FillLayout())
	, second_(new FillLayout())
{
	// TOOD when the owner of this is set, must propagate it to all children layouts
}

void SplitLayout::update(elementIterator first, elementIterator end, glm::vec2 clientSize) {
	glm::vec2 secondOffset;
	glm::vec2 firstSize;
	glm::vec2 secondSize;
	switch (direction_) {
	case HORIZONTAL:
		secondOffset = {splitOffset_.get(FlexCoord::X_LEFT, clientSize), 0};
		firstSize = {secondOffset.x, clientSize.y};
		secondSize = {clientSize.x - firstSize.x, clientSize.y};
	break;
	case VERTICAL:
		secondOffset = {0, splitOffset_.get(FlexCoord::Y_TOP, clientSize)};
		firstSize = {clientSize.x, secondOffset.y};
		secondSize = {clientSize.x, clientSize.y - firstSize.y};
	break;
	default:
		ERROR("Invalid SplitLayout direction " << (int)direction_);
		return;
	}
	unsigned nFirst = min(splitPoint_, (unsigned)(end - first));
	first_->update(first, first + nFirst, firstSize);
	unsigned nSecond = end - first - nFirst;
	if (nSecond > 0) {
		second_->update(first + nSecond, end, secondSize);
		// must apply position offset to all elements from second
		for (auto it = first + nSecond; it != end; ++it)
			setElementPosition(*it, (*it)->computedPosition() + secondOffset);
	}
}

void SplitLayout::setSplitCount(unsigned count) {
	splitPoint_ = count;
	refresh();
}

void SplitLayout::setSplitPosition(FlexCoord coord) {
	splitOffset_ = coord;
	refresh();
}

void SplitLayout::setDirection(SplitDirection dir) {
	direction_ = dir;
	refresh();
}

void SplitLayout::setFirstSub(std::shared_ptr<Layout> first) {
	first_.swap(first);
	refresh();
}
void SplitLayout::setSecondSub(std::shared_ptr<Layout> second) {
	second_.swap(second);
	refresh();
}
