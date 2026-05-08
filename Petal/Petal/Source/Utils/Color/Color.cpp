#include "Color.h"

namespace Petal {
    Color::Color(float r, float g, float b, float a) : m_value(r, g, b, a) {
    }

    glm::vec4 Color::Vec4() const {
        return m_value;
    }

    float Color::Red() const {
        return m_value.r;
    }

    float Color::Green() const {
        return m_value.g;
    }

    float Color::Blue() const {
        return m_value.b;
    }

    float Color::Alpha() const {
        return m_value.a;
    }

    void Color::SetRed(float value) {
        m_value.r = value;
    }

    void Color::SetGreen(float value) {
        m_value.g = value;
    }

    void Color::SetBlue(float value) {
        m_value.b = value;
    }

    void Color::SetAlpha(float value) {
        m_value.a = value;
    }

    void Color::Set(float r, float g, float b, float a) {
        m_value = glm::vec4(r, g, b, a);
    }

    void Color::Set(const glm::vec4 &value) {
        m_value = value;
    }

    void Color::WriteFloats(float *floats) const {
        floats[0] = m_value.r;
        floats[1] = m_value.g;
        floats[2] = m_value.b;
        floats[3] = m_value.a;
    }
} // Petal
