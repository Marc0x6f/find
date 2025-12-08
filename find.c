#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define GB_TO_BYTES (1024LL * 1024LL * 1024LL)
#define MAX_PATH_LEN 32768

typedef struct
{
  long long totalFiles;
  long long totalSize;
  long long filesFound;
  LARGE_INTEGER startTime;
  LARGE_INTEGER endTime;
  LARGE_INTEGER frequency;
} Statistics;

// Formata bytes para unidade legível (KB, MB, GB, TB)
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

// Valida entrada do usuário (apenas números positivos)
long long getMinSizeGB()
{
  long long sizeGB;
  int result;

  while (1)
  {
    printf("Digite o tamanho minimo em GB (apenas numeros positivos): ");
    result = scanf("%I64d", &sizeGB);

    // Limpa buffer de entrada
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
      ;

    if (result == 1 && sizeGB > 0)
    {
      return sizeGB;
    }

    printf("Entrada invalida. Digite um numero positivo.\n");
  }
}

// Busca recursivamente por arquivos maiores que o tamanho mínimo
void searchFiles(const char *path, long long minSizeBytes, Statistics *stats, FILE *outputFile)
{
  WIN32_FIND_DATAA findData;
  HANDLE findHandle;
  char searchPath[MAX_PATH_LEN];
  char filePath[MAX_PATH_LEN];

  // Constrói padrão de busca (path + \*)
  snprintf(searchPath, sizeof(searchPath), "%s\\*", path);

  findHandle = FindFirstFileA(searchPath, &findData);

  if (findHandle == INVALID_HANDLE_VALUE)
  {
    // Diretório inacessível, continua
    return;
  }

  do
  {
    // Pula . e ..
    if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
    {
      continue;
    }

    snprintf(filePath, sizeof(filePath), "%s\\%s", path, findData.cFileName);

    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      // É um diretório, busca recursivamente
      searchFiles(filePath, minSizeBytes, stats, outputFile);
    }
    else
    {
      // É um arquivo, verifica tamanho
      stats->totalFiles++;

      // Converte tamanho de DWORD high/low para long long
      ULARGE_INTEGER fileSize;
      fileSize.LowPart = findData.nFileSizeLow;
      fileSize.HighPart = findData.nFileSizeHigh;

      if (fileSize.QuadPart >= minSizeBytes)
      {
        stats->filesFound++;
        stats->totalSize += fileSize.QuadPart;

        char sizeStr[32];
        formatBytes(fileSize.QuadPart, sizeStr, sizeof(sizeStr));

        printf("\n%s - %s", filePath, sizeStr);

        // Salva em arquivo se fornecido
        if (outputFile)
        {
          fprintf(outputFile, "%s - %s\n", filePath, sizeStr);
        }
      }
    }
  } while (FindNextFileA(findHandle, &findData));

  FindClose(findHandle);
}

// Exibe estatísticas finais
void displayStatistics(Statistics *stats)
{
  QueryPerformanceCounter(&stats->endTime);

  double elapsedSeconds = (double)(stats->endTime.QuadPart - stats->startTime.QuadPart) / stats->frequency.QuadPart;

  char totalSizeStr[32];
  formatBytes(stats->totalSize, totalSizeStr, sizeof(totalSizeStr));

  printf("\n\n");
  printf("=====================================\n");
  printf("Estatisticas:\n");
  printf("=====================================\n");
  printf("Arquivos encontrados: %I64d\n", stats->filesFound);
  printf("Espaco total ocupado: %s\n", totalSizeStr);
  printf("Total de arquivos processados: %I64d\n", stats->totalFiles);
  printf("Tempo de execucao: %.2f segundos\n", elapsedSeconds);
  printf("=====================================\n");
}

int main()
{
  // Inicializa estrutura de estatísticas
  Statistics stats = {0};
  QueryPerformanceFrequency(&stats.frequency);
  QueryPerformanceCounter(&stats.startTime);

  // Obtém tamanho mínimo do usuário
  long long minSizeGB = getMinSizeGB();
  long long minSizeBytes = minSizeGB * GB_TO_BYTES;

  printf("\nIniciando busca a partir de C:\\ por arquivos com %I64d GB ou mais...\n", minSizeGB);
  printf("=====================================\n");

  FILE *outputFile = NULL;

  // Busca recursiva
  searchFiles("C:\\", minSizeBytes, &stats, outputFile);

  // Exibe estatísticas
  displayStatistics(&stats);

  return 0;
}
