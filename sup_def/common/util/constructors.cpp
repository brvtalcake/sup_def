#include <sup_def/common/config.h>
#include <sup_def/common/util/util.hpp>

namespace SupDef
{
    namespace Util
    {
        symbol_unused
        symbol_keep
        static_init_priority(101)
        static const std::function<void()> constructors[] = {
            []() { SupDef::configure_boost_contract(); },
            []() { } /* At least one */
        };
        symbol_unused
        symbol_keep
        static_init_priority(101)
        static std::atomic<bool> constructors_called = false;

        void call_constructors()
        {
            if (constructors_called.load() == true)
                return;
            for (auto& constructor : constructors)
            {
                constructor();
            }
            constructors_called.store(true);
        }
    }
};