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

class RenderContext;

// any callable object that has a void operator ()(RenderContext const&) is drawable
template<class Callable, typename std::result_of<Callable(RenderContext const&)>::type* = nullptr>
void draw(Callable *fn, RenderContext const& ctx) {
	(*fn)(ctx);
}

// special handling for plain function pointers:
inline void draw(void(*fn)(RenderContext const&), RenderContext const& ctx) {
	(*fn)(ctx);
}

// class types that define a draw(RenderContext const&) method are also drawables
template<class C, decltype(&C::draw) dummy = nullptr>
void draw(C* c, RenderContext const& ctx) {
	c->draw(ctx);
}

// this class can be instantiated with any argument of type T* that satisfies the drawable requirements.
// either one of these must be true:
//	A. the type T is a class that defines a method
//		void T::draw(RenderContext const&)
//  B. there exists a global template specialization function for the one on top such as:
//      void draw(T*, RenderContext const&)
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

	void draw(RenderContext const& ctx) const {
		self_->draw_(ctx);
	}

private:
	struct concept_t {
		virtual ~concept_t() noexcept = default;
		virtual void draw_(RenderContext const& ctx) const = 0;
		virtual concept_t* copy()=0;
		virtual void* getRawPtr() = 0;
	};
	template<typename T>
	struct model_t : concept_t {
		T* obj_;
		model_t(T* x) : obj_(x) {}
		~model_t() noexcept {};
		void draw_(RenderContext const& ctx) const override {
			drawImpl(obj_, ctx, true);
		}
		concept_t* copy() override {
			return new model_t<T>(obj_);
		}
		void* getRawPtr() override {
			return (void*)obj_;
		}

		template<typename T1>
		static decltype(&T1::draw) drawImpl(T1* t, RenderContext const& ctx, bool dummyToUseMember) {
			t->draw(ctx);
			return nullptr;
		}
		template<typename T1>
		static void drawImpl(T1* t, RenderContext const& ctx, ...) {
			::draw(t, ctx);
		}
	};

	std::unique_ptr<concept_t> self_;
};

#endif /* DRAWABLE_H_ */
