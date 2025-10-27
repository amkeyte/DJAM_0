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
 * Simple scheduler that queues clip start requests
 * and applies them atomically on a quantized boundary (e.g., next bar).
 */
class QuantizedScheduler
{
public:
    /** Queue a start request to trigger on the next quantized boundary. */
    void request(const StartRequest& r) { pending.push_back(r); }

    /**
     * Flush all pending requests. This should be called exactly
     * at the quantization boundary (e.g., start of bar).
     * The provided lambda or functor `apply` will be invoked for each request.
     */
    template <typename ApplyFn>
    void flushAtBar(ApplyFn&& apply)
    {
        for (auto& r : pending)
            apply(r);
        pending.clear();
    }

    /** Returns true if there are any queued requests. */
    bool hasPending() const noexcept { return !pending.empty(); }

private:
    std::vector<StartRequest> pending;
};
