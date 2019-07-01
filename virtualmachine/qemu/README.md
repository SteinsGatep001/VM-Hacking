# qemu

根据网上一些资料进行学习整理`qemu`相关细节


## abbreviation
- TCG: Tiny Code Generator
- CC: Code Cache
- TB: Translation Block
- TBD: TB Descriptor
- MPD: Memory Page Descriptor

## dir
- TranslationBlock structure in translate-all.h
- Translation cache is code gen buffer in exec.c
- cpu-exec() in cpu-exec.c orchestrates translation and block chaining.
- target-*/translate.c: guest ISA specific code.
- tcg-*/*/: host ISA specific code.
- linux-user/*: Linux usermode specific code.
- vl.c: Main loop for system emulation.
- hw/*: Hardware, including video, audio, and boards.

## internals
整理
- [QOM](./qom.md): QEMU Object Module，几乎所有的设备如CPU、内存、总线等都是利用这一面向对象的模型来实现的


## Resources
- [Excellence blog](https://people.cs.nctu.edu.tw/~chenwj/dokuwiki/doku.php#%E8%99%9B%E6%93%AC%E6%A9%9F%E5%99%A8) 专门研究了虚拟机源码(from tw)
- [Qemu Internal in hellogcc](http://www.hellogcc.org/?cat=8) 5篇`qemu internals`
- [features of qemu in wiki](https://wiki.qemu.org/Features)
- [源码说明](https://lugatgt.org/content/qemu_internals/downloads/slides.pdf) 比较老的一个
- [别人的internal](https://github.com/azru0512/slide/tree/master/QEMU) 稍微有点乱
- [qemu tech](https://stuff.mit.edu/afs/sipb/project/phone-project/share/doc/qemu/qemu-tech.html) 简单介绍


## CVEs
**CVE-2015-5165（Information Leakage)**: https://dangokyo.me/2018/03/08/qemu-escape-part-3-information-leakage-cve-2015-5165/ 

**CVE-2015-7504（Hijack Control Flow)**: https://dangokyo.me/2018/03/14/qemu-escape-part-4-hijack-control-flow/

## Appendix
- 开发`qemu`/`linux`内核设备
