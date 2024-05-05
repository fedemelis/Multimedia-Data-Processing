#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <crtdbg.h>


#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <list>
#include <algorithm>


std::vector<double> read_vector(const char* filename)
{
    std::vector<double> q;
    std::ifstream is(filename/*, std::ios::binary*/);
    if (!is) {
        return q;
    }

    double num;
    while (is >> num) {
        q.push_back(num);
    }
    //is.close(); no//n serve perché si distrugge come usciamo dallo scope
    return q;
}


int main(int argc, char* argv[])
{

    using std::vector;
    using std::list;

    if (argc != 3) {
        return 1;
    }

    vector<double> v = read_vector(argv[1]);
    if (v.size() == 0) {
        return 1;
    }

    //v.sort();

    std::ofstream os(argv[2]/*, std::ios::binary*/);
    if (!os) {
        return 1;
    }

    auto start = v.begin();
    auto stop = v.end();
    std::vector<double>::iterator current;

    for (const auto& elem : v) {
        std::cout << elem;
    }

    vector<double> x;
    x.resize(4);
    
    std::copy(v.begin(), v.end(), std::back_inserter(x));
    
    /*
    for (auto it = v.begin(), end = v.end(); it != end; ++it)
    {
        const double& x = *it;
        os << x << '\n';
    }
    */
    /*
    while (current != stop) //inportante confrontare gli iteratori con !=
    {
        double x = *current;
        os << x << '\n';
        ++current; //fare SEMPRE il preincremento;
    }
    */

    /*
    for (size_t i = 0; i < v.size(); i++) {
        os << v << '\n';
    }
    */

    return 0;
}