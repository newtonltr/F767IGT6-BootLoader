/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ThreadX Component                                                     */
/**                                                                       */
/**   Port Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    tx_port.h                                         Cortex-M7/AC5     */
/*                                                           6.0.1        */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains data type definitions that make the ThreadX      */
/*    real-time kernel function identically on a variety of different     */
/*    processor architectures.  For example, the size or number of bits   */
/*    in an "int" data type vary between microprocessor architectures and */
/*    even C compilers for the same microprocessor.  ThreadX does not     */
/*    directly use native C data types.  Instead, ThreadX creates its     */
/*    own special types that can be mapped to actual data types by this   */
/*    file to guarantee consistency in the interface and functionality.   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-30-2020     William E. Lamie         Initial Version 6.0.1         */
/*                                                                        */
/**************************************************************************/

#ifndef TX_PORT_H
#define TX_PORT_H


/* Determine if the optional ThreadX user define file should be used.  */

#ifdef TX_INCLUDE_USER_DEFINE_FILE

/* Yes, include the user defines in tx_user.h. The defines in this file may 
   alternately be defined on the command line.  */

#include "tx_user.h"
#endif


/* Define compiler library include files.  */

#include <stdlib.h>
#include <string.h>
#include "stm32f7xx_hal.h"


/* Define ThreadX basic types for this port.  */

#define VOID                                    void
typedef char                                    CHAR;
typedef unsigned char                           UCHAR;
typedef int                                     INT;
typedef unsigned int                            UINT;
typedef long                                    LONG;
typedef unsigned long                           ULONG;
typedef short                                   SHORT;
typedef unsigned short                          USHORT;


/* Define the priority levels for ThreadX.  Legal values range
   from 32 to 1024 and MUST be evenly divisible by 32.  */

#ifndef TX_MAX_PRIORITIES
#define TX_MAX_PRIORITIES                       32
#endif


/* Define the minimum stack for a ThreadX thread on this processor. If the size supplied during
   thread creation is less than this value, the thread create call will return an error.  */

#ifndef TX_MINIMUM_STACK
#define TX_MINIMUM_STACK                        200         /* Minimum stack size for this port  */
#endif


/* Define the system timer thread's default stack size and priority.  These are only applicable
   if TX_TIMER_PROCESS_IN_ISR is not defined.  */

#ifndef TX_TIMER_THREAD_STACK_SIZE
#define TX_TIMER_THREAD_STACK_SIZE              1024        /* Default timer thread stack size  */
#endif

#ifndef TX_TIMER_THREAD_PRIORITY    
#define TX_TIMER_THREAD_PRIORITY                0           /* Default timer thread priority    */
#endif


/* Define various constants for the ThreadX Cortex-M3 port.  */

#define TX_INT_DISABLE                          1           /* Disable interrupts               */
#define TX_INT_ENABLE                           0           /* Enable interrupts                */


/* Define the clock source for trace event entry time stamp. The following two item are port specific.  
   For example, if the time source is at the address 0x0a800024 and is 16-bits in size, the clock 
   source constants would be:

#define TX_TRACE_TIME_SOURCE                    *((ULONG *) 0x0a800024)
#define TX_TRACE_TIME_MASK                      0x0000FFFFUL

*/

#ifndef TX_MISRA_ENABLE
#ifndef TX_TRACE_TIME_SOURCE
#define TX_TRACE_TIME_SOURCE                    *((ULONG *) 0xE0001004)  
#endif
#else
ULONG   _tx_misra_time_stamp_get(VOID);
#define TX_TRACE_TIME_SOURCE                    _tx_misra_time_stamp_get()
#endif

#ifndef TX_TRACE_TIME_MASK
#define TX_TRACE_TIME_MASK                      0xFFFFFFFFUL
#endif


/* Define the port specific options for the _tx_build_options variable. This variable indicates
   how the ThreadX library was built.  */

#define TX_PORT_SPECIFIC_BUILD_OPTIONS          (0)


/* Define the in-line initialization constant so that modules with in-line
   initialization capabilities can prevent their initialization from being
   a function call.  */

#ifdef TX_MISRA_ENABLE
#define TX_DISABLE_INLINE
#else
#define TX_INLINE_INITIALIZATION
#endif


/* Determine whether or not stack checking is enabled. By default, ThreadX stack checking is 
   disabled. When the following is defined, ThreadX thread stack checking is enabled.  If stack
   checking is enabled (TX_ENABLE_STACK_CHECKING is defined), the TX_DISABLE_STACK_FILLING
   define is negated, thereby forcing the stack fill which is necessary for the stack checking
   logic.  */

#ifndef TX_MISRA_ENABLE
#ifdef TX_ENABLE_STACK_CHECKING
#undef TX_DISABLE_STACK_FILLING
#endif
#endif


/* Define the TX_THREAD control block extensions for this port. The main reason
   for the multiple macros is so that backward compatibility can be maintained with 
   existing ThreadX kernel awareness modules.  */

#define TX_THREAD_EXTENSION_0          
#define TX_THREAD_EXTENSION_1                  
#define TX_THREAD_EXTENSION_2                           

#define TX_THREAD_EXTENSION_3 int bsd_errno;

/* Define the port extensions of the remaining ThreadX objects.  */

#define TX_BLOCK_POOL_EXTENSION
#define TX_BYTE_POOL_EXTENSION
#define TX_EVENT_FLAGS_GROUP_EXTENSION
#define TX_MUTEX_EXTENSION
#define TX_QUEUE_EXTENSION
#define TX_SEMAPHORE_EXTENSION
#define TX_TIMER_EXTENSION


/* Define the user extension field of the thread control block.  Nothing 
   additional is needed for this port so it is defined as white space.  */

#ifndef TX_THREAD_USER_EXTENSION
#define TX_THREAD_USER_EXTENSION    
#endif


/* Define the macros for processing extensions in tx_thread_create, tx_thread_delete,
   tx_thread_shell_entry, and tx_thread_terminate.  */


#define TX_THREAD_CREATE_EXTENSION(thread_ptr)                                  
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)                                  


#ifndef TX_MISRA_ENABLE

register unsigned int _ipsr __asm("ipsr");

#endif


#ifdef __TARGET_FPU_VFP

#ifdef TX_MISRA_ENABLE

ULONG  _tx_misra_control_get(void);
void   _tx_misra_control_set(ULONG value);
ULONG  _tx_misra_fpccr_get(void);
void   _tx_misra_vfp_touch(void);

#else

#ifdef TX_SOURCE_CODE

register ULONG  _control __asm("control");

#endif
#endif

/* A completed thread falls into _thread_shell_entry and we can simply deactivate the FPU via CONTROL.FPCA
   in order to ensure no lazy stacking will occur. */

#ifndef TX_MISRA_ENABLE

#define TX_THREAD_COMPLETED_EXTENSION(thread_ptr)                   {                                                                                             \
                                                                    ULONG  _tx_vfp_state;                                                                         \
                                                                        _tx_vfp_state =  _control;                                                                \
                                                                        _tx_vfp_state =  _tx_vfp_state & ~((ULONG) 0x4);                                          \
                                                                        _control =       _tx_vfp_state;                                                           \
                                                                    }
#else

#define TX_THREAD_COMPLETED_EXTENSION(thread_ptr)                   {                                                                                             \
                                                                    ULONG  _tx_vfp_state;                                                                         \
                                                                        _tx_vfp_state =  _tx_misra_control_get();                                                 \
                                                                        _tx_vfp_state =  _tx_vfp_state & ~((ULONG) 0x4);                                          \
                                                                        _tx_misra_control_set(_tx_vfp_state);                                                     \
                                                                    }
                                                                    
#endif

/* A thread can be terminated by another thread, so we first check if it's self-terminating and not in an ISR.
   If so, deactivate the FPU via CONTROL.FPCA. Otherwise we are in an interrupt or another thread is terminating
   this one, so if the FPCCR.LSPACT bit is set, we need to save the CONTROL.FPCA state, touch the FPU to flush 
   the lazy FPU save, then restore the CONTROL.FPCA state. */

#ifndef TX_MISRA_ENABLE

void _tx_vfp_access(void);

#define TX_THREAD_TERMINATED_EXTENSION(thread_ptr)                  {                                                                                             \
                                                                    ULONG  _tx_system_state;                                                                      \
                                                                        _tx_system_state =  TX_THREAD_GET_SYSTEM_STATE();                                         \
                                                                        if ((_tx_system_state == ((ULONG) 0)) && ((thread_ptr) == _tx_thread_current_ptr))        \
                                                                        {                                                                                         \
                                                                        ULONG  _tx_vfp_state;                                                                     \
                                                                            _tx_vfp_state =  _control;                                                            \
                                                                            _tx_vfp_state =  _tx_vfp_state & ~((ULONG) 0x4);                                      \
                                                                            _control =       _tx_vfp_state;                                                       \
                                                                        }                                                                                         \
                                                                        else                                                                                      \
                                                                        {                                                                                         \
                                                                        ULONG  _tx_fpccr;                                                                         \
                                                                            _tx_fpccr =  *((ULONG *) 0xE000EF34);                                                 \
                                                                            _tx_fpccr =  _tx_fpccr & ((ULONG) 0x01);                                              \
                                                                            if (_tx_fpccr == ((ULONG) 0x01))                                                      \
                                                                            {                                                                                     \
                                                                            ULONG _tx_vfp_state;                                                                  \
                                                                                _tx_vfp_state =  _control;                                                        \
                                                                                _tx_vfp_state =  _tx_vfp_state & ((ULONG) 0x4);                                   \
                                                                                _tx_vfp_access();                                                                 \
                                                                                if (_tx_vfp_state == ((ULONG) 0))                                                 \
                                                                                {                                                                                 \
                                                                                    _tx_vfp_state =  _control;                                                    \
                                                                                    _tx_vfp_state =  _tx_vfp_state & ~((ULONG) 0x4);                              \
                                                                                    _control =       _tx_vfp_state;                                               \
                                                                                }                                                                                 \
                                                                            }                                                                                     \
                                                                        }                                                                                         \
                                                                    }
#else

#define TX_THREAD_TERMINATED_EXTENSION(thread_ptr)                  {                                                                                             \
                                                                    ULONG  _tx_system_state;                                                                      \
                                                                        _tx_system_state =  TX_THREAD_GET_SYSTEM_STATE();                                         \
                                                                        if ((_tx_system_state == ((ULONG) 0)) && ((thread_ptr) == _tx_thread_current_ptr))        \
                                                                        {                                                                                         \
                                                                        ULONG  _tx_vfp_state;                                                                     \
                                                                            _tx_vfp_state =  _tx_misra_control_get();                                             \
                                                                            _tx_vfp_state =  _tx_vfp_state & ~((ULONG) 0x4);                                      \
                                                                            _tx_misra_control_set(_tx_vfp_state);                                                 \
                                                                        }                                                                                         \
                                                                        else                                                                                      \
                                                                        {                                                                                         \
                                                                        ULONG  _tx_fpccr;                                                                         \
                                                                            _tx_fpccr =  _tx_misra_fpccr_get();                                                   \
                                                                            _tx_fpccr =  _tx_fpccr & ((ULONG) 0x01);                                              \
                                                                            if (_tx_fpccr == ((ULONG) 0x01))                                                      \
                                                                            {                                                                                     \
                                                                            ULONG _tx_vfp_state;                                                                  \
                                                                                _tx_vfp_state = _tx_misra_control_get();                                          \
                                                                                _tx_vfp_state =  _tx_vfp_state & ((ULONG) 0x4);                                   \
                                                                                _tx_misra_vfp_touch();                                                            \
                                                                                if (_tx_vfp_state == ((ULONG) 0))                                                 \
                                                                                {                                                                                 \
                                                                                    _tx_vfp_state =  _tx_misra_control_get();                                     \
                                                                                    _tx_vfp_state =  _tx_vfp_state & ~((ULONG) 0x4);                              \
                                                                                    _tx_misra_control_set(_tx_vfp_state);                                         \
                                                                                }                                                                                 \
                                                                            }                                                                                     \
                                                                        }                                                                                         \
                                                                    }
#endif

#else

#define TX_THREAD_COMPLETED_EXTENSION(thread_ptr)
#define TX_THREAD_TERMINATED_EXTENSION(thread_ptr)                  

#endif


/* Define the ThreadX object creation extensions for the remaining objects.  */

#define TX_BLOCK_POOL_CREATE_EXTENSION(pool_ptr)
#define TX_BYTE_POOL_CREATE_EXTENSION(pool_ptr)
#define TX_EVENT_FLAGS_GROUP_CREATE_EXTENSION(group_ptr)
#define TX_MUTEX_CREATE_EXTENSION(mutex_ptr)
#define TX_QUEUE_CREATE_EXTENSION(queue_ptr)
#define TX_SEMAPHORE_CREATE_EXTENSION(semaphore_ptr)
#define TX_TIMER_CREATE_EXTENSION(timer_ptr)


/* Define the ThreadX object deletion extensions for the remaining objects.  */

#define TX_BLOCK_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_BYTE_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_EVENT_FLAGS_GROUP_DELETE_EXTENSION(group_ptr)
#define TX_MUTEX_DELETE_EXTENSION(mutex_ptr)
#define TX_QUEUE_DELETE_EXTENSION(queue_ptr)
#define TX_SEMAPHORE_DELETE_EXTENSION(semaphore_ptr)
#define TX_TIMER_DELETE_EXTENSION(timer_ptr)


/* Define the get system state macro.   */
   
#ifndef TX_THREAD_GET_SYSTEM_STATE
#ifndef TX_MISRA_ENABLE
#define TX_THREAD_GET_SYSTEM_STATE()        (_tx_thread_system_state | _ipsr)
#else
ULONG   _tx_misra_ipsr_get(VOID);
#define TX_THREAD_GET_SYSTEM_STATE()        (_tx_thread_system_state | _tx_misra_ipsr_get())
#endif
#endif


/* Define the check for whether or not to call the _tx_thread_system_return function.  A non-zero value
   indicates that _tx_thread_system_return should not be called. This overrides the definition in tx_thread.h
   for Cortex-M since so we don't waste time checking the _tx_thread_system_state variable that is always
   zero after initialization for Cortex-M ports. */

#ifndef TX_THREAD_SYSTEM_RETURN_CHECK
#define TX_THREAD_SYSTEM_RETURN_CHECK(c)    (c) = ((ULONG) _tx_thread_preempt_disable); 
#endif


/* Define the macro to ensure _tx_thread_preempt_disable is set early in initialization in order to 
   prevent early scheduling on Cortex-M parts.  */
   
#define TX_PORT_SPECIFIC_POST_INITIALIZATION    _tx_thread_preempt_disable++;


/* Determine if the ARM architecture has the CLZ instruction. This is available on 
   architectures v5 and above. If available, redefine the macro for calculating the 
   lowest bit set.  */

#ifndef TX_DISABLE_INLINE
   
#define TX_LOWEST_SET_BIT_CALCULATE(m, b)       (b) =  (UINT) __clz(__rbit((m)));

#endif


/* Define ThreadX interrupt lockout and restore macros for protection on 
   access of critical kernel information.  The restore interrupt macro must 
   restore the interrupt posture of the running thread prior to the value 
   present prior to the disable macro.  In most cases, the save area macro
   is used to define a local function save area for the disable and restore
   macros.  */

#ifdef TX_DISABLE_INLINE

UINT                                            _tx_thread_interrupt_disable(VOID);
VOID                                            _tx_thread_interrupt_restore(UINT previous_posture);

#define TX_INTERRUPT_SAVE_AREA                  register UINT interrupt_save;

#define TX_DISABLE                              interrupt_save = _tx_thread_interrupt_disable();

#define TX_RESTORE                              _tx_thread_interrupt_restore(interrupt_save);

#else
#include "stm32f7xx_hal.h"
#define ThreadX_MAX_INTERRUPT_PRIORITY   		(0x10)

#define TX_INTERRUPT_SAVE_AREA                  uint32_t  was_masked;
#define TX_DISABLE                              was_masked = __get_BASEPRI(); __set_BASEPRI(ThreadX_MAX_INTERRUPT_PRIORITY);
												
#define TX_RESTORE                              __set_BASEPRI(was_masked);

#define _tx_thread_system_return                _tx_thread_system_return_inline


static void _tx_thread_system_return_inline(void)
{
unsigned int          was_masked;


    /* Set PendSV to invoke ThreadX scheduler.  */
    *((ULONG *) 0xE000ED04) = ((ULONG) 0x10000000);
    if (__get_IPSR()  == 0)
    {
		 was_masked = __get_BASEPRI();
		 __set_BASEPRI(0);
		 __set_BASEPRI(was_masked);
    }
}
#endif


/* Define FPU extension for the Cortex-M7.  Each is assumed to be called in the context of the executing
   thread. These are no longer needed, but are preserved for backward compatibility only.  */

void    tx_thread_fpu_enable(void);
void    tx_thread_fpu_disable(void);


/* Define the version ID of ThreadX.  This may be utilized by the application.  */

#ifdef TX_THREAD_INIT
CHAR                            _tx_version_id[] = 
                                    "Copyright (c) Microsoft Corporation. All rights reserved.  *  ThreadX Cortex-M7/AC5 Version 6.0 *";
#else
#ifdef TX_MISRA_ENABLE
extern  CHAR                    _tx_version_id[100];
#else
extern  CHAR                    _tx_version_id[];
#endif
#endif


#endif




