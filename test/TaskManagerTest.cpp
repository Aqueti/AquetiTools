#include "AquetiToolsTest.h"

namespace atl {

JsonBox::Value testTaskManager(int threads, bool valgrind, bool printFlag, bool assertFlag)
{
	JsonBox::Value resultString;

    if (printFlag) {
        std::cout << "Testing TaskManager" << std::endl;
        std::cout << threads << " threads: ";
    }

    atl::ThreadPool tp(threads, threads);
    atl::TaskManager<int, int> man;

    std::atomic_int i(0);
    auto fun = [&]() -> int {
            atl::sleep(0.01);
            return i++;
        };

    int j;

    for (j = 0; j < 1000; j++) {
        tp.push_job([&]() {
            man.performJob(j%5, fun);
        });
    }

    atl::Timer t;
    tp.Start();

    tp.wait_until_empty();
    tp.Stop();
    tp.Join();

    double time = t.elapsed();
    std::cout << "elapsed time: " << time << std::endl;

    bool ret = (i < 10 && time < 0.15) || valgrind;
    if (!ret) {
        if (printFlag) {
            std::cout << "Test failed!" << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["pass"] = false;
        return resultString;
    }

    if (printFlag) {
        std::cout << "TaskManager test passed!" << std::endl;
    }
    resultString["pass"] = true;
    return resultString;
}
}