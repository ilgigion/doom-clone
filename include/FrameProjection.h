#ifndef FRAME_PROJECTION_H
#define FRAME_PROJECTION_H

#include <cmath>
#include <algorithm>
#include <limits>
#include <numbers>
#include <optional>
#include <stdexcept>

struct FloorPoint {
    float x;
    float y;
};

struct FloorRay {
    double x;
    double y;
};

// Coordinates are map cells; angles are radians. This preserves the existing
// equal-angle columns and vertical scale, not a conventional pinhole camera.
class FrameProjection {
public:
    static constexpr float cameraHeight = 0.5f;

    FrameProjection(int width, int height, float direction, float fov, int verticalOffset)
        : width_(width), height_(height), direction_(direction), fov_(fov) {
        if (width <= 0 || height <= 0 || !std::isfinite(direction) ||
            !std::isfinite(fov) || fov <= 0.0f || fov >= std::numbers::pi_v<float>) {
            throw std::invalid_argument("Invalid frame projection");
        }
        const auto horizon = static_cast<long long>(height / 2) + verticalOffset;
        if (horizon < std::numeric_limits<int>::min() || horizon > std::numeric_limits<int>::max()) {
            throw std::invalid_argument("Projection horizon is out of range");
        }
        horizon_ = static_cast<int>(horizon);
    }

    int width() const { return width_; }
    int height() const { return height_; }
    int horizon() const { return horizon_; }
    float verticalScale() const { return static_cast<float>(height_); }
    float firstRayAngle() const { return direction_ - fov_ / 2.0f; }
    float angleStep() const { return fov_ / width_; }

    // Advance angle by angleStep() in the caller to preserve the original
    // floating-point accumulation of wall rays. Floor rays must use the same angles.
    float perpendicularDepth(float rayDistance, float rayAngle) const {
        return rayDistance * ::cos(rayAngle - direction_);
    }

    float wallHeight(float depth) const {
        if (!std::isfinite(depth) || depth <= 0.0f) {
            throw std::invalid_argument("Wall depth must be finite and positive");
        }
        const float result = verticalScale() / depth;
        if (!std::isfinite(result)) throw std::overflow_error("Projected wall height overflow");
        return result;
    }

    // screenY is continuous: pass row + 0.5f to sample the pixel centre.
    // nullopt means no finite forward intersection with the horizontal floor.
    std::optional<float> floorDepth(float screenY) const {
        const float belowHorizon = screenY - horizon_;
        if (!std::isfinite(belowHorizon) || belowHorizon <= 0.0f) return std::nullopt;
        const float depth = verticalScale() * cameraHeight / belowHorizon;
        if (!std::isfinite(depth) || depth <= 0.0f) return std::nullopt;
        return depth;
    }

    // Compute once per column, then multiply by the forward depth of each row.
    std::optional<FloorRay> floorRay(float rayAngle) const {
        if (!std::isfinite(rayAngle)) return std::nullopt;
        const double forward = ::cos(rayAngle - direction_);
        if (forward <= 1e-6) return std::nullopt;
        return FloorRay{::cos(rayAngle) / forward, ::sin(rayAngle) / forward};
    }

    std::optional<FloorPoint> floorPoint(float playerX, float playerY,
                                        float rayAngle, float screenY) const {
        const auto depth = floorDepth(screenY);
        const auto ray = floorRay(rayAngle);
        if (!depth || !ray || !std::isfinite(playerX) || !std::isfinite(playerY)) return std::nullopt;
        const double x = playerX + *depth * ray->x;
        const double y = playerY + *depth * ray->y;
        if (!std::isfinite(x) || !std::isfinite(y) ||
            std::abs(x) > std::numeric_limits<float>::max() ||
            std::abs(y) > std::numeric_limits<float>::max()) return std::nullopt;
        return FloorPoint{static_cast<float>(x), static_cast<float>(y)};
    }

    // One texture repeat per map cell, including negative world coordinates.
    static std::optional<int> textureCoordinate(float world, int textureSize) {
        if (!std::isfinite(world) || textureSize <= 0) return std::nullopt;
        const double fraction = static_cast<double>(world) - std::floor(static_cast<double>(world));
        // Very small negative values can round their fractional part to 1.
        return std::min(static_cast<int>(fraction * textureSize), textureSize - 1);
    }

private:
    int width_;
    int height_;
    int horizon_;
    float direction_;
    float fov_;
};

#endif
