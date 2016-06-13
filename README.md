# File-Clone
The main file is proj1.c
This project is a C or application to clone a file.The entire contents of a file are copied to a different location and the meta information for the newly created file is updated to match the original file.
The program accepts the name of a file on the command line. This should be the first argument for the program. This is the file that will be copied. It is herein referred to as the source.
(a) If the source file does not exist or cannot be read, your program  reports an error and exit.
The program accepts a destination on the command line. This can be a path or a file.
(a) If the destination is a path, your program uses the same name for the desti- nation file as the source file. This is what the cp command does.
(b) If the destination is a file, your program uses that file name for the destination file. This is what the cp command does.
If the destination file exists, your program  overwrite the file if, and only if, the -f option was specified. If the destination exists and -f is not provided, report an error and exit.
The program copies all data from the source file to the destination file.
4. The destination file will have the same meta data as the source file:
(a) The owner of the destination file shall be the same as the owner of the source file.
(b) The group of the destination file shall be the same as the group of the source file.
(c) The permission bits for the destination file shall be the same as the permission bits for the source file.
(d) The last modified time for the destination file shall be the same as the last modified time for the source file. You can truncate to the nearest second.
