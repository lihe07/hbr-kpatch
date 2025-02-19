#include "main.h"

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
    fprintf(stderr, "Error: Only %zu bytes read, expected %zu\n", bytesRead,
            size);
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

    // if (perms[0] != 'r' || perms[2] != 'x') {
    //   continue;
    // }
    // Check ELF magic
    uint8_t *magic = read_mem(start, 4);
    if (memcmp(magic, ELFMAG, 4) != 0) {
      free(magic);
      continue;
    }

    // Check if start + 000357e7 is libil2cpp.so
    uint8_t *data = read_mem(start + 0x357e7, 12);

    if (memcmp(data, "libil2cpp.so", 12) != 0) {
      free(data);
      continue;
    }
    free(data);

    printf("Found libil2cpp.so at %lx\n", start);

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

    // Read fini array
    uint64_t fini_array = 0;
    uint64_t fini_array_size = 0;
    for (Elf64_Dyn *d = dyn; d->d_tag != DT_NULL; d++) {
      if (d->d_tag == DT_FINI_ARRAY) {
        fini_array = start + d->d_un.d_ptr;
      }
      if (d->d_tag == DT_FINI_ARRAYSZ) {
        fini_array_size = d->d_un.d_val;
      }
    }
    if (!fini_array) {
      printf("Fini array not found\n");
      continue;
    }

    printf("Fini array at %lx\n", fini_array);

    uint64_t *fini_array_data = read_mem(fini_array, fini_array_size);
    for (int i = 0; i < fini_array_size / 8; i++) {
      printf("%lx\n", fini_array_data[i]);
    }

    uint64_t real_text = fini_array_data[fini_array_size / 8 - 1];

    uint64_t text = start + 0x29a9a44;
    printf("DIFF %lx\n", real_text - text);

    base_addr = start;
    text_addr = real_text;

    return real_text;
  }

  return 0;
}

void quick_print(uint64_t addr) {
  uint8_t *data = read_mem(addr, 128);
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
}
