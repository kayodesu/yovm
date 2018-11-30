/*
 * Author: Jia Yang
 */

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "classloader.h"
#include "../util/util.h"
#include "minizip/unzip.h"
#include "../rtda/ma/jclass.h"
#include "../rtda/ma/access.h"
#include "../rtda/ma/jfield.h"
#include "../rtda/ma/primitive_types.h"
#include "../util/hashmap.h"
#include "../rtda/heap/jobject.h"

struct classloader {
    struct jclass *jclass_class; // java.lang.Class 的类
    struct hashmap *loaded_class_pool; // 保存 jclass *
    struct hashmap *classobj_pool; // java.lang.Class 类的对象池，保存 jobject *
};


struct bytecode_content {
    s1 *bytecode;
    size_t len;
};

static struct bytecode_content invalid_bytecode_content = { NULL, 0 };

#define IS_INVALID(bytecode_content) ((bytecode_content).bytecode == NULL || (bytecode_content).len == 0)

static struct bytecode_content read_class_from_jar (const char *jar_path, const char *class_name)
{
    unz_global_info64 global_info;
    unz_file_info64 file_info;

    unzFile jar_file = unzOpen64(jar_path);
    if (jar_file == NULL) {
        // todo error
        printvm("unzOpen64 failed: %s\n", jar_path);
        return invalid_bytecode_content;
    }
    if (unzGetGlobalInfo64(jar_file, &global_info) != UNZ_OK) {
        // todo throw “文件错误”;
        unzClose(jar_file);
        printvm("unzGetGlobalInfo64 failed: %s\n", jar_path);
        return invalid_bytecode_content;
    }

    if (unzGoToFirstFile(jar_file) != UNZ_OK) {
        // todo throw “文件错误”;
        unzClose(jar_file);
        printvm("unzGoToFirstFile failed: %s\n", jar_path);
        return invalid_bytecode_content;
    }

    for (int i = 0; i < global_info.number_entry; i++) {
        char file_name[PATH_MAX];
        if (unzGetCurrentFileInfo64(jar_file, &file_info, file_name, sizeof(file_name), NULL, 0, NULL, 0) != UNZ_OK) {
            unzClose(jar_file);
            printvm("unzGetCurrentFileInfo64 failed: %s\n", jar_path);
            return invalid_bytecode_content;
        }

        char *p = strrchr(file_name, '.');
        if (p != NULL && strcmp(p, ".class") == 0) {
            *p = 0; // 去掉后缀

            if (strcmp(file_name, class_name) == 0) {
                // find out!
                if (unzOpenCurrentFile(jar_file) != UNZ_OK) {
                    // todo error
                    unzClose(jar_file);
                    printvm("unzOpenCurrentFile failed: %s\n", jar_path);
                    return invalid_bytecode_content;
                }

                size_t uncompressed_size = file_info.uncompressed_size;
//                s1 *bytecode = malloc(sizeof(s1) * uncompressed_size);
                VM_MALLOCS(s1, uncompressed_size, bytecode);
                if (unzReadCurrentFile(jar_file, bytecode, (unsigned int) uncompressed_size) != uncompressed_size) {
                    // todo error
                    unzCloseCurrentFile(jar_file);  // todo 干嘛的
                    unzClose(jar_file);
                    printvm("unzReadCurrentFile failed: %s\n", jar_path);
                    return invalid_bytecode_content;
                }
                unzCloseCurrentFile(jar_file); // todo 干嘛的
                unzClose(jar_file);
                return (struct bytecode_content) { bytecode, uncompressed_size };
            }
        }

        int t = unzGoToNextFile(jar_file);
        if (t == UNZ_END_OF_LIST_OF_FILE) {
            break;
        }
        if (t != UNZ_OK) {
            // todo error
            unzClose(jar_file);
            printvm("unzGoToNextFile failed: %s\n", jar_path);
            return invalid_bytecode_content;
        }
    }

    unzClose(jar_file);
    return invalid_bytecode_content;
}

static struct bytecode_content read_class_from_dir(const char *dir_path, const char *class_name)
{
    if (dir_path == NULL || strlen(dir_path) == 0) {
        return invalid_bytecode_content;
    }

    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        printvm("open dir failed. %s\n", dir_path);
    }

    struct dirent *entry;
    struct stat statbuf;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char abspath[PATH_MAX];
        sprintf(abspath, "%s/%s\0", dir_path, entry->d_name); // 绝对路径
        // 将路径中的 '\' 替换为 '/'
        char *tmp = abspath;
        while ((tmp = strchr(tmp, '\\')) != NULL) {
            *tmp = '/';
        }

        stat(abspath, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) { // 目录
            struct bytecode_content content = read_class_from_dir(abspath, class_name);
            if (!IS_INVALID(content)) {
                return content; // find out
            }
        } else if (S_ISREG(statbuf.st_mode)) { // 常规文件
            char *suffix = strrchr(abspath, '.');
            if (suffix != NULL) {
                if (strcmp(suffix, ".jar") == 0) {
                    struct bytecode_content content = read_class_from_jar(abspath, class_name);
                    if (!IS_INVALID(content)) {
                        return content; // find out
                    }
                } else if (strcmp(suffix, ".class") == 0) {
                    // class_name 不带 .class 后缀，而 abspath 有 .class 后缀
                    *suffix = 0; // 去掉 abspath 的 .class 后缀
                    if (strend(abspath, class_name)) {
                        *suffix = '.'; // 加回 abspath 的 .class 后缀
                        // 找到 class 文件了，读其内容。
                        FILE *f = fopen(abspath, "rb");
                        if (f == NULL) {
                            jvm_abort("open file failed: %s\n", abspath);
                        }

                        fseek(f, 0, SEEK_END); //定位到文件末
                        size_t file_len = (size_t) ftell(f); //文件长度
                        if (file_len == -1) {
                            jvm_abort("ftell error: %s\n", abspath);
                        }

                        VM_MALLOCS(s1, file_len, bytecode);
                        fseek(f, 0, SEEK_SET);
                        fread(bytecode, 1, file_len, f);
                        fclose(f);
                        return (struct bytecode_content) { bytecode, file_len };
                    }
                }
            }
        }
    }

    // not find
    closedir(dir);
    return invalid_bytecode_content;
}

static struct bytecode_content read_class(const char *class_name)
{
    struct bytecode_content content = read_class_from_dir(bootstrap_classpath, class_name);
    if (!IS_INVALID(content))
        return content;

    content = read_class_from_dir(extension_classpath, class_name);
    if (!IS_INVALID(content))
        return content;

   return read_class_from_dir(user_classpath, class_name);
}

static struct jobject* get_jclass_obj_from_pool(struct classloader *loader, const char *class_name)
{
    struct jobject *clsobj = hashmap_find(loader->classobj_pool, class_name);
//    HASH_FIND_STR(loader->classobj_pool, class_name, clsobj);
    if (clsobj != NULL) {
        printvm("find out class obj: %s\n", class_name);  /////////////////////////////////////
        return clsobj;
    }

    clsobj = jclassobj_create(loader->jclass_class, class_name);
    hashmap_put(loader->classobj_pool, class_name, clsobj);
//    HASH_ADD_KEYPTR(hh, loader->classobj_pool, class_name, strlen(class_name), clsobj);
    return clsobj;
}

//static void set_clsobj(struct classloader *loader, struct jclass *c)
//{
////    struct jclass *c = c0;
//    c->clsobj = get_jclass_obj_from_pool(loader, c->class_name);
//}

struct classloader* classloader_create()
{
    VM_MALLOC(struct classloader, loader);

//    loader->loaded_class_pool = NULL; // hash must be declared as a NULL-initialized pointer
//    loader->classobj_pool = NULL;     // hash must be declared as a NULL-initialized pointer
    loader->loaded_class_pool = hashmap_create_str_key();
    loader->classobj_pool = hashmap_create_str_key();

    /*
     * 先加载java.lang.Class类，
     * 这又会触发java.lang.Object等类和接口的加载。
     */
    loader->jclass_class = NULL; // 不要胡乱删除这句。因为classloader_load_class中要判断loader->jclass_class是否为NULL。
    loader->jclass_class = classloader_load_class(loader, "java/lang/Class");

    // 加载基本类型（int, float, etc.）的 class
    for (int i = 0; i < PRIMITIVE_TYPE_COUNT; i++) {
        struct jclass *pc = jclass_create_primitive_class(loader, primitive_types[i].name);
        hashmap_put(loader->loaded_class_pool, primitive_types[i].name, pc);
    }

    // todo
    //    for (auto &primitiveType : primitiveTypes) {
//        loadPrimitiveClasses(primitiveType.name);
//        loadClass(primitiveType.arrayClassName);
//        loadClass(primitiveType.wrapperClassName);
//    }

    // 给已经加载的每一个类关联类对象。
    int size = hashmap_size(loader->loaded_class_pool);
    void *values[size];
    hashmap_values(loader->loaded_class_pool, values);
    for (int i = 0; i < size; i++) {
        struct jclass *c = values[i];
        c->clsobj = get_jclass_obj_from_pool(loader, c->class_name);
    }

    return loader;
}

static struct jclass* loading(struct classloader *loader, const char *class_name)
{
    struct bytecode_content content = read_class(class_name);
    if (IS_INVALID(content)) {
        jvm_abort("class not find: %s\n", class_name);
    }

    return jclass_create_by_classfile(loader, classfile_create(content.bytecode, content.len));
}

static struct jclass* verification(struct jclass *c)
{
    if (c->magic != 0xcafebabe) {
        jvm_abort("error. magic = %u(0x%x)\n", c->magic, c->magic);
    }
    // todo 验证版本号
    return c;
}

static struct jclass* preparation(struct jclass *c)
{
    const struct rtcp* const rtcp = c->rtcp;

    // 如果静态变量属于基本类型或String类型，有final修饰符，
    // 且它的值在编译期已知，则该值存储在class文件常量池中。
    for (int i = 0; i < c->fields_count; i++) {
        struct jfield *field = c->fields[i];
        if (!IS_STATIC(field->access_flags)) {
            continue;
        }

        const int index = c->fields[i]->constant_value_index;

        // 值已经在常量池中了
        const bool b = (index != INVALID_CONSTANT_VALUE_INDEX);
// todo
//        c->setFieldValue(field.id, field.descriptor,
//        [&]() -> jint { return b ? rtcp->getInt(index) : 0; },  // todo byte short 等都是用 int 表示的吗
//        [&]() -> jfloat { return b ? rtcp->getFloat(index) : 0; },
//        [&]() -> jlong { return b ? rtcp->getLong(index) : 0; },
//        [&]() -> jdouble { return b ? rtcp->getDouble(index) : 0; },
//        [&]() -> jref {
//            if (field.descriptor == "Ljava/lang/String;" and b) {
//                // todo
//                const string &str = rtcp->getStr(index);
//                return JStringObj::newJStringObj(jclass->loader, strToJstr(str));
//            }
//            return nullptr;
//        });
    }

    return c;
}

/*
 * 解析（Resolution）是根据运行时常量池的符号引用来动态决定具体的值的过程。
 */
static struct jclass* resolution(struct jclass *c)
{
    // todo
    return c;
}

static struct jclass* initialization(struct jclass *c)
{
    // todo
    return c;
}

static struct jclass* load_non_arr_class(struct classloader *loader, const char *class_name)
{
// todo 解析，初始化是在这里进行，还是待使用的时候再进行
    struct jclass *c = loading(loader, class_name);
    return initialization(resolution(preparation(verification(c))));
}

struct jclass* classloader_load_class(struct classloader *loader, const char *class_name)
{
    struct jclass *c = hashmap_find(loader->loaded_class_pool, class_name);
    if (c != NULL) {
        assert(strcmp(c->class_name, class_name) == 0);
        return c;
    }

    if (class_name[0] == '[') {
        c = jclass_create_arr_class(loader, class_name);
    } else {
        c = load_non_arr_class(loader, class_name);
    }

    if (c == NULL) {
        jvm_abort("error"); // todo
    }

//    if (verbose)
//        printvm("load class %s\n", c->class_name);

    assert(strcmp(c->class_name, class_name) == 0);

    if (loader->jclass_class != NULL) {
        c->clsobj = get_jclass_obj_from_pool(loader, class_name);
    }

    hashmap_put(loader->loaded_class_pool, class_name, c);
    return c;
}

void classloader_destroy(struct classloader *loader)
{
    assert(loader != NULL);

    if (loader->loaded_class_pool != NULL) {
        hashmap_destroy(loader->loaded_class_pool);
    }
    if (loader->classobj_pool != NULL) {
        hashmap_destroy(loader->classobj_pool);
    }
    if (loader->jclass_class != NULL) {
        jclass_destroy(loader->jclass_class);
    }

    free(loader);
}