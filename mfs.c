/**************************************************************

 * File: mfs.c
 *
 * Description: Basic File System - Implementations of filesystem functions
 * as outlined in mfs.h and the https://www.thegeekstuff.com/2012/06/c-directory/
 *
 **************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "mfs.h"
#include "dir.h"
#include "vcb.h"

// -------------------------
// Misc directory functions

// linux chdir (set current working directory)
int fs_setcwd(char *pathname)
{
    // NOTE: Go up a directory by using input ".." to return to go up a directory
    // Use the chdir linux syscall to change the path to the given pathname
    int chDirResult = chdir(pathname);
    // printf("chdirResult is %d\n", chDirResult);
    //  Check if chDirResult failed, if it failed, stop here
    if (chDirResult != 0)
    {
        printf("chDirResult was -1, exiting command\n");
        // If it doesn't work, return -1 to indicate that this failed
        return -1;
    }

    // printf("free for previous g->curPath in setcwd\n");
    //  Free the previously allocated memory for g->curPath before it is changed
    //  segmentation faults hate this one simple trick
    free(g->curPath);

    // If chdir had a result of 0 (success)
    // Update global currentPath to this new path name
    // printf("Input pathname: %s\n", pathname);
    // printf("Current g->curPath: %s\n", g->curPath);

    // Malloc again to account for the new string length
    g->curPath = malloc(strlen(pathname) + 1); // +1 for the null terminator
    if (g->curPath == NULL)
    {
        // Debug message for malloc here
        perror("Malloc failed for g->curPath inn setcwd malloc");
        return -1;
    }

    // printf("Start of strcpy\n");
    strcpy(g->curPath, pathname);

    // Print statement to inform the user of what the heck just changed.
    if (strcmp(pathname, "..") == 0)
    {
        printf("Moving up to parent directory\n");
    }
    else
    {
        printf("Current working directory has been changed to: %s\n", pathname);
    }

    // Return 0 to indicate it succeeded
    return 0;
}

// Get current working directory (return the pathname)
char *fs_getcwd(char *pathname, size_t size)
{

    // Use getcwd linux syscall to get the current working directory
    char *cwd = getcwd(pathname, size);

    // If there is somehow no current working directory, error out of this
    if (cwd == NULL)
    {
        perror("No working directory was found by getcwd");
        return NULL;
    }

    return cwd; // Return the path of the current working directory
}

// Check if input is a file, return 1 if file, 0 otherwise
int fs_isFile(char *filename)
{
    // Iterate through all directory entries in the loaded directory (CWD)
    for (int i = 0; i < g->actualDECount; i++)
    {
        // See if the directory entry at the current index matches the filename we're looking for
        if (strcmp(g->loadedDir[i].name, filename) == 0)
        {
            // If it does, check if it indicates it's a directory or not
            if (g->loadedDir[i].isDirectory == 0)
            {
                return 1; // If it's not a directory, then it's a file, so return 1
            }
            else
            {
                return 0; // If it says it's a directory, then it's not a file , so return 0.
            }
        }
    }
}

// Essentially the same as isFile but the conditions for returns are swapped,
// and we're searching for pathname instead of filename, return 1 if directory, 0 otherwise
int fs_isDir(char *pathname)
{
    // Iterate through all directory entries in the loaded directory (CWD)
    for (int i = 0; i < g->actualDECount; i++)
    {
        // See if the directory entry at the current index matches the pathname we're looking for
        if (strcmp(g->loadedDir[i].name, pathname) == 0)
        {
            // If it does, check if it indicates it's a directory or not
            if (g->loadedDir[i].isDirectory == 1)
            {
                return 1; // If it says it's a directory, then return 1
            }
            else
            {
                return 0; // If it says it's not a directory, return 0.
            }
        }
    }
}

// The goal of fs_delete is to essentially remove a file by marking it as free space
int fs_delete(char *filename)
{
    // Using dirLocate to find the directory entry with this filename
    int location = dirLocate(filename);

    // Make sure that it was actually found before doing anything else
    if (location == -1)
    { // If it was not found
        fprintf(stderr, "That filename was not found with directoryLocate \n");
        return -1; // Return immediately so this doesn't delete anything incorrectly
    }

    // Make sure that the found location is not a directory
    // Because this function is only intended to operate on files, not directories
    if (g->loadedDir[location].isDirectory) // If it is a directory
    {
        fprintf(stderr, "This is a directory, not a file, cancelling operation. \n");
        return -1;
    }
    else
    {
        // Use dirDelete from dir.c to delete this entry
        dirDelete(g->loadedDir, location);
        return 0;
    }
}
// _________________________

// -------------------------
// Key directory functions

// Make a new directory, currently using the existing dirInit function from dir.c
int fs_mkdir(const char *pathname, mode_t mode)
{

    // printf("Running dirLocate on pathname %s\n", pathname);
    //  Check if this directory already exists
    int alreadyThere = dirLocate(pathname);
    // Again, not sure if this is the intended use of this function but I'm trying it

    // This will be -1 if it was not found, if it's anything else, then it already exists
    if (alreadyThere != -1)
    {
        fprintf(stderr, "A directory already exists at that path, cancelling operation. \n");
        return -1;
    }

    // Otherwise if this directory doesn't exist, then make a new one
    // New directory entry will use the cwd as parent
    directoryEntry *newDirectoryEntry = dirInit(2, g->loadedDir);

    // New directory's name will be based on the pathname
    char *newName = strrchr(pathname, '/');
    if (newName == NULL)
    {
        newName = (char *)pathname;
    }
    else
    {
        // If there was a '/' in the path name, advance one index, so we can get the directory name
        newName++;
    }
    strcpy(newDirectoryEntry[0].name, newName);
    printf("The new '%s' directory has been created\n", newDirectoryEntry[0].name);

    // Copy this new directory into the current working directory so it can be seen and used
    dirCopy(g->loadedDir, newDirectoryEntry, g->actualDECount);
    return 0; // Return 0 to indicate that this function completed
}

// The goal of fs_rmdir is to essentially remove a directory by marking it as free space
int fs_rmdir(const char *pathname)
{

    // Using dirLocate to find the directory entry with this pathname
    // Note: Note sure if this function was intended for this use
    int location = dirLocate(pathname);

    // Make sure that it was actually found before doing anything else
    if (location == -1)
    { // If it was not found
        fprintf(stderr, "No directory was found with pathname: %s \n", pathname);
        return -1; // Return immediately so this doesn't delete anything incorrectly
    }

    // Make sure that the found location is a directory
    // Because this function is only intended to operate on directories, not files
    if (!g->loadedDir[location].isDirectory) // If it is not a directory
    {
        fprintf(stderr, "This is not a directory, it is a file, cancelling operation. \n");
        return -1;
    }
    else
    { // If it is a directory

        // Check if it is empty
        for (int i = 2; i < g->actualDECount; i++)
        {
            // If there are any non-nulls
            if (g->loadedDir[location].name[i] != '\0')
            {
                fprintf(stderr, "Directory is not empty, and will not be deleted \n");
                return -1; // Return -1 to indicate this didn't successfully delete
            }

            // If there were no non-nulls, then delete this directory
            dirDelete(g->loadedDir, location);
        }
    }
}

// _________________________

// -------------------------
// Directory iteration functions

// Opening a directory stream
fdDir *fs_opendir(const char *pathname)
{
    printf("Opening directory stream on pathname: %s", pathname);
    // printf("Directory contents: \n");
    //  Open a directory stream on the current pathname
    DIR *directoryStream = opendir(pathname);
    // Make sure it opened properly
    if (directoryStream == NULL)
    {
        // If something went wrong opening, print an error and return NULL
        perror("Directory stream didn't open properly, something went wrong");
        return NULL;
    }

    // Allocate memory for the dirp
    fdDir *dirp = (fdDir *)malloc(sizeof(fdDir));
    if (dirp == NULL)
    {
        // If there was a problem allocating memory, print an error, close the directory stream, and return NULL
        perror("Memory for dirp was not allocated correctly");
        closedir(directoryStream);
        return NULL;
    }

    // Set initial values for fdDir struct
    dirp->d_reclen = 0;
    dirp->dirEntryPosition = 0;
    dirp->di = NULL; // Set to NULL, experimental (unused in new readdir)

    // Initialize dirp->directoryStream with the result of opendir
    dirp->directoryStream = directoryStream;

    return dirp;
}

// Used to get the next directory entry in the directory stream from the function above this
struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    // printf("Inside readdir function\n");

    // Before starting, make sure the pointers actually point to something
    if (dirp == NULL || dirp->directoryStream == NULL)
    {
        fprintf(stderr, "Readdir: One or more of the directory stream pointers is NULL: dirp = %p,  dirp->directoryStream = %p \n", (void *)dirp, (void *)dirp->di);
        return NULL;
    }

    // printf("Before readdir function");

    // Use readdir to read the next entry in the stream
    struct dirent *directoryEntry = readdir(dirp->directoryStream);

    // Make sure this doesn't overread the stream by stopping at a NULL
    if (directoryEntry == NULL)
    {
        // If we reach here, it means there are no more entries
        // printf("End of directory reached, exiting now\n");
        return NULL;
    }

    // Malloc for the directory item info structure
    struct fs_diriteminfo *dirItemInfo = (struct fs_diriteminfo *)malloc(sizeof(struct fs_diriteminfo));
    if (dirItemInfo == NULL)
    {
        perror("Malloc failed for the directory item info structure");
        return NULL;
    }

    // Set directory info structure values to values retrieved from dirent
    dirItemInfo->d_reclen = (unsigned short)sizeof(struct fs_diriteminfo);             // length of entry (unsigned short used by d_reclen)
    dirItemInfo->fileType = (unsigned char)directoryEntry->d_type;                     // type of entry (unsigned char used by d_type)
    strncpy(dirItemInfo->d_name, directoryEntry->d_name, sizeof(dirItemInfo->d_name)); // name of entry

    // printf("Got to return statement\n");
    //  Return what was found with this function
    return dirItemInfo;
}

// Close the directory stream if it is open
int fs_closedir(fdDir *dirp)
{
    // Before starting, make the stream is open
    // Correction due to changed code in previous function, dirp->di no longer checked since it will be nulled when it's done
    if (dirp == NULL)
    {
        // Print out both of them to find out which
        fprintf(stderr, "Closedir: DIRP is NULL: dirp = %p,  Which should mean that there is no stream open \n", (void *)dirp);
        return -1; // Abort the read process before things clown out
    }

    // Close the stream (only reaches this point if it didn't NULL earlier)
    // printf("Now closing the stream\n");
    closedir((DIR *)(dirp->di));
    free(dirp); // segmentation faults hate this one simple trick

    return 0;
}

// Retrieves information about a directory or file from the fs_stat struct
int fs_stat(const char *path, struct fs_stat *buf)
{
    // Make sure that both values are not null before continuing so it doesn't break
    if (path == NULL || buf == NULL)
    {
        fprintf(stderr, "One or more of the inputs is NULL: path = %p,  buf = %p \n", (void *)path, (void *)buf);
        return -1; // Abort the process before things break
    }

    struct fs_stat fileStat;
    // Call fs_stat on the given path and this new struct
    if (fs_stat(path, &fileStat) != 0)
    { // File info was not retrieved
        return -1;
    }

    printf("Starting transfer...");
    // Copy over stats from fileStat to the buffer fs_stat struct
    buf->st_size = fileStat.st_size;
    buf->st_blksize = fileStat.st_blksize;
    buf->st_blocks = fileStat.st_blocks;
    buf->st_accesstime = fileStat.st_accesstime;
    buf->st_modtime = fileStat.st_modtime;
    buf->st_createtime = fileStat.st_createtime;

    printf("Everything has been copied to the buffer\n");

    return 0; // Return once everything has been copied
}

// _________________________
