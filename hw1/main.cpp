#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <queue>

#define CHAR_ZERO '0'

//time
template<typename Type= std::chrono::nanoseconds>
inline Type time() {
    return std::chrono::duration_cast<Type>(
            std::chrono::system_clock::now().time_since_epoch());
}

template<typename U, intmax_t Num, intmax_t Den>
std::ostream &operator<<(std::ostream &out, std::chrono::duration<U, std::ratio<Num, Den>> dur) {
    out << ((dur.count() * Num)) / (double) Den << " sec.";
    return out;
}

template<typename T, typename Type =std::chrono::nanoseconds>
inline Type clock(T method) {

    auto begin = time<Type>();
    method();
    auto end = time<Type>();

    return end - begin;
}


#define NON -1
#define UP 0
#define DOWN 1
#define RIGHT 2
#define LEFT 3

using num = int;
using move = unsigned char;
using pos = std::pair<num, num>;

using vec = std::vector<num>;
using board = std::vector<vec>;



pos DIR[] = {{0,  -1},
             {0,  1},
             {1,  0},
             {-1, 0}};

pos operator+(const pos &u, const pos &v) {
    return {u.first + v.first, u.second + v.second};
}

move inv(move m) {
    switch (m) {
        case UP:
            return DOWN;
        case DOWN:
            return UP;
        case LEFT:
            return RIGHT;
        case RIGHT:
            return LEFT;
        default:
            return -1;
    }
}


bool bounds(num n, pos u) {
    return u.first >= 0 && u.first < n
           && u.second >= 0 && u.second < n;
}

pos cell(num n, num d) {
    if (d == -1) {
        return {0, n - 1};
    }
    return {d % n, d / n};
}

template<typename H>
num heuristics(const board &b,pos tCell) {
    H heuristic;
    num h = 0;
    num n = b.size();

    for (num row = 0; row < n; row++)
        for (num col = 0; col < n; col++)
            h += heuristic(b,tCell,(pos){row,col});

    return h;
}

template <typename H>
struct hist {
    H heuristic;
    hist *parent = nullptr;

    num ref = 1;

    move mv = NON;
    num moves = 0;

    num h = 0;

    board b;
    pos cell = {0, 0};

    hist(hist *parent, move d) { //O(n^2)
        this->heuristic = heuristic;
        num n = parent->b.size();

        this->parent = parent;
        parent->ref++;

        this->mv = d;
        this->moves = parent->moves + 1;


        this->h = parent->h;
        this->b = parent->b;

        this->cell = parent->cell + DIR[d];

        this->h -= heuristic(this->b,
                             this->parent->cell,
                             (pos){this->parent->cell.first,this->parent->cell.second});

        this->h -= heuristic(this->b,
                             this->parent->cell,
                             (pos){this->cell.first,this->cell.second});

        std::swap(this->b[this->parent->cell.second][this->parent->cell.first],
                  this->b[this->cell.second][this->cell.first]);


        this->h += heuristic(this->b,
                             this->cell,
                             (pos){this->parent->cell.first,this->parent->cell.second});

        this->h += heuristic(this->b,
                             this->cell,
                             (pos){this->cell.first,this->cell.second});
    }

    hist(const board &b, pos cell) {
        this->heuristic = heuristic;
        this->cell = cell;
        this->b = b;
        this->h = heuristics<H>(b,cell);
    }

    num prior() {
        return moves + h;
    }
};

template<typename H>
struct cmp {
    bool operator()(hist<H> *u, hist<H> *v) {
        return u->prior() > v->prior();
    }
};

template <typename H>
void detach(hist<H> *v) {
    if (v == nullptr)return;
    v->ref--;
    if (v->ref == 0) {
        auto p = v->parent;
        delete v;

        if (p == nullptr)return;

        detach(p);
    }
}

template<typename H>
std::vector<hist<H> *> moves(hist<H> *h) {
    move im = inv(h->mv);
    num n = h->b.size();
    std::vector<hist<H> *> out;

    for (move i = 0; i < 4; i++) {
        if (i == im)continue;
        auto s = h->cell + DIR[i];
        if (!bounds(n, s))continue;
        out.push_back(new hist<H>(h, i));
    }
    return out;
}

board inputTable(num n) {
    board b(n);
    for (num row = 0; row < n; row++) {
        b[row] = vec(n);

        for (num col = 0; col < n; col++) {
            std::cin >> b[row][col];
        }
    }

    return b;
}

template <typename H>
void output(hist<H> *h) {
    if (h == nullptr) {
        std::cout << "Non";
        return;
    }
    num n = h->b.size();

    for (num row = 0; row < n; row++) {
        for (num col = 0; col < n; col++) {
            if (row == h->cell.second && col == h->cell.first) {
                std::cout << "[" << std::setfill(CHAR_ZERO) << std::setw(2) << h->b[row][col] << "]";
            } else {
                std::cout << std::setfill(CHAR_ZERO) << std::setw(2) << h->b[row][col];
            }
            std::cout << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "H:" << h->h << std::endl << std::endl;
}

template <typename H>
hist<H> *input() {
    num n;
    pos c;

    std::cin >> n;

    num inCell;
    std::cin >> inCell;

    c = cell(n, inCell);

    return new hist<H>(inputTable(n), c);
}

template<typename H>
hist<H> *alg(hist<H> *root, num threshold ) {
    std::priority_queue<hist<H> *, std::vector<hist<H> *>, cmp<H>> q;
    q.push(root);

    while (!q.empty()) {
        auto t = q.top();
        q.pop();

        if (t->h == 0) {
            while (!q.empty()) {
                auto rmn = q.top();
                q.pop();
                if (rmn != t)detach(rmn);
            }
            return t;
        }

        auto mvs = moves(t);
        for (auto &m : mvs) {
            if (m->prior() < threshold) {
                q.push(m);
            }else{
                detach(m);
            }
        }
        detach(t);
    }

    return nullptr;
}
template<typename H>
hist<H>* dupShallow(hist<H>* o){
    return new hist<H>(o->b,o->cell);
}

template<typename H,typename T>
hist<H>* inc_alg(T method){
    auto h0 = method();

    num threshold = h0->b.size();

    for(num u =1;u<100;u++){
        auto root = dupShallow(h0);
        auto out = alg(root,threshold*u);
        if(out){
            delete h0;
            return out;
        }
    }

    delete h0;
    return nullptr;
}



template<typename H>
hist<H>* genPuzzle() {
    srand(time().count());

    num n,it;
    std::cin>> n;
    std::cin>>it;
    board b(n);

    for(num row = 0;row <n;row++){
        b[row] = vec(n);
        for(num col = 0;col <n;col++){
            b[row][col] = row*n+col;
        }
    }

    auto h = new hist<H>(b,{0,0});

    for(num mvn = 0;mvn<it;mvn++){
        auto nh = moves(h);
        auto nhnum = rand()%nh.size();
        for(auto v =0;v<nh.size();v++){
            if(nhnum != v){
                detach(nh[v]);
            }
        }
        h = nh[nhnum];
    }

    auto nh = dupShallow(h);

    while(h!= nullptr){
        auto temp = h->parent;
        detach(h);
        h= temp;
    }

    return nh ;
}

template<typename H>
void print(hist<H> *h) {
    if (h == nullptr) {
        std::cout << "end" << std::endl;
        return;
    }

    output(h);
    print(h->parent);

}

struct Manhattan {
    num operator()(const board &b, pos tCell, pos p) const {
        auto target = cell(b.size(), b[p.second][p.first]);

        return abs(target.first - p.first) + abs(target.second - p.second);
    }
};

struct Places {
    num operator()(const board &b, pos tCell, pos p) const {
        return p.second * b.size() + p.first != b[p.second][p.first];
    }
};


#define H_BIN(op,U,V) struct{\
    U u;\
    V v;\
    \
    num operator()(const board& b,pos tCell,pos p) const{\
        return u(b,tCell,p) op v(b,tCell,p);\
    }\
}\

using HTYPE = H_BIN(+,Manhattan,Places);

int main() {
  auto sol = inc_alg<HTYPE>(genPuzzle<HTYPE>);
    print(sol);
    detach(sol);
}