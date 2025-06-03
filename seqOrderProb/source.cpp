#pragma once
#include "misc.hpp"
#include "genetic.hpp"
#include "swarm.hpp"
#include "timer_utility.hpp"
#include <time.h>
import std;

//Forward Decl
void ant_colony(const int to_do);
void genetic_algorithm(const int to_do);

const int GLB_GA_pop{ 200 }, GLB_GA_iter{ 100 };
const int GLB_pop{ GLB_GA_pop }, GLB_iter{ GLB_GA_pop };
int main()
{
    srand(time(NULL));

    int select_alg{ 0 }, select_step{ 1 };
    //std::cout << "(0:Genetic | 1: Ant Swarm) (0:Single File | 1:test | 2:All files CSV) : ";
    //std::cin >> select_alg >> select_step; // 0 | 1 && 0 | 1 | 2

    //if(select_alg == 0)
    //    genetic_algorithm(select_step);
    //else if(select_alg == 1)
    //    ant_colony(select_step);

    TimerUtility ga_timer, aco_timer;
    ga_timer.startTimer();
    genetic_algorithm(select_step);
    std::u8string ga_time{ (ga_timer.getDuration(u8"genetic alg")) };

    aco_timer.startTimer();
    ant_colony(select_step);
    std::u8string aco_time{ aco_timer.getDuration(u8"ant col opt alg") };

    std::cout << reinterpret_cast<const char*>(ga_time.c_str());
    std::cout << reinterpret_cast<const char*>(aco_time.c_str());
    return 0;
}

void ant_colony(const int to_do)
{
    if (to_do == 0)
    {
        std::string file_name{ file_menu().string() };
        Adjacency_Matrix graph_mat;
        init_graph(graph_mat, file_name);
        std::cout << "file :" << file_name << '\n';
        ant_colony_algorithm_SOP(graph_mat.size(), 500, graph_mat);
    }
    else if (to_do == 1)
    {
        std::string file_name{ file_menu().string() };
        Adjacency_Matrix graph_mat;
        init_graph(graph_mat, file_name);
        std::cout << "file :" << file_name << '\n';

        std::vector<double> average_polling{ std::vector<double>(GLB_iter + 1, 0.0) };
        std::vector<double> minimum_polling{ std::vector<double>(GLB_iter + 1, 0.0) };

        for (int i{ 0 }; i < 10; ++i)
        {
            loading_screen(static_cast<float>(i) * 10.0);
            ant_colony_algorithm_SOP(GLB_pop, GLB_iter, graph_mat, average_polling, minimum_polling);
        }

        for (auto& n : average_polling)
            n /= 10.0;
        for (auto& n : minimum_polling)
            n /= 10.0;

        make_csv(average_polling, minimum_polling, "AC" + std::to_string(GLB_pop) + "_" + std::to_string(GLB_iter));
    }
    else if (to_do == 2)
    {
        std::ofstream file("ratios_ac.csv");

        file << "Name,Found Min,Given Min,Ratio\n";

        std::unordered_map<std::string, double> minimums;
        get_harcoded_map(minimums);
        std::vector <std::filesystem::path> all_paths{ get_all_files() };
        for (auto& path : all_paths)
        {
            std::string file_name{ path.string() };
            Adjacency_Matrix graph_mat;
            init_graph(graph_mat, file_name);
            std::cout << "file :" << file_name << '\n';
            const double min_result{ ant_colony_algorithm_SOP(graph_mat.size(), 200, graph_mat) };
            file << file_name << ',' << min_result << ',' << minimums[file_name] << ',' << min_result / minimums[file_name] << '\n';
        }

        file.close();
    }
}

void genetic_algorithm(const int to_do)
{
    if (to_do == 0)
    {
        std::string file_name{ file_menu().string() };
        Adjacency_Matrix graph_mat;
        init_graph(graph_mat, file_name);
        std::cout << "file :" << file_name << '\n';
        genetic_algorithm_SOP(graph_mat.size(), 1000, graph_mat);
    }
    else if (to_do == 1)
    {
        std::string file_name{ file_menu().string() };
        Adjacency_Matrix graph_mat;
        init_graph(graph_mat, file_name);
        std::cout << "file :" << file_name << '\n';

        std::vector<double> average_polling{ std::vector<double>(GLB_GA_iter + 1, 0.0) };
        std::vector<double> minimum_polling{ std::vector<double>(GLB_GA_iter + 1, 0.0) };

        for(int i{ 0 }; i < 10; ++i)
        {
            loading_screen(static_cast<float>(i) * 10.0);
            genetic_algorithm_SOP(GLB_GA_pop, GLB_GA_iter, graph_mat, average_polling, minimum_polling);
        }

        for (auto& n : average_polling)
            n /= 10.0;
        for (auto& n : minimum_polling)
            n /= 10.0;

        make_csv(average_polling, minimum_polling, "GA" + std::to_string(GLB_GA_pop) + "_" + std::to_string(GLB_GA_iter));
    }
    else if (to_do == 2)
    {
        std::ofstream file("ratios_ga.csv");

        file << "Name,Found Min,Given Min,Ratio\n";

        std::unordered_map<std::string, double> minimums;
        get_harcoded_map(minimums);
        std::vector <std::filesystem::path> all_paths{ get_all_files() };
        for (auto& path : all_paths)
        {
            std::string file_name{ path.string() };
            Adjacency_Matrix graph_mat;
            init_graph(graph_mat, file_name);
            std::cout << "file :" << file_name << '\n';
            const double min_result{ genetic_algorithm_SOP(graph_mat.size(), 200, graph_mat) };
            file << file_name << ',' << min_result << ',' << minimums[file_name] << ',' << min_result / minimums[file_name] << '\n';
        }

        file.close();
    }
}