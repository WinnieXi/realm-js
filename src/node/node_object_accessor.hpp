////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node_class.hpp"
#include "js_object_accessor.hpp"

namespace realm {

// Specialize a native accessor class for Node.
namespace js {

template<>
inline v8::Local<v8::Value> NativeAccessor<node::Types>::box(BinaryData data) {
#if REALM_V8_ARRAY_BUFFER_API
    size_t byte_count = data.size();
    void* bytes = nullptr;

    if (byte_count) {
        bytes = memcpy(malloc(byte_count), data.data(), byte_count);
    }

    // An "internalized" ArrayBuffer will free the malloc'd memory when garbage collected.
    return v8::ArrayBuffer::New(m_ctx, bytes, byte_count, v8::ArrayBufferCreationMode::kInternalized);
#else
    // TODO: Implement this for older V8
#endif
}

} // js
} // realm
