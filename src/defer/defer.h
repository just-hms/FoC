#ifndef defer

// Define a dummy struct to be used as a placeholder
struct defer_dummy {};
// Define a template struct to store a function object and execute it on
// destruction
template <class F>
struct deferrer {
    F f;
    // Destructor of deferrer, executes the stored function
    ~deferrer() { f(); }
};

// Overload the * operator to create instances of deferrer
template <class F>
deferrer<F> operator*(defer_dummy, F f) {
    return {f};
}

// Macro to generate a unique identifier based on line number
#define DEFER_(LINE) zz_defer##LINE
#define DEFER(LINE) DEFER_(LINE)

// Macro definition of defer
#define defer auto DEFER(__LINE__) = defer_dummy{} *[&]()

#endif

/*

// USAGE EXAMPLE
#include <fstream>
#include <iostream>

int main() {
    std::ofstream file("example.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    // The code inside the defer will be run after the main scope ends
    defer { file.close(); };

    return 0;
}

*/
