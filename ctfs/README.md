# VM escape

VM escape in ctfs

## 入门文章：

http://www.phrack.org/papers/vm-escape-qemu-case-study.html

https://dangokyo.me/2018/03/02/go-for-vm-escape/

## ctf problem:

* **0CTF 2017 Finals  vm_escape**: http://blog.eadom.net/writeups/qemu-escape-vm-escape-from-0ctf-2017-finals-writeup/

  > 目测基于edu.c针对mmio进行了修改，找不到原题目二进制文件，调用uinit然后进行uaf

* **HITB GSEC 2017 babyqemu**: https://kitctf.de/writeups/hitb2017/babyqemu

  > 基于edu.c修改，保留了调试信息、符号表等，改动幅度极小，适合入门，漏洞为越界写

* **Real World CTF Quals 2018 - SCSI**: https://blog.rpis.ec/2018/08/realworld-quals-2018-scsi.html

  > 完全重写的一个device设备，保留了调试信息，通过越界读写泄漏基地址，并通过uaf进行getshell

* **DefconQuals 2018 - EC3**: http://uaf.io/exploitation/2018/05/13/DefconQuals-2018-EC3.html

  https://ctftime.org/writeup/10167

  https://blog.bushwhackers.ru/defconquals2018-ec3/

  > 基于edu.c修改，删除了符号表，针对mmio进行修改，通过一些初始化字符串进行函数的定位

* **BlizzardCTF 2017 - Strng**: http://uaf.io/exploitation/2018/05/17/BlizzardCTF-2017-Strng.html

* **CSAW CTF 2018 - kvm**: [源码](https://github.com/osirislab/CSAW-CTF-2018-Quals/tree/master/rev/kvm)，[writeup by mahham](https://gitlab.com/mahham/ctf/blob/master/2018-csaw/Readme.md#kvm-500-reversing)

    > `kvm`逆向

* **HitconCTF 2018 - Super Hexagon**: https://ctftime.org/task/6900

    > 非常骚的6连pwn，自定义了machine、cpu，涉及知识主要是`arm trusted firmware`和`vm`相关

* **HitconCTF 2018 - Abyss**:[源码](https://github.com/david942j/ctf-writeups/tree/master/hitcon-2018/abyss), [writeup by cr0wnctf](https://github.com/cr0wnctf/writeups/tree/master/2018/2018_10_20_HITCON/abyss), [writeup by Nu1L](https://xz.aliyun.com/t/2953#toc-4)

    > user、kernel、kvm三层pwn，需要熟悉`kvm`运行机制

* **TokyoWesterns CTF 2018 - pwn240+300+300 EscapeMe**: https://david942j.blogspot.com/2018/09/write-up-tokyowesterns-ctf-2018.html

    题目源码：https://github.com/shift-crops/EscapeMe

    > david942j声称见过出得最好的vm escape题目，包含user、kernel、kvm三层pwn，并且包含题目源码。Nice challenge to study !!

* **Real World CTF 2018 - kid vm**: [writeup by perfect blue](https://github.com/perfectblue/ctf-writeups/blob/master/RealWorldCTF-2018/kidvm.md)
    > 用`simple kvm`出一个题

* **Real World CTF 2018 Finals - Station-Escape**: [writeup](https://zhuanlan.zhihu.com/p/52140921)

    > 大佬太强了，这居然是道vmware逃逸题，接口点vmware tools，flyyy出的题

* **35c3 virtualbox 0day**: [writeup by f1yyy](https://mp.weixin.qq.com/s/hgSl3U8BzICxev4TH7bSnQ)
    > 0day比赛。。阔怕

* **Defcon Qualifier 2019**: [Source Code](https://github.com/o-o-overflow/dc2019q-rtooos)

	> mac上hvf的hypervisor，先从远程pwn掉简单的kernel cat出hypervisor，再进行mac 堆pwn

* **QWB 2019**
	> pci设备，有点绕的状态机，详见[qwb2019_qwct](qwb2019_qwct/README.md)

