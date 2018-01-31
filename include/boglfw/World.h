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
#include "renderOpenGL/RenderContext.h"

#include <Box2D/Dynamics/b2WorldCallbacks.h>

#include <vector>
#include <memory>
#include <atomic>

class b2World;
class b2Body;
struct b2AABB;
class PhysDestroyListener;

struct WorldConfig {
	bool disableParallelProcessing = false;	// set to true to disable parallel (multi-threaded) update of entities
	float extent_Xn = -10;
	float extent_Xp = 10;
	float extent_Yn = -10;
	float extent_Yp = 10;
};

class World : public IOperationSpatialLocator {
public:
	static void setConfig(WorldConfig cfg);

	static World& getInstance();
	virtual ~World();

	/**
	 * delete all entities and reset state.
	 * set new world spatial extents
	 */
	void reset();

	void setBounds(float left, float right, float top, float bottom);

	b2Body* getBodyAtPos(glm::vec2 const& pos) override;
	void getBodiesInArea(glm::vec2 const& pos, float radius, bool clipToCircle, std::vector<b2Body*> &outBodies);

	void setPhysics(b2World* physWld);
	void setDestroyListener(PhysDestroyListener *listener) { destroyListener_ = listener; }
	PhysDestroyListener* getDestroyListener() { return destroyListener_; }
	b2World* getPhysics() { return physWld; }
	b2Body* getGroundBody() { return groundBody; }

	void takeOwnershipOf(std::unique_ptr<Entity> &&e);
	void destroyEntity(Entity* e);

	// get all entities that match ALL of the requested features
	void getEntities(std::vector<Entity*> &out, EntityType filterTypes, Entity::FunctionalityFlags filterFlags = Entity::FunctionalityFlags::NONE);

	// get all entities in a specific area that match ALL of the requested features
	void getEntitiesInBox(std::vector<Entity*> &out, EntityType filterTypes, Entity::FunctionalityFlags filterFlags, glm::vec2 const& pos, float radius, bool clipToCircle);

	void update(float dt);
	void draw(RenderContext const& ctx);

	// this is thread safe by design; if called from the synchronous loop that executes deferred actions, it's executed immediately, else added to the queue
	void queueDeferredAction(std::function<void()> &&fun);

#ifdef DEBUG
	static void assertOnMainThread() {
		assert(std::this_thread::get_id() == getInstance().ownerThreadId_);
	}
#else
	static void assertOnMainThread() {
		throw std::runtime_error("Don't call this method on Release builds!");
	}
#endif

protected:
	b2World* physWld;
	b2Body* groundBody;
	std::vector<std::unique_ptr<Entity>> entities;
	std::vector<Entity*> entsToUpdate;
	std::vector<Entity*> entsToDraw;
	MTVector<Entity*> entsToDestroy;
	MTVector<std::unique_ptr<Entity>> entsToTakeOver;
	PhysDestroyListener *destroyListener_ = nullptr;
	int frameNumber_ = 0;
	float extentXn_, extentXp_, extentYn_, extentYp_;
	SpatialCache spatialCache_;
#ifdef DEBUG
	std::thread::id ownerThreadId_;
#endif

	// this holds actions deferred from the multi-threaded update which will be executed synchronously at the end on a single thread
	MTVector<std::function<void()>> deferredActions_;
	std::atomic<bool> executingDeferredActions_ { false };

	void destroyPending();
	void takeOverPending();

	void getFixtures(std::vector<b2Fixture*> &out, b2AABB const& aabb);
	bool testEntity(Entity &e, EntityType filterTypes, Entity::FunctionalityFlags filterFlags);

private:
	World();
};

#endif /* WORLD_H_ */

