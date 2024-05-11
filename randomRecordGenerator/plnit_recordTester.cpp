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

    vector<string> title = {"Morning Jog", "Team Meeting", "Lunch with Mentor", "Code Review", "Dentist Appointment", "Grocery Shopping", "Book Club", "Gym Session",
        "Study Time", "Call Parents", "Yoga Class", "Client Meeting" ,"Write Report" ,"Cook Dinner", "Read Articles", "Plan Trip", "Guitar Practice", 
        "Check Emails", "Laundry", "Bedtime"};
    vector<string> info = {"Start the day with a 30-minute run in the park.",
        "Discuss project milestones and delegate tasks.",
        "Meet at noon at Cafe Luna to discuss career plans.",
        "Examine the latest commits before the end of the day.", 
        "Teeth cleaning session at 3 PM with Dr. Smith.", 
        "Buy vegetables, bread, and milk for the week.", 
        "Read and discuss 1984 by George Orwell.",
        "Leg day workout followed by 20 mins of cardio.",
        "Focus on algorithms and data structures.",
        "Catch up with family at 8 PM for half an hour.",
        "Relaxing mind and body with instructor Lee.",
        "Present Q2 marketing strategy and get feedback.",
        "Summarize findings from the recent survey.",
        "Try a new recipe for pasta with homemade sauce.",
        "Stay updated with the latest tech news.",
        "Research and book accommodations for summer vacation.",
        "Learn new chords and practice the song Yesterday.",
        "Reply to urgent messages and organize inbox.",
        "Wash clothes and prepare outfits for the week.",
        "Wind down by 10 PM and review plans for tomorrow."};

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
    uniform_int_distribution<unsigned long long> titleCntx(0, 19);
    uniform_int_distribution<unsigned long long> infoCntx(0, 6);
    uniform_int_distribution<int> pnum(0, 3);

    if (input == 1) {
        for (int i = 0; i < size; i++) {
            tmp = year(gen) * yearmult + month(gen) * monthmult + day(gen) * daymult + hour(gen) * hourmult + minute(gen);
            //cout << year(gen) * yearmult << endl;
            //cout << "1" << endl;
            cout << tmp << "\n" << pnum(gen) << "\n" << title[titleCntx(gen)] << "\n" << info[titleCntx(gen)] << endl;
            //printf("%lu %s %s\n", tmp, title[titleCntx(gen)], info[infoCntx(gen)]);
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            tmp = yearA(gen) * yearmult + monthA(gen) * monthmult + dayA(gen) * daymult + hour(gen) * hourmult + minute(gen);
            //cout << year(gen) * yearmult << endl;
            //cout << "1" << endl;
            cout << tmp << "\n" << pnum(gen) << "\n" << title[titleCntx(gen)] << "\n" << info[titleCntx(gen)] << endl;
            //printf("%lu %s %s\n", tmp, title[titleCntx(gen)], info[infoCntx(gen)]);
        }
    }

    Sleep(100000);

    return 0;
}