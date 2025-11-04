#pragma once
#include <vector>

/**
 * Represents a single queued start request for a clip.
 * Each request identifies a slot index and the clip index to start.
 */
struct StartRequest
{
    int slot = 0;
    int clip = -1;
};

/**
 * Clip start scheduler that queues launch requests and applies them
 * atomically at quantized boundaries (e.g., start of next bar).
 *
 * Typically used in processBlock to commit new clip starts in sync.
 */
class QuantizedScheduler
{
public:
    /** Queue a start request to trigger on the next quantized boundary. */
    void request(const StartRequest& r)
    {
        pending.emplace_back(r);
    }

    /**
     * Flush all pending requests.
     * Should be called at the quantization boundary (e.g. start of bar).
     * The provided lambda `apply` will be invoked for each request.
     */
    template <typename ApplyFn>
    void flushAtBar(ApplyFn&& apply)
    {
        for (const auto& r : pending)
            apply(r);
        pending.clear();
    }

    /** Cancels all pending requests (e.g. on stop or transport jump). */
    void stopAll()
    {
        pending.clear();
    }

    /** Clears state and optionally resets scheduler position. */
    void resetToHostPosition(double ppq)
    {
        pending.clear();
        currentPPQ = ppq;
    }

    /** Realigns without flushing (e.g. during host jump). */
    void realignTo(double ppq)
    {
        currentPPQ = ppq;
        // Optionally drop pending requests, or preserve?
        pending.clear();
    }

    /** Returns true if there are any queued requests. */
    bool hasPending() const noexcept
    {
        return !pending.empty();
    }

    /** (Optional) Get the internal PPQ position. */
    double getCurrentPPQ() const noexcept { return currentPPQ; }


private:
    std::vector<StartRequest> pending;
    double currentPPQ = 0.0;
};
