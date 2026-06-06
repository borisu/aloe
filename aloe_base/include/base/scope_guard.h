#pragma once
#include <memory>

namespace aloe
{
    template<typename F>
    struct scope_guard_t
    {
        F func;
        bool active = true;

        scope_guard_t(F f) : func(std::move(f)) {}

        ~scope_guard_t()
        {
            if (active) func();
        }

        void dismiss() { active = false; }
    };

    // helper
    template<typename F>
    scope_guard_t<F> make_scope_guard(F f)
    {
        return scope_guard_t<F>(f);
    }

}
