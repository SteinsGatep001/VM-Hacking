
struct crypto_status
{
  __uint64_t statu;
  __u_char crypt_key[2048];
  __u_char input_buf[2048];
  __u_char output_buf[2048];
  _Bool (*encrypt_function)(__u_char *, __u_char *, __u_char *);
  _Bool (*decrypt_function)(__u_char *, __u_char *, __u_char *);
};

typedef PCIDevice PCIDevice_0;
typedef MemoryRegion MemoryRegion_0;
typedef QemuThread QemuThread_0;
typedef QemuMutex QemuMutex_0;


struct QwbState
{
  PCIDevice_0 pdev;
  MemoryRegion_0 mmio;
  QemuThread_0 thread;
  QemuMutex_0 crypto_statu_mutex;
  QemuMutex_0 crypto_buf_mutex;
  crypto_status crypto;
};

