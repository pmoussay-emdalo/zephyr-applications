/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions..
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS Embedded Software
 *
 * This file provides helper functions to enable the ihc communication channel.
 * This allows to receive control messages (start, stop, etc) from Linux without
 * the need of having rpmsg enabled.
 */

#include <stdio.h>
#include <string.h>
#include "mss_extra_sw_config.h"
#include "mpfs_hal/mss_hal.h"
#include "miv_ihc.h"
#include "remoteproc.h"

#ifdef USING_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#elif defined(__ZEPHYR__)
#include <zephyr/kernel.h>
#else
#include "utils.h"
#endif

typedef void (*func_t)(void);
extern const uint64_t __SCRATCHPAD_start;

// static uint32_t rx_handler(uint32_t remote_hart_id, uint32_t * message, uint32_t message_size, bool is_ack, uint32_t *message_storage_ptr );

enum miv_rp_mbox_messages {
    MIV_RP_MBOX_READY = 0xFFFFFF00,
    MIV_RP_MBOX_PENDING_MSG = 0xFFFFFF01,
    MIV_RP_MBOX_STOP = 0xFFFFFF02,
    MIV_RP_MBOX_END_MSG = 0xFFFFFF03,
};

/* all needed registration are done in rpmsg lite*/
void rproc_setup(void){
    return;
}
// void rproc_setup(void)
// {
//     uint64_t hartid = read_csr(mhartid);
//     unsigned int irq_key;
//     uint32_t context_hart_id = 0u;

//     IHC_local_context_init((uint32_t)hartid);

//     uint32_t remote_hart_id = IHC_partner_context_hart_id(hartid);

//     IHC_local_remote_config((uint32_t)hartid, remote_hart_id, rx_handler, true, true);


// #ifdef USING_FREERTOS
//     vTaskDelay(300);
// #elif defined(__ZEPHYR__)
//     k_msleep(300);
// #else
//     SpinDelay_MilliSecs(300);
// #endif

//     context_hart_id = IHC_context_to_context_hart_id(hartid);

//     irq_key = irq_lock();

//     switch(context_hart_id) {
//         case 1:
//             IRQ_CONNECT(IHCIA_hart1_INT, 2, IHCIA_hart1_IRQHandler, NULL, 0);
//             irq_enable(IHCIA_hart1_INT);
//             break;
//         case 2:
//             IRQ_CONNECT(IHCIA_hart2_INT, 2, IHCIA_hart2_IRQHandler, NULL, 0);
//             irq_enable(IHCIA_hart2_INT);
//             break;
//         case 3:
//             IRQ_CONNECT(IHCIA_hart3_INT, 2, IHCIA_hart3_IRQHandler, NULL, 0);
//             irq_enable(IHCIA_hart3_INT);
//             break;
//         case 4:
//             IRQ_CONNECT(IHCIA_hart4_INT, 2, IHCIA_hart4_IRQHandler, NULL, 0);
//             irq_enable(IHCIA_hart4_INT);
//             break;
//         default:
//             /*  Unsupported configuration value*/
//             break;
//     }

//     irq_unlock(irq_key);
//     // set_csr(mstatus, MSTATUS_MIE);  /* mstatus Register- Machine Interrupt Enable */
// }

void rproc_stop()
{
    uint64_t my_hart_id = read_csr(mhartid);
    uint32_t context_hart_id = 0u;

    context_hart_id = IHC_context_to_context_hart_id(my_hart_id);
    switch(context_hart_id) {
        case 1:
            irq_disable(DT_IRQN_BY_IDX(DT_NODELABEL(ihc), 0));
            break;
        case 2:
            irq_disable(DT_IRQN_BY_IDX(DT_NODELABEL(ihc), 1));
            break;
        case 3:
            irq_disable(DT_IRQN_BY_IDX(DT_NODELABEL(ihc), 2));
            break;
        case 4:
            irq_disable(DT_IRQN_BY_IDX(DT_NODELABEL(ihc), 3));
            break;
        default:
            /*  Unsupported configuration value*/
            return;
    }

    // use that trick to call the function in the scratchpad with -mo-pie activated
    __asm__ volatile ("jalr ra, 0(%0)\n\t"
    :: "r" (__SCRATCHPAD_start):"ra");
}

// uint32_t rx_handler( uint32_t remote_hart_id, uint32_t * message, uint32_t message_size, bool is_ack, uint32_t *message_storage_ptr )
// {
//     (void)remote_hart_id;

//     if( is_ack == true )
//     {
//         return 0;
//     }

//     switch(*message) {
//         case MIV_RP_MBOX_STOP:
//             rproc_stop();
//             break;
//         default:
//             /* silently handle all other valid messages */
//             return(0U);
//     }

//     return(0U);
// }
