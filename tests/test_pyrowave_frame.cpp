#include "../app/streaming/video/pyrowaveframe.h"
#include <cassert>
#include <climits>

int main()
{
    char bytes[] = {1, 0, 0, 1};
    LENTRY first {}, second {};
    first.data = bytes; first.length = 2; first.next = &second;
    second.data = bytes+2; second.length = 2;
    DECODE_UNIT unit {};
    unit.fullLength = 4; unit.bufferList = &first;
    std::vector<uint8_t> storage;
    const uint8_t* data = nullptr;
    size_t size = 0;
    assert(collectPyroWaveFrame(&unit, storage, data, size));
    assert(size == 4 && data[0] == 1 && data[3] == 1);
    second.length = -1;
    assert(!collectPyroWaveFrame(&unit, storage, data, size));
    second.length = INT_MAX;
    assert(!collectPyroWaveFrame(&unit, storage, data, size));
    second.length = 1;
    assert(!collectPyroWaveFrame(&unit, storage, data, size));
    second.length = 2; second.next = &first;
    assert(!collectPyroWaveFrame(&unit, storage, data, size));
    second.next = nullptr; second.data = nullptr;
    assert(!collectPyroWaveFrame(&unit, storage, data, size));
    first.next = nullptr; first.length = 4;
    assert(collectPyroWaveFrame(&unit, storage, data, size));
    assert(data == reinterpret_cast<const uint8_t*>(bytes));
    first.bufferType = BUFFER_TYPE_SPS;
    assert(!collectPyroWaveFrame(&unit, storage, data, size));
}
