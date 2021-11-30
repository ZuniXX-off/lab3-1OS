#include <iostream>
#include <Windows.h>
#include <iomanip>

DWORD WINAPI CalculationFunction(LPVOID lpParam);

const uint64_t N = 100'000'000;
const uint64_t blockSize = 110;
const long double invN = 1.0 / N;
long double result = 0;
std::atomic<unsigned> globalIterations = 0;

int main()
{
    system("chcp 1251");
    HANDLE* myThreads;
    int numThreads;
    long double* threadCalcs;
    
    std::cout << " Введите количество потоков: ";
    
    std::cin >> numThreads;
    threadCalcs = new long double[numThreads];
    myThreads = new HANDLE[numThreads];
    
    for (int i = 0; i < numThreads; i++) {
        threadCalcs[i] = 0;
        myThreads[i] = CreateThread(NULL, 0, CalculationFunction, (LPVOID) (threadCalcs + i), CREATE_SUSPENDED, NULL);
    }
    
    uint64_t startTime = GetTickCount();
    
    for (int i = 0; i < numThreads; i++) {
        ResumeThread(myThreads[i]);
    }
    WaitForMultipleObjects(numThreads, myThreads, TRUE, INFINITE);
    
    uint64_t endTime = GetTickCount();
    
    for (int i = 0; i < numThreads; i++) {
        CloseHandle(myThreads[i]);
        result += threadCalcs[i];
    }
        
    result *= invN;

    std::cout << " Время вычислений: " << endTime - startTime << std::endl;
    std::cout << " Результат работы: " << std::setprecision(15) << result;

    delete[] myThreads;
    myThreads = nullptr;

    return 0;
}

DWORD WINAPI CalculationFunction(LPVOID lpParam) {
    long double pi = 0, xi = 0;
    uint64_t curIterations = globalIterations.fetch_add(1) * blockSize;
    while (curIterations < N) {
        for (uint64_t i = curIterations; (i < curIterations + blockSize) && i < N; i++) {
            xi = (i + 0.5) * invN;
            pi += (4.0 / (1.0 + xi * xi));
        }
        curIterations = globalIterations.fetch_add(1) * blockSize;
        *((long double*) lpParam) += pi;
        pi = 0;
    }
    return 1;
}

