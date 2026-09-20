//
// Created by Bibenpreet Cheema on 9/16/26.
//

#pragma once
#include "concepts.hpp"
#include <mutex>
#include <vector>
#include<condition_variable>
#include <utility>
#include <stdexcept>

namespace RingBuffer {
    template <Bufferable T>
    class ThreadSafeRingBuffer {
        std::size_t capacity;
        std::vector<T> bufferStorage;
        std::size_t head{0};
        std::size_t tail{0};
        std::size_t count{0};
        bool is_closed{false};

        std::mutex mtx;
        std::condition_variable cv_not_empty;
        std::condition_variable cv_not_full;

        public:
        explicit ThreadSafeRingBuffer(std::size_t cap)
            : capacity{cap}, bufferStorage(cap) {}

        void push(T item) {
            std::unique_lock<std::mutex> lock(mtx);
            cv_not_full.wait(lock, [this] () {return count < capacity || is_closed;});
            if (is_closed == true) {
                throw std::runtime_error("Cannot push on a closed Buffer");
            }
            bufferStorage[tail] = std::move(item);
            tail = (tail + 1) % capacity;
            count++;
            lock.unlock();
            cv_not_empty.notify_one();

        }

        template <typename...Args>
        void emplace(Args&&...args) {
            std::unique_lock<std::mutex> lock(mtx);
            cv_not_full.wait(lock, [this] () {return count < capacity || is_closed;});
            if (is_closed == true) {
                throw std::runtime_error("Cannot push on a closed Buffer");
            }
            bufferStorage[tail] = T(std::forward<Args>(args)...);
            tail = (tail + 1) % capacity;
            count++;
            lock.unlock();
            cv_not_empty.notify_one();
        }

        bool pop(T& item) {
            std::unique_lock<std::mutex> lock(mtx);
            cv_not_empty.wait(lock,[this] () {return count > 0|| is_closed;});
            if (count == 0 && is_closed) {
                return false;
            }

            item = std::move(bufferStorage[head]);
            head = (head + 1) % capacity;
            count--;

            lock.unlock();
            cv_not_full.notify_one();
            return true;
        }
        bool try_push(T item) {
            std::unique_lock<std::mutex> lock(mtx);
            if (count == capacity || is_closed) {
                return false;
            }

            bufferStorage[tail] = std::move(item);
            tail = (tail + 1) % capacity;
            count++;

            lock.unlock();
            cv_not_empty.notify_one();
            return true;
        }
        bool try_pop(T& item) {
            std::unique_lock<std::mutex> lock(mtx);
            if (count == 0 || is_closed) {
                return false;
            }
            item = std::move(bufferStorage[head]);
            head = (head + 1) % capacity;
            count--;

            lock.unlock();
            cv_not_full.notify_one();
            return true;
        }

        void close() {
            std::unique_lock<std::mutex> lock(mtx);
            if (is_closed ) {return;}

            is_closed = true;
            lock.unlock();
            cv_not_full.notify_all();
            cv_not_empty.notify_all();

        }
    };
}


