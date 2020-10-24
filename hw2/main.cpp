#include <iostream>
#include <chrono>

#define THRESHOLD 3

//time
template<typename U,intmax_t Num,intmax_t Den>
std::ostream& operator<<(std::ostream& out,std::chrono::duration<U, std::ratio<Num,Den>> dur ){
    out<<((dur.count()*Num))/(double)Den<<" sec.";
    return out;
}

template<typename T,typename Type =std::chrono::nanoseconds>
inline Type clock(T method){

    auto begin = std::chrono::duration_cast<Type>(
            std::chrono::system_clock::now().time_since_epoch());

    method();

    auto end = std::chrono::duration_cast<Type>(
            std::chrono::system_clock::now().time_since_epoch());

    return end-begin;
}

//definitions
#define MAX_N 15000

#define CHAR_QUEEN '*'
#define CHAR_EMPTY '_'
#define CHAR_SPACE ' '


#define diagSum(x,y) x+y
#define diagDif(x,y) N-1+(x-y)

using num = short;
using num_a =  num[MAX_N];
using num_ea = num[2*MAX_N-1];

//variables
num N;

num tries;
num resets;
num_a cache;

num_a queens;

num_a rows;
num_a cols;
num_ea diag_s;
num_ea diag_d;

inline num value(num row,num pos){
return rows[row]+cols[pos]+diag_s[diagSum(row,pos)]+diag_d[diagDif(row,pos)];
}



inline void printValues(){
    for(num row=0;row<N;row++){
        for(num pos = 0;pos<N;pos++){
            if(queens[row] == pos){
                std::cout<<(value(row,pos));
            }else {
                std::cout<<value(row,pos);
            }
        }
        std::cout<< std::endl;
    }
    std::cout<<std::endl;
}
inline void print(){
    std::string field;
    field.reserve(N*N+2*N);
    for(num row=0;row<N;row++){
        for(num pos = 0;pos<N;pos++){
            if(queens[row] == pos){
                field += CHAR_QUEEN;
            }else {
                field += CHAR_EMPTY;
            }
        }
        field += '\n';
    }
    std::ios_base::sync_with_stdio(false);
    std::cout<<field<<std::endl;
}

inline bool finished(){
    for(num row=0;row<N;row++){
        if(queens[row] == -1) return false;

        if(value(row,queens[row])-4 != 0){
            return false;
        }
    }

    return true;
}

inline void unset(num row){
    auto pos = queens[row];

    if(pos == -1)return;

    rows[row]--;
    cols[pos]--;
    diag_s[diagSum(row,pos)]--;
    diag_d[diagDif(row,pos)]--;

    queens[row]= -1;
}

inline void set(num row,num pos){
    rows[row]++;
    cols[pos]++;
    diag_s[diagSum(row,pos)]++;
    diag_d[diagDif(row,pos)]++;

    queens[row] = pos;
}

inline void update(num row){
    unset(row);

    num minimum = 0;
    num count = 0;

    for(num i=0;i<N;i++){
        auto selectedVal =value(row,minimum);
        auto curVal = value(row, i);

        if(selectedVal == curVal){
            cache[count++] = i;
        }else if(selectedVal > curVal){
            minimum = i;
            count = 0;
            cache[count++]=i;
        }
    }

    set(row, cache[rand()%count]);
}

inline num bestVal(){
    if(queens[0] == -1) return 0;

    num maxValue = value(0,queens[0]);
    num count=0;

    for(num row=0;row<N;row++){
        if(queens[row] ==-1){
            return row;
        }
        auto curValue =value(row,queens[row]);

        if(curValue == maxValue){
            cache[count++] = row;
        }else if(curValue > maxValue){
            maxValue = curValue;
            count = 0;
            cache[count++]=row;
        }
    }

    return cache[rand()%count];
}

inline void init(num n){
    N= n;
    for(num i=0;i<N;i++){
        queens[i] = -1;
        cols[i]=0;
        rows[i]=0;
    }

    for(num i=0;i<2*N-1;i++){
        diag_d[i]=0;
        diag_s[i]=0;
    }
}


inline void calculate(){
    auto flag = true;

    while(flag){
        auto val = bestVal();
        update(val);
        tries++;
        flag = !finished();
        if(tries >=THRESHOLD*N){ //probably local minimum, so restart
            init(N);
            tries=0;
            resets++;
        }
    }
}

int main(int argc,char** argv) {
    srand(time(nullptr));

    num val = MAX_N;

    if(argc == 2){
        val = atoi(argv[1]);
    }

    init(val);
    auto time = clock(calculate);
    print();
    std::cout<<"N: "<<N<<std::endl;
    std::cout<<"Tries/(Resets): "<<tries<<"/("<<resets<<")"<<std::endl;
    std::cout<<"Time for calculate(): "<<time<<std::endl;
}
