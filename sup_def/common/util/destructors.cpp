#include <sup_def/common/config.h>
#include <sup_def/common/util/util.hpp>

namespace SupDef
{
    namespace Util
    {
        symbol_unused
        symbol_keep
        static_init_priority(101)
        static const std::function<void()> destructors[] = {
            []() { }
        };
        symbol_unused
        symbol_keep
        static_init_priority(101)
        static std::atomic<bool> destructors_called = false;

        void call_destructors()
        {
            if (destructors_called.load() == true)
                return;
            for (auto& destructor : destructors)
            {
                destructor();
            }
            destructors_called.store(true);
        }
    }
};