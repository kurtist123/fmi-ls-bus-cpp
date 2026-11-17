/**
 * @file frame_queue.hpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Thread-safe bounded FIFO queue with overflow policies
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#ifndef FMI_LS_BUS_CHANNEL_FRAME_QUEUE_HPP
#define FMI_LS_BUS_CHANNEL_FRAME_QUEUE_HPP

#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>

namespace fmi_ls_bus::channel
{

/**
 * @brief Thread-safe bounded FIFO queue for buffering bus frames and operations
 *
 * @tparam T Element type stored in queue
 */
template <typename T>
class FrameQueue
{
public:
    /**
     * @brief Constructs a bounded frame queue with the specified maximum
     * capacity
     *
     * @param capacity Maximum number of elements allowed in queue
     */
    explicit FrameQueue(size_t capacity = 1'024)
        : capacity_(capacity)
    {
    }

    /**
     * @brief Pushes an item into the queue by copy if capacity permits
     *
     * @param item Const reference to item to push
     * @return True if item was queued, false if queue was at capacity
     */
    bool push(const T& item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.size() >= capacity_)
        {
            return false;
        }
        queue_.push_back(item);
        return true;
    }

    /**
     * @brief Pushes an item into the queue by move if capacity permits
     *
     * @param item Rvalue reference to item to push
     * @return True if item was queued, false if queue was at capacity
     */
    bool push(T&& item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.size() >= capacity_)
        {
            return false;
        }
        queue_.push_back(std::move(item));
        return true;
    }

    /**
     * @brief Pops and returns the front item from the queue
     *
     * @return Optional containing front item, or std::nullopt if queue is empty
     */
    std::optional<T> pop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
        {
            return std::nullopt;
        }
        T item = std::move(queue_.front());
        queue_.pop_front();
        return item;
    }

    /**
     * @brief Checks if queue contains zero elements
     *
     * @return True if empty, false otherwise
     */
    [[nodiscard]]
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief Returns current number of elements in queue
     *
     * @return Element count
     */
    [[nodiscard]]
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    /**
     * @brief Removes all elements from the queue
     */
    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }

    /**
     * @brief Returns maximum capacity of queue
     *
     * @return Maximum element capacity
     */
    [[nodiscard]]
    size_t capacity() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return capacity_;
    }

    /**
     * @brief Updates the maximum capacity of the queue
     *
     * @param new_capacity New maximum element capacity
     */
    void set_capacity(size_t new_capacity)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        capacity_ = new_capacity;
    }

    /**
     * @brief Drains all queued elements into a vector and clears queue
     *
     * @return Vector containing all previously queued elements in order
     */
    std::vector<T> drain()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<T> items;
        items.reserve(queue_.size());
        while (!queue_.empty())
        {
            items.push_back(std::move(queue_.front()));
            queue_.pop_front();
        }
        return items;
    }

private:
    mutable std::mutex mutex_;
    std::deque<T> queue_;
    size_t capacity_;
};

}  // namespace fmi_ls_bus::channel

#endif  // FMI_LS_BUS_CHANNEL_FRAME_QUEUE_HPP
