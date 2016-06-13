#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <utime.h>
#include <sys/time.h>
#include "tlpi_hdr.h"

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif


void cloneregularfiles(char * filename1,char * filename2){
	struct stat buffer;
	if(lstat(filename2,&buffer) == 0){
		switch(buffer.st_mode & S_IFMT){
			case S_IFREG:
				break;
			case S_IFDIR:
				break;
			default:
				printf("\nDestination file %s is of different type than source %s\n\n ",filename2,filename1 );
				exit(EXIT_FAILURE);
		}
	}
	int sourcefd, destfd, openflags;
	mode_t filePerms;
	ssize_t numRead;
	char buf[BUF_SIZE];
	sourcefd = open(filename1,O_RDONLY);
	if(sourcefd == -1){
		perror("open");
	 	exit(EXIT_FAILURE);
	}
	openflags = O_WRONLY | O_TRUNC | O_CREAT;
	filePerms = S_IRUSR | S_IWUSR | S_IRGRP |S_IWGRP | S_IROTH |S_IWOTH;
	destfd = open(filename2,openflags,filePerms);
	if (destfd == -1){
		perror("open");
		exit(EXIT_FAILURE);
	}
	while ((numRead = read(sourcefd,buf, BUF_SIZE)) > 0){
		if(write (destfd,buf,numRead) != numRead){
			perror("write");
			exit(EXIT_FAILURE);
		}
	}
	if(numRead == -1){
			perror("read");
			exit(EXIT_FAILURE);
		}
	if (close(sourcefd) == -1)
		perror("close source\n");
	if (close(destfd) == -1)
		perror("close destination\n");
	struct stat file1,file2;

	if(stat(filename1,&file1) == 0)
	{
		if(chown(filename2,file1.st_uid,file1.st_gid) == -1)
			perror("chown");
		chmod(filename2,file1.st_mode);
		struct utimbuf ubuf;
		ubuf.actime = file1.st_atime;
		ubuf.modtime = file1.st_mtime;
		utime(filename2,&ubuf);
	}
	char * command;
	if((command = malloc(strlen("diff -s ")+strlen(filename1)+1+strlen(filename2)+1)) != NULL){
		command[0] = '\0';
		strcat(command,"diff -s ");
		strcat(command,filename1);
		strcat(command," ");
		strcat(command,filename2);
	}
	system(command);
	exit(EXIT_SUCCESS);
}

void clonesymlinks(char * filename1,char * filename2){
	struct stat buffer;
	if(lstat(filename2,&buffer) == 0){
		switch(buffer.st_mode & S_IFMT){
			case S_IFLNK:
				break;
			case S_IFDIR:
				break;
			default:
				printf("\nDestination file %s is of different type than source %s\n\n ",filename2,filename1 );
				exit(EXIT_FAILURE);
		}
	}
	struct stat sb;
	char * linkname;
	ssize_t r;
	if(lstat(filename1,&sb) == -1){
		perror("lstat");
		exit(EXIT_FAILURE);
	}
	linkname = malloc(sb.st_size + 1);
	if(linkname == NULL){
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	r = readlink(filename1,linkname,sb.st_size + 1);
	if (r == -1){
		perror("readlink");
		exit(EXIT_FAILURE);
	}
	if(r > sb.st_size){
		perror("readlink");
		exit(EXIT_FAILURE);
	}
	linkname[r] = '\0';
	struct stat file2,file1;
	if(lstat(filename2,&file2) != 0){
		if(symlink(linkname,filename2) == -1){
			perror("symlink");
		}
	}
	else{
		if(unlink(filename2) != 0){
			perror("remove");
		}
		if(symlink(linkname,filename2) == -1){
			perror("symlink");
		}
	}

	if(stat(filename1,&file1) == 0){
		if(lchown(filename2,file1.st_uid,file1.st_gid) != 0){
			perror("lchown");
			free(linkname);
			exit(EXIT_FAILURE);
		}
		struct timeval times[2];
		times[0].tv_sec = file1.st_atime;
		times[0].tv_usec = 0;
		times[1].tv_sec = file1.st_mtime;
		times[1].tv_usec = 0;
		if(lutimes(filename2,times) != 0){
			perror("lutimes");
			free(linkname);
			exit(EXIT_FAILURE);
		}
	}
	free(linkname);
	exit(EXIT_SUCCESS);
}


int main(int argc, char* argv[]){
	if (argc == 1){
	 	printf("\nNo source file provided.\n\n");
	  	exit(EXIT_FAILURE);
	}
	if (argc == 2){
		char opt;
		int fcount=0;
		struct stat buffer;
		while((opt = getopt(argc, argv, "f")) != -1){
			switch(opt){
			case 'f':
					fcount=1;
					break;
			default:
					return -1;
			}
		}
		if(fcount == 1){
			printf("\nNo source file provided\n\n");
			exit(EXIT_FAILURE);
		}
		else{
			if(lstat(argv[1],&buffer) == 0)
				printf("\nNo destination path provided.\n\n");
			else
				printf("\nSource %s doesnot exist.\n\n",argv[1]);
			exit(EXIT_FAILURE);
		}
	}
	if(argc == 3){
	 	extern int optind;
	 	char opt;
	 	int fcount=0;
	 	struct stat buffer;
	 	while((opt = getopt(argc, argv, "f")) != -1){
	 		switch(opt){
	 			case 'f':
	 				fcount=1;
	 				break;
	 			default:
	 				return -1;
	 		}
	 	}
	 	if (fcount == 1){
	 		if(lstat(argv[optind],&buffer) == 0){
	 			printf("\nDestination path not provided.\n\n");
	 			exit(EXIT_FAILURE);
	 		}
	 		else{
	 			printf("\nSource path %s doesnot exist.\n\n",argv[optind]);
	 			exit(EXIT_FAILURE);
	 		}
	 	}
	 	else{
	 		printf("\n Option -f not provided\n\n");
	 		char * filename1 = argv[1];
	 		char * filename2 = argv[2];
	 		char * basename1 = basename(argv[1]);
	 		char * basename2 = basename(argv[2]);
	 		int sourcetype,desttype;
	 		sourcetype = desttype = 0;
	 		if(lstat(filename1,&buffer) != 0){
	 			printf("\nSource %s doesnot exist.\n\n",filename1);
	 			exit(EXIT_FAILURE);
	 		}
	 		else{
	 			switch(buffer.st_mode & S_IFMT){
	 				case S_IFDIR:
	 					printf("\nSource file %s is a directory\n\n",filename1);
	 					exit(EXIT_FAILURE);
	 					break;
	 				case S_IFLNK:
	 					sourcetype = 1;
	 					break;
	 				case S_IFREG:
	 					sourcetype = 2;
						break;
	 				default:
	 					sourcetype = -1;
	 					printf("\nSource file %s is not a regular file or symbolic link\n\n",filename1);
	 					exit(EXIT_FAILURE);
	 					break;
	 			}
	 		}
	 		if(lstat(filename2,&buffer) == 0){
	 			switch(buffer.st_mode & S_IFMT){ 			
	 				DIR *d;
	 				struct stat tempbuffer;
	 				struct dirent *dir;
	 				case S_IFDIR:	
	 					d = opendir(filename2);
	 					if(d){
	 						while ((dir = readdir(d)) != NULL){
	 							if (strcmp(dir->d_name,basename1) == 0){
	 								printf("\nFile %s already exists in %s\n\n",dir->d_name,filename2);
	 								exit(EXIT_FAILURE);
	 							}
							}
	 					}
	 					else{
	 						printf("\nCannot open directory %s\n\n",filename2);
	 						exit(EXIT_FAILURE);
	 					}
	 					break;
	 				case S_IFLNK:
	 					desttype = 1;
	 					printf("Destination file %s already exists\n\n",filename2 );
	 					exit(EXIT_FAILURE);
	 					break;
	 				case S_IFREG:
	 					desttype = 2;
	 					printf("Destination file %s already exists\n\n",filename2 );
	 					exit(EXIT_FAILURE);
	 					break;
	 				default:
	 					desttype = -1;
	 					printf("\n Destination file %s exists and is of different file type than source %s\n\n",filename2,filename1 );
	 					exit(EXIT_FAILURE);
	 					break;
	 			}		
	 			if(sourcetype == 1 && desttype == 0)
	 			{
	 				printf("\nCloning symbolic link %s with same name as source to a different directory %s\n\n",basename1,filename2);
	 				char * x;
	 				if((x = malloc(strlen(filename2)+1+strlen(basename1)+1)) != NULL){
	 					x[0] = '\0';
	 					strcat(x,filename2);
	 					strcat(x,"/");
	 					strcat(x,basename1);
	 				}
	 				clonesymlinks(filename1,x);
	 				exit(EXIT_SUCCESS);
	 			}
	 			if(sourcetype == 2 && desttype == 0 ){
	 				printf("\nCloning regular file %s with same name as source to a different directory %s\n\n",basename1,filename2);
	 				char * x;
	 				if((x = malloc(strlen(filename2)+1+strlen(basename1)+1)) != NULL){
	 					x[0] = '\0';
	 					strcat(x,filename2);
	 					strcat(x,"/");
	 					strcat(x,basename1);
	 				}
	 				cloneregularfiles(filename1,x);
	 				exit(EXIT_SUCCESS);
	 			}
	 		}
	 		else{
	 			char * dirname2 = dirname(filename2);
	 			if (lstat(dirname2,&buffer) == 0){
	 				if(sourcetype == 1 && desttype == 0){
	 					printf("\nCloning symbolic link %s to directory %s with different name %s\n\n",filename1,dirname2,basename2);
	 					char * x;
	 					if((x = malloc(strlen(dirname2)+1+strlen(basename1)+1)) != NULL){
	 						x[0] = '\0';
	 						strcat(x,dirname2);
	 						strcat(x,"/");
	 						strcat(x,basename2);
	 					}
	 					clonesymlinks(filename1,x);
	 					exit(EXIT_SUCCESS);
	 				}
	 				if(sourcetype == 2 && desttype == 0){
	 					printf("\nCloning regular file %s to directory %s with different name %s\n\n",filename1,dirname2,basename2);
	 					char * x;
	 					if((x = malloc(strlen(dirname2)+1+strlen(basename1)+1)) != NULL){
	 						x[0] = '\0';
	 						strcat(x,dirname2);
	 						strcat(x,"/");
	 						strcat(x,basename2);
	 					}
	 					cloneregularfiles(filename1,x);
	 					exit(EXIT_SUCCESS);
	 				}

	 			}
	 			else{
	 				printf("\nDestination path doesnot exist\n\n");
	 				exit(EXIT_FAILURE);
	 			}
	 		}
		}
	}
	 		
    if(argc >= 4){
	  	extern int optind;
	  	char opt;
	  	int fcount=0;
	  	struct stat buffer;
	  	while((opt = getopt(argc, argv, "f")) != -1){
	  		switch(opt){
		  		case 'f':
 					fcount=1;
 					break;
	  			default:
  					return -1;
	  		}
	  	}
	  	char * filename1 = argv[optind];
	  	char * filename2;
	  	int sourcetype, desttype;
	  	sourcetype = desttype = 0;
	  	int i;
	  	for(i = optind + 1; i < argc ; i++){
	  		if(argv[i][0] != '-' && argv[i][1] != 'f'){
	  			filename2 = argv[i];
	  			break;
	  		}
	 	}
	   	if(lstat(filename1, &buffer) != 0){
	   		printf("\nSource file %s doesnot exist.\n\n",filename1);
	   		exit(EXIT_FAILURE);
	   	}
	  	else{
	  		switch(buffer.st_mode & S_IFMT){
	  			case S_IFDIR:
	  				printf("\nSource file %s is a directory.\n\n",filename1);
	  				exit(EXIT_FAILURE);
	  				break;
	  			case S_IFLNK:
	  				sourcetype = 1;
	  				break;
	  			case S_IFREG:
	  				sourcetype = 2;
	  				break;
	  			default:
	  				sourcetype = -1;
	  				printf("\nSource file %s is not a symbolic link or regular file\n\n",filename1 );
	  				exit(EXIT_FAILURE);
	  				break;
	  		}
	  	}
	  	char * basename1 = basename(filename1);
	  	char * basename2 = basename(filename2);
	  	if (lstat(filename2,&buffer) == 0){
	  		switch(buffer.st_mode & S_IFMT){
	 			DIR *d;
	 			struct stat tempbuffer;
	 			struct dirent *dir;
	 			case S_IFDIR:	
	 				d = opendir(filename2);
	 				if(d){
	 					while ((dir = readdir(d)) != NULL){
	 						if (strcmp(dir->d_name,basename1) == 0){
	 							if(fcount == 0){
	 								printf("\nFile %s already exists in %s\n\n",dir->d_name,filename2);
	 								exit(EXIT_FAILURE);
	 							}
								else{
	 								char * desttempfilename;
	 								if((desttempfilename = malloc(strlen(filename2)+1+strlen(dir->d_name)+1)) != NULL){
	 									desttempfilename[0] = '\0';
	 									strcat(desttempfilename,filename2);
	 									strcat(desttempfilename,"/");
	 									strcat(desttempfilename,dir->d_name);
	 								}
	 								struct stat tempbuffer;
	 								if(lstat(desttempfilename,&tempbuffer) == 0){
	 									switch(tempbuffer.st_mode & S_IFMT){
	 										case S_IFLNK:
	 											if(sourcetype == 1){
	 												printf("\nOverwriting file %s with source file %s\n\n",filename2,filename1 );
	 												clonesymlinks(filename1,desttempfilename);
	 												exit(EXIT_SUCCESS);
	 											}
	 											break;
	 										case S_IFREG:
	 											if(sourcetype == 2){
	 												printf("\nOverwriting file %s with source file %s\n\n",filename2,filename1 );
	 												cloneregularfiles(filename1,desttempfilename);
	 												exit(EXIT_SUCCESS);
	 											}
												break;
	 										default:
												printf("\nDestination file %s exists and is of different type than source %s.\n\n",filename2,filename1);
	 											exit(EXIT_FAILURE);
	 											break;
	 									}
	 								}		
	 							}
	 						}
						}
	 				}
	 				else{
	 					printf("\nCannot open directory %s\n\n",filename2);
	 					exit(EXIT_FAILURE);
	 				}
	 				break;
	 			case S_IFLNK:
	 				desttype = 1;
	 				if(sourcetype == -1 || sourcetype == 2){
	 					printf("\nDestination file %s exists and is of different type than source %s.\n\n",filename2,filename1);
	 					exit(EXIT_FAILURE);
	 				}
	 				if(sourcetype == 1){
	 					if(strcmp(basename1,basename2) == 0){
	 						if (fcount == 0){
	 							printf("\nDestination file %s already exists.\n\n",filename2);
	 							exit(EXIT_FAILURE);
	 						}
	 					}	
	 				}
	 				break;
	 			case S_IFREG:
	 				desttype = 2;
	 				if(sourcetype == -1 || sourcetype == 1){
	 					printf("\nDestination file %s is of different type than source %s.\n\n",filename2,filename1);
	 					exit(EXIT_FAILURE);
	 				}
	 				if(sourcetype == 2){
	 					if(strcmp(basename1,basename2) == 0){
	 						if (fcount == 0){
	 							printf("\nDestination file %s already exists.\n\n",filename2);
	 							exit(EXIT_FAILURE);
	 						}	
		 				}
			 		}
	  				break;
	  		}
			if(sourcetype == 1 && desttype == 0){
				printf("\nCloning symbolic link %s with the same name to the directory %s\n\n",basename1,filename2 );
				char * x;
				if((x = malloc(strlen(filename2)+1+strlen(basename1)+1)) != NULL)
				{
					x[0] = '\0';
					strcat(x,filename2);
					strcat(x,"/");
					strcat(x,basename1);
				}
				clonesymlinks(filename1,x);
				exit(EXIT_SUCCESS);
			}
			if(sourcetype == 2 && desttype == 0){
				printf("\nCloning regular file %s with the same name to the directory %s\n\n",basename1,filename2 );
				char * x;
				if((x = malloc(strlen(filename2)+1+strlen(basename1)+1)) != NULL)
				{
					x[0] = '\0';
					strcat(x,filename2);
					strcat(x,"/");
					strcat(x,basename1);
				}
				cloneregularfiles(filename1,x);
				exit(EXIT_SUCCESS);
			}
			if(sourcetype == 1 && desttype == 1){
				if(fcount == 1){
					printf("\nCloning symbolic link %s\n\n",basename1);
					clonesymlinks(filename1,filename2);
					exit(EXIT_SUCCESS);
				}
				else{
					printf("\nDestination file %s already present\n\n",filename2);
					exit(EXIT_FAILURE);
				}
			}
			if(sourcetype == 2 && desttype == 2){
				if(fcount == 1){
					printf("\nCloning regular file %s\n\n",basename1);
					cloneregularfiles(filename1,filename2);
					exit(EXIT_SUCCESS);
				}
				else{
					printf("\nDestination file %s already present\n\n",filename2);
					exit(EXIT_FAILURE);
				}
			}
	 	}
	  	else{
	 		char * dirname2 = dirname(filename2);
	 		if (lstat(dirname2,&buffer) == 0){
				if(sourcetype == 1 && desttype == 0){
					printf("\nCloning symbolic link %s to directory %s with different name %s\n\n",filename1,dirname2,basename2);
					char * x;
					if((x = malloc(strlen(dirname2)+1+strlen(basename2)+1)) != NULL)
					{
					x[0] = '\0';
					strcat(x,dirname2);
					strcat(x,"/");
					strcat(x,basename2);
					}
					clonesymlinks(filename1,x);
					exit(EXIT_SUCCESS);
				}
				if(sourcetype == 2 && desttype == 0){
					printf("\nCloning regular file %s to directory %s with different name %s\n\n",filename1,dirname2,basename2);
					char * x;
					if((x = malloc(strlen(dirname2)+1+strlen(basename2)+1)) != NULL)
					{
					x[0] = '\0';
					strcat(x,dirname2);
					strcat(x,"/");
					strcat(x,basename2);
					}
					clonesymlinks(filename1,x);
					exit(EXIT_SUCCESS);
				}
	 		}
	 		else{
	 			printf("\nDestination path doesnot exist\n\n");
	 			exit(EXIT_FAILURE);
		 	}
		}
	}
}