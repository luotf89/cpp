#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>
namespace algorithm {

template<typename FunSig>
struct Function {
    static_assert(!std::is_same_v<FunSig, FunSig>, "need a true function signature");
};

template<typename Ret, typename ...Args>
struct Function<Ret(Args...)> {
    struct FunctionBase{
        virtual Ret call(Args ...args) = 0;
        virtual std::unique_ptr<FunctionBase> clone() = 0;
        virtual const std::type_info& type() = 0;
        virtual ~FunctionBase() {}
    };
    template<typename Func>
    struct FunctionImpl:public FunctionBase {
        template<typename ...FArgs>
        FunctionImpl(std::in_place_t, FArgs ...fargs): func_(std::forward<FArgs>(fargs)...) {}
        Ret call(Args... args) override {
            return std::invoke(func_, std::forward<Args>(args)...);
        }

        std::unique_ptr<FunctionBase> clone() override {
            return std::make_unique<FunctionImpl<Func>>(std::in_place, func_);
        }

        const std::type_info& type() override {
            return typeid(Func);
        }
        Func func_;
    };

    template<typename Func>
    requires std::is_invocable_r_v<Ret, std::decay<Func>, Args...> || std::is_copy_constructible_v<Func>
    Function(Func&& func) {
        impl_ = std::make_unique<FunctionImpl<Func>>(std::in_place, std::move(func));
    }

    Function(Function&&) = delete;
    Function& operator=(Function&&) = delete;

    Function(const Function& other):
        impl_(other.impl_? other.impl_->clone(): nullptr) {}
    Function& operator=(const Function& other) {
        if (this == std::addressof(other)) [[unlikely]]{
            return *this;
        }
        impl_ = other.impl_? std::move(other.impl_->clone()): nullptr;
        return *this;
    }

    Ret operator()(Args ...args) {
        if (!impl_) [[unlikely]]{
            throw std::runtime_error("funcion is nullptr!");
        }
        return impl_->call(std::forward<Args>(args)...);
    }
    template<typename Func>
    Func address() {
        return impl_ && typeid(Func) == impl_->type()? 
            std::addressof(static_cast<FunctionImpl<Func>*>(impl_->get())->func_):
            nullptr;
    }

    private:
    std::unique_ptr<FunctionBase> impl_;
};



} // namespace algorithm
