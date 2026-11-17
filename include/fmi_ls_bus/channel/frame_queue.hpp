#ifndef FMI_LS_BUS_CHANNEL_FRAME_QUEUE_HPP
#define FMI_LS_BUS_CHANNEL_FRAME_QUEUE_HPP

#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>

namespace fmi_ls_bus::channel
{

template <typename T>
class FrameQueue
{
public:
    explicit FrameQueue(size_t capacity = 1'024)
        : capacity_(capacity)
    {
    }

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

    [[nodiscard]]
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    [[nodiscard]]
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }

    [[nodiscard]]
    size_t capacity() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return capacity_;
    }

    void set_capacity(size_t new_capacity)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        capacity_ = new_capacity;
    }

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
