#include "ShaderResource.h"

std::string Petal::ShaderResource::ToString() {
    return std::format(
        "ShaderResource(Name={}, Type={}, Size={}, BindingIndex={}, BindingSet={}, Stages={})",
        Name,
        Type,
        Size,
        BindingIndex,
        BindingSet,
        Stages
    );
}
