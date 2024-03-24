#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int stopAtFirstFound = 0;

void printHelp() {
    printf("Usage: example.exe [OPTION]\n");
    printf("Searches for .exe files starting from the root of C:\\ for which BUILTIN\\Users have Full Control permissions.\n\n");
    printf("Options:\n");
    printf("  -quick\tStops search at the first .exe file found with the required permissions.\n");
    printf("  -help \tDisplays this help message and exits.\n\n");
    printf("Without any OPTION, the program will search for all .exe files with the required permissions.\n");
}

int checkPermissions(const char* filePath) {
    char command[MAX_PATH + 50]; // Buffer (change if needed)
    snprintf(command, sizeof(command), "icacls \"%s\"", filePath);
    FILE *fpIcacls = _popen(command, "r");
    if (fpIcacls) {
        char icaclsOutput[4096];
        while (fgets(icaclsOutput, sizeof(icaclsOutput), fpIcacls) != NULL) {
            if (strstr(icaclsOutput, "BUILTIN\\Users:(I)(F)")) {
                printf("\x1b[32mBUILTIN\\Users Full Control: %s\n\x1b[0m", filePath);
                _pclose(fpIcacls);
                return 1; // Return true if file found
            }
        }
        _pclose(fpIcacls);
    }
    return 0; // Return false
}

int findExecutablesAndCheckPermissionsRecursively(const char* directory) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char tempPath[MAX_PATH];
    char searchPath[MAX_PATH];

    snprintf(searchPath, MAX_PATH, "%s\\*", directory);
    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0; // Directory not found or unable to read
    }

    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }

        snprintf(tempPath, MAX_PATH, "%s\\%s", directory, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // If directory, recurse
            if (findExecutablesAndCheckPermissionsRecursively(tempPath) && stopAtFirstFound) {
                FindClose(hFind);
                return 1; // Found an executable with required permissions in a subdirectory. Stops if flag is set
            }
        } else {
            // Check if the file is an .exe and check permissions
            char *ext = strrchr(findFileData.cFileName, '.');
            if (ext && _stricmp(ext, ".exe") == 0) {
                if (checkPermissions(tempPath) && stopAtFirstFound) {
                    FindClose(hFind);
                    return 1; // Found an executable with required permissions. Stops searching if flag is set
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return 0; 
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "-help") == 0) {
            printHelp(); // Print help if -help argument is provided
            return 0;
        } else if (strcmp(argv[1], "-quick") == 0) {
            stopAtFirstFound = 1; // Stop search at the first file found with required permissions
        } else {
            printf("Invalid option. Use '-help' for usage information.\n");
            return 1;
        }
    }

    const char* rootDirectory = "C:\\"; // Start from the root of C drive
    if (!findExecutablesAndCheckPermissionsRecursively(rootDirectory) && stopAtFirstFound) {
        printf("No executable with required permissions found");
}

return 0;
}
