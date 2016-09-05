// EPOS Synchronizer Abstractions Common Package

#ifndef __synchronizer_h
#define __synchronizer_h

#include <cpu.h>
#include <thread.h>

__BEGIN_SYS

class Synchronizer_Common
{
private:
    typedef Ordered_Queue<Thread, Thread::Priority> Queue;
    Queue _sleeping;


protected:
    Synchronizer_Common() {}

    // Atomic operations
    bool tsl(volatile bool & lock) { return CPU::tsl(lock); }
    int finc(volatile int & number) { return CPU::finc(number); }
    int fdec(volatile int & number) { return CPU::fdec(number); }

    // Thread operations
    void begin_atomic() { Thread::lock(); }
    void end_atomic() { Thread::unlock(); }

    void sleep() {
        begin_atomic();
        if(!Thread::_ready.empty()) {
            Thread * prev = Thread::_running;
            prev->_state = Thread::WAITING;
            _sleeping.insert(&prev->_link);
            Thread::_running = Thread::_ready.remove()->object();
            Thread::_running->_state = Thread::RUNNING;
            Thread::dispatch(prev, Thread::_running);
        } else {
            Thread::idle();
        }
        end_atomic();
    }

    void wakeup() {
        begin_atomic();
        if (!_sleeping.empty())
            wakeupThread();
        end_atomic();
    }

    void wakeup_all() {
        begin_atomic();
        while (!_sleeping.empty())
            wakeupThread();
        end_atomic();
    }

private:
    void wakeupThread() {
        Thread * waking = _sleeping.remove()->object();
        waking->_state = Thread::READY;
        Thread::_ready.insert(&waking->_link);
    }
};

__END_SYS

#endif
