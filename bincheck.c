#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int stopAtFirstFound = 0;

void printHelp() {
    printf("Usage: example.exe [OPTION]\n");
    printf("Searches for binary from the root of C:\\ for which BUILTIN\\Users have Full Control permissions.\n\n");
    printf("Options:\n");
    printf("  -quick\tStops search at the first binary found with the required permissions.\n");
    printf("  -full \tSearches for all .exe files with the required permissions. This option is time-consuming.\n");
    printf("  -help \tDisplays this help message and exits.\n\n");
    printf("You must choose either -quick or -full to run this program.\n");

}

int checkPermissions(const char* filePath) {
    char command[32767 + 50]; // had some issues non-interactive shells
    snprintf(command, sizeof(command), "icacls \"%s\"", filePath);
    FILE *fpIcacls = _popen(command, "r");
    if (fpIcacls) {
        char icaclsOutput[4096];
        while (fgets(icaclsOutput, sizeof(icaclsOutput), fpIcacls) != NULL) {
            if (strstr(icaclsOutput, "BUILTIN\\Users:(I)(F)")) {
                printf("\x1b[32mBUILTIN\\Users Full Control: %s\n\x1b[0m", filePath);
                _pclose(fpIcacls);
                return 1;
            }
        }
        _pclose(fpIcacls);
    }
    return 0;
}

int findExecutablesAndCheckPermissionsRecursively(const char* directory) {
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

        snprintf(tempPath, sizeof(tempPath), "\\\\?\\%s\\%s", directory, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (findExecutablesAndCheckPermissionsRecursively(tempPath + 4) && stopAtFirstFound) {
                FindClose(hFind);
                return 1;
            }
        } else {
            char *ext = strrchr(findFileData.cFileName, '.');
            if (ext && (_stricmp(ext, ".exe") == 0 )) {
                if (checkPermissions(tempPath + 4) && stopAtFirstFound) { 
                    FindClose(hFind);
                    return 1;
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return 0; 
}

int main(int argc, char *argv[]) {
    if (argc != 2 || strcmp(argv[1], "-help") == 0) {
        printHelp();
        return 0;
    }

    if (strcmp(argv[1], "-quick") == 0) {
        printf("\x1b[33mRunning quick checks. Go grab a snack.\n\x1b[0m");
        stopAtFirstFound = 1;
    } else if (strcmp(argv[1], "-full") == 0) {
        printf("\x1b[33mRunning full checks. This may take a while. Go grab a snack.\n\x1b[0m");
        stopAtFirstFound = 0;
    } else {
        printf("Invalid option '%s'. Use '-help' for usage information.\n", argv[1]);
        return 1;
    }

    const char* rootDirectory = "C:\\";
    findExecutablesAndCheckPermissionsRecursively(rootDirectory);
    
    if (stopAtFirstFound) {
        printf("Search completed with '-quick' option.\n");
    } else {
        printf("Search completed with '-full' option.\n");
    }

    return 0;
}
