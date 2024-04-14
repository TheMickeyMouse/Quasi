#include "Collider2D.h"

namespace Physics2D {
    void Collider::Update(float dt) {
        velocity += acceleration * dt;
        position += velocity * dt;
    }
}
