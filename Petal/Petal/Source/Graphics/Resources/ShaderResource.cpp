#include "ShaderResource.h"

std::string Petal::ShaderResource::ToString() {
    return std::format(
        "ShaderResource(Name={}, Type={}, BindingIndex={}, BindingSet={}, Stages={})",
        Name,
        Type,
        BindingIndex,
        BindingSet,
        Stages
    );
}
