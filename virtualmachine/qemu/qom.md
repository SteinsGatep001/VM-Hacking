
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

## Reference
- [QEMU/QOM](https://wiki.qemu.org/Features/QOM)
- [QOM学习](https://www.binss.me/blog/qemu-note-of-qemu-object-model/)
