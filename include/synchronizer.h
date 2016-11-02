// EPOS Synchronizer Abstractions Common Package

#ifndef __synchronizer_h
#define __synchronizer_h

#include <cpu.h>
#include <thread.h>

__BEGIN_SYS

class ThreadAndPriority {
public:
    ThreadAndPriority(Thread* thread, Thread::Priority priority) :
        thread(thread), originalPriority(priority) {}
    Thread* thread;
    Thread::Priority originalPriority;
};

class Synchronizer_Common
{
protected:
    typedef Thread::Queue Queue;

protected:
    Synchronizer_Common() {}
    ~Synchronizer_Common() { begin_atomic(); wakeup_all(); }

    // Atomic operations
    bool tsl(volatile bool & lock) { return CPU::tsl(lock); }
    int finc(volatile int & number) { return CPU::finc(number); }
    int fdec(volatile int & number) { return CPU::fdec(number); }

    // Thread operations
    void begin_atomic() { Thread::lock(); }
    void end_atomic() { Thread::unlock(); }

    void sleep() { Thread::sleep(&_queue); }
    void wakeup() { Thread::wakeup(&_queue); }
    void wakeup_all() { Thread::wakeup_all(&_queue); }

protected:

    Queue _queue;
    List<ThreadAndPriority> _running;

    void resolve_priority() {
        Thread* running = Thread::_running;
        for(
            List<ThreadAndPriority>::Iterator i = this->_running.begin();
            i != this->_running.end(); i++
        ) {
            Thread* current = i->object()->thread;
            Thread::Priority newPriority = running->priority() < current->priority() ?
                running->priority() : current->priority();

            current->priority(newPriority);

            if (current->_state == Thread::READY) {
                Thread::_ready.remove(current);
                Thread::_ready.insert(&current->_link);
            }
        }
    }

    void revert_priority_and_leave() {
        Thread* running = Thread::_running;

        for(
            List<ThreadAndPriority>::Iterator i = this->_running.begin();
            i != this->_running.end(); i++
        ) {
            Thread* current = i->object()->thread;
            if (current == running) {
                Thread::Priority originalPriority = i->object()->originalPriority;
                running->priority(originalPriority);
                if (running->_state == Thread::READY) {
                    Thread::_ready.remove(running);
                    Thread::_ready.insert(&running->_link);
                }
                this->_running.remove(i->object());
                break;
            }
        }
    }

    void enter() {
        Thread* running = Thread::_running;
        Thread::Priority priority = running->priority();
        ThreadAndPriority* tp (new (kmalloc(sizeof(ThreadAndPriority))) ThreadAndPriority(running, priority));
        this->_running.insert_head(new (kmalloc(sizeof(List<ThreadAndPriority>::Element))) List<ThreadAndPriority>::Element(tp));
    }

};

__END_SYS

#endif
