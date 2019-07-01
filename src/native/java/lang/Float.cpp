/*
 * Author: Jia Yang
 */

#include "../../registry.h"
#include "../../../slot.h"
#include "../../../rtda/thread/Frame.h"

// public static native int floatToRawIntBits(float value);
static void floatToRawIntBits(Frame *frame)
{
    jfloat f = frame_locals_getf(frame, 0);
    frame_stack_pushi(frame, float_to_raw_int_bits(f));
}

// public static native float intBitsToFloat(int value);
static void intBitsToFloat(Frame *frame)
{
    jint i = frame_locals_geti(frame, 0);
    frame_stack_pushf(frame, int_bits_to_float(i));
}

void java_lang_Float_registerNatives()
{
    register_native_method("java/lang/Float", "floatToRawIntBits", "(F)I", floatToRawIntBits);
    register_native_method("java/lang/Float", "intBitsToFloat", "(I)F", intBitsToFloat);
}