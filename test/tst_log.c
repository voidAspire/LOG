
#include "../log.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

int i = 0, j = 0, k = 0;

void *LOG_TstPthread(void *logstring)
{
	printf("the %d pthread\n", ++j);
	char buffer[100] = {0};
	strcpy(buffer, logstring);
	strcat(buffer, "pthread");
	int n = 5;
	while (n--)
	{
		LOG_Log(1, buffer, strlen(buffer));
		LOG_LogPrint(1, "%s%d", "hello", 1);
		//sleep(1);
	}
	printf("the %d pthread done\n", ++k);
	return;

}


int main(int argc, char const *argv[])
{
	pthread_t pid[100];

	char *logstring = argv[1];
	LOG_zCfg_t kpkzCfg =
	{
		.szPath         = LOG_CFG_LOG_PATH,
		.fileName       = LOG_CFG_FILE_NAME,
		.maxSizeLogFile = LOG_CFG_FILE_MAX_SIZE_KB,
		.eDefaultLevel  = LOG_eLevelInfo
	};


	LOG_Init( &kpkzCfg );
	LOG_Start();
	LOG_SetLogLevel(LOG_eLevelWarning);
	for (; i < 10; i++)
	{
		if (pthread_create(pid + i, NULL, LOG_TstPthread, logstring) != 0)
		{
			printf("pthread error\n");
		}
	}
	// LOG_Log(1, logstring, strlen(logstring));
	// getchar();
	// LOG_Log(1, logstring, strlen(logstring));
	// int n = 5;
	// while (n--)
	// {
	// 	LOG_Log(1, logstring, strlen(logstring));
	// 	sleep(1);
	// }
	// LOG_Log(1, logstring, strlen(logstring));
	// getchar();

	// for (; i < 10; i++)
	// {
	// 	if (pthread_create(pid + i, NULL, LOG_TstPthread, logstring) != 0)
	// 	{
	// 		printf("pthread error\n");
	// 	}
	// }

	getchar();
	LOG_Stop();


	return 0;
}