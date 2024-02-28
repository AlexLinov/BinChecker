#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void executePowershellCommand() {
    system("powershell -Command \"Get-CimInstance -ClassName win32_service | "
           "Select Name,State,PathName | Where-Object {$_.State -like 'Running'} | "
           "ForEach-Object { $_.PathName }\" > temp.txt");
}

// Adjusted to check for Full Control (F) permissions only
int hasRequiredPermissionsForUsers(char *icaclsOutput) {
    char *usersPermissions = strstr(icaclsOutput, "BUILTIN\\Users");
    if (usersPermissions) {
        return strstr(usersPermissions, "(F)") != NULL; // Check for Full Control only
    }
    return 0;
}

void checkPermissionsAndPrint() {
    char line[2048]; // Adjust size as necessary
    FILE *fp = fopen("temp.txt", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        // Normalize the line to remove potential newlines
        char *newline = strchr(line, '\n');
        if (newline) *newline = 0; // Remove newline character

        // Correctly handle paths that may be enclosed in quotes
        char *executablePath = line;
        if (line[0] == '\"') {
            executablePath++; // Skip the opening quote
            char* endQuote = strrchr(executablePath, '\"');
            if (endQuote) *endQuote = 0; // Null-terminate at the closing quote
        }

        if (executablePath) {
            char icaclsCommand[2048];
            char icaclsTempFile[64];
            snprintf(icaclsTempFile, sizeof(icaclsTempFile), "icacls_temp_%d.txt", rand());

            // Formulate and execute the icacls command, redirecting output to a temporary file
            snprintf(icaclsCommand, sizeof(icaclsCommand), "icacls \"%s\" > %s 2>nul", executablePath, icaclsTempFile);
            system(icaclsCommand);

            // Open and read from the temporary file to check for required permissions
            FILE *fpIcacls = fopen(icaclsTempFile, "r");
            if (fpIcacls) {
                char icaclsOutput[4096];
                int found = 0;
                while (fgets(icaclsOutput, sizeof(icaclsOutput), fpIcacls) != NULL) {
                    if (hasRequiredPermissionsForUsers(icaclsOutput)) {
                        found = 1;
                        break; // Required permission found, stop reading further
                    }
                }
               if (found) {
    // If the required permission is found, print the executable path
                printf("\x1b[32mBUILTIN\\Users Full Control: %s\n\x1b[0m", executablePath);
}
                fclose(fpIcacls);
            }
            remove(icaclsTempFile); // Clean up the temporary file
        }
    }

    fclose(fp);
    remove("temp.txt"); // Clean up after processing
}

int main() {
    executePowershellCommand();
    checkPermissionsAndPrint();

    return 0;
}
