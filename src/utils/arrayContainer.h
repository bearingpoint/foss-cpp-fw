#ifndef __ARRAY_CONTAINER_H__
#define __ARRAY_CONTAINER_H__

// this class provides a light weight STL-style container over an underlying C-style array without copying data.
// WARNING: This class doesn't release memory upon destruction, the ownership of the underlying array belongs to the caller.
template<class C>
class arrayContainer {
public:
	using value_type = C;

	arrayContainer(C* start, unsigned count) : start_(start), n_(count) {
	}

	arrayContainer() = delete;
	arrayContainer(arrayContainer<C> const&) = delete;

	arrayContainer(arrayContainer<C> && o) : start_(o.start_), n_(o.n_) {
		o.start_ = nullptr;
		o.n_ = 0;
	}

	~arrayContainer() {
		start_ = nullptr;
		n_ = 0;
	}

	unsigned size() const {
		return n_;
	}

	bool empty() const {
		return n_ == 0;
	}

	C& operator[](unsigned i) {
		assert(i < n_);
		return *(start_ + i);
	}

	const C& operator[](unsigned i) const {
		assert(i < n_);
		return *(start_ + i);
	}

	class iterator {
	public:
		const C& operator*() const {
			return *(owner_.start_ + offset_);
		}
		C& operator* () {
			return *(owner_.start_ + offset_);
		}
		C& operator + (unsigned count) const {
			return iterator(owner_, offset_ + count);
		}
		bool operator == (iterator const& i) const {
			return &owner_ == &i.owner_ && offset_ == i.offset_;
		}
		bool operator != (iterator const& i) const {
			return !operator==(i);
		}
		bool operator < (iterator const& i) const {
			assert(&owner_ == &i.owner_);
			return offset_ < i.offset_;
		}
	private:
		friend class arrayContainer;

		iterator(arrayContainer &owner, unsigned offset) : owner_(owner), offset_(offset) {}

		arrayContainer &owner_;
		unsigned offset_ = 0;
	};

	iterator begin() {
		return iterator(*this, 0);
	}

	const iterator cbegin() const {
		return iterator(*this, 0);
	}

	iterator end() {
		return iterator(*this, n_);
	}

	const iterator cend() const {
		return iterator(*this, n_);
	}

private:
	C* start_ = nullptr;
	unsigned n_ = 0;

	friend class iterator;
};

#endif // __ARRAY_CONTAINER_H__
