/*
 * drawable.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bogdan
 */

#ifndef DRAWABLE_H_
#define DRAWABLE_H_

#include <memory>
#include <type_traits>

class Viewport;

// any callable object that has a void operator ()(Viewport*) is drawable
template<class Callable, typename std::result_of<Callable(Viewport*)>::type* = nullptr>
void draw(Callable *fn, Viewport* vp) {
	(*fn)(vp);
}

// special handling for plain function pointers:
template<class FnType=void(*)(Viewport*)>
void draw(FnType fn, Viewport* vp) {
	(*fn)(vp);
}

// class types that define a draw(Viewport*) method are also drawables
template<class C, decltype(&C::draw) dummy = nullptr>
void draw(C* c, Viewport* vp) {
	c->draw(vp);
}

// this class can be instantiated with any argument of type T* that satisfies the drawable requirements.
// either one of these must be true:
//	A. the type T is a class that defines a method
//		void T::draw(Viewport*)
//  B. there exists a global template specialization function for the one on top such as:
//      void draw(T*, Viewport*)
class drawable {
public:
	template<typename T>
	drawable(T* t)
		: self_(new model_t<T>(t)) {
	}

	drawable(const drawable& w) : self_(w.self_->copy()) {}
	drawable(drawable &&w) : self_(std::move(w.self_)) {}
	drawable& operator = (drawable const &w) { self_ = decltype(self_)(w.self_->copy()); return *this; }
	drawable& operator = (drawable &&w) { self_ = std::move(w.self_); return *this; }

	bool equal_value(drawable const& w) const {
		return self_->getRawPtr() == w.self_->getRawPtr();
	}

	void draw(Viewport* vp) const {
		self_->draw_(vp);
	}

private:
	struct concept_t {
		virtual ~concept_t() noexcept = default;
		virtual void draw_(Viewport* vp) const = 0;
		virtual concept_t* copy()=0;
		virtual void* getRawPtr() = 0;
	};
	template<typename T>
	struct model_t : concept_t {
		T* obj_;
		model_t(T* x) : obj_(x) {}
		~model_t() noexcept {};
		void draw_(Viewport* vp) const override {
			drawImpl(obj_, vp, true);
		}
		concept_t* copy() override {
			return new model_t<T>(obj_);
		}
		void* getRawPtr() override {
			return (void*)obj_;
		}

		template<typename T1>
		static decltype(&T1::draw) drawImpl(T1* t, Viewport* vp, bool dummyToUseMember) {
			t->draw(vp);
			return nullptr;
		}
		template<typename T1>
		static void drawImpl(T1* t, Viewport* vp, ...) {
			::draw(t, vp);
		}
	};

	std::unique_ptr<concept_t> self_;
};

#endif /* DRAWABLE_H_ */
