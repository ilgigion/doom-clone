#include "FrameProjection.h"
#include <iostream>
#include <utility>

namespace {
constexpr float pi = std::numbers::pi_v<float>;
constexpr float fov = pi / 3.0f;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void near(float actual, float expected, const char* message, float tolerance = 0.0001f) {
    check(std::isfinite(actual) && std::abs(actual - expected) <= tolerance, message);
}

void dimensionsAndAngles() {
    for (auto size : {std::pair{800, 600}, std::pair{801, 601}, std::pair{1280, 720}}) {
        FrameProjection frame(size.first, size.second, 0.0f, fov, 5);
        check(frame.width() == size.first && frame.height() == size.second, "Frame dimensions");
        check(frame.horizon() == size.second / 2 + 5, "Integer horizon including bobbing");
        near(frame.firstRayAngle(), -pi / 6, "Left FOV edge");
        float angle = frame.firstRayAngle();
        for (int x = 1; x < frame.width(); ++x) angle += frame.angleStep();
        check(angle < pi / 6 && angle > pi / 6 - 2 * frame.angleStep(), "Right FOV edge is exclusive");
    }
    FrameProjection odd(801, 601, 0.0f, fov, -5);
    check(odd.horizon() == 295, "Odd height uses integer division before bobbing");
    near(odd.wallHeight(3.0f), 601.0f / 3, "Existing vertical scale");
}

void knownFloorPoints() {
    FrameProjection frame(800, 600, 0.0f, fov, 0);
    auto centre = frame.floorPoint(1.5f, 1.5f, 0.0f, 400.0f);
    check(centre.has_value(), "Centre intersects floor");
    near(centre->x, 4.5f, "Floor centre X");
    near(centre->y, 1.5f, "Floor centre Y");
    auto left = frame.floorPoint(1.5f, 1.5f, -pi / 6, 400.0f);
    auto right = frame.floorPoint(1.5f, 1.5f, pi / 6, 400.0f);
    check(left && right, "FOV edges intersect floor");
    near(left->x, 4.5f, "Left edge forward depth");
    near(left->y, 1.5f - std::sqrt(3.0f), "Left edge lateral position");
    near(right->y, 1.5f + std::sqrt(3.0f), "Right edge lateral position");
    near(frame.perpendicularDepth(6.0f, pi / 6), 3 * std::sqrt(3.0f), "Ray distance versus forward depth");
    const auto pixelCentre = frame.floorDepth(400.5f);
    check(pixelCentre.has_value(), "Subpixel sample");
    check(*pixelCentre < 3.0f, "Pixel centre below row edge is closer to camera");
}

void cameraTransforms() {
    FrameProjection rotated(800, 600, pi / 2, fov, 7);
    auto point = rotated.floorPoint(-2.5f, -4.5f, pi / 2, 407.0f);
    check(point.has_value(), "Rotated floor point");
    near(point->x, -2.5f, "Rotation preserves lateral coordinate");
    near(point->y, -1.5f, "Rotation and negative translation");
    FrameProjection plain(800, 600, pi / 2, fov, 0);
    auto same = plain.floorPoint(-2.5f, -4.5f, pi / 2, 400.0f);
    check(same.has_value(), "Unshifted horizon");
    near(point->x, same->x, "Bobbing shifts horizon and pixel together X");
    near(point->y, same->y, "Bobbing shifts horizon and pixel together Y");
}

void worldRoundTrip() {
    for (auto size : {std::pair{800, 600}, std::pair{801, 601}, std::pair{1280, 720}}) {
        for (float direction : {0.0f, 0.7f, -1.2f}) {
            FrameProjection frame(size.first, size.second, direction, fov, -3);
            for (auto offset : {std::pair{3.0f, -1.0f}, std::pair{5.0f, 2.0f}}) {
                const float dx = offset.first * std::cos(direction) - offset.second * std::sin(direction);
                const float dy = offset.first * std::sin(direction) + offset.second * std::cos(direction);
                const float depth = dx * std::cos(direction) + dy * std::sin(direction);
                const float screenY = frame.horizon() + frame.verticalScale() * FrameProjection::cameraHeight / depth;
                const auto point = frame.floorPoint(-4.0f, 2.0f, std::atan2(dy, dx), screenY);
                check(point.has_value(), "World point inverse projection");
                near(point->x, -4.0f + dx, "Recovered world X");
                near(point->y, 2.0f + dy, "Recovered world Y");
            }
        }
    }
}

void wallFloorJunction() {
    FrameProjection frame(800, 600, 0.0f, fov, 4);
    for (float depth : {0.5f, 1.0f, 3.0f, 10.0f}) {
        const float wallBottom = frame.horizon() + frame.wallHeight(depth) / 2;
        const auto recovered = frame.floorDepth(wallBottom);
        check(recovered.has_value(), "Wall bottom intersects floor");
        near(*recovered, depth, "Wall and floor share vertical projection");
    }
}

void textureWrapping() {
    for (int size : {1, 63, 64, 257}) {
        check(FrameProjection::textureCoordinate(-1.0f, size) == 0, "Negative integer UV");
        check(FrameProjection::textureCoordinate(2.0f, size) == 0, "Positive integer UV");
        check(FrameProjection::textureCoordinate(-0.25f, size) ==
              FrameProjection::textureCoordinate(0.75f, size), "Negative UV wraps to positive fraction");
        check(FrameProjection::textureCoordinate(-1e-30f, size) == size - 1, "Tiny negative UV stays in bounds");
        for (float world : {-5.125f, -0.001f, 0.0f, 0.999f, 15.5f}) {
            const auto uv = FrameProjection::textureCoordinate(world, size);
            check(uv && *uv >= 0 && *uv < size, "Texture index bounds");
        }
    }
    check(FrameProjection::textureCoordinate(-0.25f, 64) == 48, "Known negative UV index");
}

void invalidInputs() {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    FrameProjection frame(800, 600, 0.0f, fov, 0);
    for (float y : {299.5f, 300.0f, nan, inf}) check(!frame.floorDepth(y), "No floor at/above invalid horizon");
    check(frame.floorDepth(300.5f).has_value(), "First pixel below horizon is finite");
    check(!frame.floorPoint(nan, 0, 0, 400), "Reject NaN camera position");
    check(!frame.floorPoint(0, 0, pi, 400), "Reject backwards ray");
    check(!frame.floorPoint(0, 0, inf, 400), "Reject infinite ray");
    check(!FrameProjection::textureCoordinate(nan, 64), "Reject NaN UV");
    check(!FrameProjection::textureCoordinate(0, 0), "Reject empty texture");
    check(!FrameProjection::textureCoordinate(0, -1), "Reject negative texture size");
    for (float invalidFov : {0.0f, -1.0f, pi, nan, inf}) {
        bool rejected = false;
        try { FrameProjection bad(800, 600, 0, invalidFov, 0); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Reject invalid FOV");
    }
    for (auto size : {std::pair{0, 600}, std::pair{800, -1}}) {
        bool rejected = false;
        try { FrameProjection bad(size.first, size.second, 0, fov, 0); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Reject invalid frame size");
    }
    for (float depth : {0.0f, -1.0f, nan, inf}) {
        bool rejected = false;
        try { frame.wallHeight(depth); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Reject invalid wall depth");
    }
}
}

int main() {
    int failures = 0;
    for (const auto& test : {std::pair<const char*, void(*)()>{"dimensionsAndAngles", dimensionsAndAngles},
             {"knownFloorPoints", knownFloorPoints}, {"cameraTransforms", cameraTransforms},
             {"worldRoundTrip", worldRoundTrip}, {"wallFloorJunction", wallFloorJunction},
             {"textureWrapping", textureWrapping}, {"invalidInputs", invalidInputs}}) {
        try { test.second(); std::cout << test.first << ": passed\n"; }
        catch (const std::exception& error) {
            ++failures;
            std::cerr << test.first << ": " << error.what() << '\n';
        }
    }
    return failures == 0 ? 0 : 1;
}
