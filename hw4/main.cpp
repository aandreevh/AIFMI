#include <iostream>
#include <vector>

using pin = char;
using num = long long;

#define  MINIMIZING false
#define  MAXIMIZING true

#define NUM_MIN INT64_MIN
#define NUM_MAX INT64_MAX
using board = std::vector<std::vector<pin>>;

#define NONE 0
#define PLAYER_WIN 1
#define PLAYER_LOSE 2

#define OPPOSITE(P)\
(P == PLAYER_WIN ? (pin)PLAYER_LOSE: (pin)PLAYER_WIN)
#define N 3

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec){
    for(auto& p : vec){
        out<<p<<std::endl;
    }

    return out;
}

template<>
std::ostream& operator<<(std::ostream& out, const board& b){
    for(auto row=0;row<b.size();row++){
        for(auto col=0;col< b[row].size();col++){
            char c;
            switch (b[row][col]) {
                case NONE:
                    c='_';
                    break;
                case PLAYER_WIN:
                    c='X';
                    break;
                case PLAYER_LOSE:
                    c='O';
                    break;
            }
            out<<c<<" ";
        }
        out<<std::endl;
    }
    return out;
}

bool hasAny(const board&b, pin player){
    for(auto x=0;x<N;x++)
        for(auto y=0;y<N;y++){
            if(b[x][y] == player){
                return true;
            }
        }
    return false;
}

bool checkAll(const board& b, pin player){
    for(auto x=0;x<N;x++){
        bool allRow=true;
        bool allCow=true;
        for(auto y=0;y<N;y++){
            if(b[x][y] != player){
                allRow = false;
            }

            if(b[y][x] != player){
                allCow = false;
            }
        }
        if(allRow || allCow){
            return true;
        }
    }

    bool allDiwF= true;
    bool allDiwS= true;

    for(auto i=0;i<3;i++){
        if(b[i][i] != player){
            allDiwF = false;
        }

        if(b[i][N-i-1] != player){
            allDiwS = false;
        }
    }

    return allDiwF || allDiwS;
}

std::vector<board> substates(board& b, pin player){
    std::vector<board> out;
    for(auto x=0;x<N;x++){
        for(auto y=0;y<N;y++){
            if(b[x][y] ==NONE){
                auto nb = b;
                nb[x][y]=player;
                out.push_back(std::forward<board>(nb));
            }
        }
    }

    return std::forward<std::vector<board>>(out);
}

#define TYPE_WIN 1
#define TYPE_LOSE -1
#define TYPE_DRAW 0
#define TYPE_NTERMINAL 2

pin getBoardType(const board& b){
        if(checkAll(b,PLAYER_WIN)){
            return TYPE_WIN;
        }else if(checkAll(b,PLAYER_LOSE)){
            return TYPE_LOSE;
        }else if(hasAny(b,NONE)){
            return TYPE_NTERMINAL;
        }else{
            return TYPE_DRAW;
        }
}

class State {
public:
    using type = std::vector<State>;
    using it_type = type::iterator;

private:
    pin player;
    num alpha,beta, value;

    bool lazyFlag = false;
    type states;

    board innerState;

    void calculateValue(){
            if(lazyFlag) return; //already calculated
            lazyFlag = true;

            this->value = getBoardType(innerState);
            if(this->value == TYPE_NTERMINAL){
                auto boards = substates(innerState,player);
                if(player == PLAYER_WIN){
                    this->value = NUM_MIN;
                    for(auto& v : boards){
                        states.push_back({v,alpha,beta,PLAYER_LOSE});
                        this->value= std::max(this->value,states[states.size()-1]());
                        alpha = std::max(alpha,this->value);
                        if(alpha>=beta){
                            break;
                        }
                    }
                }else{
                    this->value = NUM_MAX;
                    for(auto& v : boards){
                        states.push_back({v,alpha,beta,PLAYER_WIN});
                        this->value= std::min(this->value,states[states.size()-1]());
                        beta = std::min(beta,this->value);
                        if(alpha>=beta){
                            break;
                        }
                    }
                }
            }
          //  auto boards = substates(innerState,)
    }
public:

    State(board b, num alpha, num beta, pin player):
    innerState(b),alpha(alpha),beta(beta),player(player)
    {}

    State(board b):State(b,NUM_MIN,NUM_MAX,PLAYER_WIN)
    {}

    board& state(){
        return this->innerState;
    }

    //state value
    num operator()(){
        calculateValue();
        return value;
    }

    it_type begin(){
        calculateValue();
        return states.begin();
    }

    it_type end(){
        return states.end();
    }

    board  operator!(){
        board opt;
        num value= player == PLAYER_WIN ? NUM_MIN:NUM_MAX;
        for(auto& s: (*this)){
            if(player == PLAYER_WIN){
                if(s()>value){
                    value = s();
                    opt = s.innerState;
                }
            }else{
                if(s()<value){
                    value = s();
                    opt = s.innerState;
                }
            }
        }

        return opt;
    }
};


int main(){
  board  b ={{NONE,NONE,NONE},
             {NONE,NONE,NONE},
             {NONE,NONE,NONE}};

  while (true){

      State root(b);
      b = !root;

      if(getBoardType(b)!= TYPE_NTERMINAL){
          std::cout<<b;
          break;
      }

      std::cout<<b<<std::endl<<"MOVE: ";
      int x,y;
      std::cin>>x>>y;
      if(b[x][y] == NONE){
          b[x][y] = PLAYER_LOSE;
      }else{
          std::cout<<"INVALID MOVE"<<std::endl;
          break;
      }
      if(getBoardType(b)!= TYPE_NTERMINAL){
          std::cout<<b;
          break;
      }
  }

    return 0;
}