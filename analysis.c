#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // 用于目录遍历

#define BUFFER_SIZE 1024

void process_file(const char *input_path, const char *output_path) {
    FILE *inputFile, *outputFile;
    char buffer[BUFFER_SIZE];

    inputFile = fopen(input_path, "r");
    if (inputFile == NULL) {
        perror("Error opening input file");
        return;
    }

    outputFile = fopen(output_path, "w");
    if (outputFile == NULL) {
        perror("Error opening output file");
        fclose(inputFile);
        return;
    }

    while (fgets(buffer, sizeof(buffer), inputFile) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        fprintf(outputFile, "%s,0\n", buffer);
    }

    fclose(inputFile);
    fclose(outputFile);
}

int main() {
    DIR *dir;
    struct dirent *entry;
    char input_path[PATH_MAX];
    char output_path[PATH_MAX];

    dir = opendir("."); // 打开当前目录，你可以修改为任意目录路径
    if (dir == NULL) {
        perror("Error opening directory");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".csv") != NULL) {
            snprintf(input_path, PATH_MAX, "%s", entry->d_name);
            snprintf(output_path, PATH_MAX, "%s_out", entry->d_name); // 在文件名后加上 "_out" 作为输出文件名

            printf("Processing file: %s\n", input_path);
            process_file(input_path, output_path);
        }
    }

    closedir(dir);

    return 0;
}

