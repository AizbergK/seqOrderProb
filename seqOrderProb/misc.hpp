#ifndef MISC_HPP
#define MISC_HPP

#define _WIN32_WINNT 0x0A00
#include <Windows.h>
#include <conio.h>
import std;

using Chromosome = std::vector<int>;
using Ant_Path = std::vector<int>;
using Adjacency_Matrix = std::vector<std::vector<int>>;
using Pheromone_Matrix = std::vector<std::vector<double>>;
using Opt_Vect = std::optional<std::reference_wrapper<std::vector<double>>>;

//Forward Decl
std::filesystem::path file_menu();
std::vector <std::filesystem::path> get_all_files();
void init_console();
void show_menu(std::vector<std::string>&, int);
void mod_menu(std::vector<std::string>&, int, int);
void loading_screen(int percent);
void init_graph(Adjacency_Matrix&, std::string&);
void make_csv(std::vector<double>&, std::vector<double>&, std::string);
void get_harcoded_map(std::unordered_map<std::string, double>&);
std::vector<std::unordered_set<int>> get_precedence(const Adjacency_Matrix&);
double get_average(const std::vector<std::vector<int>>&, const Adjacency_Matrix&);
int hamiltonian_road_sum(const std::vector<int>&, const Adjacency_Matrix&);
double rand_percent();


//Func Defs
std::filesystem::path file_menu()
{
    init_console();

    std::filesystem::path exePath = std::filesystem::current_path().parent_path().parent_path() / "testGraphs";
    std::string folderPath = exePath.string();
    std::vector<std::string> fileNames;

    for (const auto& entry : std::filesystem::directory_iterator(folderPath))
    {
        if (entry.is_regular_file()) {
            fileNames.push_back(entry.path().filename().string());
        }
    }

    int selected{ 0 }, previous{ 0 };
    show_menu(fileNames, selected);

    while (true)
    {
        int arrow;
        int ch = _getch();
        if (ch == 0 || ch == 224)
        {
            arrow = _getch();
            previous = selected;
            if (arrow == 72) // Up arrow
            {
                --selected;
                if (selected < 0)
                    selected = static_cast<int>(fileNames.size()) - 1; // Explicit cast to int
            }
            if (arrow == 80) // Down arrow
            {
                ++selected;
                if (selected == static_cast<int>(fileNames.size())) // Explicit cast to int
                    selected = 0;
            }
        }
        mod_menu(fileNames, selected, previous);

        if (ch == 13) // Enter key
        {
            system("cls");
            return exePath / fileNames[selected];
        }
    }
}

std::vector <std::filesystem::path> get_all_files()
{
    std::vector <std::filesystem::path> all_files;
    std::filesystem::path exePath = std::filesystem::current_path().parent_path().parent_path() / "testGraphs";
    std::string folderPath = exePath.string();
    std::vector<std::string> fileNames;

    for (const auto& entry : std::filesystem::directory_iterator(folderPath))
    {
        if (entry.is_regular_file()) {
            fileNames.push_back(entry.path().filename().string());
        }
    }

    for (const auto& file : fileNames)
        all_files.emplace_back(exePath / file);

    return all_files;
}

void init_console()
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(out, &cursorInfo);

    HWND hwnd = GetWindow(GetConsoleWindow(), GW_OWNER); // Windows 11
    if (hwnd == NULL)
        hwnd = GetConsoleWindow(); // Windows 10

    MoveWindow(hwnd, 100, 100, 1200, 900, TRUE);
}

void show_menu(std::vector<std::string>& options, int selected)
{
    system("cls");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
    std::cout << "\33[2K\r" << "> " << options[0] << std::endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

    for (int i = 1; i < options.size(); ++i)
        std::cout << "  " << options[i] << std::endl;

    COORD default_coord{ 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), default_coord);
}

void mod_menu(std::vector<std::string>& options, int selected, int to_revert)
{
    COORD coord_selected{ 0, static_cast<SHORT>(selected) };
    COORD coord_to_revert{ 0, static_cast<SHORT>(to_revert) };
    COORD default_coord{ 0, 0 };

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord_selected);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
    std::cout << "\33[2K\r" << "> " << options[selected] << std::endl;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord_to_revert);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "\33[2K\r" << "  " << options[to_revert] << std::endl;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), default_coord);
}

void loading_screen(int percent)
{
    COORD default_coord{ 0, 1 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), default_coord);
    std::cout << "\33[2K\r" << "> " << percent << "%" << std::endl;
}

void init_graph(Adjacency_Matrix& graph_mat, std::string& file_name)
{
    Adjacency_Matrix graph;
    std::ifstream ipt(file_name); //select one of the graph.sop files
    file_name = file_name.substr(file_name.find_last_of("/\\") + 1);
    std::string line;
    for (size_t i = 0; i < 8; i++)
        std::getline(ipt, line);

    for (std::getline(ipt, line); line != "EOF"; std::getline(ipt, line))
    {
        graph.emplace_back(std::vector<int>());
        int dist;
        std::stringstream line_ipt(line);
        while (line_ipt >> dist)
            graph.rbegin()->emplace_back(dist);
    }

    graph_mat = graph;
}

void make_csv(std::vector<double>& average_polling, std::vector<double>& minimum_polling, std::string file_name)
{
    std::ofstream file(file_name);

    file << "Average,Minimum\n";
    for (int i{ 0 }; i < average_polling.size(); ++i)
    {
        file << average_polling[i] << ',' << minimum_polling[i] << '\n';
    }

    file.close();
}

void get_harcoded_map(std::unordered_map<std::string, double>& minimums)
{
    minimums["br17.10.sop"] = 55.0;
    minimums["br17.12.sop"] = 55.0;

    minimums["ESC07.sop"] = 2125.0;
    minimums["ESC11.sop"] = 2075.0;
    minimums["ESC12.sop"] = 1675.0;
    minimums["ESC25.sop"] = 1681.0;
    minimums["ESC47.sop"] = 1288.0;
    minimums["ESC63.sop"] = 62.0;
    minimums["ESC78.sop"] = 18230.0;

    minimums["ft53.1.sop"] = 7438.0;
    minimums["ft53.2.sop"] = 7630.0;
    minimums["ft53.3.sop"] = 9473.0;
    minimums["ft53.4.sop"] = 14425.0;

    minimums["ft70.1.sop"] = 39313.0;
    minimums["ft70.2.sop"] = 39739.0;
    minimums["ft70.3.sop"] = 41305.0;
    minimums["ft70.4.sop"] = 52269.0;

    minimums["kro124p.1.sop"] = 37722.0;
    minimums["kro124p.2.sop"] = 38534.0;
    minimums["kro124p.3.sop"] = 40967.0;
    minimums["kro124p.4.sop"] = 64858.0;

    minimums["p43.1.sop"] = 27990.0;
    minimums["p43.2.sop"] = 28175.0;
    minimums["p43.3.sop"] = 28366.0;
    minimums["p43.4.sop"] = 69569.0;

    minimums["prob.42.sop"] = 243.0;
    minimums["prob.100.sop"] = 1024.0;

    minimums["rbg048a.sop"] = 351.0;
    minimums["rbg050c.sop"] = 467.0;
    minimums["rbg109a.sop"] = 1038.0;
    minimums["rbg150a.sop"] = 1748.0;
    minimums["rbg174a.sop"] = 2053.0;
    minimums["rbg253a.sop"] = 2928.0;
    minimums["rbg323a.sop"] = 3136.0;
    minimums["rbg341a.sop"] = 2543.0;
    minimums["rbg358a.sop"] = 2518.0;
    minimums["rbg378a.sop"] = 2761.0;

    minimums["ry48p.1.sop"] = 15220.0;
    minimums["ry48p.2.sop"] = 15524.0;
    minimums["ry48p.3.sop"] = 18156.0;
    minimums["ry48p.4.sop"] = 29967.0;
}

std::vector<std::unordered_set<int>> get_precedence(const Adjacency_Matrix& graph_mat)
{
    std::vector<std::unordered_set<int>> precedence_rules{ std::vector<std::unordered_set<int>>(graph_mat.size()) };
    for (int i{ 0 }; i < graph_mat.size(); ++i)
    {
        for (int j{ 0 }; j < graph_mat[0].size(); ++j)
        {
            if (graph_mat[i][j] == -1)
                precedence_rules[i].insert(j);
        }
    }

    return precedence_rules;
}

double get_average(const std::vector<Ant_Path>& ant_paths, const Adjacency_Matrix& graph_mat)
{
    std::vector<double> ant_paths_sums;
    for (int i{ 0 }; i < ant_paths.size(); ++i)
        ant_paths_sums.emplace_back(hamiltonian_road_sum(ant_paths[i], graph_mat));

    return (std::accumulate(ant_paths_sums.begin(), ant_paths_sums.end(), 0.0) / static_cast<double>(ant_paths_sums.size()));
}

int hamiltonian_road_sum(const std::vector<int>& path, const Adjacency_Matrix& graph_mat)
{
    int sum{ 0 };
    for (int i{ 0 }; i < path.size() - 1; ++i)
        sum += graph_mat[path[i]][path[i + 1]];

    return sum;
}

double rand_percent()
{
    return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
}

#endif