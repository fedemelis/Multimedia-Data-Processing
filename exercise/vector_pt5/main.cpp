#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <crtdbg.h>

#include <algorithm>
#include <utility>
#include <fstream>
#include <vector>
#include <list>
#include <iostream>
#include <iterator>

#include <array>

//
//std::vector<double> read_vector(const char* filename)
//{
//    std::vector<double> q;
//    std::ifstream is(filename/*, std::ios::binary*/);
//    if (!is) {
//        return q;
//    }
//
//    double num;
//    while (is >> num) {
//        q.push_back(num);
//    }
//    return q;
//}

int main(int argc, char* argv[])
{
    using std::vector;

    if (argc != 3) {
        return 1;
    }

    vector<int> v;
    std::ifstream is(argv[1]/*, std::ios::binary*/);
    int num;
    while (is >> num)
    {
        v.push_back(num);
    }

    //l'istream_iterator legge i dati con l'estrattore >>
    //cioè estrea da un file dei dati in formato testuale e poi li salva in binario

    //costruiamo un vettore con un range di iteratori, il primo costruito sulla base di un istream e il secondo
    //è sempre un iteratore ma è fittizzio

    //vector<double> v(it, it_end);

    //copy(it, it_end, back_inserter(v));


    /*std::list<int> l = { 1, 2, 3, 4, 53, 45, 654, 32, 23, 12 };
    l.sort();
    for (const int& elem : l) {
        std::cout << elem << ' ';
    }*/

    // dentro le [] della lambda metto i parametri esterni che voglio usare dentro la funzione anonima
    // [&] vuol dire che nella mia lambda posso usare tutte le variabili definite nel resto del programma per riferimento   
    std::sort(v.begin(), v.end(), [] (int& a, int& b) {return a > b; });

    std::ofstream os(argv[2]/*, std::ios::binary*/);
    if (!os) {
        return 1;
    }

    for (const auto& x : v) {
        os << x << '\n';
    }

    return 0;
}