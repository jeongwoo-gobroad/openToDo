// KNU CSE 2021114026 Jeongwoo Kim
// implements c++
#include <iostream>
#include <vector>
#include <random>
#include <windows.h>

using namespace std;

int main(void) {
    const unsigned long long yearmult = 100000000;
    const unsigned long long monthmult = 1000000;
    const unsigned long long daymult = 10000;
    const unsigned long long hourmult = 100;
    unsigned long long tmp = 0;

    vector<string> title = {"Birthday", "Appointment", "Meeting", "Workout", "bonorum", "malorum", "consectetur", "exercitation"};
    vector<string> info = {"Lorem ipsum dolor sit amet, consectetur adipiscing elit", "sed do eiusmo tempo incidunt ut labore et dolor magna aliq", 
        " ullamco laboris nisi ut aliquip ex ea commodo consequat", "Duis aute irure dolor in rehendert in volupate velit esse", "cillum dolore eu fugiat nulla pariatur",
        "Excepteur sint occaecat cupidatat non proident", "sunt in culpa qui offici desunt molit aim id est laborum."};

    int size;
    int a, b;
    int input;

    random_device num;
    mt19937_64 gen(num());
    cout << "input range of year A to B" << endl;
    cout << "A: "; cin >> a;
    cout << "B: "; cin >> b;
    cout << "input number of records AND type(1 for normal random records, 2 for specific date only)" << endl;
    cin >> size;
    cin >> input;

    uniform_int_distribution<unsigned long long> year(a, b);
    uniform_int_distribution<unsigned long long> yearA(2024, 2024);
    uniform_int_distribution<unsigned long long> month(1, 12);
    uniform_int_distribution<unsigned long long> monthA(5, 5);
    uniform_int_distribution<unsigned long long> day(1, 31);
    uniform_int_distribution<unsigned long long> dayA(8, 8);
    uniform_int_distribution<unsigned long long> hour(0, 23);
    uniform_int_distribution<unsigned long long> minute(0, 59);
    uniform_int_distribution<unsigned long long> titleCntx(0, 7);
    uniform_int_distribution<unsigned long long> infoCntx(0, 6);
    uniform_int_distribution<int> pnum(0, 1);

    if (input == 1) {
        for (int i = 0; i < size; i++) {
            tmp = year(gen) * yearmult + month(gen) * monthmult + day(gen) * daymult + hour(gen) * hourmult + minute(gen);
            //cout << year(gen) * yearmult << endl;
            //cout << "1" << endl;
            cout << tmp << "\n" << pnum(gen) << "\n" << title[titleCntx(gen)] << "\n" << info[infoCntx(gen)] << endl;
            //printf("%lu %s %s\n", tmp, title[titleCntx(gen)], info[infoCntx(gen)]);
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            tmp = yearA(gen) * yearmult + monthA(gen) * monthmult + dayA(gen) * daymult + hour(gen) * hourmult + minute(gen);
            //cout << year(gen) * yearmult << endl;
            //cout << "1" << endl;
            cout << tmp << "\n" << pnum(gen) << "\n" << title[titleCntx(gen)] << "\n" << info[infoCntx(gen)] << endl;
            //printf("%lu %s %s\n", tmp, title[titleCntx(gen)], info[infoCntx(gen)]);
        }
    }

    Sleep(100000);

    return 0;
}