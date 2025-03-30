#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>

int main(int argc, char** argv) {
    int exitStatus = 0;
    int bytesWritten = 0;
    int bytesRead = 0;
    int bufSize = 4096;

    int sourceFile;
    int destFile;
    bool isEmptyBlock;
    char* restrict dataBuffer;

    static struct option longOptions[] = {
        {"block-size", required_argument, NULL, 'b'},
        {0, 0, 0, 0},
    };
    char* optionString = "b:";

    while (true) {
        if (getopt(argc, argv, optionString) == -1)
            break;

        bufSize = atoi(optarg);
        if (bufSize <= 0) {
            fprintf(stderr, "ERROR: buffer size must be a positive integer.\n");
            return exitStatus;
        }
    }

    if (optind + 1 == argc) {
        destFile = open(argv[optind], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (destFile == -1) {
            fprintf(stderr, "ERROR: failed to open file for writing.\n");
            return exitStatus;
        }
    }
    else if (optind + 2 == argc) {
        if (strcmp(argv[optind + 1], argv[optind]) == 0) {
            fprintf(stderr, "ERROR: source and destination files cannot be the same.\n");
            return exitStatus;
        }

        destFile = open(argv[optind + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (destFile == -1) {
            fprintf(stderr, "ERROR: failed to open file for writing.\n");
            return exitStatus;
        }
        sourceFile = open(argv[optind], O_RDONLY);
        if (sourceFile == -1) {
            close(destFile);
        }
    }
    else {
        fprintf(stderr, "ERROR: incorrect number of arguments.\n");
        return exitStatus;
    }

    dataBuffer = calloc(bufSize, sizeof(char));
    if (dataBuffer == NULL) {
        close(sourceFile);
        close(destFile);
        fprintf(stderr, "ERROR: memory allocation failed.\n");
        return exitStatus;
    }

    while (true) {
        isEmptyBlock = true;
        bytesRead = read(sourceFile, dataBuffer, bufSize);
        if (bytesRead == -1) {
            free(dataBuffer);
            close(sourceFile);
            close(destFile);
            fprintf(stderr, "ERROR: reading from source file failed.\n");
            return exitStatus;
        }
        if (bytesRead == 0)
            break;

        for (int i = 0; i < bytesRead; i++) {
            if (dataBuffer[i] != 0) {
                isEmptyBlock = false;
                break;
            }
        }

        if (isEmptyBlock) {
            if (lseek(destFile, bytesRead, SEEK_CUR) == -1) {
                free(dataBuffer);
                close(sourceFile);
                close(destFile);
                fprintf(stderr, "ERROR: seeking in destination file failed.\n");
                return exitStatus;
            }
        }
        else {
            bytesWritten = write(destFile, dataBuffer, bytesRead);
            if (bytesWritten != bytesRead) {
                free(dataBuffer);
                close(sourceFile);
                close(destFile);
                fprintf(stderr, "ERROR: writing to destination file failed.\n");
                return exitStatus;
            }
        }
    }

    free(dataBuffer);
    close(sourceFile);
    close(destFile);
    return exitStatus;
}
