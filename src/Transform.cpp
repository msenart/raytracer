#include "Transform.h"

glm::mat4 Transform::matrix() const {
    glm::mat4 t = glm::translate(glm::mat4(1.f), position);
    glm::mat4 r = glm::mat4_cast(rotation);
    glm::mat4 s = glm::scale(glm::mat4(1.f), scale);
    return t * r * s;
}
