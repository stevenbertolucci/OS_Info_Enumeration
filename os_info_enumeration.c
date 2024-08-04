// ----------------------------------------------------------------
// 	Author: Steven Bertolucci
// 	Course: CS 373 - Defense Against the Dark Arts
// 	Institution: Oregon State University
// 	Assignment: Homework 2 - OS Information Enumeration
// 	Description:
//
// 	I have written a C program that does 5 things on a Linux
// 	machine:
//
// 	1. Enumerate all the running processes.
// 	2. List all the running threads within process boundary.
// 	3. Enumerate all the loaded modules within the processes.
// 	4. Is able to show all the executable pages within
// 	    the processes.
// 	5. Gives us a capability to read the memory. 
//
// ----------------------------------------------------------------

// Libraries needed for this program
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#define MEMORY_READ_LIMIT 1024

// ----------------------------------------------------------------
// 		       STEP 1: Enumerate all the running processes.
// ----------------------------------------------------------------
void
listRunningProcesses() {

    printf("\033[1;31m\nHERE ARE ALL THE RUNNING PROCESSES:\n\033[0m");
    fflush(stdout);

    // Opening the /proc directory
    DIR *dirp = opendir("/proc");

    // Check to see if the /proc is able to be opened
    if (dirp == NULL)
    {
        perror("Error opening /proc directory");
        return;
    }

    struct dirent *dirp_entry;

    while ((dirp_entry = readdir(dirp)) != NULL)
    {
        // Check if the entry is a directory and the name is all digits (indicating a process ID)
        if (dirp_entry->d_type == DT_DIR)
        {
            int is_pid = 1;
            for (char *p = dirp_entry->d_name; *p != '\0'; p++)
            {
                if (!isdigit(*p))
                {
                    is_pid = 0;
                    break;
                }
            }

            if (is_pid)
            {
                pid_t pid = atoi(dirp_entry->d_name);

                // Construct the path to the comm file which contains the command name
                char comm_path[256];
                snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

                FILE *comm_file = fopen(comm_path, "r");
                if (comm_file == NULL)
                {
                    continue;
                }

                char comm[256];
                if (fgets(comm, sizeof(comm), comm_file) != NULL)
                {
                    // Remove the newline character at the end of the command name
                    comm[strcspn(comm, "\n")] = '\0';
                    printf("Process ID: %d, Command: %s\n", pid, comm);
                }

                fclose(comm_file);
            }
        }
    }

    closedir(dirp);
}

// --------------------------------------------------------------------------------
// 		    STEP 2: List all the running threads within process boundary.
// --------------------------------------------------------------------------------
void
listRunningThreads() {

    printf("\033[1;31m\nHERE ARE ALL THE RUNNING THREADS WITHIN PROCESS BOUNDARY:\n\033[0m");

    // List all the running threads within process boundary
    int proc_fd = open("/proc", O_RDONLY | O_DIRECTORY);

    if (proc_fd == -1)
    {
        perror("Error opening directory: /proc");
        return;
    }

    DIR *dirp = fdopendir(proc_fd);
    struct dirent *dirp_entry;

    while ((dirp_entry = readdir(dirp)) != NULL)
    {
        if (dirp_entry->d_type == DT_DIR)
        {
            pid_t pid = atoi(dirp_entry->d_name);

            if (pid > 0)
            {
                char threads[256];
                snprintf(threads, sizeof(threads), "/proc/%d/task", pid);

                int threads_fd = open(threads, O_RDONLY | O_DIRECTORY);

                if (threads_fd == -1)
                {
                    perror("Error opening threads directory");
                    continue;
                }

                DIR *threads_dir = fdopendir(threads_fd);
                struct dirent *threads_entry;

                while ((threads_entry = readdir(threads_dir)) != NULL)
                {
                    if (threads_entry->d_type == DT_DIR && strcmp(threads_entry->d_name, ".") != 0 && strcmp(threads_entry->d_name, "..") != 0)
                    {
                        printf("\033[1;36mThread: %s\033[0m in Process ID: %d\n", threads_entry->d_name, pid);
                    }
                }

                closedir(threads_dir);
                close(threads_fd);
            }
        }
    }

    closedir(dirp);
    close(proc_fd);
}

// ----------------------------------------------------------------------------------
// 		    STEP 3: Enumerate all the loaded modules within the processes.
// ----------------------------------------------------------------------------------
void
listLoadedModules() {

    printf("\033[1;31m\nHERE ARE ALL THE LOADED MODULES:\n\033[0m");

    FILE *file = fopen("/proc/modules", "r");
    if (file == NULL)
    {
        perror("Issues opening /proc/modules file using fopen");
        return;
    }

    char moduleInfo[256];

    printf("\033[1;36m%-20s %-10s %-10s\n\033[0m", "Module", "Size", "Used by");

    while (fgets(moduleInfo, sizeof(moduleInfo), file) != NULL)
    {
        char moduleName[50];
        unsigned long moduleSize;
        int moduleUsage;
        char moduleDetails[128];

        sscanf(moduleInfo, "%49s %lu %d %127s", moduleName, &moduleSize, &moduleUsage, moduleDetails);
        printf("%-20s %-10lu %-10d %s\n", moduleName, moduleSize, moduleUsage, moduleDetails);
    }

    fclose(file);
}

// ---------------------------------------------------------------------------------------
// 		    STEP 4: Is able to show all the executable pages within the processes.
// ---------------------------------------------------------------------------------------
void
listExecutablePages() {

    printf("\033[1;31m\n\nHERE ARE ALL THE EXECUTABLE PAGES WITHIN THE PROCESSES:\n\033[0m");

    int proc_fd = open("/proc", O_RDONLY | O_DIRECTORY);
    if (proc_fd == -1)
    {
        perror("Error opening directory /proc");
        return;
    }

    DIR *dirp = fdopendir(proc_fd);
    struct dirent *dirp_entry;

    while ((dirp_entry = readdir(dirp)) != NULL)
    {
        if (dirp_entry->d_type == DT_DIR)
        {
            pid_t pid = atoi(dirp_entry->d_name);
            if (pid > 0)
            {
                char executable[256];
                snprintf(executable, sizeof(executable), "/proc/%d/maps", pid);

                FILE *file = fopen(executable, "r");
                if (file == NULL)
                {
                    perror("Issues opening executable files using fopen");
                    continue;
                }

                char executablePages[1024];
                printf("\033[1;36m\nExecutable pages for Process ID: %d\n\033[0m", pid);

                while (fgets(executablePages, sizeof(executablePages), file) != NULL)
                {
                    if (strstr(executablePages, "x") != NULL)
                    {
                        printf("%s", executablePages);
                    }
                }

                fclose(file);
            }
        }
    }

    closedir(dirp);
    close(proc_fd);
}

// ---------------------------------------------------------------------------------------
// 		            STEP 5: Gives us a capability to read the memory.
// ---------------------------------------------------------------------------------------
void displayTheMemory()
{
    printf("\033[1;31m\nMEMORY RANGES FOR EACH PROCESS:\n\033[0m");

    int proc_fd = open("/proc", O_RDONLY | O_DIRECTORY);
    if (proc_fd == -1)
    {
        perror("Error opening directory /proc");
        return;
    }

    DIR *proc_dir = fdopendir(proc_fd);
    if (proc_dir == NULL)
    {
        perror("Error opening /proc directory with fdopendir");
        close(proc_fd);
        return;
    }

    struct dirent *dirp_entry;

    while ((dirp_entry = readdir(proc_dir)) != NULL)
    {
        if (dirp_entry->d_type == DT_DIR && atoi(dirp_entry->d_name) > 0)
        {
            pid_t pid = atoi(dirp_entry->d_name);
            char maps_path[256];
            snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
            FILE *maps_file = fopen(maps_path, "r");

            if (maps_file == NULL)
            {
                perror("Error opening maps file");
                continue;
            }

            char line[1024];
            printf("\033[1;33mProcess %d:\n\033[0m", pid);

            while (fgets(line, sizeof(line), maps_file))
            {
                char *path = strchr(line, '/');
                if (path != NULL) {
                    printf("%.*s %s", (int)(path - line), line, path);
                } else {
                    printf("%s", line);
                }
            }
            printf("\n");

            fclose(maps_file);
        }
    }

    closedir(proc_dir);
    close(proc_fd);
}

// ----------------------------------------------------------------
//                          main.c
// ----------------------------------------------------------------
int
main(int argc, char *argv[])
{
    // Step 5 requires sudo privileges
    //check_sudo(argc, argv);

    int userInput;
    char input[10];

    while (1) {
        printf("\033[1;33m\nWhat would you like me to do?\n\n");
        printf("\t1. Enumerate all the running processes\n");
        printf("\t2. List all the running threads within process boundary.\n");
        printf("\t3. Enumerate all the loaded modules within the processes.\n");
        printf("\t4. Is able to show all the executable pages within the processes.\n");
        printf("\t5. Gives us a capability to read the memory.\n\n");
        printf("Enter your choice (1-5) or 'q' to quit: \033[0m");

        fflush(stdout);
        scanf("%s", input);

        if (strcmp(input, "q") == 0 || strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            break;
        }

        // Switch the input to decimal
        userInput = atoi(input);

        switch (userInput) {
            case 1:
                listRunningProcesses();
                break;

            case 2:
                listRunningThreads();
                break;

            case 3:
                listLoadedModules();
                break;

            case 4:
                listExecutablePages();
                break;

            case 5:
                displayTheMemory();
                break;

            default:
                printf("Invalid input. Please enter a number between 1 and 5.\n");
                break;
        }
    }
    return 0;
}

// ----------------------------------------------------------------
//                         End of Program
// ----------------------------------------------------------------
