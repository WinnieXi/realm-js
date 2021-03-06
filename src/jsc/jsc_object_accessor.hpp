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

#include "jsc_class.hpp"
#include "js_object_accessor.hpp"

namespace realm {

// Specialize a native accessor class for JSC.

namespace js {

template<>
template<>
inline BinaryData NativeAccessor<jsc::Types>::unbox(ValueType value, bool, bool) {
    static jsc::String s_array_buffer = "ArrayBuffer";
    static jsc::String s_buffer = "buffer";
    static jsc::String s_byte_length = "byteLength";
    static jsc::String s_byte_offset = "byteOffset";
    static jsc::String s_is_view = "isView";
    static jsc::String s_uint8_array = "Uint8Array";

    JSObjectRef global_object = JSContextGetGlobalObject(m_ctx);
    JSObjectRef array_buffer_constructor = jsc::Object::validated_get_constructor(m_ctx, global_object, s_array_buffer);
    JSObjectRef uint8_array_constructor = jsc::Object::validated_get_constructor(m_ctx, global_object, s_uint8_array);
    JSValueRef uint8_array_arguments[3];
    uint32_t uint8_array_argc = 0;

    // Value should either be an ArrayBuffer or ArrayBufferView (i.e. TypedArray or DataView).
    if (JSValueIsInstanceOfConstructor(m_ctx, value, array_buffer_constructor, nullptr)) {
        uint8_array_arguments[0] = value;
        uint8_array_argc = 1;
    }
    else if (JSObjectRef object = JSValueToObject(m_ctx, value, nullptr)) {
        // Check if value is an ArrayBufferView by calling ArrayBuffer.isView(val).
        JSValueRef is_view = jsc::Object::call_method(m_ctx, array_buffer_constructor, s_is_view, 1, &object);

        if (jsc::Value::to_boolean(m_ctx, is_view)) {
            uint8_array_arguments[0] = jsc::Object::validated_get_object(m_ctx, object, s_buffer);
            uint8_array_arguments[1] = jsc::Object::get_property(m_ctx, object, s_byte_offset);
            uint8_array_arguments[2] = jsc::Object::get_property(m_ctx, object, s_byte_length);
            uint8_array_argc = 3;
        }
    }

    if (!uint8_array_argc) {
        throw std::runtime_error("Can only convert ArrayBuffer and TypedArray objects to binary");
    }

    JSObjectRef uint8_array = jsc::Function::construct(m_ctx, uint8_array_constructor, uint8_array_argc, uint8_array_arguments);
    uint32_t byte_count = jsc::Object::validated_get_length(m_ctx, uint8_array);
    m_string_buffer.resize(byte_count);

    for (uint32_t i = 0; i < byte_count; i++) {
        JSValueRef byteValue = jsc::Object::get_property(m_ctx, uint8_array, i);
        m_string_buffer[i] = jsc::Value::to_number(m_ctx, byteValue);
    }

    return BinaryData(m_string_buffer.data(), m_string_buffer.size());
}

template<>
inline JSValueRef NativeAccessor<jsc::Types>::box(BinaryData data) {
    static jsc::String s_buffer = "buffer";
    static jsc::String s_uint8_array = "Uint8Array";

    size_t byte_count = data.size();
    JSValueRef byte_count_value = jsc::Value::from_number(m_ctx, byte_count);
    JSObjectRef uint8_array_constructor = jsc::Object::validated_get_constructor(m_ctx, JSContextGetGlobalObject(m_ctx), s_uint8_array);
    JSObjectRef uint8_array = jsc::Function::construct(m_ctx, uint8_array_constructor, 1, &byte_count_value);

    for (uint32_t i = 0; i < byte_count; i++) {
        JSValueRef num = jsc::Value::from_number(m_ctx, data[i]);
        jsc::Object::set_property(m_ctx, uint8_array, i, num);
    }
    
    return jsc::Object::validated_get_object(m_ctx, uint8_array, s_buffer);
}

} // js
} // realm
