#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int fd;
	fd = creat("/Users/yao56383702/workspace/code/IO/test.c", 0644);	
	write(fd, "124w", 4);
	return 0;
}
