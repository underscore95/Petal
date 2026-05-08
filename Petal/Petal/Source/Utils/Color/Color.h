#pragma once

#include "pch.h"

namespace Petal {
    class Color {
    public:
        Color(float r, float g, float b, float a);

    public:
        glm::vec4 Vec4() const;

        float Red() const;

        float Green() const;

        float Blue() const;

        float Alpha() const;

        void SetRed(float value);

        void SetGreen(float value);

        void SetBlue(float value);

        void SetAlpha(float value);

        void Set(float r, float g, float b, float a);

        void Set(const glm::vec4 &value);

        // Write the colour to an array of 4 32-bit floats
        void WriteFloats(float *floats) const;

    private:
        glm::vec4 m_value;
    };
} // Petal
