/*
 * Author: Yo Ka
 */

#include <assert.h>
#include "../../../../symbol.h"
#include "../../../jnidef.h"

// private static native void initialize();
static void initialize(JNIEnv *env, jclass clazz)
{
    // todo
    jclass sys = (*env)->FindClass(env, S(java_lang_System));
    cli_initClass(sys);
    
    // Method *m = sys->lookupStaticMethod("initPhase1", S(___V));
    // assert(m != nullptr);
    // execJavaFunc(m);

    jmethodID m = (*env)->GetStaticMethodID(env, sys, "initPhase1", S(___V));
    assert(m != NULL);
    (*env)->CallStaticVoidMethod(env, sys, m);
}

/*
 * Returns the first non-null class loader up the execution stack,
 * or null if only code from the null class loader is on the stack.
 *
 * public static native ClassLoader latestUserDefinedLoader();
 */
static jobject latestUserDefinedLoader(JNIEnv *env, jclass clazz)
{
    jvm_abort("latestUserDefinedLoader"); // todo
}

static JNINativeMethod methods[] = {
        JNINativeMethod_registerNatives,
        { "initialize", "()V", (void *) initialize },
        { "latestUserDefinedLoader", "()Ljava/lang/ClassLoader;", (void *) latestUserDefinedLoader },
};

void jdk_internal_misc_VM_registerNatives()
{
    registerNatives("jdk/internal/misc/VM", methods, ARRAY_LENGTH(methods));
}