//  Copyright © 2021 Audulus LLC. All rights reserved.

#ifndef scene_h
#define scene_h

#import <Metal/Metal.h>
#define VGER_MAX_LAYERS 4

#import "prim.h"
#import <simd/simd.h>

using namespace simd;

template<class T>
struct GPUVec {
    id<MTLBuffer> buffer;
    T* ptr = nullptr;
    size_t count = 0;
    size_t capacity = 65536;

    GPUVec() { }

    GPUVec(id<MTLDevice> device) {
        buffer = [device newBufferWithLength:capacity * sizeof(T)
                                     options:MTLResourceStorageModeShared];
        ptr = static_cast<T*>(buffer.contents);
    }

    void allocate(id<MTLDevice> device, size_t cap) {
        capacity = cap;
        auto oldBuffer = buffer;
        buffer = [device newBufferWithLength:capacity * sizeof(T)
                                     options:MTLResourceStorageModeShared];
        memcpy(buffer.contents, oldBuffer.contents, count * sizeof(T));
        ptr = static_cast<T*>(buffer.contents);
    }

    void append(const T& value) {

        if(count >= capacity) {
            allocate(buffer.device, capacity*2);
        }
        ptr[count++] = value;
    }

    void reset() {
        count = 0;
    }
};

struct vgerScene {
    GPUVec<vgerPrim> prims[VGER_MAX_LAYERS];
    GPUVec<float2>   cvs;
    id<MTLBuffer> xforms; // float3x3
    id<MTLBuffer> paints; // vgerPaint
};

#endif /* scene_h */
