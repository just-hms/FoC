#include <pthread.h>

#include <cstdio>

int testsResult = 0;
// Declare the mutex;
pthread_mutex_t mutex;

typedef struct {
    int (*testFunc)();
    char *funcName;
} TestStruct;

// Function to run tests
void *run_test(void *arg) {
    TestStruct *test =
        (TestStruct *)arg;  // Cast argument to TestStruct pointer
    int res = test->testFunc();
    pthread_mutex_lock(&mutex);
    testsResult = testsResult || res;
    pthread_mutex_unlock(&mutex);

    if (res == 0) {
        printf("%-20s\t[OK]\n", test->funcName);
    } else {
        fprintf(stderr, "\033[31m%-20s\t[FAILED]\033[0m\n", test->funcName);
    }

    return NULL;
}

#define NUM_TESTS 0   /*{@NUM_TESTS}*/
#define NUM_THREADS 0 /*{@NUM_THREADS}*/

/*{@INCLUDES}*/

int main() {
    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&mutex, nullptr);  // Initialize the mutex

    TestStruct tests[NUM_TESTS] = {/*{@TESTS}*/};
    int threadIndex = 0;

    // Create and join threads
    for (int i = 0; i < NUM_TESTS; i++) {
        if (pthread_create(&threads[threadIndex], NULL, run_test, &tests[i])) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }

        if (++threadIndex >= NUM_THREADS) {
            // Wait for threads to finish
            for (int j = 0; j < NUM_THREADS; j++) {
                pthread_join(threads[j], NULL);
            }
            threadIndex = 0;
        }
    }

    // Join any remaining threads
    for (int i = 0; i < threadIndex; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);  // Destroy the mutex after use
    return testsResult;
}