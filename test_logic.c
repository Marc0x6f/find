#include <stdio.h>
#include <string.h>
#include <assert.h>

// Assinatura da função a ser testada (copiada para o teste para evitar dependência de linker complexa em C puro)
void formatBytes(long long bytes, char *buffer, size_t bufsize)
{
  double size = (double)bytes;
  const char *units[] = {"B", "KB", "MB", "GB", "TB"};
  int unitIndex = 0;

  while (size >= 1024.0 && unitIndex < 4)
  {
    size /= 1024.0;
    unitIndex++;
  }

  snprintf(buffer, bufsize, "%.2f %s", size, units[unitIndex]);
}

void test_formatBytes()
{
  char buffer[32];

  formatBytes(500, buffer, sizeof(buffer));
  assert(strcmp(buffer, "500.00 B") == 0);

  formatBytes(1024, buffer, sizeof(buffer));
  assert(strcmp(buffer, "1.00 KB") == 0);

  formatBytes(1024LL * 1024LL * 1024LL, buffer, sizeof(buffer));
  assert(strcmp(buffer, "1.00 GB") == 0);

  formatBytes(1024LL * 1024LL * 1024LL * 1024LL, buffer, sizeof(buffer));
  assert(strcmp(buffer, "1.00 TB") == 0);

  printf("Testes de formatBytes: PASSOU\n");
}

int main()
{
  test_formatBytes();
  return 0;
}
