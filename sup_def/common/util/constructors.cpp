#include <sup_def/common/config.h>
#include <sup_def/common/util/util.hpp>

namespace SupDef
{
    namespace Util
    {
        static const std::function<void()> constructors[] = {
            []() { SupDef::configure_boost_contract(); },
            []() { } /* At least one */
        };
        static std::atomic<bool> constructors_called = false;

        void call_constructors()
        {
            if (constructors_called.exchange(true))
                return;
            for (auto& constructor : constructors)
            {
                constructor();
            }
        }
    }
};