#include <bits/stdc++.h>
using namespace std;

/*
Estructuras: 
    - IntanceNodes: info de los nodos
    - UserData: info específica de cada usuario
*/
struct InstanceNodes {
    int N;
    vector<double> service;
    vector<double> tw_start;
    vector<double> tw_end;
    vector<vector<double>> dist;
};

struct UserData {
    double T;
    vector<double> nodeVal;
    vector<vector<double>> arcVal;
};

// Lee todos los números del archivo (ignora datos no númericos)
vector<double> read_numbers(const string &path) {
    ifstream f(path);
    vector<double> nums;

    if (!f) {
        cerr << "Error abriendo archivo: " << path << "\n";
        return nums;
    }

    string token;
    while (f >> token) {
        try {
            nums.push_back(stod(token));
        } catch (...) {
            // Token no es número, se ignora
        }
    }
    return nums;
}

// Parser de archivo nodos
InstanceNodes parse_nodes(const string &path) {
    auto nums = read_numbers(path);
    InstanceNodes inst;

    if (nums.empty()) {
        inst.N = 0;
        return inst;
    }

    size_t idx = 0;

    // Número de nodos
    inst.N = (int)nums[idx++];
    int N = inst.N;

    // Tiempo de servicio por nodo (service[i])
    inst.service.resize(N);
    for (int i = 0; i < N; i++)
        inst.service[i] = nums[idx++];

    // Ventanas de tiempo: inicio (tw_start) y fin (tw_end)
    inst.tw_start.resize(N);
    inst.tw_end.resize(N);

    for (int i = 0; i < N; i++)
        inst.tw_start[i] = nums[idx++];
    for (int i = 0; i < N; i++)
        inst.tw_end[i] = nums[idx++];

    // Matriz de distancias (dist[i][j])
    inst.dist.assign(N, vector<double>(N, 0));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            inst.dist[i][j] = nums[idx++];

    return inst;
}

// Parser archivo de usuarios
vector<UserData> parse_users(const string &path, int N) {
    auto nums = read_numbers(path);
    vector<UserData> users;

    if (nums.empty())
        return users;

    size_t idx = 0;
    int U = (int)nums[idx++];

    for (int u = 0; u < U; u++) {
        if (idx >= nums.size())
            break;

        UserData ud;
        
        // Tiempo máximo disponible (T)
        ud.T = nums[idx++];

        // Valores en nodos (nodeVal)
        ud.nodeVal.resize(N);
        for (int i = 0; i < N; i++)
            ud.nodeVal[i] = nums[idx++];

        // Valores en arcos (arcVal)
        ud.arcVal.assign(N, vector<double>(N, 0));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                ud.arcVal[i][j] = nums[idx++];

        users.push_back(move(ud));
    }

    return users;
}

// Resultado de una ruta
struct TourResult {
    double value;
    double timeUsed;
    vector<int> tour;
};


/*
Función de validación: 
    - is_feasible_tour: valida si la ruta cumple con todas las restricciones

Restricciones Verificadas:
    - Ventanas de tiempo: cada nodo es visitado dentro de su ventana [tw_start, tw_end)
    - Tiempo total: no excede el tiempo máximo disponible T
    - Ruta válida: empieza y termina en nodo 0

Retorno:
    - true si la ruta es factible
    - false si viola alguna restricción
*/
bool is_feasible_tour(const InstanceNodes &inst, const UserData &ud, const vector<int> &tour) {

    // RESTRICCIÓN: la ruta debe comenzar en el nodo 0
    if (tour.empty() || tour[0] != 0)
        return false;

    int N = inst.N;
    double curTime = 0;

    // Se recorre secuencia de ruta, verificando ventanas y tiempos
    for (int i = 0; i < (int)tour.size(); i++) {
        int cur = tour[i];
        
        // RESTRICCIÓN: nodo válido
        if (cur < 0 || cur >= N)
            return false;

        if (i > 0) {
            int prev = tour[i-1];
            // Tiempo de viaje desde nodo anterior
            double travel = inst.dist[prev][cur];
            double arrive = curTime + travel;
            
            // RESTRICCIÓN VENTANA: se espera hasta tw_start si llega antes
            double start = max(arrive, inst.tw_start[cur]);
            
            // RESTRICCIÓN VENTANA: tiempo de finalización en cur 
            double finish = start + inst.service[cur];

            // RESTRICCIÓN VENTANA: no se puede terminar después de tw_end
            if (finish > inst.tw_end[cur])
                return false; // Viola ventana de tiempo
            
            curTime = finish;
        }
    }

    // RESTRICCIÓN: tiempo total incluyendo retorno al nodo 0, no exceder ud.T
    int last = tour.back();
    double totalTime = curTime + inst.dist[last][0];
    
    // RESTRICCIÓN TIEMPO: no exceder tiempo máximo disponible
    if (totalTime > ud.T)
        return false; // Viola límite de tiempo

    // Solución factible
    return true;
}

/*
Función de evaluación:
    - evaluate_tour: calcula el valor total de una ruta factible 
    Se asume que tour[0] está incluido
*/
double evaluate_tour(const UserData &ud, const vector<int> &tour) {
    double value = ud.nodeVal[0]; // valor del nodo inicial

    for (int i = 1; i < (int)tour.size(); i++) {
        int prev = tour[i-1];
        int cur = tour[i];
        value += ud.nodeVal[cur] + ud.arcVal[prev][cur];
    }

    return value;
}

/*
Función de calculo de tiempo:
    - calculate_tour_time: calcula el tiempo total de una ruta factible, incluyendo el retorno al nodo 0
*/
double calculate_tour_time(const InstanceNodes &inst, const vector<int> &tour) {
    if (tour.empty() || tour[0] != 0)
        return 1e9;

    double curTime = 0;
    for (int i = 0; i < (int)tour.size(); i++) {
        int cur = tour[i];
        if (i > 0) {
            int prev = tour[i-1];
            double travel = inst.dist[prev][cur];
            double arrive = curTime + travel;
            double start = max(arrive, inst.tw_start[cur]);
            curTime = start + inst.service[cur];
        }
    }

    // Se añade el tiempo de retorno al nodo 0
    curTime += inst.dist[tour.back()][0];
    return curTime;
}

/*
Algoritmo Greedy:
    - solve_greedy_one: construye una solución inicial usando greedy

Restricciones Verificadas:
    - Ventanas de tiempo: cada nodo es visitado dentro de su ventana (tw_start, tw_end)
    - Tiempo total: no excede el tiempo máximo disponible T

Criterios de Selección:
    - score = (nodeVal[j] + arcVal[cur][j]) / tiempo_incrementado
    - Selecciona el nodo con mayor score (mejor valor por tiempo usado)
*/
TourResult solve_greedy_one(const InstanceNodes &inst, const UserData &ud) {
    int N = inst.N;
    vector<char> visited(N, 0);

    vector<int> tour;
    int cur = 0;
    visited[0] = 1;
    tour.push_back(0);

    double curTime = 0;
    double totalValue = ud.nodeVal[0];

    while (true) {
        int best = -1;
        double bestScore = -1e100;

        // Evaluar todos los nodos no visitados
        for (int j = 1; j < N; j++) {
            if (visited[j]) continue;

            // RESTRICCIÓN VENTANA: ir de cur a j
            double travel = inst.dist[cur][j];
            double arrive = curTime + travel;
            double start = max(arrive, inst.tw_start[j]);
            double finish = start + inst.service[j];
            
            // RESTRICCIÓN VENTANA: si finish excede ventana de tiempo, descartar
            if (finish > inst.tw_end[j]) continue;

            // RESTRICCIÓN TIEMPO: no exceder tiempo máximo disponible T
            double back = inst.dist[j][0];

            // RESTRICCIÓN TIEMPO: si tiempo total excede límite, descartar
            if (finish + back > ud.T) continue;

            // Calculo de score greedy
            double incTime = finish - curTime;
            double incValue = ud.nodeVal[j] + ud.arcVal[cur][j];
            double score = incValue / max(1e-6, incTime);

            // Seleccionar nodo con mejor score
            if (score > bestScore) {
                bestScore = score;
                best = j;
            }
        }

        if (best == -1)
            break; // No hay más nodos factibles

        // Se agrega nodo seleccionado a la ruta y se actualizan tiempos y valores
        double travel = inst.dist[cur][best];
        double arrive = curTime + travel;
        double start = max(arrive, inst.tw_start[best]);
        double finish = start + inst.service[best];

        totalValue += ud.nodeVal[best] + ud.arcVal[cur][best];
        curTime = finish;
        cur = best;
        visited[cur] = 1;
        tour.push_back(cur);
    }

    // Tiempo total incluyendo retorno al nodo 0
    curTime += inst.dist[cur][0];

    return {totalValue, curTime, tour};
}

/*
Algoritmo Simulated Annealing:
    - solve_with_sa: mejora una solución inicial usando Simulated Annealing

Vecindarios (Movimientos):
    - 2-opt: revertir subsecuencias de la ruta
    - Reinserción: mover un nodo de una posición a otra

Aceptación de Solución:
    - Siempre acepta soluciones mejores o iguales
    - Acepta soluciones peores con probabilidad exp((nueva - vieja) / T)
*/

// Movimiento 2-opt: revertir una subsecuencia de la ruta
vector<int> apply_2opt(const vector<int> &tour, int i, int j) {
    vector<int> new_tour = tour;
    reverse(new_tour.begin() + i, new_tour.begin() + j + 1);
    return new_tour;
}

// Movimiento de reinserción: mover un nodo de remove_pos a insert_pos
vector<int> apply_reinsertion(const vector<int> &tour, int remove_pos, int insert_pos) {
    vector<int> new_tour = tour;
    int node = new_tour[remove_pos];
    new_tour.erase(new_tour.begin() + remove_pos);
    
    if (insert_pos > remove_pos)
        insert_pos--;
    
    new_tour.insert(new_tour.begin() + insert_pos, node);
    return new_tour;
}

// Simulated Annealing
TourResult solve_with_sa(const InstanceNodes &inst, const UserData &ud, TourResult initial) {
    // Parámetros de Simulated Annealing
    double T_initial = 100.0;     
    double T_final = 0.01;        
    double cooling_rate = 0.95;   
    int iters_per_temp = 50;     

    TourResult best = initial;
    TourResult current = initial;
    double temperature = T_initial;

    // Si la solución inicial no es factible, devolver la mejor factible conocida
    if (!is_feasible_tour(inst, ud, current.tour)) {
        vector<int> trivial = {0};
        if (is_feasible_tour(inst, ud, trivial)) {
            current.tour = trivial;
            current.value = evaluate_tour(ud, trivial);
            current.timeUsed = calculate_tour_time(inst, trivial);
            best = current;
        } else {
            // No hay solución factible
            return {0.0, 1e9, {0}};
        }
    }


    // Parte principal de Simulated Annealing
    while (temperature > T_final) {
        for (int iter = 0; iter < iters_per_temp; iter++) {
            // Vecinos
            vector<int> neighbor;
            bool use_2opt = rand() % 2 == 0;

            if (use_2opt && current.tour.size() > 3) {
                // Movimiento 2-opt
                int i = 1 + rand() % (current.tour.size() - 2);
                int j = i + 1 + rand() % (current.tour.size() - i - 1);
                neighbor = apply_2opt(current.tour, i, j);
            } else if (current.tour.size() > 2) {
                // Movimiento de reinserción
                int remove_pos = 1 + rand() % (current.tour.size() - 1);
                int insert_pos = 1 + rand() % (current.tour.size() - 1);
                neighbor = apply_reinsertion(current.tour, remove_pos, insert_pos);
            } else {
                continue;
            }

            // Se verifica que se cumplan todas las restricciones, si la función falla, se descarta el vecino
            if (!is_feasible_tour(inst, ud, neighbor))
                continue; // Descartar vecino no factible

            // Si es factible, evaluar valor y tiempo
            double neighbor_value = evaluate_tour(ud, neighbor);
            double neighbor_time = calculate_tour_time(inst, neighbor);

            // Criterio de aceptación
            double delta = neighbor_value - current.value;
            
            bool accept = false;
            if (delta >= 0) {
                accept = true; // Solución mejor o igual 
            } else {
                double prob = exp(delta / temperature);
                accept = ((double)rand() / RAND_MAX) < prob;
            }

            if (accept) {
                current.value = neighbor_value;
                current.timeUsed = neighbor_time;
                current.tour = neighbor;

                // Actualizar solución  
                if (current.value > best.value) {
                    best = current;
                }
            }
        }

        // Enfriamiento
        temperature *= cooling_rate;
    }

    return best;
}

/*
Función principal:
    - main: orquesta la resolución del problema usando GREEDY + SIMULATED ANNEALING

Flujo: 
    - Leer archivo de instancia de nodos
    - Leer archivo de datos de usuarios
    - Para cada usuario:
       a) Generar solución inicial con GREEDY
       b) Mejorar con SIMULATED ANNEALING
       c) Escribir resultado al archivo de salida

*/
int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "Uso: solver <archivo_nodos> <archivo_usuarios>\n";
        return 1;
    }

    string nodes_file = argv[1];
    string users_file = argv[2];

    // Lectura instancia nodos
    InstanceNodes inst = parse_nodes(nodes_file);
    if (inst.N == 0) {
        cout << "Error leyendo la instancia.\n";
        return 1;
    }

    // Lectura datos usuarios
    auto users = parse_users(users_file, inst.N);
    if (users.empty()) {
        cout << "Error leyendo usuarios.\n";
        return 1;
    }

    // Generar nombre archivo de salida
    string base = nodes_file.substr(nodes_file.find_last_of("/\\") + 1);
    if (base.size() > 4 && base.substr(base.size()-4) == ".txt")
        base = base.substr(0, base.size()-4);

    string outname = "salida_" + base + ".txt";
    ofstream out(outname);

    // Resolver para cada usuario
    for (auto &ud : users) {
        // Construcción Greedy,se filtran candidatos que violan ventanas y tiempo
        TourResult greedy_result = solve_greedy_one(inst, ud);

        // Mejorar con Simulated Annealing, solo se aceptan vecinos factibles
        TourResult final_result = solve_with_sa(inst, ud, greedy_result);

        // Escribir resultado al archivo
        out << final_result.value << "\n";
        out << ud.T << " " << final_result.timeUsed << "\n";

        for (int i = 0; i < (int)final_result.tour.size(); i++) {
            out << (final_result.tour[i] + 1);
            if (i + 1 < (int)final_result.tour.size())
                out << " ";
        }
        out << "\n";
    }

    cout << "Archivo " << outname << " generado correctamente.\n";
    return 0;
}
