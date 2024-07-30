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

void
listRunningProcesses() {
// ----------------------------------------------------------------
// 		STEP 1: Enumerate all the running processes.

    printf("\nHERE ARE ALL THE RUNNING PROCESSES:\n");

    // Enumerate all the running processes
    int r = system("ps -eLf");
// ----------------------------------------------------------------
}

void
listRunningThreads() {
// --------------------------------------------------------------------------------
// 		STEP 2: List all the running threads within process boundary.

    printf("\nHERE ARE ALL THE RUNNING THREADS WITHIN PROCESS BOUNDARY:\n");

    // List all the running threads within process boundary
    int proc_fd = open("/proc", O_RDONLY | O_DIRECTORY);

    if (proc_fd == -1)
    {
        perror("Error opening directory: /proc");
        return;
    }

    DIR *proc_dir = fdopendir(proc_fd);
    struct dirent *proc_entry;

    while ((proc_entry = readdir(proc_dir)) != NULL)
    {
        if (proc_entry->d_type == DT_DIR)
        {
            pid_t pid = atoi(proc_entry->d_name);

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
                        printf("Thread ID: %s in Process ID: %d\n", threads_entry->d_name, pid);
                    }
                }

                closedir(threads_dir);
                close(threads_fd);
            }
        }
    }

    closedir(proc_dir);
    close(proc_fd);
// ----------------------------------------------------------------------------------
}

void
listLoadedModules() {
// ----------------------------------------------------------------------------------
// 		STEP 3: Enumerate all the loaded modules within the processes.

    printf("\nHERE ARE ALL THE LOADED MODULES WITHIN THE PROCESSES:\n");

    int proc_fd = open("/proc", O_RDONLY | O_DIRECTORY);

    if (proc_fd == -1)
    {
        perror("Issue opening /proc directory");
        return;
    }

    DIR *proc_dir = fdopendir(proc_fd);
    struct dirent *proc_entry;

    while((proc_entry = readdir(proc_dir)) != NULL)
    {
        if (proc_entry->d_type == DT_DIR)
        {
            pid_t pid = atoi(proc_entry->d_name);

            if (pid > 0)
            {
                char modules[256];
                char loadedModules[256];

                snprintf(modules, sizeof(modules), "/proc/%d/maps", pid);

                FILE *file = fopen(modules, "r");
                if (file == NULL)
                {
                    perror("fopen");
                    continue;
                }

                printf("\nLoaded modules for Process ID: %d\n", pid);

                while(fgets(loadedModules, sizeof(loadedModules), file) != NULL)
                {
                    printf("%s", loadedModules);
                }

                fclose(file);
            }
        }
    }

    closedir(proc_dir);
    close(proc_fd);
// ----------------------------------------------------------------------------------
}

void
listExecutablePages() {
// ---------------------------------------------------------------------------------------
// 		STEP 4: Is able to show all the executable pages within the processes.
    printf("\n\nHERE ARE ALL THE EXECUTABLE PAGES WITHIN THE PROCESSES:\n");

    int proc_fd = open("/proc", O_RDONLY | O_DIRECTORY);
    if (proc_fd == -1)
    {
        perror("Error opening directory /proc");
        return;
    }

    DIR *proc_dir = fdopendir(proc_fd);
    struct dirent *proc_entry;

    while ((proc_entry = readdir(proc_dir)) != NULL)
    {
        if (proc_entry->d_type == DT_DIR)
        {
            pid_t pid = atoi(proc_entry->d_name);
            if (pid > 0)
            {
                char executable[256];
                snprintf(executable, sizeof(executable), "/proc/%d/maps", pid);

                FILE *file = fopen(executable, "r");
                if (file == NULL)
                {
                    perror("fopen");
                    continue;
                }

                char executablePages[256];
                printf("\nExecutable pages for Process ID: %d\n", pid);

                while (fgets(executablePages, sizeof(executablePages), file) != NULL)
                {
                    if (strstr(executablePages, "x") != NULL)
                    {  // Check if the memory region is executable
                        printf("%s", executablePages);
                    }
                }

                fclose(file);
            }
        }
    }

    closedir(proc_dir);
    close(proc_fd);
// ---------------------------------------------------------------------------------------
}

void
displayTheMemory() {

}

// main.c
int
main(int argc, char *argv[])
{
    int userInput;
    char input[10];

    while (1) {
        printf("What would you like me to do?\n\n");
        printf("1. Enumerate all the running processes\n");
        printf("2. List all the running threads within process boundary.\n");
        printf("3. Enumerate all the loaded modules within the processes.\n");
        printf("4. Is able to show all the executable pages within the processes.\n");
        printf("5. Gives us a capability to read the memory.\n\n");
        printf("Enter your choice (1-5) or 'q' to quit: ");
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
