// this file is needed for the correct integration of GLM with SGJK
#ifndef SGJK_GLM_HPP_
#define SGJK_GLM_HPP_ 1
#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp> 

#define SGKJ_NOT_IMPLEMENT_VECTORS
#define SGJK_DEFAULT_VEC2D glm::vec2
#define SGJK_DEFAULT_VEC3D glm::vec3

namespace gkm_sgjk_details {
    template<class ScalarType>
    [[nodiscard]] ScalarType cross(const glm::tvec2<ScalarType>& a, const glm::tvec2<ScalarType>& b) noexcept {
        return (a.x * b.y) - (a.y * b.x);
    }
    template<class ScalarType>
    [[nodiscard]] ScalarType cross(const glm::tvec3<ScalarType>& a, const glm::tvec3<ScalarType>& b) noexcept {
        return glm::cross(a, b);
    }
    template<class VecType>
    [[nodiscard]] VecType normalized(const VecType& vec) noexcept {
        return glm::normalize(VecType(vec));
    }
};
#define SGJK_DOT(vec1__, vec2__) (glm::dot(vec1__, vec2__))
#define SGJK_CROSS(vec1__, vec2__) (gkm_sgjk_details::cross(vec1__, vec2__))
#define SGJK_LENGTH(vec__) (glm::length(vec__))
#define SGJK_NORMALIZED(vec__) (gkm_sgjk_details::normalized(vec__))
#define SGJK_COS(val__) (glm::cos(val__))
#define SGJK_SIN(val__) (glm::sin(val__))

namespace sgjk {
    template<class VecType>
    [[nodiscard]] VecType normalized(const VecType& vec) noexcept {
        return SGJK_NORMALIZED(vec);
    }
};

#include <sgjk_head.hpp>
#endif // ifndef SGJK_GLM_HPP_