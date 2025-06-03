#ifndef SWARM_HPP
#define SWARM_HPP

#include "misc.hpp"

//Global Vars
double alpha{ 1.0 }, beta{ 2.0 }, rho{ 0.9 }, Q, pher_min, pher_max;

//Forward Decl
double ant_colony_algorithm_SOP(const int , const int , const Adjacency_Matrix& , Opt_Vect  = std::nullopt, Opt_Vect  = std::nullopt);
std::vector<Ant_Path> get_ant_paths(const std::vector<std::unordered_set<int>>&, Pheromone_Matrix&, const int, const Adjacency_Matrix&);
Ant_Path generate_ant_path(std::vector<std::unordered_set<int>>, Pheromone_Matrix&, const Adjacency_Matrix&);
Ant_Path update_global_best(const std::vector<Ant_Path>&, Ant_Path&, const Adjacency_Matrix&);
void update_pheromones(const std::vector<Ant_Path>&, Pheromone_Matrix&, const Adjacency_Matrix&);
Ant_Path get_best_road(std::vector<std::unordered_set<int>>, const Pheromone_Matrix&);
std::vector<double> get_roulette_values(const int, const std::vector<int>&, const Pheromone_Matrix&, const Adjacency_Matrix&);


//Func Defs
double ant_colony_algorithm_SOP(const int ant_count, const int generation_count, const Adjacency_Matrix& graph_mat, Opt_Vect average_polling_opt, Opt_Vect minimum_polling_opt)
{
    const int graph_node_count{ static_cast<int>(graph_mat.size()) };
    std::vector<std::unordered_set<int>> precedence_rules{ get_precedence(graph_mat) };
    double total_len{ 0.0 };
    int count{ 0 };
    for (auto& row : graph_mat)
        for (auto edge : row)
            if (edge != -1 && edge != 0 && edge != 1000000)
                total_len += edge, ++count;
    double average_len{ total_len / count }, base_pher{ 1.0 / average_len };
    Pheromone_Matrix pheromone_matrix{ std::vector<std::vector<double>>(ant_count, std::vector<double>(ant_count, base_pher)) };
    Q = average_len * graph_node_count;
    pher_min = base_pher / 25.0;
    pher_max = base_pher * 10.0;

    int min_road{ INT_MAX };
    double average_road{ INT_MAX };
    Ant_Path global_best{ generate_ant_path(precedence_rules, pheromone_matrix, graph_mat) };
    for (int i{ 1 }; i <= generation_count; ++i)
    {
        if (!average_polling_opt)
            loading_screen(static_cast<float>(i) / generation_count * 100.0);
        std::vector<Ant_Path> ant_paths{ get_ant_paths(precedence_rules, pheromone_matrix, ant_count, graph_mat) };

        global_best = update_global_best(ant_paths, global_best, graph_mat);
        min_road = min(min_road, hamiltonian_road_sum(global_best, graph_mat));
        average_road = get_average(ant_paths, graph_mat);

        update_pheromones(ant_paths, pheromone_matrix, graph_mat);

        if (average_polling_opt)
        {
            average_polling_opt->get()[i] += average_road;
            minimum_polling_opt->get()[i] += min_road;
        }
    }
    min_road = min(min_road, hamiltonian_road_sum(global_best, graph_mat));
    min_road = min(min_road, hamiltonian_road_sum(get_best_road(precedence_rules, pheromone_matrix), graph_mat));
    if (!average_polling_opt)
        std::cout << "Lowest hamiltonian cycle is: " << min_road << std::endl;

    return min_road;
}

std::vector<Ant_Path> get_ant_paths(const std::vector<std::unordered_set<int>>& precedence_rules, Pheromone_Matrix& pheromone_matrix, const int ant_paths_count, const Adjacency_Matrix& graph_mat)
{
    std::vector<Ant_Path> ant_paths;

    for (int i{ 0 }; i < ant_paths_count; ++i)
    {
        ant_paths.emplace_back(generate_ant_path(precedence_rules, pheromone_matrix, graph_mat));
    }

    return ant_paths;
}

Ant_Path generate_ant_path(std::vector<std::unordered_set<int>> precedence_rules, Pheromone_Matrix& pheromone_matrix, const Adjacency_Matrix& graph_mat)
{
    Ant_Path ant_path;
    std::vector<bool> used_node(precedence_rules.size(), false);

    {
        std::set<int> avail_nodes;
        for (int i{ 0 }; i < precedence_rules.size(); ++i)
            if (precedence_rules[i].empty() && !used_node[i]) avail_nodes.insert(i);
        auto node_to_insert{ std::next(avail_nodes.begin() ,rand() % static_cast<int>(avail_nodes.size())) };
        ant_path.emplace_back(*node_to_insert);
        used_node[(*node_to_insert)] = true;
        for (auto& node : precedence_rules)
            if (node.find(*node_to_insert) != node.end()) node.erase(*node_to_insert);
    }

    while (ant_path.size() < precedence_rules.size())
    {
        std::vector<int> avail_nodes;
        for (int i{ 0 }; i < precedence_rules.size(); ++i)
            if (precedence_rules[i].empty() && !used_node[i]) avail_nodes.emplace_back(i);

        std::vector<double> selection_prob{ get_roulette_values(*ant_path.rbegin(), avail_nodes, pheromone_matrix, graph_mat) };

        const double rand_sel{ rand_percent() };
        int pos;
        for (int i{ 0 }; i < selection_prob.size() - 1; ++i)
        {
            if (rand_sel >= selection_prob[i] && rand_sel < selection_prob[i + 1])
            {
                pos = i;
                break;
            }
        }
        auto node_to_insert{ avail_nodes.begin() + pos };
        ant_path.emplace_back(*node_to_insert);
        used_node[(*node_to_insert)] = true;
        for (auto& node : precedence_rules)
            if (node.find(*node_to_insert) != node.end()) node.erase(*node_to_insert);
    }

    return ant_path;
}

Ant_Path update_global_best(const std::vector<Ant_Path>& ant_paths, Ant_Path& global_best, const Adjacency_Matrix& graph_mat)
{
    Ant_Path best_path;
    int best_path_value{ hamiltonian_road_sum(global_best, graph_mat) };
    int new_best{ -1 };
    for (int i{ 0 }; i < ant_paths.size(); ++i)
    {
        const int cur_path_value{ hamiltonian_road_sum(ant_paths[i], graph_mat) };
        if (cur_path_value < best_path_value)
        {
            best_path_value = cur_path_value;
            new_best = i;
        }
    }

    if (new_best != -1)
        best_path = ant_paths[new_best];
    else
        best_path = global_best;

    return best_path;
}

void update_pheromones(const std::vector<Ant_Path>& ant_paths, Pheromone_Matrix& pheromone_matrix, const Adjacency_Matrix& graph_mat)
{

    int min{ INT_MAX }, min_pos, mat_size{ static_cast<int>(pheromone_matrix.size()) };
    Pheromone_Matrix temp_pheromone_matrix{ std::vector<std::vector<double>>(mat_size, std::vector<double>(mat_size,0.0)) };
    for (auto& path : ant_paths)
    {
        const int path_len{ hamiltonian_road_sum(path, graph_mat) };
        for (int i{ 0 }; i < path.size() - 1; ++i)
        {
            temp_pheromone_matrix[path[i]][path[i + 1]] += Q / path_len;
        }
    }

    for (int i{ 0 }; i < mat_size; ++i)
        for (int j{ 0 }; j < mat_size; ++j)
        {
            const double new_val{ rho * pheromone_matrix[i][j] + temp_pheromone_matrix[i][j] };
            if (new_val < pher_min)
                pheromone_matrix[i][j] = pher_min;
            else if (new_val > pher_max)
                pheromone_matrix[i][j] = pher_max;
            else
                pheromone_matrix[i][j] = new_val;
        }

}

Ant_Path get_best_road(std::vector<std::unordered_set<int>> precedence_rules, const Pheromone_Matrix& pheromone_matrix)
{
    Ant_Path ant_path;
    std::vector<bool> used_node(precedence_rules.size(), false);

    {
        std::set<int> avail_nodes;
        for (int i{ 0 }; i < precedence_rules.size(); ++i)
            if (precedence_rules[i].empty() && !used_node[i]) avail_nodes.insert(i);
        auto node_to_insert{ std::next(avail_nodes.begin() ,rand() % static_cast<int>(avail_nodes.size())) };
        ant_path.emplace_back(*node_to_insert);
        used_node[(*node_to_insert)] = true;
        for (auto& node : precedence_rules)
            if (node.find(*node_to_insert) != node.end()) node.erase(*node_to_insert);
    }

    while (ant_path.size() < precedence_rules.size())
    {
        std::vector<int> avail_nodes;
        for (int i{ 0 }; i < precedence_rules.size(); ++i)
            if (precedence_rules[i].empty() && !used_node[i]) avail_nodes.emplace_back(i);
        float pher_max{ -1.0f };
        int pos{ 0 };
        for (int i{ 0 }; i < avail_nodes.size(); ++i)
            if (pheromone_matrix[*ant_path.rbegin()][avail_nodes[i]] > pher_max)
            {
                pher_max = pheromone_matrix[*ant_path.rbegin()][avail_nodes[i]];
                pos = i;
            }

        auto node_to_insert{ avail_nodes.begin() + pos };
        ant_path.emplace_back(*node_to_insert);
        used_node[(*node_to_insert)] = true;
        for (auto& node : precedence_rules)
            if (node.find(*node_to_insert) != node.end()) node.erase(*node_to_insert);
    }

    return ant_path;
}

std::vector<double> get_roulette_values(const int last_node, const std::vector<int>& avail_nodes, const Pheromone_Matrix& pheromone_matrix, const Adjacency_Matrix& graph_mat)
{
    std::vector<double> arcs_pheromone;
    for (int i{ 0 }; i < avail_nodes.size(); ++i)
    {
        const double first{ std::pow(pheromone_matrix[last_node][avail_nodes[i]], alpha) };

        double second;
        if (graph_mat[last_node][avail_nodes[i]] == 0)
            second = 1.0;
        else
            second = 1.0 / static_cast<double>(graph_mat[last_node][avail_nodes[i]]);

        const double third{ std::pow(second, beta) };
        const double fourth{ first * third };
        arcs_pheromone.emplace_back(fourth);
    }

    double total_sum{ std::accumulate(arcs_pheromone.begin(), arcs_pheromone.end(), 0.0) };
    std::vector<double> selection_probabilities{ 0.0f };
    for (int i{ 0 }; i < arcs_pheromone.size(); ++i)
        selection_probabilities.emplace_back(selection_probabilities[i] + arcs_pheromone[i] / total_sum);
    *selection_probabilities.begin() = -0.1;
    *selection_probabilities.rbegin() = 1.1;

    return selection_probabilities;
}

#endif