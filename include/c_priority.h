/*
 * priority switching
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_priority.h
 *
 */

#pragma once

#include <sched.h>

/**
 * @brief class c_priority
 */
class c_priority
{
public:
    /**
     * @brief switch priority
     * @param manual if true then use set/restore to switch
     */
    c_priority(bool manual = false)
    {
        // save actual priority
        sched_getparam(0, &m_sched);

        // if not manual switch priority on create
        if (!manual)
            set();
    }

    /**
     * @brief restore previous priority on destroy
     */
    ~c_priority()
    {
        restore();
    }

    /**
     * @brief switch priority manual
     */
    void set()
    {
        // set priority to FIFO
        sched_param sched;
        sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
        sched_setscheduler(0, SCHED_FIFO, &sched);
    }

    /**
     * @brief restore priority manual 
     */
    void restore()
    {
        sched_setscheduler(0, SCHED_OTHER, &m_sched);
    }

private:
    sched_param m_sched; // save priority
};
