#pragma once
#include <memory>

namespace aloe
{
    template<typename F>
    struct scope_exit_t
    {
        F func;
        bool active = true;

        scope_exit_t(F f) : func(std::move(f)) {}

        ~scope_exit_t()
        {
            if (active) func();
        }

        void dismiss() { active = false; }
    };

    // helper
    template<typename F>
    scope_exit_t<F> make_scope_exit(F f)
    {
        return scope_exit_t<F>(f);
    }

}
