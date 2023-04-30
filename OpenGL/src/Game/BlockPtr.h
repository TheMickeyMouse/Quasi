#pragma once
#include <memory>

#include "BlockBase.h"

namespace Game {
    class BlockPtr {
        private:
            std::unique_ptr<BlockBase> _ptr { };
        public:
            BlockPtr() = default;
            BlockPtr(const BlockBase& block) : _ptr(new BlockBase(block)) { }
            // ! make sure the pointer passed doesnt belong to unique_ptr !
            BlockPtr(BlockBase* block) : _ptr{block} { }
            BlockPtr(std::unique_ptr<BlockBase>&& block) : _ptr{std::move(block)} { }
            // template <class ...TArgs>
            // explicit BlockPtr(TArgs&&... args) : _ptr(std::make_unique<BlockBase, TArgs...>(args...)) {}
        
            BlockPtr(const BlockPtr& other) : _ptr(new BlockBase(*other._ptr)) { }
            BlockPtr(BlockPtr&& other) noexcept : _ptr(std::move(other._ptr)) { }
            BlockPtr& operator=(const BlockPtr& other) {
                if (this == &other)
                    return *this;
                _ptr = std::make_unique<BlockBase>(*other._ptr);
                return *this;
            }
            BlockPtr& operator=(BlockPtr&& other) noexcept {
                if (this == &other)
                    return *this;
                _ptr = std::move(other._ptr);
                return *this;
            }

            ~BlockPtr() = default;
        
            BlockBase* operator->() const { return _ptr.get(); }
            const BlockBase& operator*() const { return *_ptr; }
            BlockBase& operator*() { return *_ptr; }
            [[nodiscard]] const BlockBase& get() const { return *_ptr; }
            BlockBase& get() { return *_ptr; }
    };
}
