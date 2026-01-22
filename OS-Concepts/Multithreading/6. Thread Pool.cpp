#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>

using namespace std;

// A simple thread pool class that manages a fixed number of worker threads
class ThreadPool {
private:
    // Vector to store all worker threads
    vector<thread> workers;
    
    // Queue to store tasks (functions) that need to be executed
    queue<function<void()>> tasks;
    
    // Mutex to protect the task queue from race conditions
    mutex queueMutex;
    
    // Condition variable to notify worker threads when new tasks arrive
    condition_variable condition;
    
    // Flag to indicate if the thread pool is being stopped
    bool stop;
    
public:
    // Constructor: creates a thread pool with 'numThreads' worker threads
    ThreadPool(size_t numThreads) {
        stop = false;
        
        // Create the specified number of worker threads
        for (size_t i = 0; i < numThreads; i++) {
            workers.push_back(thread(&ThreadPool::workerThread, this));
        }
    }
    
    // Destructor: waits for all tasks to complete and joins all threads
    ~ThreadPool() {
        // Lock the queue and set the stop flag
        {
            unique_lock<mutex> lock(queueMutex);
            stop = true;
        }
        
        // Notify all worker threads to wake up and check the stop flag
        condition.notify_all();
        
        // Wait for all worker threads to finish their current tasks
        for (size_t i = 0; i < workers.size(); i++) {
            workers[i].join();
        }
    }
    
    // Add a new task to the queue
    void enqueue(function<void()> task) {
        // Lock the queue before adding a task
        {
            unique_lock<mutex> lock(queueMutex);
            tasks.push(task);
        }
        
        // Notify one waiting worker thread that a new task is available
        condition.notify_one();
    }
    
private:
    // The function that each worker thread runs continuously
    void workerThread() {
        while (true) {
            function<void()> task;
            
            // Lock the queue to check for tasks
            {
                unique_lock<mutex> lock(queueMutex);
                
                // Wait until there's a task in the queue or stop is signaled
                // This releases the lock while waiting and reacquires it when notified
                while (tasks.empty() && !stop) {
                    condition.wait(lock);
                }
                
                // If stop is true and no more tasks, exit the thread
                if (stop && tasks.empty()) {
                    return;
                }
                
                // Get the next task from the queue
                task = tasks.front();
                tasks.pop();
            }
            // Lock is automatically released here when 'lock' goes out of scope
            
            // Execute the task outside the lock so other threads can access the queue
            task();
        }
    }
};


// ===== EXAMPLE USAGE =====

// A simple task function that simulates some work
void printNumbers(int start, int end) {
    cout << "Thread " << this_thread::get_id() << " processing range " 
         << start << " to " << end << endl;
    
    // Simulate some work by sleeping
    this_thread::sleep_for(chrono::milliseconds(100));
    
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += i;
    }
    
    cout << "Thread " << this_thread::get_id() << " completed. Sum = " 
         << sum << endl;
}

// Helper function to create a task for the thread pool
function<void()> createTask(int start, int end) {
    return bind(printNumbers, start, end);
}

int main() {
    cout << "Creating thread pool with 4 worker threads..." << endl;
    
    // Create a thread pool with 4 worker threads
    ThreadPool pool(4);
    
    cout << "Submitting 10 tasks to the thread pool..." << endl;
    
    // Submit 10 tasks to the thread pool
    for (int i = 0; i < 10; i++) {
        int start = i * 10 + 1;
        int end = (i + 1) * 10;
        pool.enqueue(createTask(start, end));
    }
    
    cout << "All tasks submitted. Waiting for completion..." << endl;
    
    // Sleep to allow tasks to complete
    // In a real application, you might use a more sophisticated mechanism
    this_thread::sleep_for(chrono::seconds(2));
    
    cout << "All tasks completed. Shutting down..." << endl;
    
    // ThreadPool destructor will be called here, cleaning up all threads
    return 0;
}


/* ===== EXPLANATION =====
 
 HOW THE THREAD POOL WORKS:
 
 1. INITIALIZATION:
    - When you create a ThreadPool, it spawns a fixed number of worker threads
    - Each worker thread runs the workerThread() function in a loop
    - Workers wait for tasks to arrive in the queue
 
 2. ADDING TASKS:
    - When you call enqueue(), the task is added to the queue
    - One waiting worker thread is notified via condition variable
    - The notified thread wakes up and picks up the task
 
 3. EXECUTING TASKS:
    - Worker thread removes the task from the queue
    - Releases the lock so other threads can access the queue
    - Executes the task
    - Goes back to waiting for the next task
 
 4. SHUTDOWN:
    - When ThreadPool is destroyed, the stop flag is set
    - All worker threads are notified to wake up
    - Each thread finishes its current task and exits
    - Main thread waits for all workers to join
 
 KEY SYNCHRONIZATION CONCEPTS:
 
 - mutex: Ensures only one thread accesses the queue at a time
 - condition_variable: Allows threads to wait efficiently without busy-waiting
 - unique_lock: RAII-style lock that automatically releases when out of scope
 
 ADVANTAGES OF THREAD POOLS:
 
 - Reuses threads instead of creating/destroying them for each task
 - Limits the number of concurrent threads (prevents resource exhaustion)
 - Provides a simple interface to parallelize work
 - Better performance for many small tasks
 
*/