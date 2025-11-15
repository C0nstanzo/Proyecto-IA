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

// lee todos los tokens numéricos desde un archivo
static vector<double> read_numbers(const string &path) {
    ifstream f(path);
    vector<double> nums;
    if (!f) return nums;

    string line;
    while (getline(f, line)) {
        for (size_t i=0; i<line.size(); ++i)
            if (line[i] == ',') line[i] = ' ';

        istringstream iss(line);
        string tok;
        while (iss >> tok) {
            try {
                size_t p = 0;
                while (p < tok.size() &&
                       !(isdigit((unsigned char)tok[p]) || tok[p]=='-' || tok[p]=='+'))
                    ++p;

                size_t q = tok.size();
                while (q > p &&
                       !(isdigit((unsigned char)tok[q-1]) || tok[q-1]=='.'))
                    --q;

                if (p < q) {
                    string sub = tok.substr(p, q - p);
                    double v = stod(sub);
                    nums.push_back(v);
                }
            } catch (...) {
                // ignorar tokens no numéricos
            }
        }
    }
    return nums;
}

InstanceNodes parse_nodes(const string &path) {
    auto nums = read_numbers(path);
    InstanceNodes inst;
    if (nums.empty()) { inst.N = 0; return inst; }

    size_t idx = 0;
    inst.N = (int)nums[idx++];
    int N = inst.N;

    inst.service.assign(N, 0);
    for (int i=0; i<N && idx<nums.size(); ++i)
        inst.service[i] = nums[idx++];

    inst.tw_start.assign(N, 0);
    inst.tw_end.assign(N, numeric_limits<double>::infinity());
    bool have_tw = false;

    if (idx + 2*N <= nums.size()) {
        have_tw = true;
        for (int i=0; i<N; ++i) inst.tw_start[i] = nums[idx++];
        for (int i=0; i<N; ++i) inst.tw_end[i]   = nums[idx++];
    }

    int remain = (int)(nums.size() - idx);
    inst.dist.assign(N, vector<double>(N, 1e9));

    if (remain >= N*N) {
        for (int i=0; i<N; ++i)
            for (int j=0; j<N; ++j)
                inst.dist[i][j] = nums[idx++];
    }

    if (!have_tw) {
        for (int i=0; i<N; ++i) {
            inst.tw_start[i] = 0;
            inst.tw_end[i]   = 1e9;
        }
    }

    return inst;
}

vector<UserData> parse_users(const string &path, int N) {
    vector<UserData> users;
    auto nums = read_numbers(path);
    if (nums.empty()) return users;

    size_t idx = 0;
    int U = (int)nums[idx++];

    for (int u=0; u<U; ++u) {
        if (idx >= nums.size()) break;
        UserData ud;

        ud.T = nums[idx++];

        ud.nodeVal.assign(N,0);
        for (int i=0; i<N && idx < nums.size(); ++i)
            ud.nodeVal[i] = nums[idx++];

        ud.arcVal.assign(N, vector<double>(N,0));
        for (int i=0; i<N; ++i)
            for (int j=0; j<N && idx < nums.size(); ++j)
                ud.arcVal[i][j] = nums[idx++];

        users.push_back(move(ud));
    }

    return users;
}

struct TourResult {
    double value;
    double timeUsed;
    vector<int> tour;
};

TourResult solve_greedy_one(const InstanceNodes &inst, const UserData &ud) {
    int N = inst.N;
    vector<char> visited(N, 0);
    vector<int> tour;

    int cur = 0;
    double curTime = 0.0;

    visited[0] = 1;
    tour.push_back(0);

    double totalValue = 0.0;
    if ((int)ud.nodeVal.size() == N)
        totalValue += ud.nodeVal[0];

    while (true) {
        int best = -1;
        double bestScore = -1e100;

        for (int j = 1; j < N; ++j) if (!visited[j]) {
            double travel = inst.dist[cur][j];
            if (travel > 1e8) continue;

            double arrive = curTime + travel;
            double startService = max(arrive, inst.tw_start[j]);
            double wait = max(0.0, inst.tw_start[j] - arrive);
            double finish = startService + inst.service[j];

            if (finish > inst.tw_end[j]) continue;

            double back = inst.dist[j][0];
            double timeIfReturn = finish + back;

            if (timeIfReturn > ud.T + 1e-9) continue;

            double incTime = travel + wait + inst.service[j];
            double incValue = 0.0;

            if (j < (int)ud.nodeVal.size()) incValue += ud.nodeVal[j];
            if (cur < (int)ud.arcVal.size() && j < (int)ud.arcVal.size())
                incValue += ud.arcVal[cur][j];

            double score = incValue / max(1e-6, incTime);

            if (score > bestScore + 1e-12 ||
               (fabs(score - bestScore) < 1e-12 && incValue > 0 && best != -1)) {
                best = j;
                bestScore = score;
            }
        }

        if (best == -1) break;

        double travel = inst.dist[cur][best];
        double arrive = curTime + travel;
        double startService = max(arrive, inst.tw_start[best]);
        double wait = max(0.0, inst.tw_start[best] - arrive);
        double finish = startService + inst.service[best];

        totalValue += (best < (int)ud.nodeVal.size() ? ud.nodeVal[best] : 0.0);
        if (cur < (int)ud.arcVal.size() && best < (int)ud.arcVal.size())
            totalValue += ud.arcVal[cur][best];

        curTime = finish;
        cur = best;
        visited[cur] = 1;
        tour.push_back(cur);
    }

    double back = inst.dist[cur][0];
    if (back > 1e8)
        return {totalValue, 1e12, tour};

    curTime += back;

    return {totalValue, curTime, tour};
}

int main(int argc, char** argv) {
    cout.setf(std::ios::fixed);
    cout << setprecision(6);

    if (argc < 3) {
        cerr << "Usage: " << argv[0]
             << " <nodos_file> <usuarios_file> [output_file]\n";
        return 1;
    }

    string nodos_path = argv[1];
    string usuarios_path = argv[2];
    string out_path = (argc >= 4 ? argv[3] : string("salida.txt"));

    InstanceNodes inst = parse_nodes(nodos_path);
    if (inst.N <= 0) {
        cerr << "Error leyendo nodos\n";
        return 1;
    }

    auto users = parse_users(usuarios_path, inst.N);
    if (users.empty()) {
        cerr << "Error leyendo usuarios\n";
        return 1;
    }

    ofstream out(out_path);
    if (!out) {
        cerr << "No se pudo crear archivo de salida\n";
        return 1;
    }

    for (size_t u=0; u<users.size(); ++u) {
        auto res = solve_greedy_one(inst, users[u]);

        out << (u+1) << " ";
        out << res.value << " "
            << users[u].T << " "
            << res.timeUsed << " ";

        for (size_t i=0; i<res.tour.size(); ++i) {
            out << (res.tour[i] + 1);
            if (i + 1 < res.tour.size()) out << ",";
        }

        out << "\n";
    }

    cout << "Salida escrita en: " << out_path << "\n";
    return 0;
}
