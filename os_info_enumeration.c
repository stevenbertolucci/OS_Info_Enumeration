// ----------------------------------------------------------------------------
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
// 	4. Is able to show all the executable pages within the processes.
// 	5. Gives us a capability to read the memory.
// ----------------------------------------------------------------------------

/*
   CITATIONS:

    I researched on how to navigate through directories in a Linux system
    and I found a couple man pages that uses dirent.h. Dirent.h describes an
    entry in a directory such as d_name.

    Source: https://man7.org/linux/man-pages/man0/dirent.h.0p.html
  https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html

    I used this approach for all 5 tasks.

    To open directories, I found this man page that uses fopendir(). Source:
             https://man7.org/linux/man-pages/man3/opendir.3.html

    For Task 1 and 2, I researched on how to display all the processes and I found
    information on The Linux Kernel Documentation under
    section '3.6 /proc/<pid>/comm & /proc/<pid>/task/<tid>/comm'. The source
    to this file is:

 https://docs.kernel.org/filesystems/proc.html#proc-pid-comm-proc-pid-task-tid-comm

    For Task 3, this documentation displayed the overview of the directories
    inside the /proc directory. The source to the documentation is:

        https://tldp.org/LDP/Linux-Filesystem-Hierarchy/html/proc.html

    This article gave me the location of the loaded modules: /proc/modules

    For Task 4 and 5, I researched on how to retrieve the executable processes
    and this article provided detailed information regarding the maps file.
    The source to this article is:

             https://www.baeldung.com/linux/proc-id-maps

    According to the article above, the maps file has information regarding
    the virtual memory of that particular process.
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define MEMORY_READ_LIMIT 1024

// ----------------------------------------------------------------
// 		       STEP 1: Enumerate all the running processes.
// ----------------------------------------------------------------
// Function to list all running processes
void listRunningProcesses()
{
    // Display message
    printf("\033[1;31m\nHERE ARE ALL THE RUNNING PROCESSES:\n\033[0m");

    // For resetting the stdout color profile
    fflush(stdout);

    // Open the /proc directory using a file descriptor
    int fd = open("/proc", O_RDONLY | O_DIRECTORY);

    // Convert the file descriptor to a DIR pointer
    DIR *dirp = fdopendir(fd);

    struct dirent *dirp_entry;

    // Iterate through every PID directory in /proc
    while ((dirp_entry = readdir(dirp)) != NULL)
    {
        // Check if the entry is a directory and the name is all digits (indicating a process ID)
        if (dirp_entry->d_type == DT_DIR)
        {
            // Directory name is a string. Need to change it to int so that I can access it and display it
            pid_t pid = atoi(dirp_entry->d_name);

            // PID folders
            if (pid > 0)
            {
                // Comm file path will contain the running process
                char comm_path[MEMORY_READ_LIMIT];
                snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

                // Open the comm file for reading
                FILE *comm_file = fopen(comm_path, "r");

                // Get the running process from the file and display it
                char comm[MEMORY_READ_LIMIT];
                if (fgets(comm, sizeof(comm), comm_file) != NULL)
                {
                    // Gets rid of the newline character
                    comm[strcspn(comm, "\n")] = '\0';
                    printf("Process ID: %d, Command: %s\n", pid, comm);
                }

                // Close the file
                fclose(comm_file);
            }
        }
    }

    // Close the directory
    closedir(dirp);
    close(fd);
}

// --------------------------------------------------------------------------------
// 		    STEP 2: List all the running threads within process boundary.
// --------------------------------------------------------------------------------
void
listRunningThreads() {

    // Display message
    printf("\033[1;31m\nHERE ARE ALL THE RUNNING THREADS WITHIN PROCESS BOUNDARY:\n\033[0m");

    // Open the directory
    int fd = open("/proc", O_RDONLY | O_DIRECTORY);

    DIR *dirp = fdopendir(fd);
    struct dirent *dirp_entry;

    // Iterate through the directory
    while ((dirp_entry = readdir(dirp)) != NULL)
    {
        // If it is a directory inside a directory
        if (dirp_entry->d_type == DT_DIR)
        {
            // Convert name of directory to int
            pid_t pid = atoi(dirp_entry->d_name);

            // Iterate through valid PID directories
            if (pid > 0)
            {
                char tasks[MEMORY_READ_LIMIT];
                snprintf(tasks, sizeof(tasks), "/proc/%d/task", pid);

                // Open the directory
                int task_fd = open(tasks, O_RDONLY | O_DIRECTORY);

                DIR *task_dir = fdopendir(task_fd);
                struct dirent *task_entry;

                // Iterate through the directories inside task
                while ((task_entry = readdir(task_dir)) != NULL)
                {
                    // Filter out directories that starts with '.'
                    if (task_entry->d_type == DT_DIR && strcmp(task_entry->d_name, ".") != 0 && strcmp(task_entry->d_name, "..") != 0)
                    {
                        // Print the thread number and process ID
                        printf("\033[1;36mThread: %s\033[0m in Process ID: %d\n", task_entry->d_name, pid);
                    }
                }

                // Close directories
                closedir(task_dir);
                close(task_fd);
            }
        }
    }

    // Close directories that are higher in hierarchy
    closedir(dirp);
    close(fd);
}

// ----------------------------------------------------------------------------------
// 		    STEP 3: Enumerate all the loaded modules within the processes.
// ----------------------------------------------------------------------------------
void
listLoadedModules() {

    // Print message
    printf("\033[1;31m\nHERE ARE ALL THE LOADED MODULES:\n\033[0m");

    // Open file for reading
    FILE *file = fopen("/proc/modules", "r");

    char module[MEMORY_READ_LIMIT];

    // Display output message
    printf("\033[1;36m%-20s %-10s %-10s\n\033[0m", "Module", "Size", "Used by");

    // Iterate through the module file
    while (fgets(module, sizeof(module), file) != NULL)
    {
        // Variables to hold data for each entry
        char moduleName[50];
        unsigned long moduleSize;
        int moduleUsage;
        char moduleDetails[128];

        // Formatted the output
        sscanf(module, "%49s %lu %d %127s", moduleName, &moduleSize, &moduleUsage, moduleDetails);
        printf("%-20s %-10lu %-10d %s\n", moduleName, moduleSize, moduleUsage, moduleDetails);
    }

    // Close the file
    fclose(file);
}

// ---------------------------------------------------------------------------------------
// 		    STEP 4: Is able to show all the executable pages within the processes.
// ---------------------------------------------------------------------------------------
void
listExecutablePages() {

    // Display message
    printf("\033[1;31m\n\nHERE ARE ALL THE EXECUTABLE PAGES WITHIN THE PROCESSES:\n\033[0m");

    // Open directory
    int fd = open("/proc", O_RDONLY | O_DIRECTORY);

    DIR *dirp = fdopendir(fd);

    struct dirent *dirp_entry;

    // Iterate through the directory
    while ((dirp_entry = readdir(dirp)) != NULL)
    {
        // Check if there are directories in this directory
        if (dirp_entry->d_type == DT_DIR)
        {
            // Convert directory name to int because it is a PID
            pid_t pid = atoi(dirp_entry->d_name);

            // Iterate through each PID directory
            if (pid > 0)
            {
                char executable[MEMORY_READ_LIMIT];

                // Copy the data from the path
                snprintf(executable, sizeof(executable), "/proc/%d/maps", pid);

                // Open file
                FILE *file = fopen(executable, "r");

                // Check for file opening errors
                if (file == NULL)
                {
                    perror("HEY! You shouldn't be here. *DOOR SLAMMED CLOSED*");
                    continue;
                }

                char executablePages[MEMORY_READ_LIMIT];
                printf("\033[1;36m\nExecutable pages for Process ID: %d\n\033[0m", pid);

                // Read the file and display the content
                while (fgets(executablePages, sizeof(executablePages), file) != NULL)
                {
                    // If this is an executable process, display it
                    if (strstr(executablePages, "x") != NULL)
                    {
                        printf("%s", executablePages);
                    }
                }

                // Close the file
                fclose(file);
            }
        }
    }

    // Close directories
    closedir(dirp);
    close(fd);
}

// ---------------------------------------------------------------------------------------
// 		            STEP 5: Gives us a capability to read the memory.
// ---------------------------------------------------------------------------------------
void displayTheMemory()
{
    // Display message
    printf("\033[1;31m\nMEMORY RANGES FOR EACH PROCESS:\n\033[0m");

    // Open the /proc directory
    int fd = open("/proc", O_RDONLY | O_DIRECTORY);

    DIR *proc_dir = fdopendir(fd);

    struct dirent *dirp_entry;

    // Iterate through the directory
    while ((dirp_entry = readdir(proc_dir)) != NULL)
    {
        // Iterate through the PID directories
        if (dirp_entry->d_type == DT_DIR && atoi(dirp_entry->d_name) > 0)
        {
            // Convert PIDs to int
            pid_t pid = atoi(dirp_entry->d_name);
            char maps_path[MEMORY_READ_LIMIT];

            // Copy content of maps file to maps_path
            snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

            // Open the maps_file
            FILE *maps_file = fopen(maps_path, "r");

            // Check for issues when opening
            if (maps_file == NULL)
            {
                perror("Sorry, I can't open this file. Please keep going :)");
                continue;
            }

            char line[MEMORY_READ_LIMIT];

            // Output message
            printf("\033[1;33mProcess %d:\n\033[0m", pid);

            // Iterate through the file and display the memory content
            while (fgets(line, sizeof(line), maps_file))
            {
                char *path = strchr(line, '/');

                // Display the information
                if (path != NULL)
                {
                    printf("%.*s %s", (int)(path - line), line, path);
                } else {
                    printf("%s", line);
                }
            }

            printf("\n");

            // Close the file
            fclose(maps_file);
        }
    }

    // Close the directories
    closedir(proc_dir);
    close(fd);
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

        // Check for user input to quit
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
