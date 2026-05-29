#pragma once

#include "pch.h"

namespace Petal {
    // Represents a dynamically resizable array of values where indices never change: removing an element destroys it and marks it as available to be replaced by another
    // T - Type to store
    template<typename T>
    requires std::default_initializable<T>
    class StableVector {
    public:
        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T *;
            using reference = T &;

            Iterator(StableVector *owner, size_t index)
                : m_owner(owner), m_index(index) {
                SkipInvalid();
            }

            reference operator*() const {
                return m_owner->m_values[m_index];
            }

            pointer operator->() const {
                return &m_owner->m_values[m_index];
            }

            Iterator &operator++() {
                ++m_index;
                SkipInvalid();
                return *this;
            }

            Iterator operator++(int) {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const Iterator &other) const {
                return m_owner == other.m_owner && m_index == other.m_index;
            }

            bool operator!=(const Iterator &other) const {
                return !(*this == other);
            }

        private:
            void SkipInvalid() {
                while (m_index < m_owner->m_values.size() &&
                       !m_owner->m_presentValues[m_index]) {
                    ++m_index;
                }
            }

            StableVector *m_owner;
            size_t m_index;
        };

        size_t PushBack(const T &t) {
            if (m_freeSlots.empty()) {
                m_values.push_back(t);
                m_presentValues.push_back(true);
                return m_values.size() - 1;
            } else {
                size_t index = m_freeSlots.back();
                m_values[index] = t;
                m_presentValues[index] = true;
                m_freeSlots.pop_back();
                return index;
            }
        }

        // Remove the element at index
        // It must be present
        void Remove(size_t index) {
            assert(IsPresent(index));

            if (index + 1 == m_values.size()) {
                m_values.pop_back();
                m_presentValues.pop_back();
            } else {
                m_values[index] = {};
                m_freeSlots.push_back(index);
                m_presentValues[index] = false;
            }
        }

        bool IsPresent(size_t index) const {
            return index < m_presentValues.size() && m_presentValues[index];
        }

        T &operator[](size_t index) {
            assert(IsPresent(index));
            return m_values[index];
        }

        const T &operator[](size_t index) const {
            assert(IsPresent(index));
            return m_values[index];
        }

        Iterator begin() {
            return Iterator(this, 0);
        }

        Iterator end() {
            return Iterator(this, m_values.size());
        }

    private:
        std::vector<T> m_values;
        std::vector<size_t> m_freeSlots;
        std::vector<bool> m_presentValues;
    };
} // Petal
