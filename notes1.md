# mmap() and munmap()

  mmap(NULL,shm_pool_size,PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0);
mmap() function u already know
fd is used to give in the file descriptor, fd of what file we wnna turn into a shm

munmap() is used to close that file,basically give that memory back to the os

# shm_open()
shm_open(
    name, //name of file 
    O_RDWR | O_CREAT | O_EXCL //O_RDWR	Open the object for reading and writing
                                O_CREAT	Create the object if it doesn't exist
                                O_EXCL	When combined with O_CREAT, fail if the name already exists
    0600
//This restricts the new shared-memory object to the owner's read/write permissions, subject to the operating system's permission handling.
);
