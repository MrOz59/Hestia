#pragma once

#include <Limelight.h>
#include <cstddef>
#include <cstdint>
#include <vector>

// Validate lengths before passing a complete, opaque frame to the codec.
// The depacketizer owns the list, so one contiguous entry needs no copy.
inline bool collectPyroWaveFrame(const DECODE_UNIT* unit, std::vector<uint8_t>& storage,
                                const uint8_t*& data, size_t& size)
{
    constexpr size_t maxFrameBytes = 64u * 1024u * 1024u;
    data = nullptr;
    size = 0;
    if (!unit || unit->fullLength <= 0 ||
            static_cast<size_t>(unit->fullLength) > maxFrameBytes || !unit->bufferList) return false;
    const size_t expected = static_cast<size_t>(unit->fullLength);
    for (const LENTRY* entry = unit->bufferList; entry; entry = entry->next) {
        if (!entry->data || entry->length <= 0 || entry->bufferType != BUFFER_TYPE_PICDATA ||
                static_cast<size_t>(entry->length) > expected - size) return false;
        size += static_cast<size_t>(entry->length);
    }
    if (size != expected) return false;
    if (!unit->bufferList->next) {
        data = reinterpret_cast<const uint8_t*>(unit->bufferList->data);
        return true;
    }
    storage.clear();
    storage.reserve(expected);
    for (const LENTRY* entry = unit->bufferList; entry; entry = entry->next) {
        const auto* begin = reinterpret_cast<const uint8_t*>(entry->data);
        storage.insert(storage.end(), begin, begin + entry->length);
    }
    data = storage.data();
    return true;
}
