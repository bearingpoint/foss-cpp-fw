#pragma once

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <utility>

/**
 * Implements a lock-less read multi-threaded map that is thread-safe and works without locking most of the time.
 * The map only locks when writing to it or when a thread refreshes its local view after the map has changed.
 *
 * To use this, declare a shared instance and then use thread_local View instances to access it.
 */
template<class K, class V>
class LocklessMTMap {
public:
	class View {
	public:
		explicit View(LocklessMTMap& source) : pSource_(&source) {
			refresh();
		}

		void set(K const& key, V&& value) {
			std::lock_guard<std::mutex> lock(pSource_->mutex_);
			pSource_->mainData_[key] = std::move(value);
			pSource_->mainVersion_.fetch_add(1, std::memory_order_release);
		}

		/**
		 * Stores value only when key is missing or value is greater than the currently stored value.
		 * Returns true when the map was updated. If pOutFinalValue is provided, it receives the value that
		 * remains stored for key after this call, regardless of whether the map was updated.
		 */
		bool setIfGreater(K const& key, V value, V* pOutFinalValue = nullptr) {
			std::lock_guard<std::mutex> lock(pSource_->mutex_);
			auto it = pSource_->mainData_.find(key);
			if (it == pSource_->mainData_.end()) {
				auto insertedIt = pSource_->mainData_.emplace(key, std::move(value)).first;
				if (pOutFinalValue) {
					*pOutFinalValue = insertedIt->second;
				}
				pSource_->mainVersion_.fetch_add(1, std::memory_order_release);
				return true;
			}

			if (value > it->second) {
				it->second = std::move(value);
				if (pOutFinalValue) {
					*pOutFinalValue = it->second;
				}
				pSource_->mainVersion_.fetch_add(1, std::memory_order_release);
				return true;
			}

			if (pOutFinalValue) {
				*pOutFinalValue = it->second;
			}
			return false;
		}

		V const* get(K const& key) {
			if (pSource_->mainVersion_.load(std::memory_order_acquire) > localVersion_) {
				refresh();
			}

			auto it = localCopy_.find(key);
			if (it != localCopy_.end()) {
				return &it->second;
			}
			return nullptr;
		}

	private:
		LocklessMTMap* pSource_;
		std::map<K, V> localCopy_;
		uint64_t localVersion_ { 0 };

		void refresh() {
			std::lock_guard<std::mutex> lock(pSource_->mutex_);
			localCopy_ = pSource_->mainData_;
			localVersion_ = pSource_->mainVersion_.load(std::memory_order_acquire);
		}
	};

private:
	std::atomic<uint64_t> mainVersion_ { 1 };
	std::map<K, V> mainData_;
	std::mutex mutex_;

	friend class View;
};
