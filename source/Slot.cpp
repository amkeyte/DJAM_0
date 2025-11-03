#include "Slot.h"

// Set the shared clip bank pointer
void Slot::setClipBank(const std::vector<DJamClip>* bank)
{
    _clips = bank;
}

// Schedule a clip to start at the next quantized boundary
void Slot::armStart(int clipIndex)
{
    _slotState.armedStart = true;
    _slotState.pendingClip = clipIndex;
}

// Commit the armed start (called at bar boundary)
void Slot::applyArmedStart()
{
    if (_slotState.armedStart && _clips != nullptr)
    {
        const int clipIndex = _slotState.pendingClip;

        if (clipIndex >= 0 && clipIndex < (int)_clips->size())
        {
            _slotState.activeClip = clipIndex;
            _slotState.phaseSamples = 0;
            _slotState.name = (*_clips)[(size_t)clipIndex].getName();
        }
    }

    _slotState.armedStart = false;
    _slotState.pendingClip = -1;
}

void Slot::stopPlayback()
{
    _slotState.activeClip = -1;
    _slotState.phaseSamples = 0;
}

void Slot::jumpTo(double ppq)
{
    if (_slotState.activeClip < 0 || !_clips) return;

    const DJamClip& clip = (*_clips)[(size_t)_slotState.activeClip];
    const double samplesPerBeat = clip.sampleRate * 60.0 / clip.bpm;

    // Modulo numBeats allows looping
    _slotState.phaseSamples = (int)(std::fmod(ppq, (double)clip.numBeats) * samplesPerBeat);
}

void Slot::toggleMute()
{
    _slotState.mute = !_slotState.mute;
}

void Slot::setSolo(bool v)
{
    _slotState.solo = v;
}

bool Slot::isMuted() const noexcept { return _slotState.mute; }
bool Slot::isSolo() const noexcept { return _slotState.solo; }
bool Slot::isArmed() const noexcept { return _slotState.armedStart; }

int Slot::getActiveClipIndex() const noexcept { return _slotState.activeClip; }
int Slot::getPendingClipIndex() const noexcept { return _slotState.pendingClip; }

const DJamClip* Slot::getActiveClip() const noexcept
{
    if (!_clips || _slotState.activeClip < 0 || _slotState.activeClip >= (int)_clips->size())
        return nullptr;

    return &(*_clips)[(size_t)_slotState.activeClip];
}

const DJamClip* Slot::getPendingClip() const noexcept
{
    if (!_clips || _slotState.pendingClip < 0 || _slotState.pendingClip >= (int)_clips->size())
        return nullptr;

    return &(*_clips)[(size_t)_slotState.pendingClip];
}

juce::String Slot::getActiveClipName() const noexcept
{
    const DJamClip* c = getActiveClip();
    return c ? c->getName() : juce::String();
}

juce::String Slot::getPendingClipName() const noexcept
{
    const DJamClip* c = getPendingClip();
    return c ? c->getName() : juce::String();
}

bool Slot::render(juce::AudioBuffer<float>& out,
    int startSample,
    int numSamples,
    int destOffset,
    const HostPhase& hp)
{
    const DJamClip* clip = getActiveClip();
    if (!clip || !clip->isLoaded()) return false;

    // Render the clip into the output buffer
    clip->render(out, startSample, numSamples, destOffset, _slotState.phaseSamples);

    // Advance phase
    const double samplesPerBeat = hp.sampleRate * 60.0 / hp.bpm;
    const int loopSamples = (int)(clip->getLoopLengthBars() * hp.beatsPerBar * samplesPerBeat);

    _slotState.phaseSamples = (_slotState.phaseSamples + numSamples) % loopSamples;

    return true;
}

