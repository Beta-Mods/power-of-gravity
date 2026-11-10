#pragma once

#include <functional>
#include <utility>

namespace SKSE
{
    namespace log
    {
        template <class... Args>
        void info(Args&&...)
        {}

        template <class... Args>
        void debug(Args&&...)
        {}

        template <class... Args>
        void warn(Args&&...)
        {}

        template <class... Args>
        void error(Args&&...)
        {}

        template <class... Args>
        void critical(Args&&...)
        {}
    }

    class TaskInterface
    {
    public:
        void AddTask(std::function<void()> a_task) const
        {
            a_task();
        }
    };

    [[nodiscard]] inline const TaskInterface* GetTaskInterface() noexcept
    {
        static TaskInterface singleton;
        return &singleton;
    }
}
