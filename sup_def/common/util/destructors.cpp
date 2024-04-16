#include <sup_def/common/config.h>
#include <sup_def/common/util/util.hpp>

namespace SupDef
{
    namespace Util
    {
        static const std::function<void()> destructors[] = {
            []() { }
        };
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