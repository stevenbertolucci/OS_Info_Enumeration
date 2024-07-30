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

// main.c 
int
main(int argc, char *argv[]) 
{

// ----------------------------------------------------------------
// 		STEP 1: Enumerate all the running processes.

	printf("\nHERE ARE ALL THE RUNNING PROCESSES:\n");

	// Enumerate all the running processes
	int r = system("ps -eLf");
// ----------------------------------------------------------------


// --------------------------------------------------------------------------------
// 		STEP 2: List all the running threads within process boundary.

	printf("\nHERE ARE ALL THE RUNNING THREADS WITHIN PROCESS BOUNDARY:\n");

	// List all the running threads within process boundary	
	DIR *proc_dir = opendir("/proc");

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

				DIR *threads_dir = opendir(threads);
	
				struct dirent *threads_entry;

				while ((threads_entry = readdir(threads_dir)) != NULL)
				{
					if (threads_entry->d_type == DT_DIR && strcmp(threads_entry->d_name, ".") != 0 && strcmp(threads_entry->d_name, "..") != 0)
					{
						printf("Thread ID: %s in Process ID: %d\n", threads_entry->d_name, pid);
					}		
				}

				closedir(threads_dir);

            		}
        	}
    	}

    	closedir(proc_dir);
// ----------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------
// 		STEP 3: Enumerate all the loaded modules within the processes.

	printf("\nHERE ARE ALL THE LOADED MODULES WITHIN THE PROCESSES:\n");

	proc_dir = opendir("/proc");

	if (proc_dir == NULL)
	{
		perror("Issue opening /proc directory");
		return 1;
	}

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


// ----------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------
// 		STEP 4: Is able to show all the executable pages within the proccesses.
    printf("\n\nHERE ARE ALL THE EXECUTABLE PAGES WITHIN THE PROCESSES:\n");

    proc_dir = opendir("/proc");

    if (proc_dir == NULL)
    {
        perror("Error opening directory /proc");
        return 1;
    }

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



// ---------------------------------------------------------------------------------------


// ------------------------------------------------------------------
// 		STEP 5: Gives us a capability to read the memory.




// ------------------------------------------------------------------
	return 0;
}
