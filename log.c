
/**********************************************************************************************************
 *  A. Standard Includes
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
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
#include <stdarg.h>


/**********************************************************************************************************
*  sepecific Includes
*********************************************************************************************************/

#include "log.h"

/**********************************************************************************************************
 *  C. Object like Macro
*********************************************************************************************************/
 


/**********************************************************************************************************
 *  D. Function like Macro
*********************************************************************************************************/

/**********************************************************************************************************
 *  E. Local Function Declarations
*********************************************************************************************************/

static void     log_GetSysTime( struct tm* timeinfo );
static void     log_GetLogFileInfo( void );
static void     log_CreatNewLogFileWhenNecessary( void );
static void     log_PrepareLog( LOG_eLevel_t eLevel );
static uint32_t log_GetLogFileSizeFromOS(char *logFullName );

/**********************************************************************************************************
 *  F. Local Object/Variable
*********************************************************************************************************/

static LOG_zCfg_t       log_zCfg;
static LOG_zStatus_t    log_zStatus;

static const char*      log_akpcLogLevelString[ LOG_eNumLevels ] = 
{
    "LOG_eLevelInfo",    //LOG_eLevelInfo
    "LOG_eLevelWarning", //LOG_eLevelWarning
    "LOG_eLevelError",   //LOG_eLevelError
};

/**********************************************************************************************************
 *  G. Exported Object/Variable
 *********************************************************************************************************/

LOG_zCfg_t  LOG_kzCfg = 
{
	.szPath         = LOG_CFG_LOG_PATH,
	.fileName       = LOG_CFG_FILE_NAME,
	.maxSizeLogFile = LOG_CFG_FILE_MAX_SIZE_KB,
	.eDefaultLevel  = LOG_eLevelInfo
};


/**********************************************************************************************************
 *  Local Function Implementations
**********************************************************************************************************/

/*********************************************************************************************************
 * @brief     get current system time info
 *
 * @param   timeinfo :  hand of time info struct
 * @usage
 *
 * @return  
 *
 * 
****************************************************************************************************/
static void log_GetSysTime( struct tm* timeinfo )
{
	time_t rawtime;
	time(&rawtime);
	*timeinfo = *(localtime(&rawtime));
	return;
}


/*********************************************************************************************************
 * @brief   get log file size
 *
 * @param   logFullName :  the path and name of log file
 * @usage
 *
 * @return   log file size if no error. 
 *           zero otherwise.
 *
 * 
****************************************************************************************************/
static uint32_t log_GetLogFileSizeFromOS( char *logFullName )
{
	struct stat flog_stat;
             uint32_t    uSize = 0uL;
    
	if ( !(stat(logFullName, &flog_stat)) )
	{
		uSize =  flog_stat.st_size;
	}
	
    return uSize;
}

/*********************************************************************************************************
 * @brief   generate a log file name based on the followings:
 *          - path.
 *          - name.
 *          - file number.
 *
 * @param   buff: to hold the return file name.
 * @usage
 *
 * @return   
 *
 * 
****************************************************************************************************/
static void  log_GenerateLogFileName( char buff[LOG_CFG_FILE_NAME_MAX_LEN] )
{
    sprintf( buff, "%s/%s%d%s", log_zCfg.szPath, 
        log_zCfg.fileName, log_zStatus.logFileNum, ".txt");
}

/*********************************************************************************************************
 * @brief     Determine whether the need to create a log file, if need, created.
 *
 * @param   
 * @usage
 *
 *
 * 
****************************************************************************************************/
static void log_CreatNewLogFileWhenNecessary( void )
{
	if (log_zStatus.logFileSize >= log_zCfg.maxSizeLogFile)
	{
        if( log_zStatus.fp )
        {
            fclose( log_zStatus.fp );
        }
        
		log_zStatus.logFileSize = 0;
        log_zStatus.logFileNum++;
        log_GenerateLogFileName( log_zStatus.logFullName );
        
		log_zStatus.fp =  fopen(log_zStatus.logFullName, "a+");
        if( NULL == log_zStatus.fp )
        {
            fprintf(stderr, "FAIL to open log file \r\n");
            log_zStatus.bLogStarted = FALSE;
        }
	}
	return;
}

/*********************************************************************************************************
 * @brief     get the number of current log file 
 *
 * @param   
 * @usage
 *
 * @return   TRUE if no error.
 *           FALSE otherwise.
 * 
****************************************************************************************************/
static void log_GetLogFileInfo(void)
{
    FILE *fpbuf;
    log_zStatus.logFileNum = 1;
    log_GenerateLogFileName(log_zStatus.logFullName);

    while ( fpbuf = fopen(log_zStatus.logFullName, "r"))
    {
        fclose(fpbuf);
        log_zStatus.logFileNum++;
        log_GenerateLogFileName(log_zStatus.logFullName);
    }
    if (log_zStatus.logFileNum != 1)
    {
        log_zStatus.logFileNum--;
    }
}


/*********************************************************************************************************
 * @brief     Prepare log message into logBuffer
 *            - log level message.
 *            - time stamp.
 *            - start new log file if necessary.
 *
 * @param   
 * @usage
 *
 * @return   None.
 * 
****************************************************************************************************/
static void log_PrepareLog( LOG_eLevel_t eLevel )
{
    static char* eLevelStirng[ LOG_eNumLevels ] =
    {
        "[INFO]:",
        "[WARN]:",
        "[ERROR]:",
    };
    char *  pLevelString = eLevelStirng[LOG_eLevelInfo];
    struct  tm timeinfo;
    
    if( eLevel < LOG_eNumLevels )
    {
        pLevelString = eLevelStirng[ eLevel ];
    }
    
    log_GetSysTime( &timeinfo );
    log_CreatNewLogFileWhenNecessary();
    sprintf(log_zStatus.logBuffer, "%04d-%02d-%02d %02d:%02d:%02d %s ", 
        (timeinfo.tm_year+1900), timeinfo.tm_mon, timeinfo.tm_mday, 
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, pLevelString );
    
}

/**********************************************************************************************************
 *  Public Function Implementations
*********************************************************************************************************/

/*********************************************************************************************************
 * @brief   init LOG configuration
 *		    - set log file path, name, log level, max size of log file 
 *          - and get current log full name and it's size
 *
 * @param   kpkzCfg :  pointer to the LOG configuration struct input
 * @usage
 *
 * @return  TRUE if no error. 
 *          FALSE otherwise.
 *
 * 
********************************************************************************************************/
bool_t  LOG_Init( const  LOG_zCfg_t *const  kpkzCfg )
{
    memset( &log_zStatus, 0, sizeof(log_zStatus));
    
	if (( NULL != kpkzCfg->szPath ) && ( NULL != kpkzCfg->fileName ))
	{
                    memcpy( &log_zCfg,  kpkzCfg, sizeof( LOG_zCfg_t));
        
                    log_zStatus.bLogStarted = FALSE;
        
            #if LOG_CFG_USE_CRITICAL_SECTION
	       pthread_mutex_init(&log_zStatus.logMutex, NULL);
            #endif
        
        /* initialize the file size */
	       char logFullName[LOG_CFG_FILE_NAME_MAX_LEN] = {0};
	       log_GetLogFileInfo();
        
                    log_GenerateLogFileName( log_zStatus.logFullName );

	       log_zStatus.logFileSize = log_GetLogFileSizeFromOS(log_zStatus.logFullName);

                    log_zStatus.eCurrentLevel = log_zCfg.eDefaultLevel;
		
                    log_zStatus.bInit = TRUE;
        
	       return true;
	}
    
	return false;
}


/*********************************************************************************************************
 * @brief     Start logging
 *             - open log file if necessary
 *             - if log is already started. then not an error.
 *
 * @param   
 * @usage
 *
 * @return  TRUE if no error. 
 *          FALSE otherwise.
 *
 * 
********************************************************************************************************/
bool_t LOG_Start(void)
{
    bool_t  bRet = FALSE;
    
    if(( NULL != log_zCfg.fileName) && 
       ( log_zStatus.bInit ))
    {
        #if LOG_CFG_USE_CRITICAL_SECTION
        pthread_mutex_lock(&log_zStatus.logMutex);
        #endif
        {
            if(( ! log_zStatus.bLogStarted ) || 
               ( NULL == log_zStatus.fp))
            {
                log_zStatus.fp = fopen(log_zStatus.logFullName, "a+");
                if( log_zStatus.fp )
                {
                    log_zStatus.bLogStarted = TRUE;
                    bRet = TRUE;
                }
                else
                {
                    bRet = FALSE;
                }
            }
            else
            {
                //already started. nothing to do.
                bRet = TRUE;
            }
        }
        #if LOG_CFG_USE_CRITICAL_SECTION
        pthread_mutex_unlock(&log_zStatus.logMutex);
        #endif
    }
    
	return bRet;
}


/*********************************************************************************************************
 * @brief   Stop logging.
 *          - flush/close logging file.
 *          - update status
 *
 * @param   
 * @usage
 *
 * @return  TRUE if no error. 
 *          FALSE otherwise.
 *
 * 
********************************************************************************************************/
bool_t LOG_Stop(void)
{
    bool_t  bRet = FALSE;
    
    #if LOG_CFG_USE_CRITICAL_SECTION
	pthread_mutex_lock(&log_zStatus.logMutex);
    #endif
    {
        if( log_zStatus.bLogStarted )
        {
            if( log_zStatus.fp )
            {
                fflush(log_zStatus.fp);
                fclose(log_zStatus.fp);
                log_zStatus.fp = NULL;
            }
            
            log_zStatus.bLogStarted = FALSE; 
        }
        
        bRet = TRUE;
    }
    #if LOG_CFG_USE_CRITICAL_SECTION
    pthread_mutex_unlock(&log_zStatus.logMutex);
    #endif
    
	return bRet;
}


/*********************************************************************************************************
 * @brief      set the log level to the specified log level.
 *
 * @param   eLevel :  the log level being set 
 * @usage
 *
 * @return  TRUE if no error. 
 *          FALSE otherwise.
 *
 * 
********************************************************************************************************/
bool_t LOG_SetLogLevel(LOG_eLevel_t eLevel)
{
    bool_t      bRet = FALSE;
    
    LOG_eLevel_t    eOldLevel;
    
    eOldLevel = log_zStatus.eCurrentLevel;
    
    char   logBuffer[LOG_CFG_LOG_BUFFER_LEN];
    
    #if LOG_CFG_USE_CRITICAL_SECTION
	pthread_mutex_lock(&log_zStatus.logMutex);
    #endif
    
    if( eLevel < LOG_eNumLevels )
    {
        log_zStatus.eCurrentLevel = eLevel;
        
        #if LOG_CFG_USE_CRITICAL_SECTION
        pthread_mutex_unlock(&log_zStatus.logMutex);
        #endif
        if(( log_zStatus.bLogStarted ) && 
           ( eOldLevel != eLevel ))
        {
            sprintf( logBuffer,  "LogLevel changed from %s to %s ", 
                log_akpcLogLevelString[eOldLevel] , log_akpcLogLevelString[eLevel] );
            
            LOG_Log( LOG_eLevelWarning, logBuffer, strlen( logBuffer ) );
        }
        
        bRet = TRUE;
    }
    
	return bRet;
}


/*********************************************************************************************************
 * @brief   get current log level
 *
 * @param   
 * @usage
 *
 * @return  the current log level if it is known.
 *          LOG_eNumLevels to signal the level is not set.
 *
 * 
********************************************************************************************************/
LOG_eLevel_t LOG_GetLogLevel(void)
{
    LOG_eLevel_t  eLevel = LOG_eNumLevels; //to signal invalid level
    
    if( log_zStatus.bInit )
    {
        eLevel = log_zStatus.eCurrentLevel;
    }
    
	return eLevel;
}


/*********************************************************************************************************
 * @brief     write log message into log file
 		- add time stamp for each logged message
 		- messages are dropped silently if the message log level is lower than the current log level
 		- messages are dropped if log is not started 
 *
 * @param   eLevel :  the level of log message want to write into log file
 * @param   buff :  the log message want to write into log file
 * @param   len :  the size of log message want to write into log file
 * @usage
 *
 * 
********************************************************************************************************/
void LOG_Log( LOG_eLevel_t eLevel, char* buff, uint32_t len )
{    
    #if LOG_CFG_USE_CRITICAL_SECTION
    pthread_mutex_lock(&log_zStatus.logMutex);
    #endif
    {
        if ( (eLevel >= log_zStatus.eCurrentLevel ) && 
             ( TRUE == log_zStatus.bLogStarted)  &&
             ( NULL != log_zStatus.fp) )
        {
            log_PrepareLog( eLevel);
            log_CreatNewLogFileWhenNecessary();
            {
                uint16_t oldFpSeek = ftell(log_zStatus.fp);

                fputs(log_zStatus.logBuffer, log_zStatus.fp);
                fwrite( buff, 1, len, log_zStatus.fp);
                fputs("\n", log_zStatus.fp);
                
                fflush(log_zStatus.fp);

                uint16_t newFpSeek = ftell(log_zStatus.fp);
                log_zStatus.logFileSize += (newFpSeek - oldFpSeek);
            }
        }
        else if ( ( TRUE == log_zStatus.bLogStarted)  &&
                    ( NULL == log_zStatus.fp) )
                {
                    fprintf(stderr, "Not open log file but is started\n");
                }
    }
    #if LOG_CFG_USE_CRITICAL_SECTION
    pthread_mutex_unlock(&log_zStatus.logMutex);
    #endif 
}


/*********************************************************************************************************
 * @brief     write log message into log file with variable number of args.
        - add time stamp for each logged message
        - messages are dropped silently if the message log level is lower than the current log level
        - messages are dropped if log is not started 
 *
 * @param   eLevel :  the level of log message want to write into log file
 * @param   format :  the print format.
 * @usage
 *
 * 
********************************************************************************************************/
void LOG_LogPrint( LOG_eLevel_t eLevel, const char *format, ... )
{
    va_list args;   
    va_start(args, format);
    
    #if LOG_CFG_USE_CRITICAL_SECTION
    pthread_mutex_lock(&log_zStatus.logMutex);
    #endif
    {
        if ( (eLevel >= log_zStatus.eCurrentLevel ) && 
             ( TRUE == log_zStatus.bLogStarted)  &&
             ( NULL != log_zStatus.fp) )
        {
            log_PrepareLog( eLevel);
            log_CreatNewLogFileWhenNecessary();
            {
                uint16_t oldFpSeek = ftell(log_zStatus.fp);
                
                fputs(log_zStatus.logBuffer, log_zStatus.fp);
                vfprintf( log_zStatus.fp, format, args );
                va_end(args);
                fputs("\n", log_zStatus.fp);     
                fflush(log_zStatus.fp);

                uint16_t newFpSeek = ftell(log_zStatus.fp);
                log_zStatus.logFileSize += (newFpSeek - oldFpSeek);
            }
        }
        else if ( ( TRUE == log_zStatus.bLogStarted)  &&
                    ( NULL == log_zStatus.fp) )
                {
                    fprintf(stderr, "Not open log file but is started\n");
                }
    }
    #if LOG_CFG_USE_CRITICAL_SECTION
    pthread_mutex_unlock(&log_zStatus.logMutex);
    #endif 
}