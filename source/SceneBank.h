#pragma once
#include <array>
#include <algorithm>

/**
 * Represents a stored combination of clip assignments across all slots.
 * Each Scene contains the clip index (or -1 if empty) for up to 8 slots.
 */
struct Scene
{
    std::array<int, 8> slotClip{ {-1, -1, -1, -1, -1, -1, -1, -1} };
};

/**
 * Holds a small set of Scenes (typically 8) and provides
 * safe indexed access and modification helpers.
 */
class SceneBank
{
public:
    SceneBank()
    {
        scenes.fill(Scene{}); // Initialize all slots to -1
    }

    /** Returns a const reference to the Scene at index [0–7]. */
    const Scene& get(int i) const noexcept
    {
        return scenes[(size_t)std::clamp(i, 0, 7)];
    }

    /** Returns a mutable reference to the Scene at index [0–7]. */
    Scene& getMutable(int i) noexcept
    {
        return scenes[(size_t)std::clamp(i, 0, 7)];
    }

    /** Replace a Scene at the given index with a new one. */
    void set(int i, const Scene& s)
    {
        scenes[(size_t)std::clamp(i, 0, 7)] = s;
    }

    /** Resets all stored scenes to default (-1 assignments). */
    void clearAll()
    {
        scenes.fill(Scene{});
    }

    /** Returns the total number of scenes (always 8). */
    int size() const noexcept { return (int)scenes.size(); }

private:
    std::array<Scene, 8> scenes;
};
