

#include <condition_variable>
#include <iostream>
#include <mutex>

class fifo_mutex
{
private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    uint64_t m_current = 0;
    uint64_t m_end = 0;
public:
    void lock()
    {
	std::unique_lock ulock(m_mutex);
        if (m_current == m_end)
        {
            m_end++;
            return;
        }
        
        uint64_t my_val = ++m_end;
        
        while (m_current != my_val)
	{
            m_cond.wait(ulock);
        }
    }
   
    void unlock()
    {
        std::unique_lock ulock(m_mutex);
        m_current++;
        m_cond.notify_all();
    }
    
    void debug(const char* prefix)
    {
        std::cerr << prefix << ", m_current: " << m_current << ", m_end: " << m_end << std::endl;
    }
};

class scoped_fifo_lock
{
private:
    fifo_mutex* m_mutex = nullptr; 
public:
    scoped_fifo_lock(fifo_mutex* mutex_ptr)
        : m_mutex(mutex_ptr)
    {
        if (m_mutex)
        {
            m_mutex->lock();
        }
    }
    ~scoped_fifo_lock()
    {
        if (m_mutex)
        {
            m_mutex->unlock();
        }
        m_mutex = nullptr;
    }
};

using namespace std;


int main (int argc, char** argv)
{
    fifo_mutex mtx;

    for (uint32_t i = 0; i < 5; i++)
    {
        scoped_fifo_lock alock(&mtx);
        cout << "loop: " << i << endl;
        mtx.debug("\tmutex");
    }
}
