#pragma once
#include <type_traits>

#include "stdu/enum_utils.h"

namespace Maths {
    enum class Direction2D : int {
        RIGHT = 0,
        LEFT,
        UP,
        DOWN,
        
        POSITIVE_X = RIGHT,
        NEGATIVE_X = LEFT,
        POSITIVE_Y = UP,
        NEGATIVE_Y = DOWN,

        X_AXIS = POSITIVE_X,
        Y_AXIS = POSITIVE_Y,
        
        ZERO = 8,
        UNIT = 9
    };
    IMPL_ENUM_OPERATORS(Direction2D)
    
    enum class Direction3D : int {
        RIGHT = 0,
        LEFT,
        UP,
        DOWN,
        FRONT,
        BACK,

        POSITIVE_X = RIGHT,
        NEGATIVE_X = LEFT,
        POSITIVE_Y = UP,
        NEGATIVE_Y = DOWN,
        POSITIVE_Z = FRONT,
        NEGATIVE_Z = BACK,
        
        X_AXIS = POSITIVE_X,
        Y_AXIS = POSITIVE_Y,
        Z_AXIS = POSITIVE_Z,
    
        ZERO = 8,
        UNIT = 9
    };
    IMPL_ENUM_OPERATORS(Direction3D)

    enum class Direction4D : int {
        RIGHT = 0,
        LEFT,
        UP,
        DOWN,
        FRONT,
        BACK,
        IN,
        OUT,

        POSITIVE_X = RIGHT,
        NEGATIVE_X = LEFT,
        POSITIVE_Y = UP,
        NEGATIVE_Y = DOWN,
        POSITIVE_Z = FRONT,
        NEGATIVE_Z = BACK,
        POSITIVE_W = IN,
        NEGATIVE_W = OUT,
                
        X_AXIS = POSITIVE_X,
        Y_AXIS = POSITIVE_Y,
        Z_AXIS = POSITIVE_Z,
        W_AXIS = POSITIVE_W,

        ZERO = 8,
        UNIT = 9
    };
    IMPL_ENUM_OPERATORS(Direction4D)

    template <class E>
    concept Direction = requires {
        std::is_enum_v<E>;
        E::ZERO == (E) 8;
        E::UNIT == (E) 9;
    };
}
