/*
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2026, Renesas Electronics Corporation and/or its affiliates


    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of
       conditions and the following disclaimer in the documentation and/or other
       materials provided with the distribution.

    3. Neither the name of Renesas nor the names of its
       contributors may be used to endorse or promote products derived from this
       software without specific prior written permission.



    THIS SOFTWARE IS PROVIDED BY Renesas "AS IS" AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL RENESAS OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    ---------------------------------------------------------------

   Project     : PTX2xxR/W
   Module      : Operating System Abstraction Layer (OSAL)
   File        : ptx_OSAL.h

   Description : Contains the main interface of the OSAL.
                 Operating system specific implementations for these methods are available
                 and automatically selected based on the built platform.
*/

#if !defined(SRC_COMPS_OSAL_PTX_OSAL_H_)
#define SRC_COMPS_OSAL_PTX_OSAL_H_


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Status.h"
#include "ptx_Components.h"
#include "ptx_OSAL_Int.h"
#include <stdint.h>


#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * \addtogroup OSAL
 *
 * @{
 */

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/**
 * \brief The mode of the timer
 */
#define PTX_TIMER_SINGLE_SHOT       0           /**< Single shot timer */
#define PTX_TIMER_PERIODIC          1U          /**< Continuously running, periodic timer */


#define PTX_OSAL_FILE_MAX_DIR_PATH_LEN  256U    /**< Maximum size of a directory path. */
#define PTX_OSAL_FILE_MAX_FILE_NAME_LEN 64U     /**< Maximum size of a file name. */
#define PTX_OSAL_FILE_MAX_PATH_LEN      (PTX_OSAL_FILE_MAX_DIR_PATH_LEN + PTX_OSAL_FILE_MAX_FILE_NAME_LEN)     /**< Maximum size of a complete path to a file. */

/**
 * \brief Time difference type.
 */
typedef uint64_t ptx_TimeDiff_t;

/**
 *  \brief Thread procedure function pointer declaration.
 *
 *  \param      threadParam Pointer to thread-specific ("context") storage.
 */
typedef void (*pptx_OSAL_ThreadProc_t)(void *threadParam);

/**
 *  \brief Timer procedure function pointer declaration.
 *
 *  \param      timerParam Pointer to timer-specific ("context") storage.
 */
typedef void (*pptx_OSAL_TimerProc_t)(void *timerParam);

/**
 * \brief Thread state used for internal purpose when querying the thread properties.
 *
 * To minimize (although not rule out completely) the risk of invalid access when
 * querying a thread that has not yet been initialized or that has just quit
 * the states are encoded as double Hamming codewords with d = 3.
\verbatim
 * 00000000    00
 * 00000111    07
 * 00011001    19
 * 00011110    1E
 * 00101010    2A
 * 00101101    2D
 * 00110011    33
 * 00110100    34
 * 01001011    4B
 * 01001100    4C
 * 01010010    52
 * 01010101    55
 * 01100001    61
 * 01100110    66
 * 01111000    78
 * 01111111    7F
\endverbatim
 */
typedef enum ptx_Thread_State
{
    ptx_Thread_State_INITIAL     = 0x7F00,       /**< Pre-state or finished. */
    ptx_Thread_State_CREATING    = 0x7807,       /**< Creating, not yet running. */
    ptx_Thread_State_RUN         = 0x6619,       /**< Running thread procedure. */
    ptx_Thread_State_JOIN        = 0x611E,       /**< Running, exited thread procedure, may be in wait state if joining. */
    ptx_Thread_State_ERROR       = 0x552A        /**< Error state, creation failed, not running. */
} ptx_Thread_State_t;

/**
 * \brief OSAL main structure.
 */
typedef struct ptx_OSAL
{
    ptx_Comps_t         CompId;                 /**< Component Id */
    void                *SysCtx;                /**< Arbitrary system context, i.e. pointer to system specific storage. */

    int32_t             SemUsage;               /**< Number of semaphores allocated */
    int32_t             MutexUsage;             /**< How many mutexes are currently in use. */
    int32_t             ThreadUsage;            /**< Present thread count. */
    int32_t             FileCount;              /**< Number of files currently open. */
    int32_t             TimerCount;             /**< Number of timers currently in use. */

    int32_t             PeakSemUsage;           /**< Peak number of semaphores allocated */
    int32_t             PeakMutexUsage;         /**< How many mutexes were in use at maximum. */
    int32_t             PeakThreadUsage;        /**< Maximum thread count. */
    int32_t             PeakTimerCount;         /**< Maximum used timer count. */

    ptx_Mutex_t         Mutex;                  /**< Mutex to protect data written from different threads . */
} ptx_OSAL_t;


/**
 * \brief Thread structure.
 */
typedef struct ptx_Thread
{
    pptx_OSAL_ThreadProc_t  ThdFn;              /**< Pointer to thread procedure function. */
    void                   *ThdParam;           /**< Pointer to a parameter passed to \ref ptx_OSAL_ThreadCreate. */
    void                   *Thread;             /**< Platform specific Thread handle, assigned in \ref ptx_OSAL_ThreadCreate. */
    void                   *ThdName;            /**< Thread name. Reserved for future use. */
    ptx_OSAL_t             *OS;                 /**< Pointer to an initialized OSAL component structure. */
    void                   *ThdEventGroup;      /**< Reserved for future use. */
    ptx_Thread_State_t      ThdState;           /**< Track state of thread, see \ref ptx_Thread_State_t */
    uint32_t                ThdPriority;        /**< Reserved for future use. */
    uint32_t                ThdEventBit;        /**< Reserved for future use. */
    uint16_t                ThdStackDepth;      /**< Reserved for future use. */
} ptx_Thread_t;

/**
 * \brief Timer structure.
 */
typedef struct ptx_Timer
{
    ptx_Thread_t             Thd;                /**< Thread structure. */
    pptx_OSAL_TimerProc_t    Proc;               /**< Pointer to timer procedure function. Called within timer thread function. */
    ptx_Semaphore_t          StartSem;           /**< Timer start semaphore, used to sync timer start with timer thread. */
    ptx_Semaphore_t          WaitCancelSem;      /**< Timer wait semaphore, used to sync timer wait-functionality with timer thread. */
    uint32_t                 Interval;           /**< Interval in ms, defines maximum time to wait for WaitCancelSem to be signaled. */
    uint8_t                  Mode;               /**< Timer mode: single shot or periodic (continuously running). */
    ptx_OSAL_t              *OS;                 /**< Pointer to an initialized OSAL component structure. */
    void                    *Param;              /**< Pointer to a parameter passed to \ref ptx_OSAL_TimerCreate. */
    volatile uint8_t         Running;            /**< Timer has been successfully started with \ref ptx_OSAL_TimerStart (1) or not (0). */
    volatile uint8_t         Run;                /**< Timer has been successfully created with \ref ptx_OSAL_TimerCreate (1), or closed (0) (e.g. with \ref ptx_OSAL_TimerClose). */
    uint8_t                  Valid;              /**< Member variable signals if timer is created and timer thread has been successfully started (1), or not (0). */
} ptx_Timer_t;

/**
 * \brief File access mode values used for file handling functions.
 */
typedef enum ptx_File_Mode
{
    ptx_File_Mode_Read,                     /**< Open an existing file for input operations. */
    ptx_File_Mode_Write,                    /**< Create an empty file for output operations. Overwrite (discard) an existing file. */
    ptx_File_Mode_Append,                   /**< Open an existing file to append output to its end or create an empty file if no such file exists yet. */
    ptx_File_Mode_ReadUpdate,               /**< Open an existing file for input and output operations.*/
    ptx_File_Mode_WriteUpdate,              /**< Like \ref ptx_File_Mode_Write, but the file is open for output and input. */
    ptx_File_Mode_AppendUpdate,             /**< Like \ref ptx_File_Mode_Append, but the file is open for output and input.*/
    ptx_File_Mode_BinaryRead,               /**< Open an existing file for input operations in binary mode. */
    ptx_File_Mode_BinaryWrite,              /**< Create an empty file for output operations in binary mode. Overwrite (discard) an existing file. */
    ptx_File_Mode_BinaryAppend,             /**< Open an existing file to append output to its end or create an empty file if no such file exists yet in binary mode. */
    ptx_File_Mode_BinaryReadUpdate,         /**< Open an existing file for input and output operations in binary mode.*/
    ptx_File_Mode_BinaryWriteUpdate,        /**< Like \ref ptx_File_Mode_BinaryWrite, but the file is open for output and input. */
    ptx_File_Mode_BinaryAppendUpdate,       /**< Like \ref ptx_File_Mode_BinaryAppend, but the file is open for output and input.*/
    ptx_File_Mode_MAX,                      /**< RFU. */
} ptx_File_Mode_t;

/**
 * \brief File access configuration.
 */
typedef struct ptx_File_Config
{
    const char               *Path;     /**< Path to the directory of the file. */
    const char               *Name;     /**< Name of file. */
    ptx_File_Mode_t           Mode;     /**< Operation mode for the file. */
} ptx_File_Config_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS / ADMINISTRATIVE
 * ####################################################################################################################
 */

/**
 * \brief Initialize the OSAL.
 *
 * \brief Function that sets up the Component.
 *
 * \param[in,out]  osalCtx    Pointer to an allocated and non-initialized OSAL component structure.
 * \param[in]      sysCtx     Arbitrary platform context, as required. May be NULL on some platforms. It could be NULL.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_Init(ptx_OSAL_t *osalCtx, void *sysCtx);

/**
 * \brief Un-Initialize the OSAL.
 *
 * \brief Function that cleans up the Component: de-inits internal members.
 *
 * \param[in,out]  osalCtx    Pointer to an initialized OSAL component structure.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_DeInit(ptx_OSAL_t *osalCtx);

/*
 * ####################################################################################################################
 * API FUNCTIONS / SEMAPHORES
 * ####################################################################################################################
 */

/**
 * \brief Initialize an already allocated but non-initialized object.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  hSem            Pointer to an allocated but non-initialized semaphore object.
 * \param[in]   initialCount    Initial value.
 * \param[in]   maxCount        Maximum count.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_SemInit(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem, uint32_t initialCount, uint32_t maxCount);

/**
 * \brief Close an initialized semaphore object.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]   hSem            Pointer to an initialized semaphore object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_SemClose(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem);

/**
 * \brief Signal a semaphore, unblock a waiting thread.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]   hSem            Pointer to an initialized semaphore object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_SemPost(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem);

/**
 * \brief Wait on the semaphore.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]   hSem            Pointer to an initialized semaphore object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_SemWait(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem);

/**
 * \brief Wait on the semaphore, with timeout.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]   hSem            Pointer to an initialized semaphore object.
 * \param[in]   msTimeout       Timeout period, in ms.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_SemTimedWait(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem, uint32_t msTimeout);


/*
 * ####################################################################################################################
 * API FUNCTIONS / MUTEXES
 * ####################################################################################################################
 */

/**
 * \brief Initialize a Mutex.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  hMutex          Pointer to a location that will be initialized/receive the Mutex object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_MutexInit(ptx_OSAL_t *osalCtx, ptx_Mutex_t *hMutex);

/**
 * \brief De-Initialize a Mutex.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  hMutex          Pointer to the Mutex object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_MutexDeInit(ptx_OSAL_t *osalCtx, ptx_Mutex_t *hMutex);

/**
 * \brief Lock/acquire a Mutex.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]   hMutex          Pointer to the initialized Mutex object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_MutexLock(ptx_OSAL_t *osalCtx, ptx_Mutex_t *hMutex);

/**
 * \brief Unlock/release a Mutex.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]   hMutex          Pointer to the initialized Mutex object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_MutexUnlock(ptx_OSAL_t *osalCtx, ptx_Mutex_t *hMutex);


/*
 * ####################################################################################################################
 * API FUNCTIONS / THREADS
 * ####################################################################################################################
 */

/**
 * \brief Create a thread.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  hThd            Pointer to a specific thread descriptor/structure/handle with member
 *                              ThdState set to ptx_Thread_State_INITIAL.
 * \param[in]   thdFn           Pointer to the thread procedure.
 * \param[in]   thdParam        Parameter that the thread function will receive when it is started. May be NULL
 *                              if there is no data to share with or to hand over to the thread respectively.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_ThreadCreate(ptx_OSAL_t *osalCtx, ptx_Thread_t *hThd, pptx_OSAL_ThreadProc_t thdFn, void *thdParam);

/**
 * \brief Join on a thread, wait until it has exited.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  hThd            Pointer to the thread, created by \ref ptx_OSAL_ThreadCreate.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_ThreadJoin(ptx_OSAL_t *osalCtx, ptx_Thread_t *hThd);

/**
 * \brief Close a thread, once it has exited.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  hThd            Pointer to the thread, created by \ref ptx_OSAL_ThreadCreate.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_ThreadClose(ptx_OSAL_t *osalCtx, ptx_Thread_t *hThd);

/**
 * \brief Sleep for a number of ms.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]   msSleep         Number of ms to suspend the current thread. If 0, a Yield operation is performed.
 *
 */
void ptx_OSAL_ThreadSleep(ptx_OSAL_t *osalCtx, uint32_t msSleep);

/*
 * ####################################################################################################################
 * API FUNCTIONS / STOPWATCH
 * ####################################################################################################################
 */

/**
 * \brief Start a time measurement or get a time value.
 *
 * \param[in]       osalCtx     Pointer to an initialized OSAL component structure.
 * \param[in,out]   startVal    Pointer to a variable that will receive the start value.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_StopwatchStart(ptx_OSAL_t *osalCtx, ptx_TimeDiff_t *startVal);

/**
 * \brief End a time measurement.
 *
 * \param[in]       osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in,out]   startStopVal    Pointer to a variable that provides the start time and that will receive the time span.
 *                                  The time unit is microseconds.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_StopwatchStop(ptx_OSAL_t *osalCtx, ptx_TimeDiff_t *startStopVal);


/*
 * ####################################################################################################################
 * API FUNCTIONS / TIMERS
 * ####################################################################################################################
 */

/**
 * \brief Create a timer.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  hTimer          Pointer to a platform-specific variable that will receive the timer descriptor/structure/handle.
 * \param[in]   timerProc       Pointer to the timer procedure.
 * \param[in]   timerParam      Parameter that the timer procedure will receive when it is started. May be NULL
 *                              if there is no data to share with or to hand over to the thread respectively.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_TimerCreate(ptx_OSAL_t *osalCtx, ptx_Timer_t *hTimer, pptx_OSAL_TimerProc_t timerProc, void *timerParam);

/**
 * \brief Start or restart the timer.
 *
 * The timer is started or restarted in one-shot mode.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  hTimer          Pointer to the timer, created by \ref ptx_OSAL_TimerCreate.
 * \param[out]  msInterval      Interval, in ms.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_TimerStart(ptx_OSAL_t *osalCtx, ptx_Timer_t *hTimer, uint32_t msInterval);

/**
 * \brief Cancel a timer, if active.
 *
 * \note If timer is not running, no operation is performed and status returned is Successful.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]   hTimer          Pointer to the timer, created by \ref ptx_OSAL_TimerCreate.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_TimerCancel(ptx_OSAL_t *osalCtx, ptx_Timer_t *hTimer);

/**
 * \brief Close a timer, once it has expired.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  hTimer          Pointer to the timer, created by \ref ptx_OSAL_TimerCreate.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_TimerClose(ptx_OSAL_t *osalCtx, ptx_Timer_t *hTimer);

/*
 * ####################################################################################################################
 * API FUNCTIONS / FILE HANDLING
 * ####################################################################################################################
 */

/**
 * \brief Read a file and load the data into provided buffer.
 *
 * \param[in]        osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]        config          File access configuration.
 * \param[out]       buffer          Buffer containing data read out from the file.
 * \param[in, out]   bufferLen       Buffer size, i.e. maximum number of bytes to be written into the buffer as input ands
 *                                   the actual number of bytes read as output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_FileRead (ptx_OSAL_t *osalCtx, ptx_File_Config_t config, uint8_t *buffer, uint32_t *bufferLen);

/**
 * \brief Write a file with the data from the provided buffer.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]   config          File access configuration.
 * \param[in]   buffer          Buffer containing data to be written to the file.
 * \param[in]   bufferLen       The length of the data to be written.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_OSAL_FileWrite (ptx_OSAL_t *osalCtx, ptx_File_Config_t config, uint8_t *buffer, uint32_t bufferLen);

/*
 * ####################################################################################################################
 * API FUNCTIONS / SYSTEM CONTEXT
 * ####################################################################################################################
 */

/**
 * \brief Get the System Context.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 *
 * \return System Context, a pointer to arbitrary context.
 *
 */
void *ptx_OSAL_GetSysCtx(ptx_OSAL_t *osalCtx);

#if defined(__cplusplus)
}
#endif

/** @} */

#endif /* Guard */

