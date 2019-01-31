/*
 * World.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#include <boglfw/World.h>
#include <boglfw/entities/Entity.h>
#include <boglfw/math/math3D.h>
#include <boglfw/Infrastructure.h>
#include <boglfw/renderOpenGL/Shape3D.h>

#include <boglfw/utils/bitFlags.h>
#include <boglfw/utils/parallel.h>
#include <boglfw/utils/assert.h>
#include <boglfw/utils/log.h>

#include <boglfw/perf/marker.h>

#include <glm/glm.hpp>

#ifdef WITH_BOX2D
#include <boglfw/math/box2glm.h>
#include <boglfw/physics/PhysicsBody.h>
#include <Box2D/Box2D.h>
#endif

#include <algorithm>
#include <atomic>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static std::atomic_bool initialized { false };
static WorldConfig config;

void World::setConfig(WorldConfig cfg) {
	if (initialized.load())
		throw std::runtime_error("Called World::setConfig after World has been instantiated!!");
	config = cfg;
}

World::World()
#ifdef WITH_BOX2D
	: physWld_(nullptr)
	, groundBody_(nullptr)
	, 
#else
	:
#endif // WITH_BOX2D
	  entsToDestroy_(1024)
	, entsToTakeOver_(1024)
	, deferredActions_(8192)
	, pendingActions_(8192)
{
#ifdef DEBUG
	ownerThreadId_ = std::this_thread::get_id();
	LOGLN("World bound to thread " << ownerThreadId_);
#endif
	extentXn_ = config.extent_Xn;
	extentXp_ = config.extent_Xp;
	extentYn_ = config.extent_Yn;
	extentYp_ = config.extent_Yp;
	extentZn_ = config.extent_Zn;
	extentZp_ = config.extent_Zp;
	initialized.store(true);
}

#ifdef WITH_BOX2D
void World::setPhysics(b2World* phys) {
	physWld_ = phys;
	b2BodyDef gdef;
	gdef.type = b2_staticBody;
	groundBody_ = physWld_->CreateBody(&gdef);
}
#endif // WITH_BOX2D

World& World::getInstance() {
    static World instance;
	return instance;
}

World::~World() {
	reset();
}

void World::setBounds(float left, float right, float top, float bottom, float front, float back) {
	extentXn_ = left;
	extentXp_ = right;
	extentYp_ = top;
	extentYn_ = bottom;
	extentZn_ = back;
	extentZp_ = front;
#ifdef WITH_BOX2D
	// reconfigure cache:
	spatialCache_ = SpatialCache(left, right, top, bottom);
#endif
}

void World::reset() {
#ifdef DEBUG
	assertOnMainThread();
#endif
	deferredActions_.clear();
	pendingActions_.clear();
	for (auto &e : entities_) {
		e->markedForDeletion_= true;
		e.reset();
	}
	for (auto &e : entsToTakeOver_) {
		e->markedForDeletion_ = true;
		e.reset();
	}
	entities_.clear();
	entsToTakeOver_.clear();
	entsToDestroy_.clear();
	entsToDraw_.clear();
	entsToUpdate_.clear();
}

#ifdef WITH_BOX2D
void World::getFixtures(std::vector<b2Fixture*> &out, const b2AABB& aabb) {
	PERF_MARKER_FUNC;
	class cbWrap : public b2QueryCallback {
	public:
		cbWrap(std::vector<b2Fixture*> &fixtures) : fixtures_(fixtures) {}
		/// b2QueryCallback::
		/// Called for each fixture found in the query AABB.
		/// @return false to terminate the query.
		bool ReportFixture(b2Fixture* fixture) override {
			fixtures_.push_back(fixture);
			return true;
		}

		std::vector<b2Fixture*> &fixtures_;
	} wrap(out);
	physWld_->QueryAABB(&wrap, aabb);
}

b2Body* World::getBodyAtPos(glm::vec2 const& pos) {
	PERF_MARKER_FUNC;
	b2AABB aabb;
	aabb.lowerBound = g2b(pos) - b2Vec2(0.005f, 0.005f);
	aabb.upperBound = g2b(pos) + b2Vec2(0.005f, 0.005f);
	static thread_local std::vector<b2Fixture*> b2QueryResult;
	b2QueryResult.clear();
	getFixtures(b2QueryResult, aabb);
	if (b2QueryResult.empty())
		return nullptr;
	PERF_MARKER("precisionTest");
	b2Body* ret = nullptr;
	for (b2Fixture* f : b2QueryResult) {
		if (f->TestPoint(g2b(pos))) {
			ret = f->GetBody();
			break;
		}
	}
	return ret;
}

void World::getBodiesInArea(glm::vec2 const& pos, float radius, bool clipToCircle, std::vector<b2Body*> &outBodies) {
	PERF_MARKER_FUNC;
	b2AABB aabb;
	aabb.lowerBound = g2b(pos) - b2Vec2(radius, radius);
	aabb.upperBound = g2b(pos) + b2Vec2(radius, radius);
	static thread_local std::vector<b2Fixture*> b2QueryResult;
	b2QueryResult.clear();
	getFixtures(b2QueryResult, aabb);
	for (b2Fixture* f : b2QueryResult) {
		if (clipToCircle) {
			PERF_MARKER("clipToCircle");
			if (glm::length(b2g(f->GetAABB(0).GetCenter()) - pos) > radius)
				continue;
		}
		outBodies.push_back(f->GetBody());
	}
}

#endif // WITH_BOX2D

void World::takeOwnershipOf(std::shared_ptr<Entity> e) {
	assertDbg(e != nullptr);
	e->managed_ = true;
	entsToTakeOver_.push_back(std::move(e));
}

void World::destroyEntity(Entity* e) {
	PERF_MARKER_FUNC;
	entsToDestroy_.push_back(e);
}

void World::destroyPending() {
	PERF_MARKER_FUNC;
	static decltype(entsToDestroy_) destroyNow(entsToDestroy_.getLockFreeCapacity());
	destroyNow.swap(entsToDestroy_);
	for (auto &e : destroyNow) {
		auto it = std::find_if(entities_.begin(), entities_.end(), [&] (auto &it) {
			return it.get() == e;
		});
		if (it != entities_.end()) {
			Entity::FunctionalityFlags flags = e->getFunctionalityFlags();
			if ((flags & Entity::FunctionalityFlags::UPDATABLE) != 0) {
				auto it = std::find(entsToUpdate_.begin(), entsToUpdate_.end(), e);
				assertDbg(it != entsToUpdate_.end());
				entsToUpdate_.erase(it);
			}
			if ((flags & Entity::FunctionalityFlags::DRAWABLE) != 0) {
				auto it = std::find(entsToDraw_.begin(), entsToDraw_.end(), e);
				assertDbg(it != entsToDraw_.end());
				entsToDraw_.erase(it);
			}
			entities_.erase(it); // this will also delete
//TODO optimize this, it will be O(n^2) - must move the pointer from entities to entsToDestroy when destroy()
		} else {
			auto it2 = std::find_if(entsToTakeOver_.begin(), entsToTakeOver_.end(), [&] (auto &it) {
				return it.get() == e;
			});
			if (it2 != entsToTakeOver_.end()) {
				(*it2).reset();
			} else {
				ERROR("[WARNING] World skip DESTROY unmanaged obj: "<<e);
			}
		}
	}
	destroyNow.clear();
}

void World::takeOverPending() {
	PERF_MARKER_FUNC;
	static decltype(entsToTakeOver_) takeOverNow(entsToTakeOver_.getLockFreeCapacity());
	takeOverNow.swap(entsToTakeOver_);
	for (auto &e : takeOverNow) {
		if (!e)
			continue;	// entity was destroyed in the mean time
		// add to update and draw lists if appropriate
		Entity::FunctionalityFlags flags = e->getFunctionalityFlags();
		if ((flags & Entity::FunctionalityFlags::DRAWABLE) != 0) {
			entsToDraw_.push_back(e.get());
		}
		if ((flags & Entity::FunctionalityFlags::UPDATABLE) != 0) {
			entsToUpdate_.push_back(e.get());
		}
		entities_.push_back(std::move(e));
	}
	takeOverNow.clear();
}

void World::update(float dt) {
	PERF_MARKER_FUNC;
	++frameNumber_;

	// delete pending entities:
	destroyPending();

	// take over pending entities:
	takeOverPending();

	// do the actual update on entities:
	do {
	PERF_MARKER("entities-update");

	auto pred = [dt] (Entity* e) {
		e->update(dt);
	};
	if (config.disableParallelProcessing) {
		for (auto e : entsToUpdate_)
			pred(e);
	} else
		parallel_for(
			entsToUpdate_.begin(), entsToUpdate_.end(),
			Infrastructure::getThreadPool(),
			pred
		);
	} while (0);

	// execute deferred actions synchronously:
	{
		PERF_MARKER("deferred-actions");
		executingDeferredActions_.store(true, std::memory_order_release);
		pendingActions_.clear();
		for (auto &a : deferredActions_) {
			if (a.second-- == 0)
				a.first();
			else
				pendingActions_.push_back(std::move(a));
		}
		deferredActions_.swap(pendingActions_);
		executingDeferredActions_.store(false, std::memory_order_release);
	}
}

void World::queueDeferredAction(std::function<void()> &&fun, int delayFrames) {
	if (executingDeferredActions_.load(std::memory_order_acquire)) {
		if (delayFrames == 0)
			fun();
		else
			pendingActions_.push_back(std::make_pair(std::move(fun), delayFrames));
	} else
		deferredActions_.push_back(std::make_pair(std::move(fun), delayFrames));
}

void World::draw(Viewport* vp) {
	PERF_MARKER_FUNC;
	// draw extent lines:
	if (config.drawBoundaries) {
		glm::vec3 lineColor(0.2f, 0, 0.8f);
		const float overflow = 1.1f;
		Shape3D::get()->drawLine(glm::vec3(extentXn_, extentYp_*overflow, extentZn_),
			glm::vec3(extentXn_, extentYn_*overflow, extentZn_), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXp_, extentYp_*overflow, extentZn_),
			glm::vec3(extentXp_, extentYn_*overflow, extentZn_), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXn_*overflow, extentYp_, extentZn_),
			glm::vec3(extentXp_*overflow, extentYp_, extentZn_), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXn_*overflow, extentYn_, extentZn_),
			glm::vec3(extentXp_*overflow, extentYn_, extentZn_), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXn_, extentYp_*overflow, extentZp_),
			glm::vec3(extentXn_, extentYn_*overflow, extentZp_), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXp_, extentYp_*overflow, extentZp_),
			glm::vec3(extentXp_, extentYn_*overflow, extentZp_), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXn_*overflow, extentYp_, extentZp_),
			glm::vec3(extentXp_*overflow, extentYp_, extentZp_), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXn_*overflow, extentYn_, extentZp_),
			glm::vec3(extentXp_*overflow, extentYn_, extentZp_), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXn_, extentYn_, extentZn_*overflow),
			glm::vec3(extentXn_, extentYn_, extentZp_*overflow), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXn_, extentYp_, extentZn_*overflow),
			glm::vec3(extentXn_, extentYp_, extentZp_*overflow), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXp_, extentYn_, extentZn_*overflow),
			glm::vec3(extentXp_, extentYn_, extentZp_*overflow), lineColor);
		Shape3D::get()->drawLine(glm::vec3(extentXp_, extentYp_, extentZn_*overflow),
			glm::vec3(extentXp_, extentYp_, extentZp_*overflow), lineColor);
	}
	// draw entities

	for (auto e : entsToDraw_) {
		PERF_MARKER((std::string("Entity draw: ") + std::to_string((int)e->getEntityType())).c_str());
		e->draw(vp);
	}
}

bool World::testEntity(Entity &e, unsigned* filterTypes, unsigned filterTypesCount, Entity::FunctionalityFlags filterFlags) {
	if ((e.getFunctionalityFlags() & filterFlags) != filterFlags)
		return false;
	if (filterTypesCount == 0)
		return true;
	for (unsigned i=0; i<filterTypesCount; i++)
		if (e.getEntityType() == filterTypes[i])
			return true;
	return false;
}


void World::getEntities(std::vector<Entity*> &out, unsigned* filterTypes, unsigned filterTypesCount, Entity::FunctionalityFlags filterFlags) {
	PERF_MARKER_FUNC;
	for (auto &e : entities_) {
		if (!e->isZombie() && testEntity(*e, filterTypes, filterTypesCount, filterFlags))
			out.push_back(e.get());
	}
	for (auto &e : entsToTakeOver_) {
		if (!e->isZombie() && testEntity(*e, filterTypes, filterTypesCount, filterFlags))
			out.push_back(e.get());
	}
}

#ifdef WITH_BOX2D
void World::getEntitiesInBox(std::vector<Entity*> &out, unsigned* filterTypes, unsigned filterTypesCount, Entity::FunctionalityFlags filterFlags,
		glm::vec2 const& pos, float radius, bool clipToCircle)
{
	PERF_MARKER_FUNC;
	spatialCache_.getCachedEntities(out, pos, radius, clipToCircle, frameNumber_,
		[this, filterTypes, filterTypesCount, filterFlags] (glm::vec2 const& pos, float radius, std::vector<Entity*> &out)
	{
		static thread_local std::vector<b2Body*> bodies;
		bodies.clear();
		getBodiesInArea(pos, radius, false, bodies);
		for (b2Body* b : bodies) {
			PhysicsBody* pb = PhysicsBody::getForB2Body(b);
			if (pb == nullptr)
				continue;
			Entity* ent = pb->getAssociatedEntity();
			if (ent && !ent->isZombie())
				out.push_back(ent);
		}
	}, [this, filterTypes, filterFlags] (Entity *e) {
		return testEntity(*e, filterTypes, filterTypesCount, filterFlags);
	});
}
#endif // WITH_BOX2D

int World::registerEventHandler(std::string eventName, std::function<void(int param)> handler) {
	return mapUserEvents_[eventName].add(handler);
}

void World::removeEventHandler(std::string eventName, int handlerId) {
	mapUserEvents_[eventName].remove(handlerId);
}

void World::triggerEvent(std::string eventName, int param) {
	if (!config.disableUserEvents)
		mapUserEvents_[eventName].trigger(param);
}
