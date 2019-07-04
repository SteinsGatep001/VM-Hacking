
## Why?
理解和掌握QOM是学习QEMU代码的重要一步

### 各种架构CPU的模拟和实现 
QEMU中要实现对各种CPU架构的模拟，而且对于一种架构的CPU，比如X86_64架构的CPU，由于包含的特性不同，也会有不同的CPU模型。任何CPU中都有CPU通用的属性，同时也包含各自特有的属性。为了便于模拟这些CPU模型，面向对象的变成模型是必不可少的。

### 模拟device与bus的关系 
在主板上，一个device会通过bus与其他的device相连接，一个device上可以通过不同的bus端口连接到其他的device，而其他的device也可以进一步通过bus与其他的设备连接，同时一个bus上也可以连接多个device，这种device连bus、bus连device的关系，qemu是需要模拟出来的。为了方便模拟设备的这种特性，面向对象的编程模型也是必不可少的。

## struct
`struct TypeImpl`位于`qom/object.c`中，`ObjectClass,Object,TypeInfo`在`include/qom/object.h`中

### TypeImpl
```C
struct TypeImpl
{
    const char *name;
    size_t class_size;
    size_t instance_size;
    /** Contructor & Destructor */
    void (*class_init)(ObjectClass *klass, void *data);
    void (*class_base_init)(ObjectClass *klass, void *data);
    void (*class_finalize)(ObjectClass *klass, void *data);

    void *class_data;

    /** Contructor & Destructor */
    void (*instance_init)(Object *obj);
    void (*instance_post_init)(Object *obj);
    void (*instance_finalize)(Object *obj);

    bool abstract;
    const char *parent;
    TypeImpl *parent_type;
    /** ---该类型对应的类的指针--- */
    ObjectClass *class;
    int num_interfaces;

    InterfaceImpl interfaces[MAX_INTERFACES];
};
```
其中`InterfaceImpl`是一个`type`的名字
```C
struct InterfaceImpl
{
    const char *typename;
};
```

### ObjectClass
```C
/**
 * ObjectClass:
 *
 * The base for all classes.  The only thing that #ObjectClass contains is an
 * integer type handle.
 */
struct ObjectClass
{
    /*< private >*/
    Type type;
    GSList *interfaces;
    const char *object_cast_cache[OBJECT_CLASS_CAST_CACHE];
    const char *class_cast_cache[OBJECT_CLASS_CAST_CACHE];
    ObjectUnparent *unparent;
    /**GHashTable存储时指定Key演算出Hash值以決定数据存储位置，要取回数据，也是指定Key算出数据存储位置，以快速取得数据**/
    GHashTable *properties;
};
```

## process
一个典型的设备(`kvm`)注册、初始化、启动流程

### register module
```C
static const TypeInfo kvm_accel_type = {
    .name = TYPE_KVM_ACCEL,
    .parent = TYPE_ACCEL,
    .class_init = kvm_accel_class_init,
    .instance_size = sizeof(KVMState),
};
static void kvm_type_init(void)
{
    type_register_static(&kvm_accel_type);
}
type_init(kvm_type_init);
```
注册流程(`main`之前)
`type_init(kvm_type_init) -> module_init(function, MODULE_INIT_QOM) -> register_module_init(function, type)`

其中采用宏来写初始化的函数
```C
// do_qemu_init_kvm_type_init
/* This should not be used directly.  Use block_init etc. instead.  */
#define module_init(function, type)                                        \
static void __attribute__((constructor)) do_qemu_init_ ## function(void)    \
{                                                                           \
    register_module_init(function, type);                                   \
}
// __attribute__((constructor)) execute before main

#define type_init(function) module_init(function, MODULE_INIT_QOM)
```
`do_qemu_init_kvm_type_init`真正调用了`register_module_init`
```C
void register_module_init(void (*fn)(void), module_init_type type)
{
    ModuleEntry *e;
    ModuleTypeList *l;
    e = g_malloc0(sizeof(*e));
    e->init = fn;
    e->type = type;
    l = find_type(type);
    QTAILQ_INSERT_TAIL(l, e, node);
}
```
将`ModuleEntry`添加到`MODULE_INIT_QOM`类型的`list`尾部

### call init
在`vl.c`中，最开始`main`函数调用
```C
void module_call_init(module_init_type type)
{
    ModuleTypeList *l;
    ModuleEntry *e;
    l = find_type(type);
    QTAILQ_FOREACH(e, l, node) {
        e->init();
    }
}
```
即初始化所有的`MODULE_INIT_TRACE`，这里还不是`QOM`，然后初始化`cpu`相关结构、设置工作路径
接下来
```C
module_call_init(MODULE_INIT_QOM);
```
如上面注册的`kvm`，在这里会调用`kvm_type_init -> type_register -> type_register_internal`
```C
static TypeImpl *type_register_internal(const TypeInfo *info)
{
    TypeImpl *ti;
    ti = type_new(info);
    type_table_add(ti);
    return ti;
}
TypeImpl *type_register(const TypeInfo *info)
{
    assert(info->parent);
    return type_register_internal(info);
}
```

### type initialize
在`vl.c`的`main`函数解析完命令行一系列参数之后，首先调用了`select_machine`
```C
static MachineClass *select_machine(void)
{
    MachineClass *machine_class = find_default_machine();
    const char *optarg;
    /** ... */
}
MachineClass *find_default_machine(void)
{
    GSList *el, *machines = object_class_get_list(TYPE_MACHINE, false);
    /** ... */
}
```

`object_class_get_list`循环调用`TYPE_MACHINE`类型的对象
```C
GSList *object_class_get_list(const char *implements_type,
                              bool include_abstract)
{
    GSList *list = NULL;
    object_class_foreach(object_class_get_list_tramp,
                         implements_type, include_abstract, &list);
    return list;
}
void object_class_foreach(void (*fn)(ObjectClass *klass, void *opaque),
                          const char *implements_type, bool include_abstract,
                          void *opaque)
{
    // { object_class_get_list_tramp, implements_type, include_abstract, &list };
    OCFData data = { fn, implements_type, include_abstract, opaque };
    enumerating_types = true;
    g_hash_table_foreach(type_table_get(), object_class_foreach_tramp, &data);
    enumerating_types = false;
}
```
最终`object_class_foreach_tramp`初始化`machine`

流程`select_machine -> find_default_machine -> object_class_get_list -> object_class_foreach(object_class_get_list_tramp) -> g_hash_table_foreach(type_table_get(), object_class_foreach_tramp, &data) -> object_class_foreach_tramp -> type_initialize`
拿到各个`type`的`object`

具体调用类型相关参数`type_table_get`决定的，就是之前`type_init`注册的`type`

实际各个`Module`真正的初始化在`object_class_foreach_tramp`的`type_initialize`
```C
static void object_class_foreach_tramp(gpointer key, gpointer value,
                                       gpointer opaque)
{
    OCFData *data = opaque;
    TypeImpl *type = value;
    ObjectClass *k;
    type_initialize(type);
    k = type->class;
    if (!data->include_abstract && type->abstract) {
        return;
    }
    if (data->implements_type && 
        !object_class_dynamic_cast(k, data->implements_type)) {
        return;
    }
    // object_class_get_list_tramp
    data->fn(k, data->opaque);
}
```
`type_initialize`分析
```c
=> 如果 TypeImpl 已创建(class成员有值)，返回
=> ti->class = g_malloc0(ti->class_size)                    根据class_size分配内存空间
=> type_get_parent(ti)                                      获取父类的TypeImpl
=> memcpy(ti->class, parent->class, parent->class_size)     将parent的class拷贝到自己class的最前面
=> ti->class->properties = g_hash_table_new_full            创建存放property的hash table
=> type_initialize_interface                                初始化class的接口，包括父类和自己的
=> ti->class->type = ti                                     设置class的type为对应TypeImpl
=> parent->class_base_init                                  如果parent定义了 class_base_init ，调用之
=> ti->class_init(ti->class, ti->class_data)                调用class的 class_init
```

### Summary

- 注册模块(main之前)
`type_init(kvm_type_init) -> module_init(function, MODULE_INIT_QOM) -> register_module_init(function, type)`
- 调用初始化
`select_machine -> find_default_machine -> object_class_get_list -> object_class_foreach(object_class_get_list_tramp) -> g_hash_table_foreach(type_table_get(), object_class_foreach_tramp, &data) -> class_init`
- 模块初始化注册
`object_type_init -> type_register -> type_register_internal`


## Reference
- [QEMU/QOM](https://wiki.qemu.org/Features/QOM)
- [QOM学习](https://www.binss.me/blog/qemu-note-of-qemu-object-model/)
- [vcpu热插拔](https://www.cnblogs.com/fangying7/p/6001667.html)
