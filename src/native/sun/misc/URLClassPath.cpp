#include "../../jnidef.h"
#include "../../../vmdef.h"

/*
 * Author: Yo Ka
 */

// private static native URL[] getLookupCacheURLs(ClassLoader var0);
static jobjectArray getLookupCacheURLs(jclsref clazz)
{
    // todo
    return nullptr;
}

static JNINativeMethod methods[] = {
        JNINativeMethod_registerNatives,
        { "getLookupCacheURLs", "(Ljava/lang/ClassLoader;)[Ljava/net/URL;", (void *) getLookupCacheURLs },
};

void sun_misc_URLClassPath_registerNatives()
{
    registerNatives("sun/misc/URLClassPath", methods, ARRAY_LENGTH(methods));
}