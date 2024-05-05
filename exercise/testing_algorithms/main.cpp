#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <crtdbg.h>

#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>

#include <list>
#include <algorithm>

#include <vector>
#include <set>
#include <random>
#include <numeric>

void raddoppia(int& n) {
    n *= 2;
}

bool divisibile2(const int& n) {
    return (n % 2) == 0;
}

bool not_divisibile(const int& n) {
    return !(divisibile2(n));
}
    //for each n
    /*
    std::vector<int> v = { 1, 3, 4, 6, 7, 4 };

    std::for_each_n(v.begin(), 5, raddoppia);

    for (const auto& elem : v) {
        std::cout << elem << '\n';
    }
    */

    // all_of, any_of, none_of
    /*
    std::vector<int> v = { 1, 3, 5, 9, 99};
    if (std::all_of(v.begin(), v.end(), divisibile))
    {
        std::cout << "Sono tutti divisibili\n";
    }
    else
    {
        std::cout << "zio pera\n";
    }
    if (std::any_of(v.begin(), v.end(), divisibile))
    {
        std::cout << "Almeno uno divisibile\n";
    }
    else
    {
        std::cout << "zio pera\n";
    }
    if (std::none_of(v.begin(), v.end(), divisibile))
    {
        std::cout << "Nessuno divisibile\n";
    }
    else
    {
        std::cout << "zio pera\n";
    }
    */

    //find, find_if, find_if_not  
    /*std::vector<int> v = { 2, 4, 6, 8, 6, 6, 8 };
    auto it1 = std::find(v.begin(), v.end(), 4);
    if (it1 != v.end())
    {
        std::cout << "e vamosoooo\n";
    }

    auto it2 = std::find_if(v.begin(), v.end(), not_divisibile);
    if (it2 != v.end())
    {
        std::cout << "vamos\n";
    }

    auto it3 = std::find_if_not(v.begin(), v.end(), divisibile);
    if (it3 != v.end())
    {
        std::cout << "zio mela\n";
    }*/

    //find_end, find_first_of
    /*std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 2, 3, 4, 5, 8, 2, 3};
    std::vector<int> sv = { 1, 6 };

    auto res = std::find_end(v.begin(), v.end(), sv.begin(), sv.end());
    if (res != v.end())
    {
        std::cout << "Find_end: " << std::distance(v.begin(), res) << '\n';
    }

    auto res2 = std::find_first_of(v.begin(), v.end(), sv.begin(), sv.end());
    if (res2 != v.end())
    {
        std::cout << "Find_first_of: " << std::distance(v.begin(), res2) << '\n';
    }*/

    //adjacent_find
    /*std::vector<int> v = { 1, 2, 3, 3, 4, 5, 5 };
    auto ires = std::adjacent_find(v.begin(), v.end());
    std::cout << "Valore uguale adiacente trovato: " << * ires;*/

    //count, count_if
    /*std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2 };
    for (const int& target : {1, 2, 3}) {
        int res = std::count(v.begin(), v.end(), target);
        std::cout << "Elementi uguali a " << target << ", count: " << res << '\n';
    }


    auto res2 = std::count_if(v.begin(), v.end(), divisibile);
    std::cout << "Elementi divisibili per 2: " << res2 << '\n'*/;

    //equal
    /*std::set<int> s = {1, 2, 4};
    std::vector<int> v = { 1, 2, 4 };

    if (std::equal(s.begin(), s.end(), v.begin(), v.end()))
    {
        std::cout << "Uguali";
    }
    else
    {
        std::cout << "Diversi";
    }*/

    //search
    /*std::vector<int> v = { 3, 4, 3, 4, 5, 1, 2, 3, 7, 8, 1, 2, 3 };
    std::set<int> vv = { 1, 2 };

    auto res = std::search(v.begin(), v.end(), vv.begin(), vv.end());

    std::cout << "Ho trovato l'occorrenza in posizione: " << std::distance(v.begin(), res);*/

    //copy, copy_if
    /*std::vector<int> v = { 1, 2, 3 };
    std::vector<int> vv;
    vv.resize(v.size());

    std::copy(v.begin(), v.end(), vv.begin());

    for (const int& elem : vv) {
        std::cout << elem << '\n';
    }


    std::vector<int> vvv;
    auto count = std::count_if(v.begin(), v.end(), divisibile2);
    vvv.resize(count);
    std::copy_if(v.begin(), v.end(), vvv.begin(), [](const int& n) {return (n % 2) == 0; });
    for (const int& elem : vvv) {
        std::cout << elem << '\n';
    }*/

    //move
    /*std::vector<int> v = { 1, 2, 3 };
    std::vector<int> vv = { 0, 0, 0 };

    std::move(v.begin(), v.end(), vv.begin());

    for (int& elem : vv) {
        std::cout << elem;
        elem = 99;
    }*/

    //swap
    /*std::vector<char> v1 = { 'a', 'b', 'c' };
    std::vector<char> v2 = { 'x', 'y', 'z' };

    std::swap(v1, v2);

    for (const char& c : v1) {
        std::cout << c << " <- v1\n";
    }
    for (const char& c : v2) {
        std::cout << c << " <- v2\n";
    }

    std::vector<std::vector<int>> v1 = { {1, 2, 3}, {3, 4, 5} };
    std::vector<std::vector<int>> v2 = { {10, 20, 30}, {30, 40, 50} };

    std::swap(v1, v2);

    for (const auto& v : v1) {
        for (const int& c : v) {
            std::cout << c << " <- v1\n";
        }
    }
    for (const auto& v : v2) {
        for (const int& c : v) {
            std::cout << c << " <- v2\n";
        }
    }
    */

    //swap_ranges
    /*std::vector<int> v = { 1, 2, 3, 4, 5, 6 };
    std::vector<int> vv = { 7, 8, 9, 10, 11, 12, 13 };

    std::swap_ranges(v.begin(), v.begin() + 3, vv.begin());
    for (const int& c : v) {
        std::cout << c << " <- v\n";
    }
    for (const int& c : vv) {
        std::cout << c << " <- vv\n";
    }*/

    //transform
    /*std::vector<char> v1 = { 'a', 'b', 'c' };
    std::vector<char> v2 = { 'x', 'y', 'z' };

    std::transform(v1.begin(), v1.end(), v2.begin(), [](char& c) {return c -= 32; });

    for (const char& c : v2) {
        std::cout << c << '\n';
    }*/

    //replace, replace_if
    /*std::vector<int> v = { 14, 16, 17, 23, 53, 67, 14 };

    std::replace(v.begin(), v.end(), 14, 0);

    for (const int& elem : v) {
        std::cout << "Elemento di v: " << elem << '\n';
    }

    std::replace_if(v.begin(), v.end(), [](int& i) {return (i % 2) != 0; }, 0);

    for (const int& elem : v) {
        std::cout << "Elemento di v: " << elem << '\n';
    }*/

    //fill, fill_n
    /*std::vector<int> v = { 1, 2, 4, 5, 68, 3 };

        std::fill_n(v.begin(), 4, 0);
        for (const int& elem : v) {
            std::cout << elem << '\n';
        }

        std::fill(v.begin(), v.end(), 0);

        for (const int& elem : v) {
            std::cout << elem << '\n';
        }*/

    //generate
    /*std::vector<int> v = { 1, 3, 5, 7, 9, 11 };
        std::generate(v.begin(), v.end(), []() {return 1; });

        for (const int& elem : v) {
            std::cout << elem << '\n';
        }*/

    //remove, remove_if
    /*std::vector<char> v = { 'a', 'b', 'c', 'b' };
        auto res = std::remove(v.begin(), v.end(), 'b');

        v.resize(std::distance(v.begin(), res));

        for (const char& c : v) {
            std::cout << c << '\n';
        }

        std::vector<int> vv = { 1, 2, 3, 4, 5, 6 ,7 ,8 ,9 };
        auto sres = std::remove_if(vv.begin(), vv.end(), [](const int& n) {return (n % 2) == 0; });

        vv.resize(std::distance(vv.begin(), sres));

        for (const int& elem : vv) {
            std::cout << elem << '\n';
        }*/

    //reverse
    /*std::vector<int> v = { 1, 2, 3, 4, 5, 6 };
        std::reverse(v.begin(), v.end());

        for (const int& elem : v) {
            std::cout << elem << '\n';
        }*/

    //rotate
    /*std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 8 };

        std::rotate(v.begin(), v.begin()+3, v.end()-3);

        for (const int& elem : v) {
            std::cout << elem << '\n';
        }*/

    //shift_left, shift_right
    /*std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 8 };

        std::shift_left(v.begin(), v.end(), 2);

        for(const int& elem : v){
            std::cout << elem << ' ';
        }

        std::cout << '\n';
        std::vector<int> vv = { 1, 2, 3, 4, 5, 6, 7, 8 };

        std::shift_right(vv.begin(), vv.end(), 2);

        for (const int& elem : vv) {
            std::cout << elem << ' ';
        }*/

    //shuffle
    /*std::vector<int> v = { 1, 2, 4, 5, 6, 7, 2, 46 };
        std::random_device rng;

        std::shuffle(v.begin(), v.end(), rng);

        for (const int& elem : v) {
            std::cout << elem << ' ';
        }*/

    //sample
    /*std::vector<int> v = { 1, 2, 3, 4, 5, 7, 8, 10, 14, 15, 18 };
            std::vector<int> vv(5);
            std::random_device rnd;

            std::sample(v.begin(), v.end(), vv.begin(), 5, rnd);

            for (const int& elem : vv) {
                std::cout << elem << ' ';
            }*/

    //partition
    /*std::vector<int> v = { 1, 4, 23, 654, 324, 13, 5678, 43, 32, 25, 74, 76, 543, 235, 985, 345, 542, 23, 674, 867 };

            std::partition(v.begin(), v.end(), divisibile2);

            for (const int& elem : v) {
                std::cout << elem << ' ';
            }*/

    //partition_copy
    /*std::vector<int> numbers = { 1, 345, 213, 132, 67, 76, 235, 524, 76, 53, 67, 1436, 1364, 6135};
            std::vector<int> even;
            std::vector<int> odd;

            std::partition_copy(numbers.begin(), numbers.end(), std::back_inserter(even), std::back_inserter(odd), divisibile2);

            for (const int& elem : even) {
                std::cout << "Pari: " << elem << '\n';
            }
            for (const int& elem : odd) {
                std::cout << "Dispari: " << elem << '\n';
            }*/

    //sort, stable_sort, partial_sort, partial_sort_copy
    /*std::vector<int> v = { 1, 2, 413, 12, 5, 63, 132, 137,756, 31, 123, 354,56, 46,2, 14,14234, 52436, 432 };
            std::vector<int> vv(5);

            std::sort(v.begin(), v.end());
            std::stable_sort(v.begin(), v.end());
            std::partial_sort(v.begin(), v.begin()+5, v.end());
            std::partial_sort_copy(v.begin(), v.end(), vv.begin(), vv.end());


            for (const int& elem : vv) {
                std::cout << elem << ' ';
            }*/

    //is_sorted
    /*std::vector<int> v = { 1, 31, 5226, 625 ,6573,5,14,4143, 123, 4,  4,3543 };

            if (std::is_sorted(v.begin(), v.end())) {
                std::cout << "è sorted\n";
            }
            else {
                std::cout << "non è sorted\n";
            }
            std::sort(v.begin(), v.end());
            if (std::is_sorted(v.begin(), v.end())) {
                std::cout << "è sorted\n";
            }
            else {
                std::cout << "non è sorted\n";
            }*/




    /*operazioni su strutture dati ordinate*/

    //lower_bound, upper_bound
    /*std::vector<int> v = {231, 432, 6, 526 ,26 ,25, 263, 563, 2563, 256, 256,1543, 341, 7624, 72 };
            std::sort(v.begin(), v.end());

            auto res = std::lower_bound(v.begin(), v.end(), 20);
            std::cout << "Lower bound: " << *res << '\n';
            res = std::upper_bound(v.begin(), v.end(), 100);
            std::cout << "Upper bound: " << *res << '\n';*/

    //binary_search
    /*std::vector<int> v = { 23, 425, 65, 586, 875, 23, 324, 635, 5674, 587, 76, 234, 3654, 213, 564, 245 };
                std::sort(v.begin(), v.end());

                if (std::binary_search(v.begin(), v.end(), 875)) {
                    std::cout << "Trovato";
                }
                else {
                    std::cout << "Non trovato";
                }*/

    //includes
    /*std::set<int> v = { 1, 2, 3, 4, 5, 6, 7, 8 };
                std::set<int> vv = { 3, 4};

                if (std::includes(v.begin(), v.end(), vv.begin(), vv.end())) {
                    std::cout << "Trovato";
                }
                else {
                    std::cout << "Non trovato";
                }*/


    
    /*operazioni sui set*/

    //set_union, set_intersection, set_difference 
    /*std::set<int> s = { 1, 2, 3, 4, 5, 7, 8 };
    std::set<int> ss = { 3, 4, 5, 6, 7, 8, 9, 10 };

    std::set<int> res1;
    std::set_union(s.begin(), s.end(), ss.begin(), ss.end(), std::inserter(res1, res1.begin()));
    for (const int& elem : res1) {
        std::cout << elem << ' ';
    }
    std::cout << "<- Union\n";

    std::set<int> res2;
    std::set_intersection(s.begin(), s.end(), ss.begin(), ss.end(), std::inserter(res2, res2.begin()));
    for (const int& elem : res2) {
        std::cout << elem << ' ';
    }
    std::cout << "<- Intersection\n";

    std::set<int> res3;
    std::set_difference(s.begin(), s.end(), ss.begin(), ss.end(), std::inserter(res3, res3.begin()));
    for (const int& elem : res3) {
        std::cout << elem << ' ';
    }
    std::cout << "<- Difference\n";*/


int main(void) {

    

    return 0;
}
  