#ifndef _LOG_H_
#define _LOG_H_

#include "stdint.h"
/**********************************************************************************************************
 *  Standard Includes
 *********************************************************************************************************/


#define  LOG_CFG_FILE_NAME                  ("log")
#define  LOG_CFG_LOG_PATH                   ("./logs")
#define  LOG_CFG_USE_CRITICAL_SECTION       (0)
#define  LOG_CFG_FILE_MAX_SIZE_KB           (1024*1024u)
#define  LOG_CFG_FILE_NAME_MAX_LEN          (256u)
#define  LOG_CFG_LOG_BUFFER_LEN             (512u)

/**********************************************************************************************************
 *  E. Type defines
**********************************************************************************************************/

/**********************************************************************************************************
 *  Standard Includes
 *********************************************************************************************************/


#define  LOG_CFG_FILE_NAME                  ("log")
#define  LOG_CFG_LOG_PATH                   ("./logs")
#define  LOG_CFG_USE_CRITICAL_SECTION       (0)
#define  LOG_CFG_FILE_MAX_SIZE_KB           (1024*1024u)
#define  LOG_CFG_FILE_NAME_MAX_LEN          (256u)
#define  LOG_CFG_LOG_BUFFER_LEN             (512u)

/**********************************************************************************************************
 *  E. Type defines
**********************************************************************************************************/
#define  LOG_CFG_FILE_NAME_MAX_LEN          (256u)
#define  LOG_CFG_LOG_BUFFER_LEN             (512u)

/**********************************************************************************************************
 *  E. Type defines
**********************************************************************************************************/

typedef enum LOG_eLevel_t
{
    LOG_eLevelInfo = 0u,
    LOG_eLevelWarning,
    LOG_eLevelError,
    
    LOG_eNumLevels,
}   LOG_eLevel_t;

typedef struct LOG_zCfg_t
{
    char*           szPath;         // log file path.
    char*           fileName;       // log file name.
    long            maxSizeLogFile; //max log file size
    LOG_eLevel_t    eDefaultLevel;  // default log level

    /* add in more configuration items if necessary */
}   LOG_zCfg_t;

typedef struct  LOG_zStatus_t
{
    bool_t          bInit;
    bool_t          bLogStarted;
    FILE*           fp;
    char             logBuffer[LOG_CFG_LOG_BUFFER_LEN];
    char             logFullName[LOG_CFG_FILE_NAME_MAX_LEN];
    uint32_t       logFileSize;
    uint16_t       logFileNum;
    LOG_eLevel_t    eCurrentLevel;
    
    #if LOG_CFG_USE_CRITICAL_SECTION
    pthread_mutex_t logMutex;
    #endif
} LOG_zStatus_t;


extern LOG_zCfg_t LOG_kzCfg;

/**********************************************************************************************************
 *  Public Function 
*********************************************************************************************************/
bool_t          LOG_Init( const LOG_zCfg_t *const  kpkzCfg );

bool_t          LOG_Start( void );
bool_t          LOG_Stop( void );

bool_t          LOG_SetLogLevel( LOG_eLevel_t  eLevel );
LOG_eLevel_t    LOG_GetLogLevel( void );

/* requirement: 
    - add time stamp for each logged message.
    - log to log file.
    - multi-thread safe, i.e. messages are not fragmented into pieces by other message
    - messages are dropped silently if the message log level is lower than the current log level 
    - messages are dropped if log is not started.
  */
void            LOG_Log( LOG_eLevel_t eLevel, char* buff, uint32_t len );
void            LOG_LogPrint( LOG_eLevel_t eLevel, const char *format, ... );


#endif //_LOG_H_