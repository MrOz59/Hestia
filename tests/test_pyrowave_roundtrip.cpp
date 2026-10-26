// Exercise the same C API and opaque frame collector as host/client, on
// separate Vulkan devices, including recovery after an omitted frame.
#include <vulkan/vulkan.h>
#include <pyrowave.h>
#include "../app/streaming/video/pyrowaveframe.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>

struct Codec {
    pyrowave_device encodeDevice = nullptr, decodeDevice = nullptr;
    pyrowave_encoder encoder = nullptr;
    pyrowave_decoder decoder = nullptr;
    ~Codec() {
        if (decoder) pyrowave_decoder_destroy(decoder);
        if (encoder) pyrowave_encoder_destroy(encoder);
        if (decodeDevice) pyrowave_device_destroy(decodeDevice);
        if (encodeDevice) pyrowave_device_destroy(encodeDevice);
    }
};

static void check(pyrowave_result result) {
    if (result != PYROWAVE_SUCCESS) {
        std::fprintf(stderr, "PyroWave status %d\n", static_cast<int>(result));
        std::abort();
    }
}

static pyrowave_cpu_buffer view(std::vector<uint8_t>& bytes, int width, int height) {
    const size_t luma = static_cast<size_t>(width) * height;
    pyrowave_cpu_buffer buffer {};
    buffer.data[0] = bytes.data(); buffer.data[1] = bytes.data()+luma; buffer.data[2] = bytes.data()+luma+luma/4;
    buffer.row_stride_in_bytes[0] = width;
    buffer.row_stride_in_bytes[1] = buffer.row_stride_in_bytes[2] = width/2;
    buffer.plane_size_in_bytes[0] = luma;
    buffer.plane_size_in_bytes[1] = buffer.plane_size_in_bytes[2] = luma/4;
    buffer.width = width; buffer.height = height;
    buffer.format = PYROWAVE_CPU_BUFFER_FORMAT_YUV420P;
    return buffer;
}

int main() {
    constexpr int width = 130, height = 98;
    constexpr size_t budget = 256 * 1024;
    Codec codec;
    check(pyrowave_create_default_device(&codec.encodeDevice));
    check(pyrowave_create_default_device(&codec.decodeDevice));
    pyrowave_encoder_create_info encoderInfo {};
    encoderInfo.device = codec.encodeDevice; encoderInfo.width = width; encoderInfo.height = height;
    encoderInfo.chroma = PYROWAVE_CHROMA_SUBSAMPLING_420;
    check(pyrowave_encoder_create(&encoderInfo, &codec.encoder));
    pyrowave_decoder_create_info decoderInfo {};
    decoderInfo.device = codec.decodeDevice; decoderInfo.width = width; decoderInfo.height = height;
    decoderInfo.chroma = PYROWAVE_CHROMA_SUBSAMPLING_420;
    decoderInfo.fragment_path = pyrowave_decoder_device_prefers_fragment_path(codec.decodeDevice);
    check(pyrowave_decoder_create(&decoderInfo, &codec.decoder));
    std::vector<uint8_t> input(width*height*3/2), output(input.size()), bitstream(budget), collected;
    const pyrowave_rate_control rate {budget};
    int decoded = 0;
    for (int frame = 0; frame < 3; ++frame) {
        for (size_t i = 0; i < input.size(); ++i) input[i] = static_cast<uint8_t>(16+(i+37*frame)%220);
        auto source = view(input, width, height);
        check(pyrowave_encoder_encode_cpu_synchronous(codec.encoder, &source, &rate));
        size_t count = 0;
        check(pyrowave_encoder_compute_num_packets(codec.encoder, budget, &count));
        assert(count == 1);
        pyrowave_packet packet {};
        check(pyrowave_encoder_packetize(codec.encoder, &packet, budget, &count, bitstream.data(), bitstream.size()));
        assert(count == 1 && packet.offset == 0 && packet.size > 2 && packet.size <= bitstream.size());
        if (frame == 1) continue; // A whole network frame is lost; do not reset either device.
        LENTRY first {}, second {};
        first.data = reinterpret_cast<char*>(bitstream.data()); first.length = static_cast<int>(packet.size/2); first.next = &second;
        second.data = first.data+first.length; second.length = static_cast<int>(packet.size)-first.length;
        DECODE_UNIT unit {}; unit.fullLength = static_cast<int>(packet.size); unit.bufferList = &first;
        const uint8_t* data = nullptr;
        size_t size = 0;
        assert(collectPyroWaveFrame(&unit, collected, data, size));
        pyrowave_decoder_clear(codec.decoder);
        check(pyrowave_decoder_push_packet(codec.decoder, data, size));
        assert(pyrowave_decoder_decode_is_ready(codec.decoder, false));
        auto destination = view(output, width, height);
        check(pyrowave_decoder_decode_cpu_buffer_synchronous(codec.decoder, &destination));
        int worstError = 0;
        for (size_t i = 0; i < input.size(); ++i)
            worstError = std::max(worstError, std::abs(int(input[i])-int(output[i])));
        assert(worstError <= 2);
        ++decoded;
    }
    assert(decoded == 2);
    std::puts("PASS: Vulkan encode/decode, fragmented opaque frame, pixels and recovery after omitted frame");
}
