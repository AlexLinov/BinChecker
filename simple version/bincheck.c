#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int stopAtFirstFound = 0;

void printHelp() {
    printf("Usage: example.exe [OPTION]\n");
    printf("Searches from the root of C:\\ for binaries with Full Control permissions granted to BUILTIN\\Users, excluding C:\\Windows and its subdirectories.\n\n");
    printf("Options:\n");
    printf("  -quick\tStops search at the first executable found with the required permissions.\n");
    printf("  -full \tSearches for all .exe files with the required permissions. This option is time-consuming.\n");
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

int checkPermissions(const char* filePath) {
    char command[32767 + 50];
    snprintf(command, sizeof(command), "icacls \"%s\"", filePath);
    FILE *fpIcacls = _popen(command, "r");
    if (fpIcacls) {
        char icaclsOutput[4096];
        while (fgets(icaclsOutput, sizeof(icaclsOutput), fpIcacls) != NULL) {
            if (strstr(icaclsOutput, "BUILTIN\\Users:(I)(F)")) {
                printf("\x1b[32mFOUND Executable: %s\n\x1b[0m", filePath);
                _pclose(fpIcacls);
                return 1;
        }
        _pclose(fpIcacls);
    }
    return 0;
}

int findExecutablesAndCheckPermissionsRecursively(const char* directory) {
    if (shouldSkipDirectory(directory)) {
        printf("INFO: Skipping directory: %s\n", directory);
        return 0;
    }

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char tempPath[32767];
    char searchPath[32767];

    snprintf(searchPath, sizeof(searchPath), "\\\\?\\%s\\*", directory);
    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }

        snprintf(tempPath, sizeof(tempPath), "%s\\%s", directory, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            findExecutablesAndCheckPermissionsRecursively(tempPath);
        } else {
            char *ext = strrchr(findFileData.cFileName, '.');
            if (ext && (_stricmp(ext, ".exe") == 0)) {
                if (checkPermissions(tempPath) && stopAtFirstFound) {
                    return 1; // Exit if found
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData));

    FindClose(hFind);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || strcmp(argv[1], "-help") == 0) {
        printHelp();
        return 0;
    }

    int result = 0; // To store the result of the search
    if (strcmp(argv[1], "-quick") == 0) {
        printf("\x1b[33mRunning quick checks. Searching for the first match...\n\x1b[0m");
        stopAtFirstFound = 1;
        result = findExecutablesAndCheckPermissionsRecursively("C:\\");
        if (!result) {
            printf("No executable with full control by BUILTIN\\Users was found.\n");
        }
    } else if (strcmp(argv[1], "-full") == 0) {
        printf("\x1b[33mRunning full checks. This may take a while...\n\x1b[0m");
        result = findExecutablesAndCheckPermissionsRecursively("C:\\");
        if (!result) {
            printf("No executables with full control by BUILTIN\\Users were found.\n");
        }
    } else {
        printf("Invalid option '%s'. Use '-help' for usage information.\n", argv[1]);
        return 1;
    }

    printf("Operation completed.\n");
    return 0;
}
