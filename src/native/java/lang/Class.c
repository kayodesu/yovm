/*
 * Author: Jia Yang
 */

#include "../../registry.h"
#include "../../../jvm.h"
#include "../../../rtda/heap/jobject.h"
#include "../../../rtda/ma/jfield.h"
#include "../../../rtda/heap/strpool.h"
#include "../../../util/util.h"
#include "../../../rtda/primitive_types.h"

// native ClassLoader getClassLoader0();
static void getClassLoader0(struct stack_frame *frame)
{
    jvm_abort("error\n");
}

/*
 * Called after security check for system loader access checks have been made.
 *
 * private static native Class<?> forName0
 *  (String name, boolean initialize, ClassLoader loader, Class<?> caller) throws ClassNotFoundException;
 */
static void forName0(struct stack_frame *frame)
{
    struct jobject *so = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_STROBJ(so);
#endif

    const char *class_name0 = jstrobj_value(so);
    char class_name[strlen(class_name0) + 1];
    strcpy(class_name, class_name0);
    // 这里class name 是形如 xxx.xx.xx的形式，将其替换为 xxx/xx/xx的形式
    vm_strrpl(class_name, '.', '/');

    struct jclass *c = classloader_load_class(frame->method->jclass->loader, class_name);

    int initialize = slot_geti(frame->local_vars + 1);
    if (initialize && !c->inited) {
        // todo do init
        jclass_clinit(c, frame->thread);
//        bcr_set_pc(frame->reader, jthread_get_pc(frame->thread));
    }

    os_pushr(frame->operand_stack, (jref) c->clsobj);
}

/*
 * Return the Virtual Machine's Class object for the named primitive type.
 */
// static native Class<?> getPrimitiveClass(String name);
static void getPrimitiveClass(struct stack_frame *frame)
{
    struct jobject *so = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_STROBJ(so);
#endif
    const char *class_name = jstrobj_value(so); // 这里得到的 class_name 是诸如 "int~float" 之类的 primitive type
    struct jclass *c = classloader_load_class(bootstrap_loader, class_name);
    os_pushr(frame->operand_stack, (jref) c->clsobj);
}

/**
 * Returns the  name of the entity (class, interface, array class,
 * primitive type, or void) represented by this {@code Class} object,
 * as a {@code String}.
 *
 * <p> If this class object represents a reference type that is not an
 * array type then the binary name of the class is returned, as specified
 * by
 * <cite>The Java&trade; Language Specification</cite>.
 *
 * <p> If this class object represents a primitive type or void, then the
 * name returned is a {@code String} equal to the Java language
 * keyword corresponding to the primitive type or void.
 *
 * <p> If this class object represents a class of arrays, then the internal
 * form of the name consists of the name of the element type preceded by
 * one or more '{@code [}' characters representing the depth of the array
 * nesting.  The encoding of element type names is as follows:
 *
 * <blockquote><table summary="Element types and encodings">
 * <tr><th> Element Type <th> &nbsp;&nbsp;&nbsp; <th> Encoding
 * <tr><td> boolean      <td> &nbsp;&nbsp;&nbsp; <td align=center> Z
 * <tr><td> byte         <td> &nbsp;&nbsp;&nbsp; <td align=center> B
 * <tr><td> char         <td> &nbsp;&nbsp;&nbsp; <td align=center> C
 * <tr><td> class or interface
 *                       <td> &nbsp;&nbsp;&nbsp; <td align=center> L<i>classname</i>;
 * <tr><td> double       <td> &nbsp;&nbsp;&nbsp; <td align=center> D
 * <tr><td> float        <td> &nbsp;&nbsp;&nbsp; <td align=center> F
 * <tr><td> int          <td> &nbsp;&nbsp;&nbsp; <td align=center> I
 * <tr><td> long         <td> &nbsp;&nbsp;&nbsp; <td align=center> J
 * <tr><td> short        <td> &nbsp;&nbsp;&nbsp; <td align=center> S
 * </table></blockquote>
 *
 * <p> The class or interface name <i>classname</i> is the binary name of
 * the class specified above.
 *
 * <p> Examples:
 * <blockquote><pre>
 * String.class.getName()
 *     returns "java.lang.String"
 * byte.class.getName()
 *     returns "byte"
 * (new Object[3]).getClass().getName()
 *     returns "[Ljava.lang.Object;"
 * (new int[3][4][5][6][7][8][9]).getClass().getName()
 *     returns "[[[[[[[I"
 * </pre></blockquote>
 *
 * @return  the name of the class or interface
 *          represented by this object.
 */
// private native String getName0();
static void getName0(struct stack_frame *frame)
{
    struct jobject *this = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    struct jclass *c = jclsobj_entity_class(this);
    char class_name[strlen(c->class_name) + 1];
    strcpy(class_name, c->class_name);
    // 这里需要的是 java.lang.Object 这样的类名，而非 java/lang/Object
    // 所以需要进行一下替换
    vm_strrpl(class_name, '/', '.');
    os_pushr(frame->operand_stack, (jref) jstrobj_create(class_name));
}

/**
 * Returns the assertion status that would be assigned to this
 * class if it were to be initialized at the time this method is invoked.
 * If this class has had its assertion status set, the most recent
 * setting will be returned; otherwise, if any package default assertion
 * status pertains to this class, the most recent setting for the most
 * specific pertinent package default assertion status is returned;
 * otherwise, if this class is not a system class (i.e., it has a
 * class loader) its class loader's default assertion status is returned;
 * otherwise, the system class default assertion status is returned.
 * <p>
 * Few programmers will have any need for this method; it is provided
 * for the benefit of the JRE itself.  (It allows a class to determine at
 * the time that it is initialized whether assertions should be enabled.)
 * Note that this method is not guaranteed to return the actual
 * assertion status that was (or will be) associated with the specified
 * class when it was (or will be) initialized.
 *
 * @return the desired assertion status of the specified class.
 * @see    java.lang.ClassLoader#setClassAssertionStatus
 * @see    java.lang.ClassLoader#setPackageAssertionStatus
 * @see    java.lang.ClassLoader#setDefaultAssertionStatus
 * @since  1.4
 */
//public boolean desiredAssertionStatus() {
//    ClassLoader loader = getClassLoader();
//    // If the loader is null this is a system class, so ask the VM
//    if (loader == null)
//        return desiredAssertionStatus0(this);
//
//    // If the classloader has been initialized with the assertion
//    // directives, ask it. Otherwise, ask the VM.
//    synchronized(loader.assertionLock) {
//        if (loader.classAssertionStatus != null) {
//            return loader.desiredAssertionStatus(getName());
//        }
//    }
//    return desiredAssertionStatus0(this);
//}
//

/*
 * Retrieves the desired assertion status of this class from the VM
 *
 * private static native boolean desiredAssertionStatus0(Class<?> clazz);
 */
static void desiredAssertionStatus0(struct stack_frame *frame)
{
    // todo 本书不讨论断言。desiredAssertionStatus0（）方法把false推入操作数栈顶
    os_pushi(frame->operand_stack, 0);
}

/**
 * Determines if the specified {@code Object} is assignment-compatible
 * with the object represented by this {@code Class}.  This method is
 * the dynamic equivalent of the Java language {@code instanceof}
 * operator. The method returns {@code true} if the specified
 * {@code Object} argument is non-null and can be cast to the
 * reference type represented by this {@code Class} object without
 * raising a {@code ClassCastException.} It returns {@code false}
 * otherwise.
 *
 * <p> Specifically, if this {@code Class} object represents a
 * declared class, this method returns {@code true} if the specified
 * {@code Object} argument is an instance of the represented class (or
 * of any of its subclasses); it returns {@code false} otherwise. If
 * this {@code Class} object represents an array class, this method
 * returns {@code true} if the specified {@code Object} argument
 * can be converted to an object of the array class by an identity
 * conversion or by a widening reference conversion; it returns
 * {@code false} otherwise. If this {@code Class} object
 * represents an interface, this method returns {@code true} if the
 * class or any superclass of the specified {@code Object} argument
 * implements this interface; it returns {@code false} otherwise. If
 * this {@code Class} object represents a primitive type, this method
 * returns {@code false}.
 *
 * @param   obj the object to check
 * @return  true if {@code obj} is an instance of this class
 *
 * @since JDK1.1
 *
 * public native boolean isInstance(Object obj);
 */
static void isInstance(struct stack_frame *frame)
{
    struct jobject *this = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    jref obj = slot_getr(frame->local_vars + 1);
    os_pushi(frame->operand_stack,
             (obj != NULL && jobject_is_instance_of(obj, jclsobj_entity_class(this))) ? 1 : 0);
}

/**
 * Determines if the class or interface represented by this
 * {@code Class} object is either the same as, or is a superclass or
 * superinterface of, the class or interface represented by the specified
 * {@code Class} parameter. It returns {@code true} if so;
 * otherwise it returns {@code false}. If this {@code Class}
 * object represents a primitive type, this method returns
 * {@code true} if the specified {@code Class} parameter is
 * exactly this {@code Class} object; otherwise it returns
 * {@code false}.
 *
 * <p> Specifically, this method tests whether the type represented by the
 * specified {@code Class} parameter can be converted to the type
 * represented by this {@code Class} object via an identity conversion
 * or via a widening reference conversion. See <em>The Java Language
 * Specification</em>, sections 5.1.1 and 5.1.4 , for details.
 *
 * @param cls the {@code Class} object to be checked
 * @return the {@code boolean} value indicating whether objects of the
 * type {@code cls} can be assigned to objects of this class
 * @exception NullPointerException if the specified Class parameter is null.
 * @since JDK1.1
 *
 * public native boolean isAssignableFrom(Class<?> cls);
 */
static void isAssignableFrom(struct stack_frame *frame)
{
    struct jobject *this = slot_getr(frame->local_vars);
    struct jobject *cls = (struct jobject *) slot_getr(frame->local_vars + 1);
    if (cls == NULL) {
        jthread_throw_null_pointer_exception(frame->thread);
    }
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
    JOBJECT_CHECK_CLSOBJ(cls);
#endif
    bool b = jclass_is_subclass_of(jclsobj_entity_class(cls), jclsobj_entity_class(this));
    os_pushi(frame->operand_stack, b ? 1 : 0);
}

/*
 * Determines if the specified class object represents an interface type.
 *
 * public native boolean isInterface();
 */
static void isInterface(struct stack_frame *frame)
{
    struct jobject *this = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    os_pushi(frame->operand_stack, IS_INTERFACE(jclsobj_entity_class(this)->access_flags) ? 1 : 0);
}

/*
 * Determines if this class object represents an array class.
 *
 * public native boolean isArray();
 */
static void isArray(struct stack_frame *frame)
{
    struct jobject *this = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    os_pushi(frame->operand_stack, jclass_is_array(jclsobj_entity_class(this)) ? 1 : 0);  // todo
}

/**
 * Determines if the specified {@code Class} object represents a
 * primitive type.
 *
 * <p> There are nine predefined {@code Class} objects to represent
 * the eight primitive types and void.  These are created by the Java
 * Virtual Machine, and have the same names as the primitive types that
 * they represent, namely {@code boolean}, {@code byte},
 * {@code char}, {@code short}, {@code int},
 * {@code long}, {@code float}, and {@code double}.
 *
 * <p> These objects may only be accessed via the following public static
 * final variables, and are the only {@code Class} objects for which
 * this method returns {@code true}.
 *
 * @return true if and only if this class represents a primitive type
 *
 * @see     java.lang.Boolean#TYPE
 * @see     java.lang.Character#TYPE
 * @see     java.lang.Byte#TYPE
 * @see     java.lang.Short#TYPE
 * @see     java.lang.Integer#TYPE
 * @see     java.lang.Long#TYPE
 * @see     java.lang.Float#TYPE
 * @see     java.lang.Double#TYPE
 * @see     java.lang.Void#TYPE
 * @since JDK1.1
 *
 * public native boolean isPrimitive();
 */
static void isPrimitive(struct stack_frame *frame)
{
    struct jobject *this =slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    bool b = jclass_is_primitive(jclsobj_entity_class(this));
    os_pushi(frame->operand_stack, b ? 1 : 0);
}

/**
 * Returns the {@code Class} representing the superclass of the entity
 * (class, interface, primitive type or void) represented by this
 * {@code Class}.  If this {@code Class} represents either the
 * {@code Object} class, an interface, a primitive type, or void, then
 * null is returned.  If this object represents an array class then the
 * {@code Class} object representing the {@code Object} class is
 * returned.
 *
 * @return the superclass of the class represented by this object.
 *
 * public native Class<? super T> getSuperclass();
 */
static void getSuperclass(struct stack_frame *frame)
{
    struct jobject *this =slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    struct jclass *entity_class = jclsobj_entity_class(this);

    struct jclass *c = classloader_load_class(frame->method->jclass->loader, entity_class->class_name);
    os_pushr(frame->operand_stack, (jref) (c->super_class != NULL ? c->super_class->clsobj : NULL));
}

/**
 * Determines the interfaces implemented by the class or interface
 * represented by this object.
 *
 * <p> If this object represents a class, the return value is an array
 * containing objects representing all interfaces implemented by the
 * class. The order of the interface objects in the array corresponds to
 * the order of the interface names in the {@code implements} clause
 * of the declaration of the class represented by this object. For
 * example, given the declaration:
 * <blockquote>
 * {@code class Shimmer implements FloorWax, DessertTopping { ... }}
 * </blockquote>
 * suppose the value of {@code s} is an instance of
 * {@code Shimmer}; the value of the expression:
 * <blockquote>
 * {@code s.getClass().getInterfaces()[0]}
 * </blockquote>
 * is the {@code Class} object that represents interface
 * {@code FloorWax}; and the value of:
 * <blockquote>
 * {@code s.getClass().getInterfaces()[1]}
 * </blockquote>
 * is the {@code Class} object that represents interface
 * {@code DessertTopping}.
 *
 * <p> If this object represents an interface, the array contains objects
 * representing all interfaces extended by the interface. The order of the
 * interface objects in the array corresponds to the order of the interface
 * names in the {@code extends} clause of the declaration of the
 * interface represented by this object.
 *
 * <p> If this object represents a class or interface that implements no
 * interfaces, the method returns an array of length 0.
 *
 * <p> If this object represents a primitive type or void, the method
 * returns an array of length 0.
 *
 * <p> If this {@code Class} object represents an array type, the
 * interfaces {@code Cloneable} and {@code java.io.Serializable} are
 * returned in that order.
 *
 * @return an array of interfaces implemented by this class.
 */
//private native Class<?>[] getInterfaces0();
static void getInterfaces0(struct stack_frame *frame)
{
    struct jobject *this = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    struct jclass *entity_class = jclsobj_entity_class(this);

    struct jclass *arr_cls = classloader_load_class(frame->method->jclass->loader, "[java/lang/Class;");
    struct jobject *interfaces = jarrobj_create(arr_cls, entity_class->interfaces_count);
    for (int i = 0; i < entity_class->interfaces_count; i++) {
        assert(entity_class->interfaces[i] != NULL);
        jarrobj_set(struct jobject *, interfaces, i, entity_class->interfaces[i]->clsobj);
    }

    os_pushr(frame->operand_stack, (jref) interfaces);
}

/*
 * Returns the representing the component type of an array.
 * If this class does not represent an array class this method returns null.
 *
 * public native Class<?> getComponentType();
 */
static void getComponentType(struct stack_frame *frame)
{
    struct jobject *this = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    struct jclass *entity_class = jclsobj_entity_class(this);

    struct jclass *component_cls = jclass_component_class(entity_class);
    if (component_cls != NULL) {
        os_pushr(frame->operand_stack, component_cls->clsobj);
    } else {
        os_pushr(frame->operand_stack, NULL);
    }
}

/**
 * Returns the Java language modifiers for this class or interface, encoded
 * in an integer. The modifiers consist of the Java Virtual Machine's
 * constants for {@code public}, {@code protected},
 * {@code private}, {@code final}, {@code static},
 * {@code abstract} and {@code interface}; they should be decoded
 * using the methods of class {@code Modifier}.
 *
 * <p> If the underlying class is an array class, then its
 * {@code public}, {@code private} and {@code protected}
 * modifiers are the same as those of its component type.  If this
 * {@code Class} represents a primitive type or void, its
 * {@code public} modifier is always {@code true}, and its
 * {@code protected} and {@code private} modifiers are always
 * {@code false}. If this object represents an array class, a
 * primitive type or void, then its {@code final} modifier is always
 * {@code true} and its interface modifier is always
 * {@code false}. The values of its other modifiers are not determined
 * by this specification.
 *
 * <p> The modifier encodings are defined in <em>The Java Virtual Machine
 * Specification</em>, table 4.1.
 *
 * @return the {@code int} representing the modifiers for this class
 * @see     java.lang.reflect.Modifier
 * @since JDK1.1
 */
//public native int getModifiers();
static void getModifiers(struct stack_frame *frame)
{
    struct jobject *this = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    os_pushi(frame->operand_stack, jclsobj_entity_class(this)->access_flags);
}

/**
 * Gets the signers of this class.
 *
 * @return  the signers of this class, or null if there are no signers.  In
 *          particular, this method returns null if this object represents
 *          a primitive type or void.
 * @since   JDK1.1
 */
//public native Object[] getSigners();
//static void getSigners(struct stack_frame *frame)
//{
//    jvm_abort("");
//}


/**
 * Set the signers of this class.
 */
//native void setSigners(Object[] signers);
//static void setSigners(struct stack_frame *frame)
//{
//    jvm_abort("");
//}

// private native Object[] getEnclosingMethod0();
static void getEnclosingMethod0(struct stack_frame *frame)
{
    jref this = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this);
#endif
    struct jclass *c = jclsobj_entity_class(this);
    if (c->enclosing_info[0] == NULL) {
        os_pushr(frame->operand_stack, NULL);
        return;
    }

    jref result = jarrobj_create(classloader_load_class(frame->method->jclass->loader, "[Ljava/lang/Object;"), 3);
    for (int i = 0; i < 3; i++) {
        jarrobj_set(struct jobject *, result, i, c->enclosing_info[i]);
    }

    os_pushr(frame->operand_stack, result);
}

/*
 * Returns the ProtectionDomain of this class.
 */
//private native java.security.ProtectionDomain getProtectionDomain0();
//static void getProtectionDomain0(struct stack_frame *frame)
//{
//    jvm_abort("");
//}

// Generic signature handling
//private native String getGenericSignature0();
static void getGenericSignature0(struct stack_frame *frame)
{
    jvm_abort("");
}

// Annotations handling
//native byte[] getRawAnnotations();
static void getRawAnnotations(struct stack_frame *frame)
{
    jvm_abort("");
}

//// Since 1.8
//native byte[] getRawTypeAnnotations();
//static void getRawTypeAnnotations(struct stack_frame *frame)
//{
//    jvm_abort("");
//}


//static byte[] getExecutableTypeAnnotationBytes(Executable ex) {
//return getReflectionFactory().getExecutableTypeAnnotationBytes(ex);
//}
//

// native ConstantPool getConstantPool();
static void getConstantPool(struct stack_frame *frame)
{
    jvm_abort("");
}

// private native Field[] getDeclaredFields0(boolean publicOnly);
static void getDeclaredFields0(struct stack_frame *frame)
{
    struct jobject *this_cls_obj = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this_cls_obj);
#endif

    bool public_only = slot_geti(frame->local_vars + 1) != 0;

    struct classloader *loader = frame->method->jclass->loader;
    struct jclass *cls = classloader_load_class(loader, jclsobj_entity_class(this_cls_obj)->class_name);

    struct jfield *fields = cls->fields;
    jint fields_count = public_only ? cls->public_fields_count : cls->fields_count;

    struct jclass *jlrf_cls = classloader_load_class(loader, "java/lang/reflect/Field");
    char *arr_cls_name = get_arr_class_name(jlrf_cls->class_name);
    struct jobject *jlrf_arr = jarrobj_create(classloader_load_class(loader, arr_cls_name), fields_count);
    os_pushr(frame->operand_stack, (jref) jlrf_arr);
    free(arr_cls_name);

    /*
     * Field(Class<?> declaringClass, String name, Class<?> type,
     *      int modifiers, int slot, String signature, byte[] annotations)
     */
    struct jmethod *field_constructor = jclass_get_constructor(jlrf_cls,
            "(Ljava/lang/Class;" "Ljava/lang/String;" "Ljava/lang/Class;" "II" "Ljava/lang/String;" "[B)V");
    assert(field_constructor != NULL);

    // invoke constructor of class java/lang/reflect/Field
    for (int i = 0; i < fields_count; i++) {
        struct jobject *jlrf_obj = jobject_create(jlrf_cls);
//        *(struct jobject **)jobject_index(jlrf_arr, i) = jlrf_obj;
        jarrobj_set(struct jobject *, jlrf_arr, i, jlrf_obj);

        jthread_invoke_method(frame->thread, field_constructor, (struct slot[]) {
                rslot(jlrf_obj), // this
                rslot((jref) this_cls_obj), // declaring class
                rslot((jref) get_str_from_pool(frame->method->jclass->loader, fields[i].name)), // name
                rslot((jref) jfield_get_type(fields + i)), // type
                islot(fields[i].access_flags), // modifiers
                islot(fields[i].id), // slot   todo
                rslot(NULL), // signature  todo
                rslot(NULL), // annotations  todo
        });
    }
}

/*
 * 注意 getDeclaredMethods 和 getMethods 方法的不同。
 * getDeclaredMethods(),该方法是获取本类中的所有方法，包括私有的(private、protected、默认以及public)的方法。
 * getMethods(),该方法是获取本类以及父类或者父接口中所有的公共方法(public修饰符修饰的)
 *
 * getDeclaredMethods 强调的是本类中定义的方法，不包括继承而来的。
 *
 * private native Method[] getDeclaredMethods0(boolean publicOnly);
 */
static void getDeclaredMethods0(struct stack_frame *frame)
{
    struct jobject *this_cls_obj = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this_cls_obj);
#endif

    bool public_only = slot_geti(frame->local_vars + 1) != 0;

    struct classloader *loader = frame->method->jclass->loader;
    struct jclass *cls = classloader_load_class(loader, jclsobj_entity_class(this_cls_obj)->class_name);

    struct jmethod *methods = cls->methods;
    jint methods_count = public_only ? cls->public_methods_count : cls->methods_count;

    struct jclass *jlrm_cls = classloader_load_class(loader, "java/lang/reflect/Method");
    char *arr_cls_name = get_arr_class_name(jlrm_cls->class_name);
    struct jobject *jlrm_arr = jarrobj_create(classloader_load_class(loader, arr_cls_name), methods_count);
    os_pushr(frame->operand_stack, (jref) jlrm_arr);
    free(arr_cls_name);

    /*
     * Method(Class<?> declaringClass, String name, Class<?>[] parameterTypes, Class<?> returnType,
     *      Class<?>[] checkedExceptions, int modifiers, int slot, String signature,
     *      byte[] annotations, byte[] parameterAnnotations, byte[] annotationDefault)
     */
    struct jmethod *method_constructor = jclass_get_constructor(jlrm_cls,
                "(Ljava/lang/Class;" "Ljava/lang/String;" "[Ljava/lang/Class;" "Ljava/lang/Class;"
                "[Ljava/lang/Class;" "II" "Ljava/lang/String;" "[B[B[B)V");
    assert(method_constructor != NULL);

    // invoke constructor of class java/lang/reflect/Method
    for (int i = 0; i < methods_count; i++) {
        struct jobject *jlrf_obj = jobject_create(jlrm_cls);
        jarrobj_set(struct jobject *, jlrm_arr, i, jlrf_obj);

        jthread_invoke_method(frame->thread, method_constructor, (struct slot[]) {
                rslot(jlrf_obj),        // this
                rslot((jref) this_cls_obj), // declaring class
                rslot((jref) get_str_from_pool(frame->method->jclass->loader, methods[i].name)), // name
                rslot((jref) jmethod_get_parameter_types(methods + i)), // parameter types
                rslot(jmethod_get_return_type(methods + i)),     // return type
                rslot((jref) jmethod_get_exception_types(methods + i)), // checked exceptions
                islot(methods[i].access_flags), // modifiers
                islot(0), // slot   todo
                rslot(NULL), // signature  todo
                rslot(NULL), // annotations  todo
                rslot(NULL), // parameter annotations  todo
                rslot(NULL), // annotation default  todo
        });
    }
}

// private native Constructor<T>[] getDeclaredConstructors0(boolean publicOnly);
static void getDeclaredConstructors0(struct stack_frame *frame)
{
    struct jobject *this_cls_obj = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this_cls_obj);
#endif
    bool public_only = slot_geti(frame->local_vars + 1) != 0;

    struct classloader *loader = frame->method->jclass->loader;
    struct jclass *cls = classloader_load_class(loader, jclsobj_entity_class(this_cls_obj)->class_name);

    int constructors_count;
    struct jmethod **constructors = jclass_get_constructors(cls, public_only, &constructors_count);

    struct jclass *jlrc_cls = classloader_load_class(loader, "java/lang/reflect/Constructor");
    char *arr_cls_name = get_arr_class_name(jlrc_cls->class_name);
    struct jobject *jlrc_arr = jarrobj_create(classloader_load_class(loader, arr_cls_name), constructors_count);
    os_pushr(frame->operand_stack, (jref) jlrc_arr);
    free(arr_cls_name);

    /*
     * Constructor(Class<T> declaringClass, Class<?>[] parameterTypes,
     *      Class<?>[] checkedExceptions, int modifiers, int slot,
     *      String signature, byte[] annotations, byte[] parameterAnnotations)
     */
    struct jmethod *constructor_constructor = jclass_get_constructor(jlrc_cls,
                "(Ljava/lang/Class;" "[Ljava/lang/Class;" "[Ljava/lang/Class;" "II" "Ljava/lang/String;" "[B[B)V");
    assert(constructor_constructor != NULL);

    // invoke constructor of class java/lang/reflect/Constructor
    for (int i = 0; i < constructors_count; i++) {
        struct jobject *jlrf_obj = jobject_create(jlrc_cls);
        jarrobj_set(struct jobject *, jlrc_arr, i, jlrf_obj);

        jthread_invoke_method(frame->thread, constructor_constructor, (struct slot[]) {
                rslot(jlrf_obj), // this
                rslot((jref) this_cls_obj), // declaring class
                rslot((jref) jmethod_get_parameter_types(constructors[i])),  // parameter types
                rslot((jref) jmethod_get_exception_types(constructors[i])),  // checked exceptions
                islot(constructors[i]->access_flags), // modifiers
                islot(0), // slot   todo
                rslot(NULL), // signature  todo
                rslot(NULL), // annotations  todo
                rslot(NULL), // parameter annotations  todo
        });
    }
}

/**
 * If the class or interface represented by this {@code Class} object
 * is a member of another class, returns the {@code Class} object
 * representing the class in which it was declared.  This method returns
 * null if this class or interface is not a member of any other class.  If
 * this {@code Class} object represents an array class, a primitive
 * type, or void,then this method returns null.
 *
 * 如果此类为内部类，返回其外部类
 *
 * @return the declaring class for this class
 * @throws SecurityException
 *         If a security manager, <i>s</i>, is present and the caller's
 *         class loader is not the same as or an ancestor of the class
 *         loader for the declaring class and invocation of {@link
 *         SecurityManager#checkPackageAccess s.checkPackageAccess()}
 *         denies access to the package of the declaring class
 * @since JDK1.1
 *
 * private native Class<?> getDeclaringClass0();
 */
static void getDeclaringClass0(struct stack_frame *frame)
{
    struct jobject *this_cls_obj = slot_getr(frame->local_vars);
#ifdef JVM_DEBUG
    JOBJECT_CHECK_CLSOBJ(this_cls_obj);
#endif
    struct jclass *entity_class = jclsobj_entity_class(this_cls_obj);

    if (jclass_is_array(entity_class) || is_primitive_array(entity_class)) {
        os_pushr(frame->operand_stack, NULL);
        return;
    }

    char declaring_class_name[strlen(entity_class->class_name) + 1];
    strcpy(declaring_class_name, entity_class->class_name);
    char *last_dollar = strrchr(declaring_class_name, '$'); // 内部类标识：out_class_name$inner_class_name
    if (last_dollar == NULL) {
        os_pushr(frame->operand_stack, NULL);
        return;
    }

    *last_dollar = 0;
    os_pushr(frame->operand_stack,
             (jref) classloader_load_class(frame->method->jclass->loader, declaring_class_name)->clsobj);
}

/*
 * getClasses和getDeclaredClasses的区别：
 * getClasses得到该类及其父类所有的public的内部类。
 * getDeclaredClasses得到该类所有的内部类，除去父类的。
 *
 * private native Class<?>[] getDeclaredClasses0();
 */
//static void getDeclaredClasses0(struct stack_frame *frame)
//{
//    jvm_abort("");
//}


void java_lang_Class_registerNatives()
{
    register_native_method("java/lang/Class~registerNatives~()V", registerNatives);
    register_native_method("java/lang/Class~getClassLoader0~()Ljava/lang/ClassLoader;", getClassLoader0);
    register_native_method("java/lang/Class~getPrimitiveClass~(Ljava/lang/String;)Ljava/lang/Class;", getPrimitiveClass);
    register_native_method("java/lang/Class~getName0~()Ljava/lang/String;", getName0);
    register_native_method("java/lang/Class~forName0~"
                                   "(Ljava/lang/String;ZLjava/lang/ClassLoader;Ljava/lang/Class;)Ljava/lang/Class;",
                           forName0);
    register_native_method("java/lang/Class~desiredAssertionStatus0~(Ljava/lang/Class;)Z", desiredAssertionStatus0);

    register_native_method("java/lang/Class~isInstance~(Ljava/lang/Object;)Z", isInstance);
    register_native_method("java/lang/Class~isAssignableFrom~(Ljava/lang/Class;)Z", isAssignableFrom);
    register_native_method("java/lang/Class~isInterface~()Z", isInterface);
    register_native_method("java/lang/Class~isArray~()Z", isArray);
    register_native_method("java/lang/Class~isPrimitive~()Z", isPrimitive);

    register_native_method("java/lang/Class~getSuperclass~()Ljava/lang/Class;", getSuperclass);
    register_native_method("java/lang/Class~getInterfaces0~()[Ljava/lang/Class;", getInterfaces0);
    register_native_method("java/lang/Class~getComponentType~()Ljava/lang/Class;", getComponentType);
    register_native_method("java/lang/Class~getModifiers~()I", getModifiers);
    register_native_method("java/lang/Class~getEnclosingMethod0~()[Ljava/lang/Object;", getEnclosingMethod0);
    register_native_method("java/lang/Class~getDeclaringClass0~()Ljava/lang/Class;", getDeclaringClass0);
    register_native_method("java/lang/Class~getGenericSignature0~()Ljava/lang/String;", getGenericSignature0);
    register_native_method("java/lang/Class~getConstantPool~()Lsun/reflect/ConstantPool;", getConstantPool);

    register_native_method("java/lang/Class~getRawAnnotations~()[B", getRawAnnotations);

    register_native_method("java/lang/Class~getDeclaredFields0~(Z)[Ljava/lang/reflect/Field;", getDeclaredFields0);
    register_native_method("java/lang/Class~getDeclaredMethods0~(Z)[Ljava/lang/reflect/Method;", getDeclaredMethods0);
    register_native_method("java/lang/Class~getDeclaredConstructors0~(Z)[Ljava/lang/reflect/Constructor;", getDeclaredConstructors0);
}
