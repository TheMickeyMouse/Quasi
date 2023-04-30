#pragma once
#include "Direction.h"

namespace Maths {
    enum class Corner2D : int {
        SIDE_RIGHT   = 0,
        SIDE_LEFT    = 1 << ((int)Direction2D::LEFT / 2),
        SIDE_TOP     = 0,
        SIDE_BOTTOM  = 1 << ((int)Direction2D::DOWN / 2),

        TOP_RIGHT     = SIDE_RIGHT | SIDE_TOP,
        TOP_LEFT      = SIDE_LEFT  | SIDE_TOP,
        BOTTOM_RIGHT  = SIDE_RIGHT | SIDE_BOTTOM,
        BOTTOM_LEFT   = SIDE_LEFT  | SIDE_BOTTOM,

        CENTER = 16
    };
    IMPL_ENUM_OPERATORS(Corner2D);
    
    enum class Corner3D : int {
        SIDE_RIGHT   = 0,
        SIDE_LEFT    = 1 << ((int)Direction3D::LEFT / 2),
        SIDE_TOP     = 0,
        SIDE_BOTTOM  = 1 << ((int)Direction3D::DOWN / 2),
        SIDE_FRONT   = 0,
        SIDE_BACK    = 1 << ((int)Direction3D::BACK / 2),

        FRONT_TOP_RIGHT     = SIDE_RIGHT | SIDE_TOP    | SIDE_FRONT,
        FRONT_TOP_LEFT      = SIDE_LEFT  | SIDE_TOP    | SIDE_FRONT,
        FRONT_BOTTOM_RIGHT  = SIDE_RIGHT | SIDE_BOTTOM | SIDE_FRONT,
        FRONT_BOTTOM_LEFT   = SIDE_LEFT  | SIDE_BOTTOM | SIDE_FRONT,
        BACK_TOP_RIGHT      = SIDE_RIGHT | SIDE_TOP    | SIDE_BACK,
        BACK_TOP_LEFT       = SIDE_LEFT  | SIDE_TOP    | SIDE_BACK,
        BACK_BOTTOM_RIGHT   = SIDE_RIGHT | SIDE_BOTTOM | SIDE_BACK,
        BACK_BOTTOM_LEFT    = SIDE_LEFT  | SIDE_BOTTOM | SIDE_BACK,
        
        CENTER = 16
    };
    IMPL_ENUM_OPERATORS(Corner3D);

    enum class Corner4D : int {
        SIDE_RIGHT   = 0,
        SIDE_LEFT    = 1 << ((int)Direction4D::LEFT / 2),
        SIDE_TOP     = 0,
        SIDE_BOTTOM  = 1 << ((int)Direction4D::DOWN / 2),
        SIDE_FRONT   = 0,
        SIDE_BACK    = 1 << ((int)Direction4D::BACK / 2),
        SIDE_INNER   = 0,
        SIDE_OUTER   = 1 << ((int)Direction4D::OUT  / 2),

        INNER_FRONT_TOP_RIGHT     = SIDE_RIGHT | SIDE_TOP    | SIDE_FRONT | SIDE_INNER,
        INNER_FRONT_TOP_LEFT      = SIDE_LEFT  | SIDE_TOP    | SIDE_FRONT | SIDE_INNER,
        INNER_FRONT_BOTTOM_RIGHT  = SIDE_RIGHT | SIDE_BOTTOM | SIDE_FRONT | SIDE_INNER,
        INNER_FRONT_BOTTOM_LEFT   = SIDE_LEFT  | SIDE_BOTTOM | SIDE_FRONT | SIDE_INNER,
        INNER_BACK_TOP_RIGHT      = SIDE_RIGHT | SIDE_TOP    | SIDE_BACK  | SIDE_INNER,
        INNER_BACK_TOP_LEFT       = SIDE_LEFT  | SIDE_TOP    | SIDE_BACK  | SIDE_INNER,
        INNER_BACK_BOTTOM_RIGHT   = SIDE_RIGHT | SIDE_BOTTOM | SIDE_BACK  | SIDE_INNER,
        INNER_BACK_BOTTOM_LEFT    = SIDE_LEFT  | SIDE_BOTTOM | SIDE_BACK  | SIDE_INNER,
        OUTER_FRONT_TOP_RIGHT     = SIDE_RIGHT | SIDE_TOP    | SIDE_FRONT | SIDE_OUTER,
        OUTER_FRONT_TOP_LEFT      = SIDE_LEFT  | SIDE_TOP    | SIDE_FRONT | SIDE_OUTER,
        OUTER_FRONT_BOTTOM_RIGHT  = SIDE_RIGHT | SIDE_BOTTOM | SIDE_FRONT | SIDE_OUTER,
        OUTER_FRONT_BOTTOM_LEFT   = SIDE_LEFT  | SIDE_BOTTOM | SIDE_FRONT | SIDE_OUTER,
        OUTER_BACK_TOP_RIGHT      = SIDE_RIGHT | SIDE_TOP    | SIDE_BACK  | SIDE_OUTER,
        OUTER_BACK_TOP_LEFT       = SIDE_LEFT  | SIDE_TOP    | SIDE_BACK  | SIDE_OUTER,
        OUTER_BACK_BOTTOM_RIGHT   = SIDE_RIGHT | SIDE_BOTTOM | SIDE_BACK  | SIDE_OUTER,
        OUTER_BACK_BOTTOM_LEFT    = SIDE_LEFT  | SIDE_BOTTOM | SIDE_BACK  | SIDE_OUTER,

        CENTER = 16
    };
    IMPL_ENUM_OPERATORS(Corner4D);

    template <class E>
    concept Corner = requires {
        std::is_enum_v<E>;
        E::CENTER == (E) 16;
    };
}
