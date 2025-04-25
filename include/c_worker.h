/*
 * C++ class definition of worker thread
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_worker.h
 *
 */

#pragma once

#include <thread>
using namespace std;

/**
 * @brief class c_worker
 */
class c_worker
{
public:
    /**
     * @brief virtual class destructor
     * @note must be implemented in derived class
     */
    virtual ~c_worker() {}

    /**
     * @brief called once in thread
     * @note must be implemented in derived class
     */
    virtual void Execute() {}

    /**
     * @brief start thread
     * @param wait false: return immediately, true: wait until ends
     */
    void Queue(bool wait = false)
    {
        thread th(&c_worker::execute, this);

        if (wait)
            th.join();
        else
            th.detach();
    }

private:
    /**
     * @brief main execute function
     */
    void execute()
    {
        Execute();
        delete this; // harakiri class
    }
};
