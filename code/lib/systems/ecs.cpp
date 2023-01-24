#include "ecs.h"

namespace ecs {
    ComponentFlags componentMaskFromGuid(Guid componentGuid)
    {
        return (static_cast<u64>(1) << componentGuid);
    }
}
