#include "Slot.h"

// Safe lookup into the clip bank
static inline const DJamClip* clipAt(const std::vector<DJamClip>* bank, int idx) noexcept
{
    if (bank == nullptr || idx < 0 || idx >= static_cast<int>(bank->size()))
        return nullptr;
    return &(*bank)[static_cast<size_t>(idx)];
}

const DJamClip* Slot::getActiveClip() const noexcept
{
    return clipAt(_clips, _slotState.activeClip);
}

const DJamClip* Slot::getPendingClip() const noexcept
{
    return clipAt(_clips, _slotState.pendingClip);
}

juce::String Slot::getActiveClipName() const noexcept
{
    if (const auto* c = getActiveClip())
        return c->getName();
    return {};
}

juce::String Slot::getPendingClipName() const noexcept
{
    if (const auto* c = getPendingClip())
        return c->getName();
    return {};
}

void Slot::applyArmedStart()
{
    if (_slotState.armedStart)
    {
        if (const auto* c = clipAt(_clips, _slotState.pendingClip))
        {
            _slotState.activeClip = _slotState.pendingClip;
            _slotState.phaseSamples = 0;
            _slotState.name = c->getName(); // optional cache for UI
        }

        _slotState.armedStart = false;
        _slotState.pendingClip = -1; // optional: clear after commit
    }
}

bool Slot::render(juce::AudioBuffer<float>& out,
    int startSample,
    int numSamples,
    int destOffset,
    const HostPhase& hp)
{
    if (_slotState.mute)
        return false;

    const auto* clip = getActiveClip();
    if (clip == nullptr || !clip->isLoaded())
        return false;

    clip->render(out, startSample, numSamples, destOffset, _slotState.phaseSamples);

    _slotState.phaseSamples += numSamples;

    const int len = clip->totalSamplesAt(hp.sampleRate, hp);
    if (len > 0 && _slotState.phaseSamples >= len)
        _slotState.phaseSamples %= len;

    return true;
}
