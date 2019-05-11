#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024*16

unsigned int cal_checksum(unsigned char *ptr, unsigned int len, unsigned int oldCheckSum)   
{   
	
        unsigned int CheckSum,i;
        unsigned int *D_ptr;
        D_ptr=(unsigned int *)ptr;
        CheckSum=oldCheckSum;
        for(i=0;i<len/sizeof(unsigned int);i++)   
        {
                CheckSum += *D_ptr;
                D_ptr++;
        }
	if(len%sizeof(unsigned int) > 0){
		//printf("Last package, len: %d\n", len);
		CheckSum += *D_ptr;
	}
        return CheckSum;
}

void userManual(char *name)
{
	if(name == NULL)
		return;

	printf("\n%s <filePath>\n\n", name);
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		userManual(argv[0]);
		return 1;
	}
	char *filePath = argv[1];
	char *outPath = NULL;
	int i = 0, ret = 1;
	int fd1 = -1, fd2 = -1;
	unsigned int checksum = 0;
	char buff[BUFSIZE];

	if(access(filePath, F_OK) != 0)
	{
		printf("File[%s] is not exist!!\n", filePath);
		return 1;
	}
	int len = strlen(filePath)+16;
	outPath = (char *)malloc(len);
	if(outPath == NULL)
	{
		perror("malloc error");
		return 1;
	}
	snprintf(outPath, len, "%s.ui", filePath);
	while(access(outPath, F_OK) == 0){
		snprintf(outPath, len, "%s_%02d.ui", filePath, ++i);
	}

	printf("source file: %s\nout file: %s\n", filePath, outPath);
	fd1 = open(filePath, O_RDONLY);
	if(fd1 < 0)
	{
		perror("Open filePath file");
		goto __END__;
	}
	fd2 = open(outPath, O_RDWR|O_CREAT, 0644);
	if(fd2 < 0)
	{
		perror("Open outPath file");
		goto __END__;
	}
	
	lseek(fd2, 8, SEEK_SET);
int filesum = 0;
	for(;;)
	{
		memset(buff, 0, sizeof(buff));
		i=read(fd1, buff, sizeof(buff));
		if(i <= 0) break;
		checksum = cal_checksum(buff, i, checksum);
		i=write(fd2, buff, i);
		if(i <= 0)
		{
			perror("Write file");
			goto __END__;
		}
		printf("Read size: %d\n", i);
		filesum += i;
	}

	printf("Total size: %d\n", filesum);
	printf("Checksum: %ud\n", checksum);
	lseek(fd2, 0, SEEK_SET);
	i = 1;
	i = write(fd2, (void *)&i, sizeof(unsigned int));
	if(i <= 0)
	{
		perror("Write file head1");
		goto __END__;
	}
	i = write(fd2, (void *)&checksum, sizeof(unsigned int));
	if(i <= 0)
        {
                perror("Write file head2");
                goto __END__;
        }

	ret = 0;

__END__:
	if(outPath != NULL) free(outPath);
	if(fd1 >= 0) close(fd1);
	if(fd2 >= 0) close(fd2);
	return ret;
}
