#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

int stopAtFirstFound = 0;

void printHelp() {
    printf("Usage: example.exe [OPTION]\n");
    printf("Searches from the root of C:\\ for binaries with specific permissions or directories writable by BUILTIN\\Users, excluding C:\\Windows and its subdirectories.\n\n");
    printf("Options:\n");
    printf("  -quick\tStops search at the first binary found with the required permissions.\n");
    printf("  -full \tSearches for all .exe files with the required permissions. This option is time-consuming.\n");
    printf("  -write\tContinuously searches directories with write permissions, excluding C:\\Windows.\n");
    printf("  -help \tDisplays this help message and exits.\n\n");
}

int shouldSkipDirectory(const char* directory) {
    char lowerPath[32768];
    strcpy(lowerPath, directory);
    for (int i = 0; lowerPath[i]; i++) {
        lowerPath[i] = tolower((unsigned char)lowerPath[i]);
    }
    if (strncmp(lowerPath, "c:\\windows", strlen("c:\\windows")) == 0) {
        size_t len = strlen("c:\\windows");
        if (lowerPath[len] == '\0' || lowerPath[len] == '\\') {
            return 1;
        }
    }
    return 0;
}

int checkPermissions(const char* filePath, int checkWrite) {
    char command[32767 + 50];
    snprintf(command, sizeof(command), "icacls \"%s\"", filePath);
    FILE *fpIcacls = _popen(command, "r");
    if (fpIcacls) {
        char icaclsOutput[4096];
        while (fgets(icaclsOutput, sizeof(icaclsOutput), fpIcacls) != NULL) {
            if (checkWrite) {
                if (strstr(icaclsOutput, "BUILTIN\\Users:(OI)(CI)(WD)") || strstr(icaclsOutput, "BUILTIN\\Users:(I)(CI)(AD)")) {
                    printf("\x1b[32mBUILTIN\\Users Write Permission: %s\n\x1b[0m", filePath);
                    return 1;
                }
            } else {
                if (strstr(icaclsOutput, "BUILTIN\\Users:(I)(F)")) {
                    printf("\x1b[32mBUILTIN\\Users Full Control: %s\n\x1b[0m", filePath);
                    if (stopAtFirstFound) return 1;
                }
            }
        }
        _pclose(fpIcacls);
    }
    return 0;
}

int findExecutablesAndCheckPermissionsRecursively(const char* directory, int mode) {
    if (shouldSkipDirectory(directory)) {
        printf("Skipping directory: %s\n", directory);
        return 0;
    }

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char tempPath[32767];
    char searchPath[32767];

    snprintf(searchPath, sizeof(searchPath), "\\\\?\\%s\\*", directory);
    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        //printf("Failed to access directory: %s\n", directory);
        return 0;
    }

    int found = 0;
    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }

        snprintf(tempPath, sizeof(tempPath), "%s\\%s", directory, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (mode == 2) {
                checkPermissions(tempPath, 1);
            } else {
                findExecutablesAndCheckPermissionsRecursively(tempPath, mode);
            }
        } else if (mode != 2) {
            char *ext = strrchr(findFileData.cFileName, '.');
            if (ext && (_stricmp(ext, ".exe") == 0)) {
                checkPermissions(tempPath, 0);
            }
        }
    } while (FindNextFile(hFind, &findFileData));

    FindClose(hFind);
    return found;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || strcmp(argv[1], "-help") == 0) {
        printHelp();
        return 0;
    }

    if (strcmp(argv[1], "-quick") == 0) {
        printf("\x1b[33mRunning quick checks. Go grab a snack.\n\x1b[0m");
        stopAtFirstFound = 1;
        findExecutablesAndCheckPermissionsRecursively("C:\\", 1);
    } else if (strcmp(argv[1], "-full") == 0) {
        printf("\x1b[33mRunning full checks. This may take a while. Go grab a snack.\n\x1b[0m");
        findExecutablesAndCheckPermissionsRecursively("C:\\", 1);
    } else if (strcmp(argv[1], "-write") == 0) {
        printf("\x1b[33mChecking directories continuously. Go grab a snack.\n\x1b[0m");
        findExecutablesAndCheckPermissionsRecursively("C:\\", 2);
    } else {
        printf("Invalid option '%s'. Use '-help' for usage information.\n", argv[1]);
        return 1;
    }

    printf("Done.\n");
    return 0;
}
