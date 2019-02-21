# Virtual Machine Overview

在公有云的市场份额上，KVM/QEMU占有着不低的比例，也是当前的趋势。它起初是一套由Quramnet公司开发的自由软件，支持全虚拟化(full virtualization)方案，对所模拟的虚拟机实例无需做出任何的修改即可直接运行。后来该公司于2008年被Red Hat收购，相应的软件也归RedHat所有。早在2005年，Red Hat便为Xen专门设计了一种管理API，称之为libvirt，它后来被扩展为支持多个虚拟机监控程序[2]。随着Red Hat对KVM/QEMU的大力推广，libvirt自然也成为KVM/QEMU管理软件的最佳选择，成为在向外扩展计算中最重要的库之一，作为云计算平台（比如OpenStack）与KVM/QEMU对接的桥梁存在。

Libvirt是一组开源软件的集合，包括三个部分：一个长期稳定的C语言应用程序接口（API）、一个守护进程（libvirtd）和一个命令行工具（virsh），主要目标是提供一个单一途径以管理多种不同虚拟化方案以及虚拟机。

---

1. qemu是一种虚拟化的软件，支持全虚拟化。

2. libvirt是虚拟化软件的管理软件，用于单机环境，支持管理qemu。

3. OpenStack用于集群环境，用于虚拟化软件的拓展，背后使用libvirt管理虚拟化软件



## What is qemu

qemu是模拟器，为啥这么说？因为它可以模拟一系列硬件：CPU、硬盘、主板等等。相当于模拟了整个计算机运行的环境。

qemu模拟CPU，支持俩种模式：

硬件辅助虚拟化和tcg（二进制指令翻译）。

前者要求CPU的配合，后者不需要，但是比较慢。

---

为了提高虚拟磁盘的性能，因此有了virtio方式的disk。

它的核心在于，使虚拟机上运行着的系统意识到他是一个模拟设备。

这种叫做半虚拟化设备。

Xen称作半虚拟化而非全虚拟化，因为运行在Xen架构上的OS要求特定的内核，而全虚拟化并不要求特定的内核

## Suggestion to learn virtualization

1. 阅读qemu文档 https://www.qemu.org/documentation/
2. 阅读libvirt文档 https://libvirt.org/docs.html



## About container

现在容器很流行，研究容器安全的很多。

它的核心在于：容器与host共享一个内核

所以需要一种手段，隔离各种资源，这种方法就是namespace

到现在，已经有了：

* mnt namespace，

* ipc namespace，

* pid namespace,

* uts namespace，

* cgroup namespace

* user namespace，

* network namespace
* **security namespace**（新提出）



### Some namespace implementation

1. **mnt namespace** -> root directory的问题

   都知道 `/` 代表绝对目录，那么 `/` 实际上定义在每一个task struct的成员中

   成员定义了俩个成员：

   一个是根目录的位置，一个是当前程序运行的目录

   也就是说可以给一个进程定义`/home/me` 为根目录，那么在这个进程中打开 `/usr` 实际上位置在

   `/home/me/usr`

2. **pid namespace** 

   同一个OS下面，pid是有限的，进程pid 1的进程，在不同的container中从host上看，应该是不同的pid，但是在容器中pid必须是1。

   因此需要一种机制：

   在容器中看到的pid 为1，但是在host上是另一种

3. **uts namespace**

   uname -a 这个命令的输出内容。

   既然是不同的OS（毕竟容器模拟了OS），那么不同的OS下可以有不同的输出。

   就是用了一种方法，在容器中uname -a 的输出与host上不同

4. **user namespace**

   有些命令在container中必须以root权限运行，但是container作为一组在host上运行的进程，为了安全考虑，必然为了最小权限，不能赋予root权限。

   于是有了这种手段：

   container中该进程是root权限，host上不是（具体实现不知道）

5. **network namespace**

   网络协议栈是内核实现的，但是container作为虚拟化的OS，必然需要有独立的网络协议栈，它解决的就是这。

   从实现上，实现了一个网络命名空间。

6. **cgroup namespace**

   cgroup是资源限制的手段，可以在host上使用cgroup限制虚拟机、container对资源的使用。

   但是在container内部也应该有这种机制。

7. **security namespace**

   apparmor 与 selinux 作为Linux security subsystem的实现，都是利用lsm实现的。

   但问题在于：

   selinux这种在host上设置了一个profile

   对于所有的容器都使用这种profile，但是太不灵活了，所以就有了一种手段：利用security namespace不同的container使用不同的security profile

---

对于容器，其内存管理还是由host的内核实现

Intel CPU有种机制pex，得以内存可以像真实的CPU一样使用那么大的内存范围。

但是，对于虚拟机来讲：

vm一个内核，host一个内核，对host来说，vm就是一个进程（每一个vcpu代表一个进程）

对于容器来讲…与host使用的都是同一个内核，所以container并没有独立的内存管理功能



**Q: 那么类似docker for windows、docker for mac的内存管理是如何实现**

A: docker for Windows，这个好像ms贡献了一个方案，native实现。原本是用VirtualBox跑了一个虚拟机

而docker for Mac，mac是Unix，Unix下也有相应的namespace与cgroup，只是不是那么的成熟（或者说feature多）



**容器现在有三种：**

1. 上述提到的
2. 基于虚拟化的解决方案，意思是：这种共享内核的，隔离性太差了，所以有了katacontainer，使用定制化的qemu，跑了一个轻量级的虚拟机
3. 今年Google捣鼓的gvisor，使用ptrace截获系统调用，在userspace实现了一个虚拟内核，这个虚拟内核也有task struct、namespace等这些东西，也就是说，在gvisor上运行的容器，实际上运行在gvisor虚拟的内核之上，又多了一层隔离

---

上述资料来源一个研究虚拟化的同学对我的科普。。Thank a lot to him!!!
