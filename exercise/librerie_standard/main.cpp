#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <crtdbg.h>

#include <algorithm>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>

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
    return q;
}

int main(int argc, char* argv[])
{
    using std::vector;

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

    for (const auto& x : v) {
        os << x << '\n';    
    }

    vector<double> x;
    //x.resize(4);
    /*
    auto itx = x.begin();
    for (auto itv = v.begin(), endv = v.end(); itv != endv; ++itv) {
        *itx = *itv;
        ++itx;
    }*/
    std::copy(v.begin(), v.end(), std::back_inserter(x));

    /*
    for (auto it = v.begin(), end = v.end(); it != end; ++it) {
        const double& x = *it;
        os << x << '\n';
    }
    */

    /*
    for (std::vector<double>::iterator current = v.begin(), stop = v.end(); current != stop; ++current) {
        double x = *current;
        os << x << '\n';
    }
    */

    /*
    std::vector<double>::iterator start = v.begin();
    std::vector<double>::iterator stop = v.end();
    std::vector<double>::iterator current;

    current = start;
    while (current != stop) {
        double x = *current;
        os << x << '\n';
        ++current;
    }
    */

    //for (size_t i = 0; i < v.size(); i++) {
    //    os << v[i] << '\n';
    //}

    return 0;
}