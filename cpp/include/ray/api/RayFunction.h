
#pragma once

#include "RayObject.h"

namespace ray {

template <typename F>
class RayFunction : public RayObject<F> {
 public:
  RayFunction(UniqueId &&id);
  RayFunction(UniqueId &id);
};
}
