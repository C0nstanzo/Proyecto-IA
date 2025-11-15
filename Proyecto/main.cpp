#include <bits/stdc++.h>
using namespace std;

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

// ---------------------------
// Lee todos los números del archivo
// ---------------------------
vector<double> read_numbers(const string &path) {
    ifstream f(path);
    vector<double> nums;
    if (!f) return nums;

    string token;
    while (f >> token) {
        try {
            nums.push_back(stod(token));
        } catch(...) {
            // ignora basura
        }
    }
    return nums;
}

// ---------------------------
// Parser del archivo de nodos
// ---------------------------
InstanceNodes parse_nodes(const string &path) {
    auto nums = read_numbers(path);
    InstanceNodes inst;
    if (nums.empty()) { inst.N = 0; return inst; }

    size_t idx = 0;

    inst.N = (int)nums[idx++];
    int N = inst.N;

    inst.service.resize(N);
    for (int i=0;i<N;i++) inst.service[i] = nums[idx++];

    inst.tw_start.resize(N);
    inst.tw_end.resize(N);
    for (int i=0;i<N;i++) inst.tw_start[i] = nums[idx++];
    for (int i=0;i<N;i++) inst.tw_end[i]   = nums[idx++];

    inst.dist.assign(N, vector<double>(N,0));

    for (int i=0;i<N;i++)
        for (int j=0;j<N;j++)
            inst.dist[i][j] = nums[idx++];

    return inst;
}

// ---------------------------
// Parser del archivo de usuarios
// ---------------------------
vector<UserData> parse_users(const string &path, int N) {
    auto nums = read_numbers(path);
    vector<UserData> users;

    if (nums.empty()) return users;

    size_t idx = 0;
    int U = (int)nums[idx++];

    for (int u=0; u<U; u++) {
        if (idx >= nums.size()) break;
        UserData ud;

        ud.T = nums[idx++];

        ud.nodeVal.resize(N);
        for (int i=0;i<N;i++)
            ud.nodeVal[i] = nums[idx++];

        ud.arcVal.assign(N, vector<double>(N,0));
        for (int i=0;i<N;i++)
            for (int j=0;j<N;j++)
                ud.arcVal[i][j] = nums[idx++];

        users.push_back(move(ud));
    }

    return users;
}

// ---------------------------
// Estructura de resultado
// ---------------------------
struct TourResult {
    double value;
    double timeUsed;
    vector<int> tour;
};

// ---------------------------
// Algoritmo greedy
// ---------------------------
TourResult solve_greedy_one(const InstanceNodes &inst, const UserData &ud) {
    int N = inst.N;
    vector<char> visited(N, 0);

    vector<int> tour;
    int cur = 0;
    tour.push_back(0);
    visited[0] = 1;

    double curTime = 0;
    double totalValue = ud.nodeVal[0];

    while (true) {
        int best = -1;
        double bestScore = -1e100;

        for (int j=1; j<N; j++) if (!visited[j]) {

            double travel = inst.dist[cur][j];
            if (travel > 1e8) continue;

            double arrive = curTime + travel;
            double startService = max(arrive, inst.tw_start[j]);
            double finish = startService + inst.service[j];

            if (finish > inst.tw_end[j]) continue;

            double back = inst.dist[j][0];
            if (finish + back > ud.T) continue;

            double incTime = finish - curTime;
            double incValue = ud.nodeVal[j] + ud.arcVal[cur][j];

            double score = incValue / max(1e-6, incTime);

            if (score > bestScore) {
                bestScore = score;
                best = j;
            }
        }

        if (best == -1) break;

        double travel = inst.dist[cur][best];
        double arrive = curTime + travel;
        double startService = max(arrive, inst.tw_start[best]);
        double finish = startService + inst.service[best];

        totalValue += ud.nodeVal[best] + ud.arcVal[cur][best];

        curTime = finish;
        cur = best;
        visited[cur] = 1;
        tour.push_back(cur);
    }

    double back = inst.dist[cur][0];
    curTime += back;

    return { totalValue, curTime, tour };
}

// ---------------------------
// MAIN
// ---------------------------
int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "Uso: solver <archivo_nodos> <archivo_usuarios>\n";
        return 1;
    }

    InstanceNodes inst = parse_nodes(argv[1]);
    if (inst.N == 0) {
        cout << "Error leyendo nodos.\n";
        return 1;
    }

    auto users = parse_users(argv[2], inst.N);
    if (users.empty()) {
        cout << "Error leyendo usuarios.\n";
        return 1;
    }

    ofstream out("salida.txt");

    for (auto &ud : users) {
        TourResult r = solve_greedy_one(inst, ud);

        out << r.value << "\n";
        out << ud.T << " " << r.timeUsed << "\n";

        for (int i=0;i<(int)r.tour.size();i++) {
            out << (r.tour[i] + 1);
            if (i+1 < (int)r.tour.size()) out << " ";
        }
        out << "\n";
    }

    cout << "Archivo salida.txt generado correctamente.\n";
    return 0;
}
