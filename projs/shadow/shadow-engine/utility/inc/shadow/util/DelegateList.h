#pragma once

#include <shadow/util/Delegate.h>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace ShadowEngine {

  template<typename T> struct DelegateList;

  template<typename R, typename... Args>
  struct DelegateList<R(Args...)> {
    explicit DelegateList() : m_delegates() {}

    template<auto Function, typename C>
    void bind(C *instance) {
        Delegate<R(Args...)> cb;
        cb.template bind<Function>(instance);
        m_delegates.push_back(cb);
    }

    void bindRaw(void *obj, void (*f)(void *, Args...)) {
        Delegate<R(Args...)> cb;
        cb.bindRaw(obj, f);
        m_delegates.push_back(cb);
    }

    void unbindRaw(void *obj, void (*f)(void *, Args...)) {
        for (int32_t i = 0; i < m_delegates.size(); ++i) {
            if (m_delegates[i].m_stub.first == obj && m_delegates[i].m_stub.second == f) {
                m_delegates.swapAndPop(i);
                break;
            }
        }
    }

    template<R (*Function)(Args...)>
    void bind() {
        Delegate<R(Args...)> cb;
        cb.template bind<Function>();
        m_delegates.push_back(cb);
    }

    template<R (*Function)(Args...)>
    void unbind() {
        Delegate<R(Args...)> cb;
        cb.template bind<Function>();
        for (int i = 0; i < m_delegates.size(); ++i) {
            if (m_delegates[i] == cb) {
                m_delegates.swapAndPop(i);
                break;
            }
        }
    }

    template<auto Function, typename C>
    void unbind(C *instance) {
        Delegate<R(Args...)> cb;
        cb.template bind<Function>(instance);
        for (int i = 0; i < m_delegates.size(); ++i) {
            if (m_delegates[i] == cb) {
                std::iter_swap(m_delegates.begin() + i, m_delegates.end() - 1);
                m_delegates.pop_back();
                break;
            }
        }
    }

    void invoke(Args... args) {
        for (int32_t i = 0, c = m_delegates.size(); i < c; ++i) m_delegates[i].invoke(args...);
    }

  private:
    std::vector<Delegate<R(Args...)>> m_delegates;
  };
}