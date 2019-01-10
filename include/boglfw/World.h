/*
 * World.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "entities/Entity.h"
#include "SpatialCache.h"
#include "input/operations/IOperationSpatialLocator.h"
#include "utils/MTVector.h"
#include "utils/Event.h"

#ifdef WITH_BOX2D
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#endif // WITH_BOX2D

#include <vector>
#include <memory>
#include <atomic>
#include <map>

#ifdef WITH_BOX2D
class b2World;
class b2Body;
struct b2AABB;
class PhysDestroyListener;
#endif // WITH_BOX2D

class Viewport;

struct WorldConfig {
	bool disableParallelProcessing = false;	// set to true to disable parallel (multi-threaded) update of entities
	bool disableUserEvents = false;			// set to true to disable propagation of user events
	bool drawBoundaries = true;				// draw world boundaries
	float extent_Xn = -10;
	float extent_Xp = 10;
	float extent_Yn = -10;
	float extent_Yp = 10;
	float extent_Zn = -10;
	float extent_Zp = 10;
};

class World 
#ifdef WITH_BOX2D
: public IOperationSpatialLocator 
#endif // WITH_BOX2D
{
public:
	static void setConfig(WorldConfig cfg);

	static World& getInstance();
	virtual ~World();

	// delete all entities and reset state.
	void reset();

	// set new world spatial extents
	void setBounds(float left, float right, float top, float bottom, float front, float back);

#ifdef WITH_BOX2D
	b2Body* getBodyAtPos(glm::vec2 const& pos) override;
	void getBodiesInArea(glm::vec2 const& pos, float radius, bool clipToCircle, std::vector<b2Body*> &outBodies);

	void setPhysics(b2World* physWld);
	void setDestroyListener(PhysDestroyListener *listener) { destroyListener_ = listener; }
	PhysDestroyListener* getDestroyListener() { return destroyListener_; }
	b2World* getPhysics() { return physWld_; }
	b2Body* getGroundBody() { return groundBody_; }
#endif // WITH_BOX2D

	void takeOwnershipOf(std::shared_ptr<Entity> e);
	void destroyEntity(Entity* e);

	// get all entities that match ALL of the requested features
	void getEntities(std::vector<Entity*> &out, unsigned* filterTypes, unsigned filterTypesCount, Entity::FunctionalityFlags filterFlags = Entity::FunctionalityFlags::NONE);

#ifdef WITH_BOX2D
	// get all entities in a specific area that match ALL of the requested features
	void getEntitiesInBox(std::vector<Entity*> &out, unsigned* filterTypes, unsigned filterTypesCount, Entity::FunctionalityFlags filterFlags, glm::vec2 const& pos, float radius, bool clipToCircle);
#endif // WITH_BOX2D

	void update(float dt);
	void draw(Viewport* vp);

	// this is thread safe by design; if called from the synchronous loop that executes deferred actions, it's executed immediately (if delayFrames=0),
	// else it's added to the queue
	// delayFrames - number of frames to delay the execution of the action
	void queueDeferredAction(std::function<void()> &&fun, int delayFrames=0);

	bool hasQueuedDeferredActions() const { return deferredActions_.size() > 0; }

	int registerEventHandler(std::string eventName, std::function<void(int param)> handler);
	void removeEventHandler(std::string eventName, int handlerId);
	void triggerEvent(std::string eventName, int param = 0);

#ifdef DEBUG
	static void assertOnMainThread() {
		assert(std::this_thread::get_id() == getInstance().ownerThreadId_);
	}
#else
	static void assertOnMainThread() {
		throw std::runtime_error(std::string("Don't call this method on Release builds! : ") + __PRETTY_FUNCTION__ + " : " + std::to_string(__LINE__));
	}
#endif

protected:
#ifdef WITH_BOX2D
	b2World* physWld_;
	b2Body* groundBody_;
	PhysDestroyListener *destroyListener_ = nullptr;
	SpatialCache spatialCache_;
#endif // WITH_BOX2D
	
	std::vector<std::shared_ptr<Entity>> entities_;
	std::vector<Entity*> entsToUpdate_;
	std::vector<Entity*> entsToDraw_;
	MTVector<Entity*> entsToDestroy_;
	MTVector<std::shared_ptr<Entity>> entsToTakeOver_;
	int frameNumber_ = 0;
	float extentXn_, extentXp_, extentYn_, extentYp_, extentZn_, extentZp_;
#ifdef DEBUG
	std::thread::id ownerThreadId_;
#endif

	// this holds actions deferred from the multi-threaded update which will be executed synchronously at the end on a single thread
	MTVector<std::pair<std::function<void()>, int>> deferredActions_;
	decltype(deferredActions_) pendingActions_;
	std::atomic<bool> executingDeferredActions_ { false };

	std::map<std::string, Event<void(int param)>> mapUserEvents_;

	void destroyPending();
	void takeOverPending();

#ifdef WITH_BOX2D
	void getFixtures(std::vector<b2Fixture*> &out, b2AABB const& aabb);
#endif // WITH_BOX2D

	bool testEntity(Entity &e, unsigned* filterTypes, unsigned filterTypesCount, Entity::FunctionalityFlags filterFlags);

private:
	World();
};

#endif /* WORLD_H_ */
