#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "misc.hpp"

//Forward Decl
double genetic_algorithm_SOP(const int, const int, const Adjacency_Matrix&, Opt_Vect = std::nullopt, Opt_Vect = std::nullopt);
bool correct_precendence(const Chromosome&, const std::vector<std::unordered_set<int>>&);
bool is_add_node_correct(const Chromosome&, int, const std::vector<std::unordered_set<int>>&, std::vector<int>&);
Chromosome chromosome_mutation(Chromosome&, const std::vector<std::unordered_set<int>>&);
int get_following_node(Chromosome&, int);
Chromosome chromosome_crossover(const std::vector<Chromosome>&, const std::vector<double>&, const std::vector<std::unordered_set<int>>&, const Adjacency_Matrix&);
Chromosome select_chromosome(const std::vector<Chromosome>&, const std::vector<double>&);
std::vector<double> get_roulette_values(const std::vector<Chromosome>&, const Adjacency_Matrix&);
std::vector<double> cost_function(const std::vector<Chromosome>&, const Adjacency_Matrix&);
std::vector<Chromosome> get_chromosomes(const std::vector<std::unordered_set<int>>&, const int);
Chromosome generate_chromosome(std::vector<std::unordered_set<int>>);
int get_min_road(const std::vector<Chromosome>&, const Adjacency_Matrix&);
void get_average_number_of_runs_ga(Adjacency_Matrix&, std::string, const int, const int, const int);


//Func Defs
double genetic_algorithm_SOP(const int chromosome_count, const int generation_count, const Adjacency_Matrix& graph_mat, Opt_Vect average_polling_opt, Opt_Vect minimum_polling_opt)
{
    const double crossover_chance{ 0.6 }, mutation_chance{ 0.1 };
    const int node_count{ static_cast<int>(graph_mat.size()) };
    std::vector<std::unordered_set<int>> precedence_rules{ get_precedence(graph_mat) };
    std::vector<Chromosome> chromosomes{ get_chromosomes(precedence_rules, chromosome_count) };

    int min_road{ INT_MAX };
    double average_road{ INT_MAX }, percent_diff{ 1.0 };
    min_road = min(min_road, get_min_road(chromosomes, graph_mat));
    average_road = get_average(chromosomes, graph_mat);

    if (average_polling_opt)
    {
        average_polling_opt->get()[0] += average_road;
        minimum_polling_opt->get()[0] += min_road;
    }

    for (int i{ 1 }; i <= generation_count; ++i)
    {
        if (!average_polling_opt)
            loading_screen(static_cast<float>(i) / generation_count * 100.0);
        std::vector<Chromosome> selected_chromosomes;
        const std::vector<double> roulette_selection{ get_roulette_values(chromosomes, graph_mat) };

        while (selected_chromosomes.size() != chromosome_count)
        {
            Chromosome to_add;

            if (rand_percent() <= crossover_chance)
            {
                to_add = chromosome_crossover(chromosomes, roulette_selection, precedence_rules, graph_mat);
            }
            else
            {
                to_add = select_chromosome(chromosomes, roulette_selection);
            }

            if (rand_percent() <= mutation_chance)
            {
                to_add = chromosome_mutation(to_add, precedence_rules);
            }
            selected_chromosomes.emplace_back(to_add);
        }

        chromosomes = selected_chromosomes;

        min_road = min(min_road, get_min_road(chromosomes, graph_mat));
        average_road = get_average(chromosomes, graph_mat);

        if (average_polling_opt)
        {
            average_polling_opt->get()[i] += average_road;
            minimum_polling_opt->get()[i] += min_road;
        }
    }

    if (!average_polling_opt)
        std::cout << "Lowest hamiltonian cycle is: " << min_road << std::endl;

    return min_road;
}

bool correct_precendence(const Chromosome& chromosome, const std::vector<std::unordered_set<int>>& precedence_rules)
{
    for (int i{ static_cast<int>(chromosome.size()) - 2 }; i >= 0; --i)
    {
        for (int j{ i + 1 }; j < chromosome.size(); j++)
        {
            if (precedence_rules[chromosome[i]].find(chromosome[j]) != precedence_rules[chromosome[i]].end()) return false;
        }
    }

    return true;
}

bool is_add_node_correct(const Chromosome& child, int node, const std::vector<std::unordered_set<int>>& precedence_rules, std::vector<int>& unused_nodes)
{
    if (std::find(unused_nodes.begin(), unused_nodes.end(), node) == unused_nodes.end()) return false;
    if (precedence_rules[node].size() > child.size()) return false;
    int counter{ 0 };
    for (int i{ 0 }; i < child.size(); ++i)
    {
        if (precedence_rules[node].find(child[i]) != precedence_rules[node].end())
            ++counter;
    }
    if (counter == precedence_rules[node].size()) return true;

    return false;
}

Chromosome chromosome_mutation(Chromosome& chromosome, const std::vector<std::unordered_set<int>>& precedence_rules)
{
    Chromosome to_test{ chromosome };
    int rand_i{ rand() % static_cast<int>(chromosome.size()) };
    int rand_j{ rand() % static_cast<int>(chromosome.size()) };
    std::swap(to_test[rand_i], to_test[rand_j]);
    if (correct_precendence(to_test, precedence_rules))
        return to_test;
    else
        return chromosome;
}

Chromosome chromosome_crossover(const std::vector<Chromosome>& chromosomes, const std::vector<double>& roulette_selection, const std::vector<std::unordered_set<int>>& precedence_rules, const Adjacency_Matrix& graph_mat)
{
    Chromosome ch1 = select_chromosome(chromosomes, roulette_selection);
    Chromosome ch2 = select_chromosome(chromosomes, roulette_selection);
    Chromosome child{ 0 };
    const int idx{ 1 + rand() % static_cast<int>(ch1.size() - 1) };
    std::vector<int> unused_nodes(ch1.begin(), ch1.end());
    unused_nodes.erase(std::find(unused_nodes.begin(), unused_nodes.end(), 0));

    while (child.size() < ch1.size())
    {
        const int last_node{ *child.rbegin() };
        const std::pair<int, int> following_nodes{ get_following_node(ch1, last_node), get_following_node(ch2, last_node) };

        bool is_first_constrained{ is_add_node_correct(child, following_nodes.first, precedence_rules, unused_nodes) };
        bool is_second_constrained{ is_add_node_correct(child, following_nodes.second, precedence_rules, unused_nodes) };

        if (is_first_constrained && is_second_constrained)
        {
            if (graph_mat[last_node][following_nodes.first] < graph_mat[last_node][following_nodes.second] && is_first_constrained)
            {
                child.emplace_back(following_nodes.first);
                unused_nodes.erase(std::find(unused_nodes.begin(), unused_nodes.end(), following_nodes.first));
            }
            else if (graph_mat[last_node][following_nodes.first] > graph_mat[last_node][following_nodes.second] && is_second_constrained)
            {
                child.emplace_back(following_nodes.second);
                unused_nodes.erase(std::find(unused_nodes.begin(), unused_nodes.end(), following_nodes.second));
            }
            else
            {
                const double chance{ rand_percent() };
                if (chance < 0.5)
                {
                    child.emplace_back(following_nodes.first);
                    unused_nodes.erase(std::find(unused_nodes.begin(), unused_nodes.end(), following_nodes.first));
                }
                else
                {
                    child.emplace_back(following_nodes.second);
                    unused_nodes.erase(std::find(unused_nodes.begin(), unused_nodes.end(), following_nodes.second));
                }
            }
        }
        else if (is_first_constrained)
        {
            child.emplace_back(following_nodes.first);
            unused_nodes.erase(std::find(unused_nodes.begin(), unused_nodes.end(), following_nodes.first));
        }
        else if (is_second_constrained)
        {
            child.emplace_back(following_nodes.second);
            unused_nodes.erase(std::find(unused_nodes.begin(), unused_nodes.end(), following_nodes.second));
        }
        else
        {
            int min_dist{ INT_MAX }, follower;
            for (int i{ 0 }; i < unused_nodes.size(); ++i)
            {
                if (graph_mat[last_node][unused_nodes[i]] < min_dist && is_add_node_correct(child, unused_nodes[i], precedence_rules, unused_nodes))
                {
                    min_dist = graph_mat[last_node][unused_nodes[i]];
                    follower = unused_nodes[i];
                }
            }
            child.emplace_back(follower);
            unused_nodes.erase(std::find(unused_nodes.begin(), unused_nodes.end(), follower));
        }
    }

    if (!correct_precendence(child, precedence_rules))
    {
        std::cout << "HUGE ONE" << std::endl;
    }

    return child;
}

int get_following_node(Chromosome& chromosome, int node)
{
    auto it = std::find(chromosome.begin(), chromosome.end(), node) + 1;

    if (it != chromosome.end())
        return *it;

    return node;
}

Chromosome select_chromosome(const std::vector<Chromosome>& chromosomes, const std::vector<double>& roulette_selection)
{
    const double chance{ rand_percent() };
    for (int i{ 0 }; i < chromosomes.size(); ++i)
    {
        if (roulette_selection[i] <= chance && roulette_selection[i + 1] > chance)
        {
            return chromosomes[i];
        }
    }
}

std::vector<double> get_roulette_values(const std::vector<Chromosome>& chromosomes, const Adjacency_Matrix& graph_mat)
{
    std::vector<double> chromosome_cost{ cost_function(chromosomes, graph_mat) };
    double total_sum{ std::accumulate(chromosome_cost.begin(), chromosome_cost.end(), 0.0) };
    std::vector<double> selection_probabilities{ 0.0f };
    for (int i{ 0 }; i < chromosome_cost.size(); ++i)
        selection_probabilities.emplace_back(selection_probabilities[i] + chromosome_cost[i] / total_sum);
    *selection_probabilities.begin() = -0.1;
    *selection_probabilities.rbegin() = 1.1;

    return selection_probabilities;
}

std::vector<double> cost_function(const std::vector<Chromosome>& chromosomes, const Adjacency_Matrix& graph_mat)
{
    std::vector<double> hamiltonian_length;
    for (int i{ 0 }; i < chromosomes.size(); ++i)
        hamiltonian_length.emplace_back(static_cast<double>(hamiltonian_road_sum(chromosomes[i], graph_mat)));
    double min{ *std::min_element(hamiltonian_length.begin(), hamiltonian_length.end()) };
    double max{ *std::max_element(hamiltonian_length.begin(), hamiltonian_length.end()) };

    std::vector<double> chromosome_costs;
    for (auto length : hamiltonian_length)
    {
        double val{ min / length };
        chromosome_costs.emplace_back(val * val);
    }

    return chromosome_costs;
}

std::vector<Chromosome> get_chromosomes(const std::vector<std::unordered_set<int>>& precedence_rules, const int chromosome_count)
{
    std::vector<Chromosome> chromosomes;

    for (int i{ 0 }; i < chromosome_count; ++i)
    {
        chromosomes.emplace_back(generate_chromosome(precedence_rules));
    }

    return chromosomes;
}

Chromosome generate_chromosome(std::vector<std::unordered_set<int>> precedence_rules)
{
    Chromosome chromosome;
    std::vector<bool> used_node(precedence_rules.size(), false);
    while (chromosome.size() < precedence_rules.size())
    {
        std::set<int> avail_nodes;
        for (int i{ 0 }; i < precedence_rules.size(); ++i)
            if (precedence_rules[i].empty() && !used_node[i]) avail_nodes.insert(i);
        auto node_to_insert{ std::next(avail_nodes.begin() ,rand() % static_cast<int>(avail_nodes.size())) };
        chromosome.emplace_back(*node_to_insert);
        used_node[(*node_to_insert)] = true;
        for (auto& node : precedence_rules)
            if (node.find(*node_to_insert) != node.end()) node.erase(*node_to_insert);
    }

    return chromosome;
}

int get_min_road(const std::vector<Chromosome>& chromosomes, const Adjacency_Matrix& graph_mat)
{
    std::vector<int> chromosome_sums;
    for (int i{ 0 }; i < chromosomes.size(); ++i)
        chromosome_sums.emplace_back(hamiltonian_road_sum(chromosomes[i], graph_mat));

    return (*std::min_element(chromosome_sums.begin(), chromosome_sums.end()));
}

void get_average_number_of_runs_ga(Adjacency_Matrix& graph_mat, std::string file_name, const int chromosome_count, const int generation_count, const int no_runs)
{
    std::vector<double> average_polling(generation_count + 1, 0.0);
    std::vector<double> minimum_polling(generation_count + 1, 0.0);

    for (int i{ 0 }; i < no_runs; ++i)
        genetic_algorithm_SOP(chromosome_count, generation_count, graph_mat, average_polling, minimum_polling);

    std::string csv_file_name{ file_name + "_" + std::to_string(chromosome_count) + "_" + std::to_string(generation_count) + ".csv" };

    for (auto& val : average_polling) val /= no_runs;
    for (auto& val : minimum_polling) val /= no_runs;

    make_csv(average_polling, minimum_polling, csv_file_name);
}

#endif