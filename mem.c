#include "main.h"
#include <stdint.h>

void patch_mem(uint64_t addr, uint8_t *data, size_t size) {
  char path[100];
  sprintf(path, "/proc/%d/mem", game_pid);
  FILE *mem = fopen(path, "r+b");
  fseek(mem, addr, SEEK_SET);
  fwrite(data, 1, size, mem);
  fclose(mem);
}

void *read_mem(uint64_t addr, size_t size) {
  char path[100];

  // Safely format the file path
  snprintf(path, sizeof(path), "/proc/%d/mem", game_pid);

  // Open the memory file for the given process
  FILE *mem = fopen(path, "rb");
  if (!mem) {
    perror("fopen");
    return NULL;
  }

  // Seek to the specified memory address
  if (fseek(mem, addr, SEEK_SET) != 0) {
    perror("fseek");
    fclose(mem);
    return NULL;
  }

  // Allocate memory to store the read data
  void *data = malloc(size);
  if (!data) {
    perror("malloc");
    fclose(mem);
    return NULL;
  }

  // Read the memory into the allocated buffer
  size_t bytesRead = fread(data, 1, size, mem);
  if (bytesRead != size) {
    ERR("read_mem: Only %zu bytes read, expected %zu\n", bytesRead, size);
    free(data);
    fclose(mem);
    return NULL;
  }

  // Close the memory file
  fclose(mem);

  // Return the read data
  return data;
}

int find_process(char *process_name) {
  // 1. Open /proc
  DIR *proc = opendir("/proc");
  if (!proc) {
    perror("opendir");
    return -1;
  }
  // 2. Iterate over /proc
  struct dirent *entry;

  for (entry = readdir(proc); entry; entry = readdir(proc)) {
    // 3. Check if the entry is a directory
    if (entry->d_type != DT_DIR) {
      continue;
    }
    // 4. Check if the entry is a process directory
    int pid = atoi(entry->d_name);
    if (pid == 0) {
      continue;
    }
    // 5. Read the process name
    char path[100];
    sprintf(path, "/proc/%d/comm", pid);
    FILE *comm = fopen(path, "r");
    if (!comm) {
      continue;
    }
    char name[100];
    fscanf(comm, "%s", name);
    fclose(comm);
    // 6. Compare the process name
    if (strcmp(name, process_name) == 0) {
      closedir(proc);
      return pid;
    }
  }

  return -1;
}

uint64_t find_base() {
  char path[100];
  sprintf(path, "/proc/%d/maps", game_pid);
  FILE *maps = fopen(path, "r");
  if (!maps) {
    perror("fopen");
    return 0;
  }

  // For each line in the file
  char line[1000];
  char map_name[1000];
  char perms[5];

  while (fgets(line, sizeof(line), maps)) {
    // Parse the start and end addresses
    uint64_t start, end;
    sscanf(line, "%lx-%lx", &start, &end);
    sscanf(line, "%*x-%*x %s %*x %*s %*d %s", perms, map_name);

    if (strstr(map_name, "split_config.arm64_v8a") == NULL) {
      continue;
    }

    // Check ELF magic
    uint8_t *magic = read_mem(start, 4);
    if (memcmp(magic, ELFMAG, 4) != 0) {
      free(magic);
      continue;
    }
    free(magic);

    INFO("find_base: Found ELF at %lx\n", start);

    // Read the ELF header
    Elf64_Ehdr *ehdr = read_mem(start, sizeof(Elf64_Ehdr));
    // Find the program header table
    Elf64_Phdr *phdrs =
        read_mem(start + ehdr->e_phoff, ehdr->e_phnum * sizeof(Elf64_Phdr));

    // Find the dynamic section
    Elf64_Dyn *dyn = NULL;
    for (int i = 0; i < ehdr->e_phnum; i++) {
      if (phdrs[i].p_type == PT_DYNAMIC) {
        dyn = read_mem(start + phdrs[i].p_vaddr, phdrs[i].p_filesz);
        break;
      }
    }

    if (!dyn) {
      WARN("find_base: Dynamic section not found, skipping...\n");
      continue;
    }

    // Read SONAME
    uint64_t soname_off = 0;
    uint64_t strtab_off = 0;
    for (Elf64_Dyn *d = dyn; d->d_tag != DT_NULL; d++) {
      if (d->d_tag == DT_SONAME) {
        soname_off = d->d_un.d_val;
        continue;
      }
      if (d->d_tag == DT_STRTAB) {
        strtab_off = d->d_un.d_val;
        continue;
      }
    }
    char *soname = read_mem(start + strtab_off + soname_off, 12);
    if (strcmp(soname, "libil2cpp.so") != 0) {
      free(soname);
      free(dyn);
      continue;
    }

    base_addr = start;

    INFO("find_base: Found libil2cpp.so at %lx\n", base_addr);

    return start;
  }

  return 0;
}

void quick_print(uint64_t addr) {
  uint8_t *data = read_mem(addr, 128);
  printf("Hexdump at %lx:\n", addr);
  for (int i = 0; i < 32; i++) {
    printf("%02x", data[i]);
  }
  printf("\n");

  for (int i = 0; i < 128; i++) {
    printf("%02x", data[i]);
    if (i % 4 == 3) {
      printf(" ");
    }
  }
  printf("\n");
  printf("End of hexdump\n");
}
