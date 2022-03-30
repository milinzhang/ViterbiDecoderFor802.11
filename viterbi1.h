# ifndef __VITERBI_H_
# define __VITERBI_H_

const int nstates = 64;
const int table_length = 48;
const int traceback_depth = 32;

class Viterbi {

    public:

    Viterbi(){};
    ~Viterbi(){};
    void decode(unsigned char*,unsigned char*,int);

    protected:

    unsigned char hamming_distance(unsigned char,unsigned char);
    void branch_distance_compute(unsigned char*,unsigned char*,unsigned char*,unsigned char);
    void add_compare_select(unsigned int*, unsigned char*, unsigned char*, unsigned char*);
    void init_state(unsigned char);
    void find_max_score(unsigned int &,unsigned char &);
    void generate_output(unsigned char*,unsigned char,unsigned char&,int);
    void trace_back(unsigned char&, unsigned char &);
    //void depuncture();

    private:

    unsigned char survivor_path[table_length][nstates]; // to store path (previous state)
    unsigned char branch_metric_0[nstates]; // to store hamming distance for outter branch
    unsigned char branch_metric_1[nstates]; // to store hamming distance for inner branch
    unsigned int path_metric[nstates];  // to store cumulative hamming distance for path 
    unsigned char branch_table[2][nstates/2] = {{0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1}, // bit 0 outter branch
                                                {0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1}};// bit 1 outter branch
};

# endif