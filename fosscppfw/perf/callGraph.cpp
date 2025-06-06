/*
 * callGraph.cpp
 *
 *  Created on: Aug 7, 2016
 *	  Author: bog
 */
#ifdef ENABLE_PERF_PROFILING

#include "./callGraph.h"
#include "./results.h"

#include <algorithm>

namespace perf {

thread_local std::shared_ptr<CallGraph> CallGraph::crtThreadInstance_;

CallGraph& CallGraph::getCrtThreadInstance() {
	if (!crtThreadInstance_) {
		crtThreadInstance_.reset(new CallGraph());
		Results::registerGraph(crtThreadInstance_);
	}
	return *crtThreadInstance_;
}

void CallGraph::pushSection(const char name[], bool deadTime) {
	// add to call-trees:
	std::vector<std::shared_ptr<sectionData>> &treeContainer =
		getCrtThreadInstance().crtStack_.empty()
			? getCrtThreadInstance().rootTrees_
			: getCrtThreadInstance().crtStack_.top()->callees_;
	auto treeIt = std::find_if(treeContainer.begin(), treeContainer.end(), [&name] (auto &sec) {
		return !std::strncmp(sec->name_, name, sizeof(sec->name_) / sizeof(sec->name_[0]) - 1);
	});
	if (treeIt == treeContainer.end()) {
		treeContainer.emplace_back(sectionData::make_shared(name));
		treeIt = treeContainer.end()-1;
	}
	(*treeIt)->deadTime_ = deadTime;
	getCrtThreadInstance().crtStack_.push(treeIt->get());
}

void CallGraph::popSection(uint64_t nanoseconds) {
	auto &stack = getCrtThreadInstance().crtStack_;
	// add time to secion, ++callCount
	sectionData *pCrt = stack.top();
	pCrt->executionCount_++;
	pCrt->nanoseconds_ += nanoseconds;
	stack.pop();

	// add time to flat list:
	auto &flatList = getCrtThreadInstance().flatSectionData_;

	auto it = flatList.find(pCrt->name_);
	if (it == flatList.end()) {
		it = flatList.emplace(pCrt->name_, sectionData::make_unique(pCrt->name_)).first;
	}
	it->second->executionCount_++;
	it->second->nanoseconds_ += nanoseconds;
}

} // namespace

#endif // ENABLE_PERF_PROFILING
