#include <sup_def/common/sup_def.hpp>

namespace SupDef
{
    std::mutex ErrorPrinterBase::mtx = std::mutex();
}