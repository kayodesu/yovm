/*
 * Author: Jia Yang
 */

#ifndef JVM_CLASSFILE_H
#define JVM_CLASSFILE_H

#include "../jtypes.h"

struct attribute;

struct member_info {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    struct attribute *attributes;
};

struct constant;

struct classfile {
    u4 magic;
    u2 minor_version;
    u2 major_version;

    u2 constant_pool_count;
    struct constant *constant_pool; // The constant_pool table is indexed from 1 to constant_pool_count - 1.

    u2 access_flags;
    u2 this_class;
    u2 super_class;

    u2 interfaces_count;
    u2 *interfaces;

    u2 fields_count, methods_count;
    struct member_info *fields, *methods;

    u2 attributes_count;
    struct attribute *attributes;
};

struct classfile* classfile_create(s1 *bytecode, size_t len);

void classfile_destroy(struct classfile* cf);

#endif //JVM_CLASSFILE_H
